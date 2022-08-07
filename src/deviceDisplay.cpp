#include <deviceDisplay.h>

DeviceDisplay::DeviceDisplay()
{
    this->_display = new OLED_CLASS_OBJ(OLED_ADDRESS, OLED_SDA, OLED_SCL);
    this->_initDisplay();
}

DeviceDisplay::~DeviceDisplay()
{
    delete this->_display;
}

void DeviceDisplay::_initDisplay()
{
    this->_display->init();
    // display.flipScreenVertically();
    this->_display->setFont(ArialMT_Plain_16);
}

void DeviceDisplay::drawToScreen(String message)
{
    this->_display->clear();
    this->_display->setTextAlignment(TEXT_ALIGN_CENTER);
    this->_display->drawString(this->_display->getWidth() / 2, this->_display->getHeight() / 3 + 5, message);
    this->_display->display();
}

void DeviceDisplay::drawTwoToScreen(String message1, String message2)
{
    this->_display->clear();
    this->_display->setTextAlignment(TEXT_ALIGN_CENTER);
    this->_display->drawString(this->_display->getWidth() / 2, this->_display->getHeight() / 4, message1);
    this->_display->drawString(this->_display->getWidth() / 2, (this->_display->getHeight() / 4) * 2 + 2, message2);
    this->_display->display();
}

void DeviceDisplay::drawThreeToScreen(String message1, String message2, String message3)
{
    this->_display->clear();
    this->_display->setTextAlignment(TEXT_ALIGN_CENTER);
    this->_display->drawString(this->_display->getWidth() / 2, this->_display->getHeight() / 6 - 5, message1);
    this->_display->drawString(this->_display->getWidth() / 2, (this->_display->getHeight() / 6) * 3 - 5, message2);
    this->_display->drawString(this->_display->getWidth() / 2, (this->_display->getHeight() / 6) * 5 - 5, message3);
    this->_display->display();
}

void DeviceDisplay::drawDashboard(int player, int points)
{
    this->_display->clear();
    this->_display->setTextAlignment(TEXT_ALIGN_CENTER);
    this->_display->drawString(this->_display->getWidth() / 2, this->_display->getHeight() / 4, "Giocatore " + String(player));
    this->_display->drawString(this->_display->getWidth() / 2, (this->_display->getHeight() / 4) * 3 - 5, "Punti: " + String(points));
    this->_display->display();
}