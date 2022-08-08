#pragma once
#include <game.h>

class MainMenu : public Game
{
public:
    MainMenu(Device *device) : Game(device){};
    virtual void loop() override{};
    virtual void start() override{};
    virtual void initalize() override{};
    virtual void end() override{};
    virtual void servePointsEndpoint(AsyncWebServerRequest *request) override{};

protected:
    virtual void onLightOn() override{};
    virtual void onLightOff() override{};
    virtual void onNewDeviceConnected() override{};
    virtual void onDeviceDisconnected() override{};
    virtual void configServerEndpoints() override{};
    virtual void onButtonPressed() override{};
};