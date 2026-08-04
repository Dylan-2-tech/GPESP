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

namespace
{
constexpr unsigned long BUTTON_DEBOUNCE_MS = 40;

bool lastButtonReading = HIGH;
bool stableButtonState = HIGH;
unsigned long lastDebounceTime = 0;

void drawCurrentScreen()
{
  currentScreen->draw(display);
}

void advanceScreen()
{
  if (currentScreen == &home)
  {
    currentScreen = &summary;
  }
  else if (currentScreen == &summary)
  {
    currentScreen = &navigation;
  }
  else
  {
    currentScreen = &home;
  }

  drawCurrentScreen();
}

void pollScreenButton()
{
  bool reading = digitalRead(TRIGGER_PIN);

  if (reading != lastButtonReading)
  {
    lastDebounceTime = millis();
    lastButtonReading = reading;
  }

  if ((millis() - lastDebounceTime) > BUTTON_DEBOUNCE_MS)
  {
    if (reading != stableButtonState)
    {
      stableButtonState = reading;
      if (stableButtonState == LOW)
      {
        advanceScreen();
      }
    }
  }
}
}

void showHomeScreenState(int type, const char* param)
{
  home.setState(type, param);

  if (currentScreen == &home)
  {
    drawCurrentScreen();
  }
}

void showHomeScreen(){currentScreen = &home; drawCurrentScreen();}
void showNavigationScreen(){currentScreen = &navigation; drawCurrentScreen();}
void showSummaryScreen(){currentScreen = &summary; drawCurrentScreen();}

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

  drawCurrentScreen();

  // GPIO 4 is now the screen-cycle button.
  pinMode(TRIGGER_PIN, INPUT_PULLUP);

  //setupWifi();
  //setupWebServer();
}

void loop()
{
  //handleWifiResetButton();
  pollScreenButton();
}
