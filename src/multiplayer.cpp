#include <multiplayer.h>

Multiplayer::Multiplayer(Device *device) : Game(device)
{
    if (this->device->isHost())
    {
        this->_initPlayerIDMap();
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
            }
        }
        case C_TIME:
        {
            if (this->device->isHost())
            {
                unsigned short time;
                uint8_t playerID = this->_playerDevice.find(from)->first;
                memcpy(&time, message->payload, sizeof(short));
                this->_manageWinnerTime(playerID, time);
            }
        }
        default:
            break;
        }; });
}

void Multiplayer::loop()
{
    if (this->device->isHost() && millis() > this->_matchStartTime + MATCH_DURATION)
    {
        this->end();
        delay(random(2000, 8000));
        this->start();
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
    Winner.id = 0;
    Winner.time = SHRT_MAX;
    this->_playerButtonPressDelay.clear();
}

void Multiplayer::start()
{
    this->device->sendSwitchLightOn();
    this->_matchStartTime = millis();
    this->device->setLightOn(true);
}

void Multiplayer::end()
{
    this->device->sendSwitchLightOff();
    this->device->setLightOn(false);
    if (Winner.id > 0)
    {
        this->sendWinner(Winner.id);
        // checkResults(Winner.id);
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

void Multiplayer::_addPlayer(uint8_t deviceID)
{
    for (uint8_t id = 1; id <= MAX_PLAYERS; id++)
    {
        if (_playerDevice[id] == 255)
        {
            _playerDevice[id] = deviceID;
            this->device->socket()->sendMessage(deviceID, C_PLAYER_ID, &id, sizeof(id));
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
}