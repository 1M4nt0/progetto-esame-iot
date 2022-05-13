#include <screen_utils.h>

OLED_CLASS_OBJ display(OLED_ADDRESS, OLED_SDA, OLED_SCL);

void initDisplay()
{
    display.init();
    // display.flipScreenVertically();
    display.setFont(ArialMT_Plain_16);
}

void drawToScreen(String message)
{
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(display.getWidth() / 2, display.getHeight() / 2, message);
    display.display();
}

void drawTwoToScreen(String message1, String message2)
{
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(display.getWidth() / 2, display.getHeight() / 4, message1);
    display.drawString(display.getWidth() / 2, (display.getHeight() / 4) * 3, message2);
    display.display();
}

void drawDashboard(int player, int points)
{
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(display.getWidth() / 2, display.getHeight() / 4, "Giocatore " + String(player));
    display.drawString(display.getWidth() / 2, (display.getHeight() / 4) * 3, "Punti: " + String(points));
    display.display();
}