#include <gameManager.h>

GameManager::GameManager()
{
    this->_device = new Device();
    this->_game = nullptr;
    if (this->_device->isHost())
    {
        this->_initGame(0);
        this->_initServerEndpoints();
    }
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
            break;
        }
        case C_NEW_GAMEMODE:
        {
            if (!this->_device->isHost()){
                this->_initGame(message->payload[0]);
            }
            break;
        }
        default:
            break;
        }; });
    this->_device->socket()->on(WSHE_WIFI_DISCONNECTED, [&](WSH_Event event)
                                { this->_initGame(this->_gameID); });
    if (this->_device->isHost())
    {
        this->_device->socket()->on(WSHE_SOCKET_CONNECTED, [&](WSH_Event event, uint8_t from)
                                    { this->_sendChangeGame(from, this->_gameID); });
    }
}

GameManager::~GameManager()
{
    delete this->_game;
    delete this->_device;
}

void GameManager::_initGame(uint8_t gameID)
{
    drawToScreen("Avvio gioco...");
    if (this->_game != nullptr)
    {
        delete this->_game;
    }

    if (gameID == 0)
    {
        if (this->_device->isHost())
        {
            this->_game = new MultiplayerHost(this->_device);
        }
        else
        {
            this->_game = new MultiplayerClient(this->_device);
        }
    }
    else
    {
        if (this->_device->isHost())
        {
            this->_game = new MultiplayerHost(this->_device);
        }
        else
        {
            this->_game = new MultiplayerClient(this->_device);
        }
    }
    this->_gameID = gameID;
    this->_setPause(false);
    if (!this->_device->isHost())
    {
        this->_sendIsReadyToHost();
    }
}

void GameManager::_setPause(bool isPaused)
{
    this->_game->end();
    if (this->_device->isHost())
    {
        this->_sendIsPaused(isPaused);
    }
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

void GameManager::_sendIsPaused(bool isPaused)
{
    uint8_t u_paused = (uint8_t)isPaused;
    if (this->_device->isHost())
    {
        this->_device->socket()->sendMessageAll(C_PAUSE, &u_paused, sizeof(bool));
    }
}

void GameManager::_sendIsReadyToHost()
{
    this->_device->socket()->sendMessage(0, C_READY_TO_PLAY);
}

void GameManager::_sendChangeGame(uint8_t newGameID)
{
    this->_device->socket()->sendMessageAll(C_NEW_GAMEMODE, &newGameID, sizeof(uint8_t));
}

void GameManager::_sendChangeGame(uint8_t deviceID, uint8_t newGameID)
{
    this->_device->socket()->sendMessage(deviceID, C_NEW_GAMEMODE, &newGameID, sizeof(uint8_t));
}

void GameManager::_initServerEndpoints()
{
    this->_device->webServer()->on("/pause", HTTP_GET, [&](AsyncWebServerRequest *request)
                                   { 
                if(request->hasParam("pause")){
                bool pause = request->getParam("pause")->value().toInt();
                if(pause != this->_isPaused){
                    request->send(200, "text", "OK!");
                    this->_setPause(pause);
                }else{
                    request->send(200, "text", "NOT OK!");
                }}else{
                    AsyncJsonResponse* response = new AsyncJsonResponse();
                    const JsonObject& jsonData = response->getRoot();
                    jsonData["pause"] = this->_isPaused;
                    response->setLength();
                    request->send(response);
                } });
    this->_device->webServer()->on("/gamemode", HTTP_GET, [&](AsyncWebServerRequest *request)
                                   { 
                if(request->hasParam("id")){
                int newGameID = request->getParam("id")->value().toInt();
                if(newGameID != this->_gameID){
                    request->send(200, "text", "OK!");
                    drawToScreen("Cambio gioco...");
                    this->_initGame(newGameID); 
                    this->_sendChangeGame(newGameID);                
                }else{
                    request->send(403, "text", "NOT OK!");
                }}else{
                    AsyncJsonResponse* response = new AsyncJsonResponse();
                    const JsonObject& jsonData = response->getRoot();
                    jsonData["gamemode"] = this->_gameID;
                    response->setLength();
                    request->send(response);
                } });
}