#include <game.h>

Game::Game(Device *device)
{
    this->device = device;
    this->device->socket()->on(DSM_GAME, WSHM_BIN, [&](WSH_Message msgType, uint8_t from, SocketDataMessage *message)
                               {
        switch (message->code)
        {
        case C_LIGHTS_ON:
            this->onLightOn();
            break;
        case C_LIGHTS_OFF:
            this->onLightOff();
            break; 
        default:
            break;
        }; });
}

Game::~Game()
{
    this->device->socket()->clearGameHandlers();
}

void Game::incrementPlayerPoints(uint8_t playerID, int increment)
{
    _playerPoints[playerID] = _playerPoints[playerID] + increment;
}

int Game::getPlayerPoints(uint8_t playerID)
{
    return _playerPoints[playerID];
}

void Game::sendWinner(uint8_t winnerID)
{
    this->device->socket()->sendMessageAll(C_WINNER, &winnerID, sizeof(winnerID));
}

void Game::resetPoints()
{
    this->_playerPoints.clear();
}