#include "navigationScreen.h"

// Screen dimensions = 128x64 pixels

void NavigationScreen::draw(Adafruit_SSD1306& display)
{
    display.clearDisplay();

    display.setTextColor(SSD1306_WHITE);

    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("Navigation");
    display.drawFastHLine(0, 16, 128, SSD1306_WHITE);

    switch (current_direction)
    {
    case Direction::LEFT:
        draw_left_arrow(display);
        break;
    case Direction::RIGHT:
        draw_right_arrow(display);
        break;
    case Direction::STRAIGHT:
        draw_up_arrow(display);
        break;
    case Direction::U_TURN:
        draw_down_arrow(display);
        break;
    
    default:
        display.setTextSize(1);
        display.setCursor(10, 24);
        display.println("Direction not implemented");
        break;
    }
    
    display.display();
}

void NavigationScreen::set_current_direction(Direction direction)
{
    current_direction = direction;
}

void NavigationScreen::draw_left_arrow(Adafruit_SSD1306& display)
{
    display.drawFastHLine(42, 44, 43, SSD1306_WHITE);
    display.drawFastHLine(42, 45, 43, SSD1306_WHITE);
    display.drawFastHLine(42, 46, 43, SSD1306_WHITE);

    // Draw a trigle at the end of the line to represent the arrowhead
    display.drawTriangle(42, 30, 42, 60, 27, 45, SSD1306_WHITE);
    // fill the triangle to make it more visible
    display.fillTriangle(42, 30, 42, 60, 27, 45, SSD1306_WHITE);
}

void NavigationScreen::draw_right_arrow(Adafruit_SSD1306& display)
{
    display.drawFastHLine(42, 44, 43, SSD1306_WHITE);
    display.drawFastHLine(42, 45, 43, SSD1306_WHITE);
    display.drawFastHLine(42, 46, 43, SSD1306_WHITE);

    // Draw a trigle at the end of the line to represent the arrowhead
    display.drawTriangle(85, 30, 85, 60, 100, 45, SSD1306_WHITE);
    // fill the triangle to make it more visible
    display.fillTriangle(85, 30, 85, 60, 100, 45, SSD1306_WHITE);
}

void NavigationScreen::draw_up_arrow(Adafruit_SSD1306& display)
{
    display.drawFastVLine(63, 33, 43, SSD1306_WHITE);
    display.drawFastVLine(64, 33, 43, SSD1306_WHITE);
    display.drawFastVLine(65, 33, 43, SSD1306_WHITE);

    // Draw a trigle at the end of the line to represent the arrowhead
    display.drawTriangle(50, 33, 78, 33, 64, 19, SSD1306_WHITE);
    // fill the triangle to make it more visible
    display.fillTriangle(50, 33, 78, 33, 64, 19, SSD1306_WHITE);
}

void NavigationScreen::draw_down_arrow(Adafruit_SSD1306& display)
{
    display.drawFastVLine(63, 19, 43, SSD1306_WHITE);
    display.drawFastVLine(64, 19, 43, SSD1306_WHITE);
    display.drawFastVLine(65, 19, 43, SSD1306_WHITE);

    // Draw a trigle at the end of the line to represent the arrowhead
    display.drawTriangle(50, 47, 78, 47, 64, 62, SSD1306_WHITE);
    // fill the triangle to make it more visible
    display.fillTriangle(50, 47, 78, 47, 64, 62, SSD1306_WHITE);
}