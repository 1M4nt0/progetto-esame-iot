#include <device.h>
#include <messageCodes.h>

Device::Device(bool isHost)
{
    pinMode(BUTTON_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    this->_deviceSocket = new DeviceSocket(&isHost);
    this->setDefaultHandlers();
    if (isHost)
    {
        this->_deviceID = 1;
    }
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

void Device::setDefaultHandlers()
{
    this->_deviceSocket->on(T_BIN_MASSAGE, [&](uint8_t from, uint8_t *payload, int len)
                            { 
                                if (payload[0] == C_LIGHTS_ON) {
                                      this->setLight(true);
                                  }else if(payload[0] == C_LIGHTS_OFF){
                                      this->setLight(false);
                                  }else if(payload[0] == C_DEVICE_ID){
                                      this->_deviceID = payload[1];
                                  } });
}