#pragma once
#include <game.h>
#include <screen_utils.h>

class Multiplayer : public Game
{
private:
    unsigned long matchStartTime = 0;
    unsigned short matchDuration = 10000;
    struct
    {
        uint8_t id = 0;
        unsigned short time = SHRT_MAX;
    } Winner;
    void manageCurrentWinner(uint8_t id, unsigned short pressingTime);
    std::map<uint8_t, short> deviceButtonPressDelay;

protected:
    void checkResults(uint8_t winnerID);
    void startGame() override;
    void endGame() override;
    void gameLoop() override;
    void onTimeRecieved(uint8_t deviceID, short time) override;
    void onButtonPressed() override;
    void onSwitchLightOn(){};
    void onSwitchLightOff(){};
    void onDeviceIDRecieved() override;
    void onWinnerResultsRecieved(uint8_t winnerID) override;
    void onNewDeviceConnected(uint8_t deviceID) override;
    void onDeviceDisconnected(uint8_t deviceID){};
    void setDeviceButtonPressDelay(uint8_t deviceID, short time);
    short getDeviceButtonPressDelay(uint8_t deviceID);
    void resetButtonPressDelay();
    void configServerEndpoints(AsyncWebServer *_server) override;

public:
    Multiplayer(AsyncWebServer *_server, std::vector<uint8_t> *connectedDevicesID) : Game(_server, connectedDevicesID){};
};