#ifndef WEB_SERVER_H
#define WEB_SERVER_H

// Call once from the main setup(), after WiFi is connected (setupWifi()).
// Starts the ESP-IDF HTTPS server (esp_https_server component) on port
// 443, using the self-signed certificate embedded in certs.h.
void setupWebServer();

// Last GPS fix received from the phone's continuous location tracking
// (the web page sends one every 5 seconds). Returns false if none has
// been received yet.
bool getTrackedLocation(double &lat, double &lng);

#endif