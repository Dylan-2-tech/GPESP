#ifndef WEB_SERVER_H
#define WEB_SERVER_H

// Call once from the main setup(), after WiFi is connected (setupWifi()).
// Starts the HTTP server and registers routes for the Leaflet map page.
void setupWebServer();

// Call every loop() to process incoming HTTP requests. Must not block.
void handleWebServer();

#endif
