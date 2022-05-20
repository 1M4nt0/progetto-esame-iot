#include <multiplayerHost.h>

MultiplayerHost::MultiplayerHost(Device *device) : Game(device)
{
    this->_initPlayerIDMap();
    this->_playerID = 1;
    this->_canStart = true;
    this->_nextRestartTime = millis() + 5000; // Due secondo e incomincia il gioco
    this->device->socket()->on(WSHE_SOCKET_DISCONNECTED, [&](WSH_Event event, uint8_t from)
                               { this->_deletePlayer(from); });
    this->device->socket()->on(WSHM_BIN, [&](WSH_Message msgType, uint8_t from, SocketDataMessage *message)
                               {
                                   Serial.printf("Message from: %i\ncode: %i\n\n", from,message->code);
        switch (message->code)
        {
        case C_TIME:
        {
            unsigned short time;
            uint8_t playerID = this->_findPlayerIDbyDeviceID(from);
            memcpy(&time, message->payload, sizeof(short));
            this->_playerButtonPressDelay[playerID] = time;
            this->_manageWinnerTime(playerID, time);
            break;
        }
        case C_READY_TO_PLAY:
        {
            this->_addPlayer(from);
            break;
        }  
        default:
            break;
        }; });
    this->device->webServer()->on("/", HTTP_ANY, [](AsyncWebServerRequest *request)
                                  { request->send(SPIFFS, "/index.html"); });
    this->device->webServer()->on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
                                  { request->send(SPIFFS, "/style.css", "text/css"); });
    this->device->webServer()->on("/functions.js", HTTP_GET, [](AsyncWebServerRequest *request)
                                  { request->send(SPIFFS, "/functions.js", "text/js"); });
    this->device->webServer()->on("/points", HTTP_GET, [&](AsyncWebServerRequest *request)
                                  { 
              AsyncJsonResponse* response = new AsyncJsonResponse();
              const JsonObject& jsonData = response->getRoot();
              int index = 0;
              for (auto player = this->_playerDevice.begin(); player != this->_playerDevice.end(); player++)
              {
                if(player->second != 255){
                    jsonData["players"][index]["id"] = player->first;
                    jsonData["players"][index]["points"] = this->getPlayerPoints(player->first);
                    jsonData["players"][index]["time"] = this->_playerButtonPressDelay[player->first];
                    index++;
                }  
              }
              response->setLength();
              request->send(response); });
}

void MultiplayerHost::loop()
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
    if (this->device->isLightOn())
    {
        if (this->device->isButtonPressed())
        {

            this->onButtonPressed();
        }
    }
}

void MultiplayerHost::initalize()
{
    Winner.id = 0;
    Winner.time = SHRT_MAX;
    this->_playerButtonPressDelay.clear();
    drawDashboard(this->_playerID, this->getPlayerPoints(this->_playerID));
}

void MultiplayerHost::start()
{
    this->device->socket()->sendMessageAll(C_LIGHTS_ON);
    this->_matchStartTime = millis();
    this->device->setLightOn(true);
    this->_lightOnTime = millis();
    this->_canStart = false;
}

void MultiplayerHost::end()
{
    this->device->socket()->sendMessageAll(C_LIGHTS_OFF);
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

void MultiplayerHost::_initPlayerIDMap()
{
    _playerDevice.clear();
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

uint8_t MultiplayerHost::_findPlayerIDbyDeviceID(uint8_t deviceID)
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

void MultiplayerHost::_addPlayer(uint8_t deviceID)
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

void MultiplayerHost::_deletePlayer(uint8_t deviceID)
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

void MultiplayerHost::onButtonPressed()
{
    unsigned long timeButtonPressed = millis();
    this->device->setLightOn(false);
    unsigned short buttonPressDelay = timeButtonPressed - this->_lightOnTime;
    this->_playerButtonPressDelay[this->_playerID] = buttonPressDelay;
    this->_manageWinnerTime(1, buttonPressDelay);
}

void MultiplayerHost::_manageWinnerTime(uint8_t playerID, unsigned short buttonPressDelay)
{
    if (this->Winner.time > buttonPressDelay)
    {
        this->Winner.time = buttonPressDelay;
        this->Winner.id = playerID;
    }
}

void MultiplayerHost::_displayResults(bool isWinner)
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