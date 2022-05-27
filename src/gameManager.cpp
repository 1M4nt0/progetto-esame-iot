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
    this->_device->socket()->on(DSM_CORE, WSHM_BIN, [&](WSH_Message msgType, uint8_t from, SocketDataMessage *message)
                                {
        switch (message->code)
        {
        case C_PAUSE:
        {
            if (!this->_device->isHost())
            {
                bool newPause = message->payload[0];
                this->_setPause(newPause);
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
    this->_device->socket()->on(DSM_CORE, WSHE_WIFI_DISCONNECTED, [&](WSH_Event event)
                                { this->_initGame(this->_gameID); });
    if (this->_device->isHost())
    {
        this->_device->socket()->on(DSM_CORE, WSHE_SOCKET_CONNECTED, [&](WSH_Event event, uint8_t from)
                                    { this->_sendChangeGame(from, this->_gameID); 
                                      this->_sendIsPaused(from, this->_isPaused); });
    }
}

GameManager::~GameManager()
{
    delete this->_game;
    delete this->_device;
}

void GameManager::_initGame(uint8_t gameID)
{
    this->_device->display()->drawToScreen("Avvio gioco...");
    this->_gameID = gameID;
    if (this->_game != nullptr)
    {
        delete this->_game;
    }
    if (gameID == 0)
    {
        this->_game = new MainMenu(this->_device);
        this->_device->display()->drawTwoToScreen("Scegli il", "gioco...");
    }
    else if (gameID == 1)
    {
        if (this->_device->isHost())
        {
            this->_game = new MultiplayerHost(this->_device);
        }
        else
        {
            this->_game = new MultiplayerClient(this->_device);
            this->_sendIsReadyToHost();
        }
    }
    else if (gameID == 2)
    {
        if (this->_device->isHost())
        {
            this->_game = new SingleplayerHost(this->_device);
        }
        else
        {
            this->_game = new SingleplayerClient(this->_device);
            this->_sendIsReadyToHost();
        }
    }
    this->_setPause(true);
}

void GameManager::_setPause(bool isPaused)
{
    this->_device->setLightOn(false);
    if (this->_device->isHost())
    {
        this->_sendIsPaused(isPaused);
    }
    this->_isPaused = isPaused;
    if (this->_isPaused)
    {
        if (this->_gameID == 1)
        {
            this->_device->display()->drawTwoToScreen("Multiplayer", "in pausa...");
        }
        else if (this->_gameID == 2)
        {
            this->_device->display()->drawTwoToScreen("Singleplayer", "in pausa...");
        }
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

void GameManager::_sendIsPaused(uint8_t deviceID, bool isPaused)
{
    uint8_t u_paused = (uint8_t)isPaused;
    if (this->_device->isHost())
    {
        this->_device->socket()->sendMessage(deviceID, C_PAUSE, &u_paused, sizeof(bool));
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
    Serial.printf("Change game  device %i, gamemode %i\n", deviceID, newGameID);
    this->_device->socket()->sendMessage(deviceID, C_NEW_GAMEMODE, &newGameID, sizeof(uint8_t));
}

void GameManager::_initServerEndpoints()
{
    this->_device->webServer()->on("/", HTTP_ANY, [](AsyncWebServerRequest *request)
                                   { request->send(SPIFFS, "/index.html"); });
    this->_device->webServer()->on("/multiplayer.html", HTTP_ANY, [](AsyncWebServerRequest *request)
                                   { request->send(SPIFFS, "/multiplayer.html"); });
    this->_device->webServer()->on("/singleplayer.html", HTTP_ANY, [](AsyncWebServerRequest *request)
                                   { request->send(SPIFFS, "/singleplayer.html"); });
    this->_device->webServer()->on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
                                   { request->send(SPIFFS, "/style.css", "text/css"); });
    this->_device->webServer()->on("/functions.js", HTTP_GET, [](AsyncWebServerRequest *request)
                                   { request->send(SPIFFS, "/functions.js", "text/js"); });
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
    this->_device->webServer()->on("/points", HTTP_GET, [&](AsyncWebServerRequest *request)
                                   { this->_game->servePointsEndpoint(request); });
}