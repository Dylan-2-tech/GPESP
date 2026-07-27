#include "wifi_connection.h"

// Seconds to run the on-demand config portal for
int configPortalTimeout = 120;

// Flag for saving data
bool shouldSaveConfig = false;

// Variable to hold data from custom textbox
char userNameString[50] = "";

// WiFiManager object
WiFiManager wm;

void saveConfigFile()
// Save Config in JSON format
{
  Serial.println(F("Saving configuration..."));

  // Create a JSON document
  StaticJsonDocument<512> json;
  json["userNameString"] = userNameString;

  // Open config file
  File configFile = SPIFFS.open(JSON_CONFIG_FILE, "w");
  if (!configFile)
  {
    // Error, file did not open
    Serial.println("failed to open config file for writing");
  }

  // Serialize JSON data to write to file
  serializeJsonPretty(json, Serial);
  if (serializeJson(json, configFile) == 0)
  {
    // Error writing file
    Serial.println(F("Failed to write to file"));
  }
  // Close file
  configFile.close();
}

bool loadConfigFile()
// Load existing configuration file
{
  // Uncomment if we need to format filesystem
  // SPIFFS.format();

  // Read configuration from FS json
  Serial.println("Mounting File System...");

  // May need to make it begin(true) first time you are using SPIFFS
  if (SPIFFS.begin(false) || SPIFFS.begin(true))
  {
    Serial.println("mounted file system");
    if (SPIFFS.exists(JSON_CONFIG_FILE))
    {
      // The file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open(JSON_CONFIG_FILE, "r");
      if (configFile)
      {
        Serial.println("Opened configuration file");
        StaticJsonDocument<512> json;
        DeserializationError error = deserializeJson(json, configFile);
        serializeJsonPretty(json, Serial);
        if (!error)
        {
          Serial.println("Parsing JSON");

          const char* loadedUserName = json["userNameString"] | "";
          strncpy(userNameString, loadedUserName, sizeof(userNameString));

          return true;
        }
        else
        {
          // Error loading JSON data
          Serial.println("Failed to load json config");
        }
      }
    }
  }
  else
  {
    // Error mounting file system
    Serial.println("Failed to mount FS");
  }

  return false;
}

void saveConfigCallback()
// Callback notifying us of the need to save configuration
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void configModeCallback(WiFiManager *myWiFiManager)
// Called when config mode launched
{
  Serial.println("Entered Configuration Mode");

  Serial.print("Config SSID: ");
  Serial.println(myWiFiManager->getConfigPortalSSID());

  Serial.print("Config IP Address: ");
  Serial.println(WiFi.softAPIP());
}

void setupWifi()
{
  // Change to true when testing to force configuration every time we run
  bool forceConfig = false;

  bool spiffsSetup = loadConfigFile();
  if (!spiffsSetup)
  {
    Serial.println(F("Forcing config mode as there is no saved config"));
    forceConfig = true;
  }

  // Explicitly set WiFi mode
  WiFi.mode(WIFI_STA);

  // Reset settings (only for development) - commented out so settings persist;
  // use handleWifiResetButton() in loop() to reset instead
  // wm.resetSettings();

  // Set config save notify callback
  wm.setSaveConfigCallback(saveConfigCallback);

  // Set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wm.setAPCallback(configModeCallback);

  // Custom elements

  // Text box (Username) - 50 characters maximum
  WiFiManagerParameter custom_text_box_username("key_username", "Enter your username here", userNameString, 50);

  // Add all defined parameters
  wm.addParameter(&custom_text_box_username);

  if (forceConfig)
    // Run if we need a configuration
  {
    if (!wm.startConfigPortal("GPESP"))
    {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.restart();
      delay(5000);
    }
  }
  else
  {
    if (!wm.autoConnect("GPESP"))
    {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      // if we still have not connected restart and try all over again
      ESP.restart();
      delay(5000);
    }
  }

  // If we get here, we are connected to the WiFi

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Lets deal with the user config values

  // Copy the username value
  strncpy(userNameString, custom_text_box_username.getValue(), sizeof(userNameString));
  Serial.print("userNameString: ");
  Serial.println(userNameString);

  // Save the custom parameters to FS
  if (shouldSaveConfig)
  {
    saveConfigFile();
  }
}

void handleWifiResetButton()
{
  // Is the settings-reset / configuration portal requested?
  // if (digitalRead(TRIGGER_PIN) == LOW)
  // {
  //   Serial.println("Trigger pin pressed - resetting settings");
  //
  //   // Wipe saved WiFi credentials and custom parameters
  //   wm.resetSettings();
  //
  //   // Set config portal timeout
  //   wm.setConfigPortalTimeout(configPortalTimeout);
  //
  //   if (!wm.startConfigPortal("GPESP"))
  //   {
  //     Serial.println("failed to connect and hit timeout");
  //     delay(3000);
  //     // reset and try again, or maybe put it to deep sleep
  //     ESP.restart();
  //     delay(5000);
  //   }
  //
  //   // If we get here you have connected to the WiFi
  //   Serial.println("connected...yeey :)");
  // }
}