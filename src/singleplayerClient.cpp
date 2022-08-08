#include <singleplayerClient.h>

void SingleplayerClient::initalize()
{
    this->device->display()->drawToScreen("In gioco...");
}

void SingleplayerClient::loop()
{
    if (this->device->isLightOn())
    {
        if (this->device->isButtonPressed())
        {
            this->onButtonPressed();
        }
    }
}

void SingleplayerClient::onButtonPressed()
{
    this->device->setLightOn(false);
    unsigned short buttonPressDelay = millis() - this->_lightOnTime;
    this->_sendButtonPressDelay(buttonPressDelay);
}

void SingleplayerClient::_sendButtonPressDelay(unsigned short buttonPressDelay)
{
    uint8_t payload[2];
    memcpy(payload, &buttonPressDelay, 2);
    this->device->socket()->sendMessage(0, C_TIME, payload, sizeof(short));
}

void SingleplayerClient::onLightOn()
{
    this->_lightOnTime = millis();
}