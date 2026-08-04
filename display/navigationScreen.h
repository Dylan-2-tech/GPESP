#pragma once

#include "screen.h"

// Enumeration for the four cardinal directions
enum Direction
{
    LEFT,
    RIGHT,
    SHARP_LEFT,
    SHARP_RIGHT,
    SLIGHT_LEFT,
    SLIGHT_RIGHT,
    STRAIGHT,
    ENTER_ROUNDABOUT,
    EXIT_ROUNDABOUT,
    U_TURN,
    GOAL,
    DEPART,
    KEEP_LEFT,
    KEEP_RIGHT
};

class NavigationScreen : public Screen
{
public:

    void draw(Adafruit_SSD1306& display) override;

    void draw_left_arrow(Adafruit_SSD1306& display);
    void draw_right_arrow(Adafruit_SSD1306& display);
    void draw_up_arrow(Adafruit_SSD1306& display);
    void draw_down_arrow(Adafruit_SSD1306& display);

    void set_current_direction(Direction direction);

    private:
    Direction current_direction = Direction::STRAIGHT;
};