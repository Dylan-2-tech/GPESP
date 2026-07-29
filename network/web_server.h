#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WiFi.h>
#include <stdlib.h>

#include "../display/screen.h"

// Espressif's official HTTPS server component - built into the arduino-esp32
// core, no third-party library needed.
#include "esp_http_server.h"
#include "esp_https_server.h"

#include "certs.h"
#include "../navigation/ors_api_call.h"

// Call once from the main setup(), after WiFi is connected (setupWifi()).
// Starts the ESP-IDF HTTPS server (esp_https_server component) on port
// 443, using the self-signed certificate embedded in certs.h.
void setupWebServer();

// Last GPS fix received from the phone's continuous location tracking
// (the web page sends one every 5 seconds). Returns false if none has
// been received yet.
bool getTrackedLocation(double &lat, double &lng);

#endif