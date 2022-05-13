#pragma once
#include <map>
#include <stdint.h>
#include <device.h>
#include <messageCodes.h>

class Game
{
public:
    virtual void loop() = 0;
    virtual void start() = 0;
    virtual void initalize() = 0;
    virtual void stop() = 0;

protected:
    Game(Device *device);
    Device *device;
    std::map<uint8_t, int> _playerPoints;
    std::map<uint8_t, uint8_t> _playerDevice;
    virtual void onLightOn() = 0;
    virtual void onLightOff() = 0;
    virtual void onNewDeviceConnected() = 0;
    virtual void onDeviceDisconnected() = 0;
    virtual void onPlayerIDRecieved() = 0;
    virtual void configServerEndpoints() = 0;
    virtual void onMessageRecieved() = 0;
    int getPlayerPoints(uint8_t playerID);
    void incrementPlayerPoints(uint8_t playerID, int increment);
    void sendWinner(uint8_t winnerID);

private:
    void sendPlayerID(uint8_t deviceID);
    void deletePlayer(uint8_t deviceID);
};