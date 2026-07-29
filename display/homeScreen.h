#pragma once

#include "screen.h"
#include <cstring>


// Different types of home screen states
/*
0 - Starting state and waiting for WiFi connection
1 - Connected state
2 - Setting up the Web Server state
3 - WebServer started


*/

class HomeScreen : public Screen
{
public:

    void setState(int type, const char* param = nullptr);

    void draw(Adafruit_SSD1306& display) override;

private:

    int stateType = 0;
    char stateParam[96] = "";

};