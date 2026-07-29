#pragma once

#include "screen.h"
#include "../navigation/ors_api_call.h"


class SummaryScreen : public Screen
{
public:

    void draw(Adafruit_SSD1306& display) override;
};