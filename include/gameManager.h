#pragma once
#include <device.h>
#include <multiplayerHost.h>
#include <multiplayerClient.h>
#include <singleplayerHost.h>
#include <singleplayerClient.h>
#include <mainMenu.h>

class GameManager
{
private:
    Device *_device{nullptr};
    Game *_game{nullptr};
    bool _isPaused;
    uint8_t _gameID;
    void _initGame(uint8_t gameID);
    void _setPause(bool isPaused);
    void _initServerEndpoints();
    void _sendPause(bool isPaused);
    void _sendIsPaused(bool isPaused);
    void _sendIsPaused(uint8_t deviceID, bool isPaused);
    void _sendChangeGame(uint8_t newGameID);
    void _sendChangeGame(uint8_t deviceID, uint8_t newGameID);
    void _sendIsReadyToHost();
    void _handlePauseEndpointRequest(AsyncWebServerRequest *request);
    void _handleGamemodeEndpointRequest(AsyncWebServerRequest *request);
    void _handlePointsEndpointRequest(AsyncWebServerRequest *request);
    void _handleResetPointsEndpointRequest(AsyncWebServerRequest *request);

public:
    GameManager();
    ~GameManager();
    void loop();
};