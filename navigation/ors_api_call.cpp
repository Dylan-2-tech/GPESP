#include "ors_api_call.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "../display/screen.h"
#include "ors_api_key.h"

// ORS API endpoint and key (the start/end points are appended dynamically,
// see getBikeRoute() below)
const char* ORS_BASE_URL = "https://api.openrouteservice.org/v2/directions/cycling-regular";

// Default departure/arrival points (same spot as the original hardcoded
// request) - overwritten by setRoutePoints() once the web page sends new ones
double startLat;
double startLng;
double endLat;
double endLng;

unsigned long previousRequest = 61;
const unsigned long requestInterval = 60000; // 60 seconds

// Set when new points arrive from the web page, so updateBikeRoute() fetches
// a fresh route on the very next loop() instead of waiting for the timer
bool routePointsChanged = false;

bool hasRouteSummary = false;
float latestDistanceMeters = 0.0f;
float latestDurationSeconds = 0.0f;

void setRoutePoints(double newStartLat, double newStartLng, double newEndLat, double newEndLng)
{
  startLat = newStartLat;
  startLng = newStartLng;
  endLat   = newEndLat;
  endLng   = newEndLng;
  routePointsChanged = true;

  Serial.println("New route points received:");
  Serial.printf("  Start: %f, %f", startLat, startLng);
    Serial.println();
  Serial.printf("  End:   %f, %f", endLat, endLng);
    Serial.println();

  getBikeRoute();
}

// Function that displays the result of the GET call to the serial Monitor
void getBikeRoute()
{
  // Looks if it's connected to any wifi
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi not connected");
        return;
    }

    // Build the request URL from the current start/end points.
    // ORS expects "lng,lat" order for both start and end.
    String url = String(ORS_BASE_URL) + "?api_key=" + ORS_API_KEY +
                 "&start=" + String(startLng, 6) + "," + String(startLat, 6) +
                 "&end="   + String(endLng, 6)   + "," + String(endLat, 6);

    HTTPClient http;
    Serial.println("Sending GET request...");
    Serial.println(url);
    http.begin(url);

    http.addHeader(
    "Accept",
    "application/json, application/geo+json, application/gpx+xml, img/png; charset=utf-8");

    int httpCode = http.GET();

    // 0 means the query didn't work
    if (httpCode > 0)
    {
        Serial.printf("HTTP Code: %d", httpCode);
        Serial.println();

        // If the returned code is 200
        if (httpCode == HTTP_CODE_OK)
        {
            // obtain and print the response from the API
            String payload = http.getString();

            JsonDocument doc;

            DeserializationError error = deserializeJson(doc, payload);

            if (error)
            {
                Serial.print("JSON parsing failed: ");
                Serial.println(error.c_str());
                // Display in the serial the json
                Serial.println(payload);
                return;
            }

            JsonArray features = doc["features"].as<JsonArray>();

            if (features.isNull() || features.size() == 0)
            {
                Serial.println("No routes returned.");
                return;
            }

            JsonObject firstRoute = features[0];

            // ================================
            // Route summary
            // ================================

            JsonObject summary = firstRoute["properties"]["summary"];

            float totalDistance = summary["distance"];
            float totalDuration = summary["duration"];

            latestDistanceMeters = totalDistance;
            latestDurationSeconds = totalDuration;
            hasRouteSummary = true;

            Serial.println();
            Serial.println("========== ROUTE ==========");
            Serial.printf("Distance : %.1f m", totalDistance);
            Serial.println();
            Serial.printf("Duration : %.1f s", totalDuration);
            Serial.println();

            // ================================
            // Segments
            // ================================

            JsonArray segments =
                firstRoute["properties"]["segments"].as<JsonArray>();

            Serial.printf("Segments : %d", segments.size());
            Serial.println();

            for (int seg = 0; seg < segments.size(); seg++)
            {
                JsonObject segment = segments[seg];

                Serial.printf("---- Segment %d ----", seg);
                Serial.println();

                JsonArray steps = segment["steps"];

                for (int step = 0; step < steps.size(); step++)
                {
                    JsonObject currentStep = steps[step];

                    int type = currentStep["type"];
                    const char* instruction =
                        currentStep["instruction"];

                    Serial.printf("[%d] %s",
                                  type,
                                  instruction);
                    Serial.println();
                }

                Serial.println();
            }
        }
    }
    else // If the querry didn't work
    {
        Serial.print("GET failed: ");
        Serial.println(http.errorToString(httpCode));
    }
    // Closing http to not use to much resources
    http.end();
}

bool getRouteSummary(float& distanceMeters, float& durationSeconds)
{
    if (!hasRouteSummary)
    {
        return false;
    }

    distanceMeters = latestDistanceMeters;
    durationSeconds = latestDurationSeconds;
    return true;
}