#include "web_server.h"


httpd_handle_t server = NULL;

// Last GPS fix received from the phone's continuous tracking (every 5s)
double trackedLat = 0;
double trackedLng = 0;
bool haveTrackedLocation = false;

// The map page itself. Leaflet's CSS/JS are loaded from the unpkg CDN (the
// ESP32 already has internet access via the ORS calls), so only this small
// shell needs to be stored on the device.
//
// Geolocation requires a secure context on mobile browsers, which is why
// this page is served over HTTPS (self-signed cert in certs.h).
const char MAP_PAGE[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>Bike Route Picker</title>
  <link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.4/dist/leaflet.css"
    integrity="sha256-p4NxAoJBhIIN+hmNHrzRCf9tD/miZyoHS5obTRR9BMY=" crossorigin="" />
  <style>
    html, body { margin: 0; height: 100%; width: 100%; font-family: sans-serif; overflow: hidden; }
    #map { position: absolute; top: 0; left: 0; right: 0; bottom: 0; }

    #panel {
      position: absolute;
      left: 50%;
      bottom: 16px;
      transform: translateX(-50%);
      z-index: 1000; /* above Leaflet's own controls/tiles */

      display: flex;
      flex-wrap: wrap;
      align-items: center;
      justify-content: center;
      gap: 8px;

      background: rgba(255, 255, 255, 0.92);
      padding: 8px 10px;
      border-radius: 10px;
      box-shadow: 0 2px 8px rgba(0, 0, 0, 0.3);
      max-width: 92vw;
    }

    #status { font-size: 13px; white-space: nowrap; }

    button { padding: 6px 12px; font-size: 14px; border: none; border-radius: 6px; background: #2c7be5; color: #fff; white-space: nowrap; }
    button:disabled { opacity: 0.5; }
    #resetBtn { background: #6c757d; }
    #locateBtn { background: #2ea36c; }
  </style>
</head>
<body>
  <div id="map"></div>
  <div id="panel">
    <span id="status">Set departure</span>
    <button id="locateBtn">Use Location</button>
    <button id="resetBtn">Reset</button>
    <button id="sendBtn" disabled>Send</button>
  </div>

  <script src="https://unpkg.com/leaflet@1.9.4/dist/leaflet.js"
    integrity="sha256-20nQCchB9co0qIjJZRGuk2/Z9VM+kNiyxNV1lvTlZBo=" crossorigin=""></script>
  <script>
    // Default view: Paris. Re-centers on the phone's actual location below
    // if geolocation succeeds.
    const map = L.map('map').setView([48.8566, 2.3522], 13);
    L.tileLayer('https://tile.openstreetmap.org/{z}/{x}/{y}.png', {
      attribution: '&copy; OpenStreetMap contributors'
    }).addTo(map);

    let startMarker = null;
    let endMarker = null;
    let myLocationMarker = null; // kept across Reset - only Departure/Arrival get cleared
    let myLatLng = null;         // last known GPS fix, reused by the Use Location button

    const statusEl = document.getElementById('status');
    const sendBtn = document.getElementById('sendBtn');
    const resetBtn = document.getElementById('resetBtn');
    const locateBtn = document.getElementById('locateBtn');

    function updateStatus() {
      if (!startMarker) {
        statusEl.textContent = 'Set departure';
      } else if (!endMarker) {
        statusEl.textContent = 'Set arrival';
      } else {
        statusEl.textContent = 'Ready';
      }
      sendBtn.disabled = !(startMarker && endMarker);
    }

    // Places latlng into whichever of departure/arrival is still empty.
    function applyToNextSlot(latlng) {
      if (!startMarker) {
        startMarker = L.marker(latlng, { title: 'Departure' }).addTo(map)
          .bindPopup('Departure').openPopup();
      } else if (!endMarker) {
        endMarker = L.marker(latlng, { title: 'Arrival' }).addTo(map)
          .bindPopup('Arrival').openPopup();
      }
      updateStatus();
    }

    // Creates/updates the blue "my location" dot
    function updateMyLocationMarker(latlng) {
      myLatLng = latlng;
      if (myLocationMarker) {
        myLocationMarker.setLatLng(latlng);
      } else {
        myLocationMarker = L.circleMarker(latlng, {
          radius: 8,
          color: '#2c7be5',
          fillColor: '#2c7be5',
          fillOpacity: 0.9,
          weight: 2
        }).addTo(map).bindPopup('My location');
      }
    }

    // One-shot fix, used on page load and by the Use Location button.
    function locateUser(centerMap, onDone) {
      if (!navigator.geolocation) {
        statusEl.textContent = 'No GPS support';
        return;
      }

      navigator.geolocation.getCurrentPosition(
        function (pos) {
          const latlng = L.latLng(pos.coords.latitude, pos.coords.longitude);
          updateMyLocationMarker(latlng);
          if (centerMap) map.setView(latlng, 15);
          if (onDone) onDone(latlng);
        },
        function (err) {
          statusEl.textContent = 'Location unavailable';
        },
        { enableHighAccuracy: true, timeout: 10000 }
      );
    }

    map.on('click', function (e) {
      applyToNextSlot(e.latlng);
    });

    resetBtn.addEventListener('click', function () {
      if (startMarker) { map.removeLayer(startMarker); startMarker = null; }
      if (endMarker) { map.removeLayer(endMarker); endMarker = null; }
      // myLocationMarker is intentionally left on the map
      updateStatus();
    });

    locateBtn.addEventListener('click', function () {
      if (startMarker && endMarker) {
        statusEl.textContent = 'Ready';
        return;
      }
      statusEl.textContent = 'Locating...';
      locateUser(false, function (latlng) {
        applyToNextSlot(latlng);
      });
    });

    sendBtn.addEventListener('click', function () {
      if (!startMarker || !endMarker) return;
      const s = startMarker.getLatLng();
      const e = endMarker.getLatLng();
      const url = `/route?startLat=${s.lat}&startLng=${s.lng}&endLat=${e.lat}&endLng=${e.lng}`;

      statusEl.textContent = 'Sending...';
      sendBtn.disabled = true;

      fetch(url)
        .then(r => r.text())
        .then(t => { statusEl.textContent = 'Sent'; })
        .catch(err => { statusEl.textContent = 'Error'; })
        .finally(() => { sendBtn.disabled = false; });
    });

    // Continuous tracking: every 5 seconds, grab a fresh GPS fix, update the
    // blue dot, and push it to the ESP32 so it knows where the phone is
    // along the route. Runs silently in the background - failures here
    // don't touch the status text so they don't fight with the buttons above.
    setInterval(function () {
      if (!navigator.geolocation) return;
      navigator.geolocation.getCurrentPosition(
        function (pos) {
          const latlng = L.latLng(pos.coords.latitude, pos.coords.longitude);
          updateMyLocationMarker(latlng);
          fetch(`/location?lat=${latlng.lat}&lng=${latlng.lng}`).catch(function () {});
        },
        function (err) { /* ignore intermittent GPS failures */ },
        { enableHighAccuracy: true, timeout: 4000 }
      );
    }, 5000);

    // Try to center on the phone's location as soon as the page loads.
    // Silently falls back to the Paris default view if denied/unavailable.
    locateUser(true);
  </script>
</body>
</html>
)rawliteral";

