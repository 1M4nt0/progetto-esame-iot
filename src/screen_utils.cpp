#include "screen_utils.h"

OLED_CLASS_OBJ display(OLED_ADDRESS, OLED_SDA, OLED_SCL);

void initDisplay()
{
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_16);
}

void drawToScreen(String text)
{
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(display.getWidth() / 2, display.getHeight() / 2, text);
    display.display();
}