#include "homeScreen.h"

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

void HomeScreen::draw(
    Adafruit_SSD1306& display)
{
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    drawCenteredText(display, "GPESP v0.1", 0, 2);
    drawCenteredText(display, "Bike GPS", 24, 1);

    display.drawLine(10, 42, 118, 42, SSD1306_WHITE);

    display.setTextSize(1);
    display.setCursor(10, 46);
    display.println("Waiting for you to");
    display.setCursor(10, 56);
    display.println("connect the device");

    display.display();
}