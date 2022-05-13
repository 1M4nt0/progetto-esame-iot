#include <game.h>

Game::Game(Device *device)
{
    this->device = device;
    this->device->socket()->on(WSHM_BIN, [&](WSH_Message msgType, uint8_t from, SocketDataMessage *message)
                               {
        switch (message->code)
        {
        case C_LIGHTS_ON:
            this->onLightOn();
            break;
        case C_LIGHTS_OFF:
            this->onLightOff();
            break;
        case C_DEVICE_ID:
        {
            if(!this->device->isHost()){
                this->onPlayerIDRecieved();
            }
        }     
        default:
            break;
        }; });
    this->device->socket()->on(WSHM_CONNECTED, [&](WSH_Message msgType, uint8_t from, SocketDataMessage *message)
                               { this->sendPlayerID(from); });
    this->device->socket()->on(WSHM_DISCONNECTED, [&](WSH_Message msgType, uint8_t from, SocketDataMessage *message)
                               { this->deletePlayer(from); });
}

void Game::sendPlayerID(uint8_t deviceID)
{
    uint8_t id = 1;
    for (auto player = _playerDevice.begin(); player != _playerDevice.end(); player++)
    {
        if (player->second == 0)
        {
            this->device->socket()->sendMessage(deviceID, C_DEVICE_ID, &id, sizeof(id));
        }
        id = id + 1;
    }
}

void Game::deletePlayer(uint8_t deviceID)
{
    for (auto player = _playerDevice.begin(); player != _playerDevice.end(); player++)
    {
        if (player->second == deviceID)
        {
            player->second = 0;
        }
    }
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