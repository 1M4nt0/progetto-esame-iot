#pragma once
#include <stdint.h>
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <deviceSocket.h>
#include <screen_utils.h>
#include <messageCodes.h>

#define BUTTON_PIN 16
#define LED_PIN 17

class Device
{
protected:
    void setDefaultHandlers();

public:
    Device();
    void setDeviceID(uint8_t deviceID);
    uint8_t getDeviceID();
    void setLightOn(bool on);
    void loop();
    DeviceSocket *socket();
    AsyncWebServer *webServer();
    bool isHost();
    void sendSwitchLightOn();
    void sendSwitchLightOn(uint8_t deviceID);
    void sendSwitchLightOff();
    void sendSwitchLightOff(uint8_t deviceID);
    bool isButtonPressed() { return this->_buttonPressed; };
    bool isLightOn() { return this->_lightON; };

private:
    uint8_t _deviceID;
    bool _lightON;
    bool _buttonPressed;
    DeviceSocket *_deviceSocket;
    void initOTA();
};