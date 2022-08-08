#pragma once
#include <stdint.h>
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <messageCodes.h>
#include <socketHost.h>
#include <socketClient.h>
#include "WiFi.h"
#include <deviceDisplay.h>

#define BUTTON_PIN 16
#define LED_PIN 17

class Device
{
protected:
    void setDefaultHandlers();

public:
    Device();
    ~Device();
    void setDeviceID(uint8_t deviceID);
    uint8_t getDeviceID();
    void setLightOn(bool on);
    void loop();
    DeviceSocket *socket();
    DeviceDisplay *display();
    AsyncWebServer *webServer();
    bool isHost() { return this->_isHost; };
    bool isButtonPressed() { return this->_buttonPressed; };
    bool isLightOn() { return this->_lightON; };
    void connectToWifi();

private:
    uint8_t _deviceID;
    bool _lightON;
    bool _buttonPressed;
    bool _isHost;
    DeviceSocket *_deviceSocket{nullptr};
    DeviceDisplay *_deviceDisplay{nullptr};
    void initOTA();
};