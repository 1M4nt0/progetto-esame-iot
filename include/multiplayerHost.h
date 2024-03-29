#pragma once
#include <game.h>

#define MATCH_DURATION 5000

class MultiplayerHost : public Game
{
public:
    MultiplayerHost(Device *device);
    virtual void loop() override;
    virtual void start() override;
    virtual void initalize() override;
    virtual void end() override;
    virtual void servePointsEndpoint(AsyncWebServerRequest *request) override;

private:
    void _manageWinnerTime(uint8_t playerID, unsigned short buttonPressDelay);
    void _addPlayer(uint8_t deviceID);
    void _deletePlayer(uint8_t deviceID);
    uint8_t _getPlayerID() { return this->_playerID; };
    void _setPlayerID(uint8_t playerID) { this->_playerID = playerID; };
    void _initPlayerIDMap();
    void _displayResults(bool isWinner);
    uint8_t _findPlayerIDbyDeviceID(uint8_t deviceID);
    std::map<uint8_t, uint8_t> _playerDevice;
    unsigned long _matchStartTime;
    unsigned long _lightOnTime;
    uint8_t _playerID;
    std::map<uint8_t, short> _playerButtonPressDelay;
    struct
    {
        uint8_t id = 0;
        unsigned short time = SHRT_MAX;
    } Winner;
    bool _canStart;
    unsigned long _nextRestartTime;

protected:
    virtual void onLightOn() override{};
    virtual void onLightOff() override{};
    virtual void onNewDeviceConnected() override{};
    virtual void onDeviceDisconnected() override{};
    virtual void configServerEndpoints() override{};
    virtual void onButtonPressed() override;
};