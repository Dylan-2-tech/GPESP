#include "ors_api_call.h"

// ORS API endpoint and key (POST JSON response variant)
const char* ORS_BASE_URL = "https://api.heigit.org/openrouteservice/v2/directions/cycling-regular/json";

// Default departure/arrival points (same spot as the original hardcoded
// request) - overwritten by setRoutePoints() once the web page sends new ones
double startLat;
double startLng;
double endLat;
double endLng;

bool hasRouteSummary = false;
float latestDistanceKm = 0.0f;
float latestDurationSeconds = 0.0f;
String latestRouteGeometry;

void setRoutePoints(double newStartLat, double newStartLng, double newEndLat, double newEndLng)
{
  startLat = newStartLat;
  startLng = newStartLng;
  endLat   = newEndLat;
  endLng   = newEndLng;
    Serial.println(F("New route points received:"));
  Serial.printf("  Start: %f, %f", startLat, startLng);
    Serial.println();
  Serial.printf("  End:   %f, %f", endLat, endLng);
    Serial.println();

  getBikeRoute();
}

// Function that displays the result of the POST call to the serial Monitor
void getBikeRoute()
{
        latestRouteGeometry = F("");
        hasRouteSummary = false;

  // Looks if it's connected to any wifi
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println(F("WiFi not connected"));
        return;
    }

    // ORS expects coordinate order as [lng, lat] in the JSON body.
    // Request the full route payload we need for the serial output and
    // summary screen, but keep it limited to a single optimized route.
    char body[320];
    int bodyLength = snprintf(
        body,
        sizeof(body),
        "{\"coordinates\":[[%.6f,%.6f],[%.6f,%.6f]],\"instructions\":true,\"language\":\"en\",\"maneuvers\":true,\"preference\":\"recommended\",\"roundabout_exits\":true,\"units\":\"km\",\"geometry\":true}",
        startLng,
        startLat,
        endLng,
        endLat);

    if (bodyLength < 0 || static_cast<size_t>(bodyLength) >= sizeof(body))
    {
        Serial.println(F("ORS request body is too large."));
        return;
    }

    HTTPClient http;
    Serial.println(F("Sending POST request..."));
    Serial.println(ORS_BASE_URL);
    http.setTimeout(30000);
    http.setReuse(false);

    if (!http.begin(ORS_BASE_URL))
    {
        Serial.println(F("Could not start ORS HTTPS request."));
        http.end();
        return;
    }

    http.addHeader("Authorization", ORS_API_KEY);
    http.addHeader("Content-Type", "application/json");
    http.addHeader(
    "Accept",
    "application/json; charset=utf-8");

    int httpCode = http.POST(body);

    // 0 means the query didn't work
    if (httpCode > 0)
    {
        // If the returned code is 200
        if (httpCode == HTTP_CODE_OK)
        {
            String response = http.getString();

            if (response.length() == 0)
            {
                Serial.println(F("ORS returned an empty response body."));
                http.end();
                return;
            }

            DynamicJsonDocument doc(24576);

            DeserializationError error = deserializeJson(doc, response);

            if (error)
            {
                Serial.print(F("JSON parsing failed: "));
                Serial.println(error.c_str());
                http.end();
                return;
            }

            JsonObject route = doc["routes"][0].as<JsonObject>();

            if (route.isNull())
            {
                Serial.println(F("No routes returned."));
                http.end();
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
            Serial.println(F("========== ROUTE =========="));
            Serial.printf("Distance : %.1f km", totalDistance);
            Serial.println();
            Serial.printf("Duration : %.1f s", totalDuration);
            Serial.println();

            const char* geometry = route["geometry"] | "";
            latestRouteGeometry = geometry;
            Serial.println(F("Route geometry:"));
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
        Serial.print(F("POST failed: "));
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

bool getRouteGeometry(String& geometry)
{
    if (latestRouteGeometry.length() == 0)
    {
        return false;
    }

    geometry = latestRouteGeometry;
    return true;
}