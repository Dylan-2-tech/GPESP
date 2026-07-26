#include "web_server.h"

#include <WiFi.h>
#include <WebServer.h>
#include "ors_api_call.h"

// HTTP server on the default web port
WebServer server(80);

// The map page itself. Leaflet's CSS/JS are loaded from the unpkg CDN (the
// ESP32 already has internet access via the ORS calls), so only this small
// shell needs to be stored on the device.
const char MAP_PAGE[] PROGMEM = R"rawliteral(
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
    // Does nothing if both are already set.
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

    // Updates/creates the "my location" marker and optionally recenters the
    // map on it. Calls onDone(latlng) once a fix is obtained (if provided).
    function locateUser(centerMap, onDone) {
      if (!navigator.geolocation) {
        statusEl.textContent = 'No GPS support';
        return;
      }

      navigator.geolocation.getCurrentPosition(
        function (pos) {
          const latlng = L.latLng(pos.coords.latitude, pos.coords.longitude);
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

          if (centerMap) {
            map.setView(latlng, 15);
          }

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

    // Try to center on the phone's location as soon as the page loads.
    // Silently falls back to the Paris default view if denied/unavailable.
    locateUser(true);
  </script>
</body>
</html>
)rawliteral";

void handleRoot()
{
  server.send_P(200, "text/html", MAP_PAGE);
}

void handleSetRoute()
{
  if (!server.hasArg("startLat") || !server.hasArg("startLng") ||
      !server.hasArg("endLat") || !server.hasArg("endLng"))
  {
    server.send(400, "text/plain", "Missing lat/lng parameters");
    return;
  }

  double newStartLat = server.arg("startLat").toDouble();
  double newStartLng = server.arg("startLng").toDouble();
  double newEndLat   = server.arg("endLat").toDouble();
  double newEndLng   = server.arg("endLng").toDouble();

  setRoutePoints(newStartLat, newStartLng, newEndLat, newEndLng);

  server.send(200, "text/plain", "Route points received, fetching new route...");
}

void handleNotFound()
{
  server.send(404, "text/plain", "Not found");
}

void setupWebServer()
{
  server.on("/", HTTP_GET, handleRoot);
  server.on("/route", HTTP_GET, handleSetRoute);
  server.onNotFound(handleNotFound);

  server.begin();

  Serial.println("Web server started");
  Serial.print("Open http://");
  Serial.println(WiFi.localIP());
}

void handleWebServer()
{
  server.handleClient();
}
