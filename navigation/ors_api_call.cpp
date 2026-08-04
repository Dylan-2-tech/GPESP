#include "ors_api_call.h"

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
bool routeReturned = false;

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
    // Request the full route payload we need for the serial output and
    // summary screen, but keep it limited to a single optimized route.
    String body = String("{\"coordinates\":[[") + String(startLng, 6) + "," + String(startLat, 6) +
                  "],[" + String(endLng, 6) + "," + String(endLat, 6) +
                  "]]," +
                  "\"instructions\":true," +
                  "\"language\":\"en\"," +
                  "\"maneuvers\":true," +
                  "\"preference\":\"recommended\"," +
                  "\"roundabout_exits\":true," +
                  "\"units\":\"km\"," +
                  "\"geometry\":true}";

    HTTPClient http;
    Serial.println("Sending POST request...");
    Serial.println(ORS_BASE_URL);
    http.useHTTP10(true); // Disable the chunked data transfer encoding, which the ORS API doesn't support
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
            // whole response in RAM. Only one route is returned, so we can
            // read the full payload and access routes[0] directly.
            DynamicJsonDocument doc(6144);
            
            DeserializationError error = deserializeJson(doc, http.getStream());

            if (error)
            {
                Serial.print("JSON parsing failed: ");
                Serial.println(error.c_str());
                Serial.print("HTTP content length: ");
                Serial.println(http.getSize());
                return;
            }

            JsonObject route = doc["routes"][0].as<JsonObject>();

            if (route.isNull())
            {
                Serial.println("No routes returned.");
                return;
            }

            // ================================
            // Route summary
            // ================================

            JsonObject summary = route["summary"];

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

            const char* geometry = route["geometry"] | "";
            Serial.println("Route geometry:");
            Serial.println(geometry);

            // ================================
            // Steps
            // ================================

            JsonArray segments = route["segments"].as<JsonArray>();

            if (!segments.isNull())
            {
                for (JsonVariant segmentVariant : segments)
                {
                    JsonObject segment = segmentVariant.as<JsonObject>();
                    if (segment.isNull())
                    {
                        continue;
                    }

                    JsonArray steps = segment["steps"].as<JsonArray>();

                    if (steps.isNull())
                    {
                        continue;
                    }
                    
                    int InstructionsIndex = 0;
                    for (JsonVariant stepVariant : steps)
                    {
                        JsonObject step = stepVariant.as<JsonObject>();
                        if (step.isNull())
                        {
                            continue;
                        }

                        const char* instruction = step["instruction"] | "";
                        Serial.println(instruction);
                        routeInstructions[InstructionsIndex++] = step["type"] | -1;
                    }
                }
            }
        }
    }
    else // If the querry didn't work
    {
        Serial.print("POST failed: ");
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