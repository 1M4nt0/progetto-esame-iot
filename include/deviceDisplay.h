#pragma once
#include <board_conf.h>

class DeviceDisplay
{
public:
    DeviceDisplay();
    ~DeviceDisplay();
    void drawDashboard(int playerid, int points);
    void drawToScreen(String message);
    void drawTwoToScreen(String message1, String message2);
    void drawThreeToScreen(String message1, String message2, String message3);

private:
    OLED_CLASS_OBJ *_display;
    void _initDisplay();
};