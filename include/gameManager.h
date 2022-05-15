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

public:
    GameManager();
    void loop();
};