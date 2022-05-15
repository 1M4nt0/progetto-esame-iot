#include <multiplayer.h>

Multiplayer::Multiplayer(Device *device) : Game(device)
{
    if (this->device->isHost())
    {
        this->_initPlayerIDMap();
        this->_playerID = 1;
        this->_canStart = true;
        this->_nextRestartTime = millis() + 5000; // Due secondo e incomincia il gioco
        this->device->socket()->on(WSHM_CONNECTED, [&](WSH_Message msgType, uint8_t from, SocketDataMessage *message)
                                   { _addPlayer(from); });
        this->device->socket()->on(WSHM_DISCONNECTED, [&](WSH_Message msgType, uint8_t from, SocketDataMessage *message)
                                   { _deletePlayer(from); });
    }
    this->device->socket()->on(WSHM_BIN, [&](WSH_Message msgType, uint8_t from, SocketDataMessage *message)
                               {
        switch (message->code)
        {
        case C_PLAYER_ID:
        {
            if (!this->device->isHost())
            {
                uint8_t playerID = message->payload[0];
                this->_setPlayerID(playerID);
                drawDashboard(this->_playerID, this->getPlayerPoints(this->_playerID));
            }
        }
        case C_TIME:
        {
            if (this->device->isHost())
            {
                unsigned short time;
                uint8_t playerID = this->_findPlayerIDbyDeviceID(from);
                memcpy(&time, message->payload, sizeof(short));
                this->_manageWinnerTime(playerID, time);
            }
        }
        case C_WINNER:
        {
            if(!this->device->isHost() && this->_playerID > 0){
                Serial.printf("Payload: %i, player: %i, from %i", message->payload[0], this->_playerID, from);
                this->_displayResults(message->payload[0] == this->_playerID);
            }
        }
        default:
            break;
        }; });
}

void Multiplayer::loop()
{
    if (this->device->isHost())
    {
        if (this->_canStart && millis() > this->_nextRestartTime)
        {
            this->initalize();
            this->start();
        }
        if (millis() > this->_matchStartTime + MATCH_DURATION && this->_canStart == false)
        {
            this->end();
        }
    }
    if (this->device->isLightOn())
    {
        if (this->device->isButtonPressed())
        {
            this->onButtonPressed();
        }
    }
}

void Multiplayer::initalize()
{
    if (this->device->isHost())
    {
        Winner.id = 0;
        Winner.time = SHRT_MAX;
        this->_playerButtonPressDelay.clear();
    }
    if (this->_playerID > 0)
    {
        drawDashboard(this->_playerID, this->getPlayerPoints(this->_playerID));
    }
}

void Multiplayer::start()
{
    if (this->device->isHost())
    {
        this->device->sendSwitchLightOn();
        this->_matchStartTime = millis();
        this->device->setLightOn(true);
        this->_lightOnTime = millis();
        this->_canStart = false;
    }
}

void Multiplayer::end()
{
    if (this->device->isHost())
    {
        this->device->sendSwitchLightOff();
        this->device->setLightOn(false);
        if (Winner.id > 0)
        {
            this->sendWinner(Winner.id);
            this->incrementPlayerPoints(Winner.id, 1);
            this->_displayResults(Winner.id == this->_playerID);
            Serial.printf("Vincitore: Giocatore %i\n", Winner.id);
        }
        this->_nextRestartTime = millis() + random(2000, 8000);
        this->_canStart = true;
    }
}

void Multiplayer::_initPlayerIDMap()
{
    _playerDevice[1] = 0;
    uint8_t playerID = 2;
    for (uint8_t connectedDevice : this->device->socket()->getConnectedDevicesIDVector())
    {
        _playerDevice[playerID] = connectedDevice;
        playerID = playerID + 1;
    }
    while (playerID != MAX_PLAYERS)
    {
        _playerDevice[playerID] = 255;
        playerID = playerID + 1;
    }
}

uint8_t Multiplayer::_findPlayerIDbyDeviceID(uint8_t deviceID)
{
    for (auto player = this->_playerDevice.begin(); player != this->_playerDevice.end(); player++)
    {
        if (player->second == deviceID)
        {
            return player->first;
        }
    }
    return 255;
}

void Multiplayer::_addPlayer(uint8_t deviceID)
{
    for (uint8_t id = 1; id <= MAX_PLAYERS; id++)
    {
        if (_playerDevice[id] == 255)
        {
            _playerDevice[id] = deviceID;
            this->device->socket()->sendMessage(deviceID, C_PLAYER_ID, &id, sizeof(uint8_t));
            Serial.printf("Connected device %i with playerID %i\n", deviceID, id);
            break;
        }
    }
}

void Multiplayer::_deletePlayer(uint8_t deviceID)
{
    for (uint8_t id = 1; id <= MAX_PLAYERS; id++)
    {
        if (_playerDevice[id] == deviceID)
        {
            _playerDevice[id] = 255;
            _playerPoints[id] == 0;
            Serial.printf("Disconnected device %i with playerID %i\n", deviceID, id);
            break;
        }
    }
}

void Multiplayer::_sendButtonPressDelay(unsigned short buttonPressDelay)
{
    uint8_t payload[2];
    memcpy(payload, &buttonPressDelay, 2);
    this->device->socket()->sendMessage(0, C_TIME, payload, sizeof(short));
}

void Multiplayer::onButtonPressed()
{
    unsigned long timeButtonPressed = millis();
    this->device->setLightOn(false);
    unsigned short buttonPressDelay = timeButtonPressed - this->_lightOnTime;
    if (this->device->isHost())
    {
        this->_playerButtonPressDelay[this->_playerID] = buttonPressDelay;
        this->_manageWinnerTime(1, buttonPressDelay);
    }
    else
    {
        this->_sendButtonPressDelay(buttonPressDelay);
    }
}

void Multiplayer::_manageWinnerTime(uint8_t playerID, unsigned short buttonPressDelay)
{
    if (this->Winner.time > buttonPressDelay)
    {
        this->Winner.time = buttonPressDelay;
        this->Winner.id = playerID;
    }
}

void Multiplayer::onLightOn()
{
    this->_lightOnTime = millis();
    drawDashboard(this->_playerID, this->getPlayerPoints(this->_playerID));
}

void Multiplayer::_displayResults(bool isWinner)
{
    if (isWinner)
    {
        drawToScreen("Hai vinto!");
    }
    else
    {
        drawToScreen("Hai perso!");
    }
}