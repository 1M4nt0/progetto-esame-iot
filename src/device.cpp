#include <device.h>

Device::Device(bool isHost)
{
    pinMode(BUTTON_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    this->_deviceSocket = new DeviceSocket(&isHost);
}

void Device::setDeviceID(uint8_t deviceID)
{
    this->_deviceID = deviceID;
}

uint8_t Device::getDeviceID()
{
    return this->_deviceID;
}

void Device::setLight(bool on)
{
    this->_lightON = on;
    digitalWrite(LED_PIN, (on) ? HIGH : LOW);
}

void Device::loop()
{
    if (digitalRead(BUTTON_PIN) == LOW)
    {
        this->_buttonPressed = true;
    }
    else
    {
        this->_buttonPressed = false;
    }
}

DeviceSocket *Device::socket()
{
    return this->_deviceSocket;
}
