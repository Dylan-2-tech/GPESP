/*

  Split into:
    - network/   : WiFi connection and web server
    - navigation/: ORS API GET request and JSON parsing
    - display/   : OLED screen classes and drawing code
*/

#define ESP_DRD_USE_SPIFFS true

#include "network/wifi_connection.h"
#include "network/web_server.h"

#include "navigation/ors_api_call.h"

#include "display/screen.h"
#include "display/homeScreen.h"
#include "display/navigationScreen.h"
#include "display/summaryScreen.h"

#include <Adafruit_SSD1306.h>

static const uint8_t OLED_ADDR = 0x3C;

Adafruit_SSD1306 display(
    128,
    64,
    &Wire,
    -1
);

HomeScreen home;
NavigationScreen navigation;
SummaryScreen summary;

Screen* currentScreen = &home;

void showHomeScreen(){currentScreen = &home;}
void showNavigationScreen(){currentScreen = &navigation;}
void showSummaryScreen(){currentScreen = &summary;}

void setup()
{
  // Setup Serial monitor
  Serial.begin(115200);
  delay(10);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
  {
    Serial.println("SSD1306 allocation failed");
    for (;;)
    {
      delay(1000);
    }
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.display();

  setupWifi();
  setupWebServer();
  currentScreen->draw(display);

}

void loop()
{
  handleWifiResetButton();
  handleWebServer();
  updateBikeRoute();
}
