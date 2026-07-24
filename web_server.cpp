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
    html, body { margin: 0; height: 100%; font-family: sans-serif; }
    #map { height: 85vh; }
    #panel { height: 15vh; display: flex; align-items: center; justify-content: center; gap: 12px; }
    button { padding: 8px 16px; font-size: 16px; }
    button:disabled { opacity: 0.5; }
  </style>
</head>
<body>
  <div id="map"></div>
  <div id="panel">
    <span id="status">Click the map to set the departure point</span>
    <button id="resetBtn">Reset</button>
    <button id="sendBtn" disabled>Send route</button>
  </div>

  <script src="https://unpkg.com/leaflet@1.9.4/dist/leaflet.js"
    integrity="sha256-20nQCchB9co0qIjJZRGuk2/Z9VM+kNiyxNV1lvTlZBo=" crossorigin=""></script>
  <script>
    // Centered on the original default route so the map opens somewhere useful
    const map = L.map('map').setView([49.41461, 8.681495], 13);
    L.tileLayer('https://tile.openstreetmap.org/{z}/{x}/{y}.png', {
      attribution: '&copy; OpenStreetMap contributors'
    }).addTo(map);

    let startMarker = null;
    let endMarker = null;
    const statusEl = document.getElementById('status');
    const sendBtn = document.getElementById('sendBtn');
    const resetBtn = document.getElementById('resetBtn');

    function updateStatus() {
      if (!startMarker) {
        statusEl.textContent = 'Click the map to set the departure point';
      } else if (!endMarker) {
        statusEl.textContent = 'Click the map to set the arrival point';
      } else {
        statusEl.textContent = 'Departure and arrival set';
      }
      sendBtn.disabled = !(startMarker && endMarker);
    }

    map.on('click', function (e) {
      if (!startMarker) {
        startMarker = L.marker(e.latlng, { title: 'Departure' }).addTo(map)
          .bindPopup('Departure').openPopup();
      } else if (!endMarker) {
        endMarker = L.marker(e.latlng, { title: 'Arrival' }).addTo(map)
          .bindPopup('Arrival').openPopup();
      }
      updateStatus();
    });

    resetBtn.addEventListener('click', function () {
      if (startMarker) { map.removeLayer(startMarker); startMarker = null; }
      if (endMarker) { map.removeLayer(endMarker); endMarker = null; }
      updateStatus();
    });

    sendBtn.addEventListener('click', function () {
      if (!startMarker || !endMarker) return;
      const s = startMarker.getLatLng();
      const e = endMarker.getLatLng();
      const url = `/route?startLat=${s.lat}&startLng=${s.lng}&endLat=${e.lat}&endLng=${e.lng}`;

      statusEl.textContent = 'Sending route to ESP32...';
      sendBtn.disabled = true;

      fetch(url)
        .then(r => r.text())
        .then(t => { statusEl.textContent = t; })
        .catch(err => { statusEl.textContent = 'Error: ' + err; })
        .finally(() => { sendBtn.disabled = false; });
    });
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
