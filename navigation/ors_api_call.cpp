#include "ors_api_call.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "../display/screen.h"
#include "ors_api_key.h"

// ORS API endpoint and key (POST JSON response variant)
const char* ORS_BASE_URL = "https://api.openrouteservice.org/v2/directions/cycling-regular/json";

// Default departure/arrival points (same spot as the original hardcoded
// request) - overwritten by setRoutePoints() once the web page sends new ones
double startLat;
double startLng;
double endLat;
double endLng;

// Set when new points arrive from the web page, so updateBikeRoute() fetches
// a fresh route on the very next loop() instead of waiting for the timer
bool routePointsChanged = false;

bool hasRouteSummary = false;
float latestDistanceKm = 0.0f;
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

// Function that displays the result of the POST call to the serial Monitor
void getBikeRoute()
{
  // Looks if it's connected to any wifi
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi not connected");
        return;
    }

    // ORS expects coordinate order as [lng, lat] in the JSON body.
    // Keep the response small: we only need the route summary on the ESP32.
    String body = String("{\"coordinates\":[[") + String(startLng, 6) + "," + String(startLat, 6) +
                  "],[" + String(endLng, 6) + "," + String(endLat, 6) +
                  "]]," +
                  "\"instructions\":false," +
                  "\"language\":\"en\"," +
                  "\"maneuvers\":false," +
                  "\"preference\":\"recommended\"," +
                  "\"roundabout_exits\":false," +
                  "\"units\":\"m\"," +
                  "\"geometry\":false}";

    HTTPClient http;
    Serial.println("Sending POST request...");
    Serial.println(ORS_BASE_URL);
    http.begin(ORS_BASE_URL);

    http.addHeader("Authorization", ORS_API_KEY);
    http.addHeader("Content-Type", "application/json");
    http.addHeader(
    "Accept",
    "application/json; charset=utf-8");

    int httpCode = http.POST(body);

    // 0 means the query didn't work
    if (httpCode > 0)
    {
        Serial.printf("HTTP Code: %d", httpCode);
        Serial.println();

        // If the returned code is 200
        if (httpCode == HTTP_CODE_OK)
        {
            // Parse directly from the network stream to avoid buffering the
            // whole response in RAM.
            StaticJsonDocument<128> filter;
            filter["routes"][0]["summary"]["distance"] = true;
            filter["routes"][0]["summary"]["duration"] = true;

            StaticJsonDocument<384> doc;

            DeserializationError error = deserializeJson(
                doc,
                http.getStream(),
                DeserializationOption::Filter(filter));

            if (error)
            {
                Serial.print("JSON parsing failed: ");
                Serial.println(error.c_str());
                Serial.print("HTTP content length: ");
                Serial.println(http.getSize());
                return;
            }

            JsonArray routes = doc["routes"].as<JsonArray>();

            if (routes.isNull() || routes.size() == 0)
            {
                Serial.println("No routes returned.");
                return;
            }

            JsonObject firstRoute = routes[0];

            // ================================
            // Route summary
            // ================================

            JsonObject summary = firstRoute["summary"];

            float totalDistance = summary["distance"];
            float totalDuration = summary["duration"];

            latestDistanceKm = totalDistance;
            latestDurationSeconds = totalDuration;
            hasRouteSummary = true;

            Serial.println();
            Serial.println("========== ROUTE ==========");
            Serial.printf("Distance : %.1f km", totalDistance);
            Serial.println();
            Serial.printf("Duration : %.1f s", totalDuration);
            Serial.println();

            // ================================
            // Segments
            // ================================

            //JsonArray segments = firstRoute["segments"].as<JsonArray>();
            //
            //Serial.printf("Segments : %d", segments.size());
            //Serial.println();

            //for (int seg = 0; seg < segments.size(); seg++)
            //{
            //    JsonObject segment = segments[seg];

            //    Serial.printf("---- Segment %d ----", seg);
            //    Serial.println();

            //    JsonArray steps = segment["steps"];

            //    for (int step = 0; step < steps.size(); step++)
            //    {
            //        JsonObject currentStep = steps[step];

            //        int type = currentStep["type"];
            //        const char* instruction =
            //            currentStep["instruction"];

            //        Serial.printf("[%d] %s",
            //                      type,
            //                      instruction);
            //        Serial.println();
            //    }

            //    Serial.println();
            //}
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

bool getRouteSummary(float& distanceKm, float& durationSeconds)
{
    if (!hasRouteSummary)
    {
        return false;
    }

    distanceKm = latestDistanceKm;
    durationSeconds = latestDurationSeconds;
    return true;
}