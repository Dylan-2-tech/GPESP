/*
  Wifi_Route.ino

  Connects to WiFi via WiFiManager (with a saved-settings config portal),
  then periodically fetches a bike route from the ORS API.

  Split into:
    - wifi_connection.h / .cpp : WiFi connection, config portal, saved settings
    - ors_api_call.h / .cpp    : ORS API GET request and JSON parsing
    - display.h / .cpp         : route display (placeholder - TODO)

  DroneBot Workshop 2022
  https://dronebotworkshop.com
  Functions based upon sketch by Brian Lough
  https://github.com/witnessmenow/ESP32-WiFi-Manager-Examples
*/

#define ESP_DRD_USE_SPIFFS true

#include "wifi_connection.h"
#include "ors_api_call.h"
#include "display.h"

void setup()
{
  // Setup Serial monitor
  Serial.begin(115200);
  delay(10);

  setupWifi();

  // setupDisplay(); // TODO: enable once display.cpp is implemented
}

void loop()
{
  handleWifiResetButton();
  updateBikeRoute();

  // updateDisplay(); // TODO: enable once display.cpp is implemented
}
