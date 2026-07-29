#include "homeScreen.h"

#include <cstring>

namespace
{
void drawCenteredText(Adafruit_SSD1306& display, const char* text, int16_t y, uint8_t textSize)
{
    int16_t x1, y1;
    uint16_t w, h;

    display.setTextSize(textSize);
    display.getTextBounds(text, 0, y, &x1, &y1, &w, &h);

    int16_t x = (128 - w) / 2;
    display.setCursor(x, y);
    display.println(text);
}
}

void HomeScreen::setState(int type, const char* param)
{
    stateType = type;

    if (param == nullptr)
    {
        stateParam[0] = '\0';
        return;
    }

    strncpy(stateParam, param, sizeof(stateParam) - 1);
    stateParam[sizeof(stateParam) - 1] = '\0';
}

void HomeScreen::draw(Adafruit_SSD1306& display)
{

    // Main home screen display with GPESP
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    drawCenteredText(display, "GPESP v0.1", 0, 2);
    drawCenteredText(display, "Bike GPS", 24, 1);

    display.drawLine(10, 42, 118, 42, SSD1306_WHITE);


    if (stateType == 0) // Starting state and waiting for WiFi connection
    {
        display.setTextSize(1);
        display.setCursor(10, 46);
        display.println("Starting...");
        display.setCursor(10, 56);
        display.println("Connect to GPESP WiFi to configure");
    }
    else if (stateType == 1) // Connected state
    {
        display.setTextSize(1);
        display.setCursor(10, 46);
        display.println("Connected to WiFi:");
        display.setCursor(10, 56);
        display.println(stateParam[0] != '\0' ? stateParam : "--");
    }
    else if (stateType == 2) // Setting up the Web Server state
    {
        display.setTextSize(1);
        display.setCursor(10, 46);
        display.println("Setting up Web Server...");
    }
    else if (stateType == 3) // WebServer started
    {
        display.setTextSize(1);
        display.setCursor(10, 46);
        display.println("Web Server started:");
        display.setCursor(10, 56);
        display.println(stateParam[0] != '\0' ? stateParam : "--");
    }


    display.display();
}