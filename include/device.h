#pragma once
#include <stdint.h>
#include <Arduino.h>
#include <deviceSocket.h>

#define BUTTON_PIN 16
#define LED_PIN 17

class Device
{
protected:
    void setDefaultHandlers();

public:
    Device(bool isHost);
    void setDeviceID(uint8_t deviceID);
    uint8_t getDeviceID();
    void setLight(bool on);
    void loop();
    DeviceSocket *socket();

private:
    uint8_t _deviceID;
    bool _lightON;
    bool _buttonPressed;
    DeviceSocket *_deviceSocket;
};