// Reads a single query-string parameter into outBuf. Returns false if the
// query string or the specific key is missing.
static bool getQueryParam(httpd_req_t *req, const char *key, char *outBuf, size_t outBufLen)
{
  char query[256];
  if (httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK)
  {
    return false;
  }
  return httpd_query_key_value(query, key, outBuf, outBufLen) == ESP_OK;
}

static esp_err_t handleRoot(httpd_req_t *req)
{
  httpd_resp_set_type(req, "text/html");
  httpd_resp_send(req, MAP_PAGE, HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

static esp_err_t handleSetRoute(httpd_req_t *req)
{
  char sLat[32], sLng[32], eLat[32], eLng[32];

  bool ok = getQueryParam(req, "startLat", sLat, sizeof(sLat)) &&
            getQueryParam(req, "startLng", sLng, sizeof(sLng)) &&
            getQueryParam(req, "endLat", eLat, sizeof(eLat)) &&
            getQueryParam(req, "endLng", eLng, sizeof(eLng));

  httpd_resp_set_type(req, "text/plain");

  if (!ok)
  {
    httpd_resp_set_status(req, "400 Bad Request");
    httpd_resp_send(req, "Missing lat/lng parameters", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
  }

  setRoutePoints(atof(sLat), atof(sLng), atof(eLat), atof(eLng));
  httpd_resp_send(req, "Route points received, fetching new route...", HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

static esp_err_t handleUpdateLocation(httpd_req_t *req)
{
  char sLat[32], sLng[32];

  httpd_resp_set_type(req, "text/plain");

  if (!getQueryParam(req, "lat", sLat, sizeof(sLat)) ||
      !getQueryParam(req, "lng", sLng, sizeof(sLng)))
  {
    httpd_resp_set_status(req, "400 Bad Request");
    httpd_resp_send(req, "Missing lat/lng parameters", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
  }

  trackedLat = atof(sLat);
  trackedLng = atof(sLng);
  haveTrackedLocation = true;

  Serial.printf("Tracked location: %f, %f", trackedLat, trackedLng);
  Serial.println();

  httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

static esp_err_t handle404(httpd_req_t *req, httpd_err_code_t err)
{
  httpd_resp_set_status(req, "404 Not Found");
  httpd_resp_set_type(req, "text/plain");
  httpd_resp_send(req, "Not Found", HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

void setupWebServer()
{

  // Display onto the OLED display that we are starting the web server
  showHomeScreenState(2);

  httpd_ssl_config_t conf = HTTPD_SSL_CONFIG_DEFAULT();

  conf.servercert = (const uint8_t *)servercert_pem;
  conf.servercert_len = strlen(servercert_pem) + 1;
  conf.prvtkey_pem = (const uint8_t *)prvtkey_pem;
  conf.prvtkey_len = strlen(prvtkey_pem) + 1;

  esp_err_t ret = httpd_ssl_start(&server, &conf);
  if (ret != ESP_OK)
  {
    Serial.printf("Failed to start HTTPS server, error = %d", ret);
    Serial.println();
    return;
  }

  httpd_uri_t uriRoot     = {.uri = "/",         .method = HTTP_GET, .handler = handleRoot,           .user_ctx = NULL};
  httpd_uri_t uriRoute    = {.uri = "/route",    .method = HTTP_GET, .handler = handleSetRoute,       .user_ctx = NULL};
  httpd_uri_t uriLocation = {.uri = "/location", .method = HTTP_GET, .handler = handleUpdateLocation, .user_ctx = NULL};

  httpd_register_uri_handler(server, &uriRoot);
  httpd_register_uri_handler(server, &uriRoute);
  httpd_register_uri_handler(server, &uriLocation);
  httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, handle404);

  Serial.println("HTTPS server started");
  Serial.print("Open https://");
  Serial.println(WiFi.localIP());
  Serial.println("(Your browser will warn about the self-signed certificate the first time - choose Advanced/Proceed to continue)");

  String serverAddress = "https://" + WiFi.localIP().toString() + "/";
  showHomeScreenState(3, serverAddress.c_str());

  // Display onto the OLED display that the web server has started and show the IP address
}

bool getTrackedLocation(double &lat, double &lng)
{
  if (!haveTrackedLocation)
  {
    return false;
  }
  lat = trackedLat;
  lng = trackedLng;
  return true;
}