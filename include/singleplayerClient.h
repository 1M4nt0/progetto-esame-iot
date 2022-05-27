#pragma once
#include <game.h>

#define MAX_NUMBER_OF_ATTEMPTS 5
#define MAX_LIGHT_ON_TIME 3000

class SingleplayerClient : public Game
{
public:
    SingleplayerClient(Device *device);
    virtual void loop() override;
    virtual void start() override{};
    virtual void initalize() override{};
    virtual void end() override{};
    virtual void servePointsEndpoint(AsyncWebServerRequest *request) override{};

protected:
    virtual void onLightOn() override;
    virtual void onLightOff() override{};
    virtual void onNewDeviceConnected() override{};
    virtual void onDeviceDisconnected() override{};
    virtual void configServerEndpoints() override{};
    virtual void onButtonPressed() override;

private:
    unsigned long _lightOnTime;
    void _sendButtonPressDelay(unsigned short buttonPressDelay);
};