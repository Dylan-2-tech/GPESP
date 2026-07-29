#ifndef WIFI_CONNECTION_H
#define WIFI_CONNECTION_H

#include <WiFi.h>
#include <FS.h>
#include <SPIFFS.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>

// JSON configuration file
#define JSON_CONFIG_FILE "/test_config.json"

// GPIO 4 is shared with the main sketch's screen-cycle button.
#define TRIGGER_PIN 4

// Seconds to run the on-demand config portal for
extern int configPortalTimeout;

// Flag for saving data
extern bool shouldSaveConfig;

// Variable to hold data from custom textbox
extern char userNameString[50];

// WiFiManager object
extern WiFiManager wm;

// Call once from the main setup(). Connects to WiFi (or opens the config
// portal if no saved network / no saved config exists yet), and loads any
// previously saved settings from SPIFFS.
void setupWifi();

// Kept for compatibility with the old reset-button flow; currently a no-op.
void handleWifiResetButton();

// Configuration file helpers
void saveConfigFile();
bool loadConfigFile();

// WiFiManager callbacks
void saveConfigCallback();
void configModeCallback(WiFiManager *myWiFiManager);

#endif