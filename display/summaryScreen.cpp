#include "summaryScreen.h"

#include "../navigation/ors_api_call.h"

void SummaryScreen::draw(
    Adafruit_SSD1306& display)
{
    float distanceMeters = 0.0f;
    float durationSeconds = 0.0f;
    bool hasSummary = getRouteSummary(distanceMeters, durationSeconds);

    display.clearDisplay();

    display.setTextColor(SSD1306_WHITE);

    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("Route");

    display.drawLine(0, 18, 127, 18, SSD1306_WHITE);

    display.setTextSize(1);
    display.setCursor(0, 24);
    display.print("Distance: ");
    if (hasSummary)
    {
        float distanceKm = distanceMeters / 1000.0f;
        display.print(distanceKm, 1);
    }
    else
    {
        display.print("--");
    }
    display.println(" km");

    display.setCursor(0, 36);
    display.print("Time: ");
    if (hasSummary)
    {
        float durationMin = durationSeconds / 60.0f;
        display.print(durationMin, 0);
    }
    else
    {
        display.print("--");
    }
    display.println(" min");

    display.setCursor(0, 48);
    display.println("ETA: not implemented");



    display.display();
}