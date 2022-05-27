#pragma once
#include <game.h>

#define MAX_NUMBER_OF_ATTEMPTS 5
#define MAX_LIGHT_ON_TIME 3000

class SingleplayerHost : public Game
{
public:
    SingleplayerHost(Device *device);
    virtual void loop() override;
    virtual void start() override;
    virtual void initalize() override;
    virtual void end() override;
    virtual void servePointsEndpoint(AsyncWebServerRequest *request) override{};

protected:
    virtual void onLightOn() override{};
    virtual void onLightOff() override{};
    virtual void onNewDeviceConnected() override{};
    virtual void onDeviceDisconnected() override{};
    virtual void configServerEndpoints() override{};
    virtual void onButtonPressed() override;

private:
    uint8_t _deviceWithLightON = 1;
    uint8_t _numberOfAttempts = 0;
    long _timeSinceLastDeviceLightOn = 0;
    uint8_t _currentPlayer = 1;
    uint8_t _numberOfPlayers = 1;
    std::map<uint8_t, short[MAX_NUMBER_OF_ATTEMPTS]> _playerButtonPressDelays;
    void _initPlayerButtonDelaysVector(uint8_t playerID);
    struct
    {
        uint8_t id = 0;
        unsigned short meanTime = SHRT_MAX;
    } Winner;
    void _checkIfCurrentWinner(uint8_t playerID);
    bool _canRestart;
    unsigned long _nextRestartTime;
    unsigned long _lightOnTime;
    short _arrayTimeMean(short *timeArray, int length);
    void _startAttempt();
    void _endAttempt();
    bool _byAttempting;
    unsigned long _nextAttemptDelay;
};