#pragma once
#include <game.h>
#include <screen_utils.h>

class Multiplayer : public Game
{
private:
    unsigned long matchStartTime = 0;
    unsigned short matchDuration = 10 * (10 ^ 3);
    std::map<uint8_t, uint8_t> playerID;
    std::map<uint8_t, int> playerPoints; // Device ID to playerID
    struct
    {
        uint8_t id = 0;
        unsigned short time = SHRT_MAX;
    } Winner;
    void manageCurrentWinner(uint8_t id, unsigned short pressingTime);

protected:
    uint8_t getPlayerID();
    int getPlayerPoints();
    void incrementPlayerPoints(uint8_t playerID, int increment);
    uint8_t getPlayerIDFromDevice(uint8_t deviceID);
    void checkResults(uint8_t winnerID);
    void startGame();
    void endGame();
    void gameLoop();
    void onTimeRecieved(uint8_t deviceID, short time);
    void onButtonPressed();
    void onSwitchLightOnRecived();
    void onSwitchLightOffRecived();
    void onDeviceIDRecieved();
    void onWinnerResultsRecieved(uint8_t winnerID);
    void onNewDeviceConnected(uint8_t deviceID);

public:
};