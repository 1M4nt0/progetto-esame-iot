#pragma once
#include <game.h>
#include <screen_utils.h>

#define MAX_NUMBER_OF_ATTEMPTS 10
#define MAX_LIGHT_ON_TIME 3000

class Singleplayer : public Game
{
private:
    uint8_t deviceWithLightON = 1;
    uint8_t numberOfAttempts = 0;
    long timeSinceLastDeviceLightOn = 0;
    uint8_t currentPlayer = 1;
    uint8_t numberOfPlayers = 1;
    std::map<uint8_t, short[MAX_NUMBER_OF_ATTEMPTS]> playerResponseDelays;
    struct
    {
        uint8_t id = 0;
        unsigned short meanTime = SHRT_MAX;
    } Winner;

protected:
    void startGame() override;
    void endGame() override;
    void gameLoop() override;
    void onTimeRecieved(uint8_t deviceID, short time) override;
    void onButtonPressed() override;
    void onSwitchLightOn(){};
    void onSwitchLightOff(){};
    void onDeviceIDRecieved(){};
    void onWinnerResultsRecieved(uint8_t winnerID){};
    void onNewDeviceConnected(uint8_t deviceID){};
    void onDeviceDisconnected(uint8_t deviceID){};
    void setDeviceButtonPressDelay(uint8_t deviceID, short time);
    short getDeviceButtonPressDelay(uint8_t deviceID);
    void resetButtonPressDelay();
    void configServerEndpoints(AsyncWebServer *_server) override;
    void checkIfCurrentWinner(uint8_t playerID);
    void managePressingTime(uint8_t deviceID, short time);

public:
    Singleplayer(AsyncWebServer *_server, std::vector<uint8_t> *connectedDevicesID) : Game(_server, connectedDevicesID){};
};