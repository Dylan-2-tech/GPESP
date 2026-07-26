#pragma once

#include "screen.h"

class SummaryScreen : public Screen
{
public:

    void draw(Adafruit_SSD1306& display) override;
};