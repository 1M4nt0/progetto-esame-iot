#pragma once
#include <map>
#include <stdint.h>
#include <device.h>
#include <messageCodes.h>

#define MAX_PLAYERS 8

class Game
{
public:
    virtual void loop() = 0;
    virtual void start() = 0;
    virtual void initalize() = 0;
    virtual void end() = 0;
    Game(Device *device);

protected:
    virtual void onLightOn() = 0;
    virtual void onLightOff() = 0;
    virtual void onNewDeviceConnected() = 0;
    virtual void onDeviceDisconnected() = 0;
    virtual void configServerEndpoints() = 0;
    virtual void onButtonPressed() = 0;
    Device *device;
    std::map<uint8_t, int> _playerPoints;
    int getPlayerPoints(uint8_t playerID);
    void incrementPlayerPoints(uint8_t playerID, int increment);
    void sendWinner(uint8_t winnerID);

private:
};