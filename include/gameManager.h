#pragma once
#include <device.h>
#include <multiplayer.h>

class GameManager
{
private:
    Device *_device;
    Game *_game;
    bool _isPaused;
    uint8_t _gameID;
    void _initGame(uint8_t gameID);
    void _setPause(bool isPaused);
    void _initServerEndpoints();
    void _sendPause(bool isPaused);
    void _sendIsPaused(bool isPaused);
    void _sendChangeGame(uint8_t newGameID);

public:
    GameManager();
    void loop();
};