#include "summaryScreen.h"

void SummaryScreen::draw(Adafruit_SSD1306& display)
{
    float distanceKm = 0.0f;
    float durationSeconds = 0.0f;
    bool hasSummary = getRouteSummary(distanceKm, durationSeconds);

    display.clearDisplay();

    display.setTextColor(SSD1306_WHITE);

    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println(F("Route"));

    display.drawLine(0, 18, 127, 18, SSD1306_WHITE);

    display.setTextSize(1);
    display.setCursor(0, 24);
    display.print(F("Distance: "));
    if (hasSummary)
    {
        display.print(distanceKm, 1);
    }
    else
    {
        display.print(F("--"));
    }
    display.println(F(" km"));

    display.setCursor(0, 36);
    display.print(F("Time: "));
    if (hasSummary)
    {
        float durationMin = durationSeconds / 60.0f;
        display.print(durationMin, 0);
    }
    else
    {
        display.print(F("--"));
    }
    display.println(F(" min"));

    display.setCursor(0, 48);
    display.println(F("ETA: not implemented"));

    display.display();
}