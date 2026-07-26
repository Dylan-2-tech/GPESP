#ifndef ORS_API_CALL_H
#define ORS_API_CALL_H

// Sets the departure/arrival points to use for the next ORS request(s).
// Coordinates are in decimal degrees (lat/lng, WGS84 - what Leaflet gives you).
void setRoutePoints(double startLat, double startLng, double endLat, double endLng);

// Fetches the current bike route from the ORS API and prints it to the
// Serial monitor. Requires WiFi to already be connected.
void getBikeRoute();

// Call every loop(). Internally tracks timing and calls getBikeRoute()
// once every requestInterval milliseconds, or immediately after new route
// points are set via setRoutePoints().
void updateBikeRoute();

#endif