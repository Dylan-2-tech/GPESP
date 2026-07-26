#pragma once

#include "screen.h"

class HomeScreen : public Screen
{
public:

    void draw(Adafruit_SSD1306& display) override;
};