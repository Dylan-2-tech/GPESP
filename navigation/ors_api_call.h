#ifndef ORS_API_CALL_H
#define ORS_API_CALL_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "ors_api_key.h"

// Sets the departure/arrival points to use for the next ORS request(s).
// Coordinates are in decimal degrees (lat/lng, WGS84 - what Leaflet gives you).
void setRoutePoints(double startLat, double startLng, double endLat, double endLng);

// Fetches the current bike route from the ORS API and prints it to the
// Serial monitor. Requires WiFi to already be connected.
void getBikeRoute();

// Returns true when at least one route summary has been fetched successfully.
// Output units are meters and seconds.
bool getRouteSummary(float& distanceKm, float& durationSeconds);

// Copies the latest ORS encoded route geometry into geometry.
// Returns false when the last route request did not provide usable geometry.
bool getRouteGeometry(String& geometry);

#endif