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

static bool fetchRouteResponse(const char* body, DynamicJsonDocument& doc)
{
    HTTPClient http;
    http.setTimeout(30000);
    http.setReuse(false);

    if (!http.begin(ORS_BASE_URL))
    {
        Serial.println(F("Could not start ORS HTTPS request."));
        http.end();
        return false;
    }

    http.addHeader("Authorization", ORS_API_KEY);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Accept", "application/json; charset=utf-8");

    int httpCode = http.POST(body);
    if (httpCode != HTTP_CODE_OK)
    {
        if (httpCode > 0)
        {
            Serial.printf("ORS returned HTTP %d", httpCode);
            Serial.println();
        }
        else
        {
            Serial.print(F("POST failed: "));
            Serial.println(http.errorToString(httpCode));
        }
        http.end();
        return false;
    }

    String response = http.getString();
    http.end();

    if (response.length() == 0)
    {
        Serial.println(F("ORS returned an empty response body."));
        return false;
    }

    DeserializationError error = deserializeJson(doc, response);
    if (error)
    {
        Serial.print(F("JSON parsing failed: "));
        Serial.println(error.c_str());
        return false;
    }

    return true;
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
    char instructionsBody[320];
    char geometryBody[240];
    int instructionsLength = snprintf(
        instructionsBody,
        sizeof(instructionsBody),
        "{\"coordinates\":[[%.6f,%.6f],[%.6f,%.6f]],\"instructions\":true,\"instructions_format\":\"text\",\"maneuvers\":true,\"preference\":\"recommended\",\"units\":\"km\",\"geometry\":false}",
        startLng,
        startLat,
        endLng,
        endLat);
    int geometryLength = snprintf(
        geometryBody,
        sizeof(geometryBody),
        "{\"coordinates\":[[%.6f,%.6f],[%.6f,%.6f]],\"instructions\":false,\"maneuvers\":false,\"preference\":\"recommended\",\"units\":\"km\",\"geometry\":true}",
        startLng,
        startLat,
        endLng,
        endLat);

    if (instructionsLength < 0 || static_cast<size_t>(instructionsLength) >= sizeof(instructionsBody) ||
        geometryLength < 0 || static_cast<size_t>(geometryLength) >= sizeof(geometryBody))
    {
        Serial.println(F("ORS request body is too large."));
        return;
    }

    Serial.println(F("Sending ORS instructions request..."));
    Serial.println(ORS_BASE_URL);
    {
        DynamicJsonDocument instructionsDoc(24576);
        if (!fetchRouteResponse(instructionsBody, instructionsDoc))
        {
            return;
        }

        JsonObject instructionsRoute = instructionsDoc["routes"][0].as<JsonObject>();
        if (instructionsRoute.isNull())
        {
            Serial.println(F("No route returned from instructions request."));
            return;
        }

        JsonObject summary = instructionsRoute["summary"];
        latestDistanceKm = summary["distance"];
        latestDurationSeconds = summary["duration"];
        hasRouteSummary = true;

        Serial.println();
        Serial.println(F("========== ROUTE =========="));
        Serial.printf("Distance : %.1f km", latestDistanceKm);
        Serial.println();
        Serial.printf("Duration : %.1f s", latestDurationSeconds);
        Serial.println();

        JsonArray segments = instructionsRoute["segments"].as<JsonArray>();
        int instructionsIndex = 0;
        if (!segments.isNull())
        {
            for (JsonVariant segmentVariant : segments)
            {
                JsonArray steps = segmentVariant["steps"].as<JsonArray>();
                if (steps.isNull())
                {
                    continue;
                }

                for (JsonVariant stepVariant : steps)
                {
                    JsonObject step = stepVariant.as<JsonObject>();
                    if (step.isNull())
                    {
                        continue;
                    }

                    if (instructionsIndex < MAX_INSTRUCTIONS)
                    {
                        Serial.println(step["instruction"] | "");
                        routeInstructions[instructionsIndex++] = step["type"] | -1;
                    }
                }
            }
        }
    }

    Serial.println(F("Sending ORS geometry request..."));
    DynamicJsonDocument geometryDoc(12288);
    if (!fetchRouteResponse(geometryBody, geometryDoc))
    {
        latestRouteGeometry = F("");
        Serial.println(F("Geometry request failed."));
        return;
    }

    JsonObject geometryRoute = geometryDoc["routes"][0].as<JsonObject>();
    if (geometryRoute.isNull())
    {
        latestRouteGeometry = F("");
        Serial.println(F("No route returned from geometry request."));
        return;
    }

    latestRouteGeometry = geometryRoute["geometry"] | "";
    Serial.println(F("Route geometry:"));
    Serial.println(latestRouteGeometry);
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