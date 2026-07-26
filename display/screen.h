#pragma once

#include <Adafruit_SSD1306.h>

class Screen
{
public:

    virtual ~Screen() {}

    virtual void draw(Adafruit_SSD1306& display) = 0;
};

void showHomeScreen();
void showNavigationScreen();
void showSummaryScreen();