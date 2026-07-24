#include "ors_api_call.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// URL to make the GET call to ORS
const char* ORS_API =
"https://api.openrouteservice.org/v2/directions/cycling-regular?"
"api_key=eyJvcmciOiI1YjNjZTM1OTc4NTExMTAwMDFjZjYyNDgiLCJpZCI6ImFjYTRhMWI5NGM3NzRiMDFhZDAzY2NkODI4NTQyZWVlIiwiaCI6Im11cm11cjY0In0="
"&start=8.681495,49.41461"
"&end=8.687872,49.420318";

unsigned long previousRequest = 61000;
const unsigned long requestInterval = 60000; // 60 seconds

// Function that displays the result of the GET call to the serial Monitor
void getBikeRoute()
{
  // Looks if it's connected to any wifi
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi not connected");
        return;
    }

    HTTPClient http;
    Serial.println("Sending GET request...");
    http.begin(ORS_API);

    http.addHeader(
    "Accept",
    "application/json, application/geo+json, application/gpx+xml, img/png; charset=utf-8");

    int httpCode = http.GET();

    // 0 means the query didn't work
    if (httpCode > 0)
    {
        Serial.printf("HTTP Code: %d\n", httpCode);

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
                return;
            }

            // ================================
            // First route (first feature)
            // ================================

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

void updateBikeRoute()
{
  // Every requestInterval milliseconds
  if (millis() - previousRequest >= requestInterval)
  {
      previousRequest = millis();
      getBikeRoute();
  }
}
