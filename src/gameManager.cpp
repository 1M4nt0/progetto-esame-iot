#include <gameManager.h>

GameManager::GameManager()
{
    this->_device = new Device();
    this->_initGame(0);
    this->_initServerEndpoints();
    this->_device->socket()->on(WSHM_BIN, [&](WSH_Message msgType, uint8_t from, SocketDataMessage *message)
                                {
        switch (message->code)
        {
        case C_PAUSE:
        {
            if (!this->_device->isHost())
            {
                bool newPause = message->payload[0];
                this->_setPause(pause);
            }
        }
        case C_NEW_GAMEMODE:
        {
            if (this->_device->isHost())
            {
                uint8_t newGameID = message->payload[0];
                this->_initGame(newGameID);
            }
        }
        default:
            break;
        }; });
}

void GameManager::_initGame(uint8_t gameID)
{
    drawToScreen("Cambio gioco...");
    if (gameID == 0)
    {
        _game = new Multiplayer(this->_device);
    }
    else
    {
        _game = new Multiplayer(this->_device);
    }
    this->_gameID = gameID;
    this->_setPause(false);
}

void GameManager::_setPause(bool isPaused)
{
    if (this->_device->isHost())
    {
        this->_game->end();
        this->_sendIsPaused(isPaused);
        this->_isPaused = isPaused;
        if (this->_isPaused)
        {
            drawTwoToScreen("Gioco " + String(this->_gameID), "in pausa...");
        }
        else
        {
            delay(2000);
            this->_game->initalize();
            this->_game->start();
        }
    }
    else
    {
        if (this->_isPaused)
        {
            drawTwoToScreen("Gioco " + String(this->_gameID), "in pausa...");
        }
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

void GameManager::_sendIsPaused(bool isPaused)
{
    uint8_t u_paused = (uint8_t)isPaused;
    if (this->_device->isHost())
    {
        this->_device->socket()->sendMessageAll(C_PAUSE, &u_paused, sizeof(bool));
    }
}

void GameManager::_sendChangeGame(uint8_t newGameID)
{
    if (this->_device->isHost())
    {
        this->_device->socket()->sendMessageAll(C_PAUSE, &newGameID, sizeof(uint8_t));
    }
}

void GameManager::_initServerEndpoints()
{
    this->_device->webServer()->on("/pause", HTTP_GET, [=](AsyncWebServerRequest *request)
                                   { 
                if(request->hasParam("pause")){
                bool pause = request->getParam("pause")->value().toInt();
                if(pause != this->_isPaused){
                    request->send(200, "text", "OK!");
                    this->_setPause(pause);
                }else{
                    request->send(200, "text", "NOT OK!");
                }}else{
                    request->send(200, "text", String(this->_isPaused));
                } });
    this->_device->webServer()->on("/gamemode", HTTP_GET, [=](AsyncWebServerRequest *request)
                                   { 
                if(request->hasParam("id")){
                int newGameID = request->getParam("id")->value().toInt();
                if(newGameID != this->_gameID){
                    request->send(200, "text", "OK!");
                    drawToScreen("Cambio gioco...");
                    this->_sendChangeGame(newGameID);
                    this->_initGame(newGameID);                   
                }else{
                    request->send(200, "text", "NOT OK!");
                }}else{
                    request->send(200, "text", String(this->_gameID));
                } });
}