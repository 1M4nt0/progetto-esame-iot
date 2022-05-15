#include <gameManager.h>

GameManager::GameManager()
{
    this->_device = new Device();
    this->_gameID = 0;
    this->_initGame(_gameID);
}

void GameManager::_initGame(uint8_t gameID)
{
    if (gameID == 0)
    {
        _game = new Multiplayer(this->_device);
    }
    else
    {
        _game = new Multiplayer(this->_device);
    }
    this->_setPause(false);
}

void GameManager::_setPause(bool isPaused)
{
    this->_isPaused = isPaused;
    if (this->_isPaused)
    {
        drawTwoToScreen("Gioco " + String(this->_gameID), "in pausa...");
    }
    else
    {
        this->_game->initalize();
        this->_game->start();
    }
}

void GameManager::loop()
{
    this->_device->loop();
    if (!this->_isPaused)
    {
        _game->loop();
    }
}