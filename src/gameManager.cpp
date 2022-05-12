#include <gameManager.h>

uint8_t dataBuffer2[3];

GameManager::GameManager(bool isHost)
{
    this->_isHost = isHost;
    this->_server = new AsyncWebServer(80);
    SPIFFS.begin(true);
    this->_server->begin();
    if (this->_isHost)
    {
        connectedDevicesID.push_back(1);
        _ws = new AsyncWebSocket("/ws");
        _ws->onEvent([&](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
                     { webSocketServerEvent(server, client, type, arg, data, len); });
        _server->addHandler(_ws);
        _server->on("/gamemode", HTTP_GET, [=](AsyncWebServerRequest *request)
                    { 
                if(request->hasParam("id")){
                int gameID = request->getParam("id")->value().toInt();
                if(gameID != this->_currentGameID){
                    request->send(200, "text", "OK!");
                    drawToScreen("Cambio gioco...");
                    this->sendChangeGame(gameID);
                    this->initializeGame(gameID);                   
                }else{
                    request->send(200, "text", "NOT OK!");
                }}else{
                    request->send(200, "text", String(this->_currentGameID));
                } });
        _server->on("/pause", HTTP_GET, [=](AsyncWebServerRequest *request)
                    { 
                if(request->hasParam("pause")){
                bool pause = request->getParam("pause")->value().toInt();
                if(pause != this->_paused){
                    request->send(200, "text", "OK!");
                    this->sendPause(pause);
                    this->setPaused(pause);
                }else{
                    request->send(200, "text", "NOT OK!");
                }}else{
                    request->send(200, "text", String(this->_paused));
                } });
    }
    else
    {
        _webSocketClient = new WebSocketsClient();
        _webSocketClient->begin("192.168.4.1", 80, "/ws");
        _webSocketClient->onEvent([&](WStype_t type, uint8_t *payload, size_t length)
                                  { webSocketClientEvent(type, payload, length); });
        _webSocketClient->setReconnectInterval(5000);
    }
};

int GameManager::getCurrentGame()
{
    return this->_currentGameID;
}

void GameManager::initializeGame(int gameID)
{
    this->_currentGameID = gameID;
    if (this->_currentGameID == 0)
    {
        _game = new Multiplayer(this->_server, &connectedDevicesID);
    }
    else if (this->_currentGameID == 1)
    {
        _game = new Singleplayer(this->_server, &connectedDevicesID);
    }
    else
    {
        Serial.println("NON ESISTE QUESTA MODALITA");
    }
    (this->_isHost) ? _game->initServer(this->_ws) : _game->initClient(this->_webSocketClient);
    this->setPaused(true);
}

void GameManager::loop()
{
    if (!this->_isHost)
    {
        _webSocketClient->loop();
    }
    if (!this->_paused)
    {
        _game->loop();
    }
}

void GameManager::setPaused(bool pause)
{
    this->_paused = pause;
    if (_paused)
    {
        drawTwoToScreen("Gioco " + String(this->getCurrentGame()), "In pausa...");
    }
    else
    {
        if (this->_isHost)
        {
            _game->startGame();
        }
    }
}

bool GameManager::isPaused()
{
    return this->_paused;
}

void GameManager::sendChangeGame(uint8_t gameID)
{
    memset(dataBuffer2, (uint8_t)C_NEW_GAMEMODE, sizeof(uint8_t));
    memcpy(dataBuffer2 + 1, &gameID, sizeof(uint8_t));
    this->_ws->binaryAll(dataBuffer2, sizeof(uint8_t) + sizeof(uint8_t));
}

void GameManager::sendChangeGame(uint8_t gameID, uint8_t deviceID)
{
    memset(dataBuffer2, (uint8_t)C_NEW_GAMEMODE, sizeof(uint8_t));
    memcpy(dataBuffer2 + 1, &gameID, sizeof(uint8_t));
    this->_ws->binary(deviceID - 1, dataBuffer2, sizeof(uint8_t) + sizeof(uint8_t));
}

void GameManager::sendPause(bool pause)
{
    memset(dataBuffer2, (uint8_t)C_PAUSE, sizeof(uint8_t));
    memcpy(dataBuffer2 + 1, &pause, sizeof(bool));
    this->_ws->binaryAll(dataBuffer2, sizeof(uint8_t) + sizeof(bool));
}

void GameManager::sendDeviceID(uint32_t clientID, uint8_t deviceID)
{
    memset(dataBuffer2, (uint8_t)C_PLAYER_ID, sizeof(uint8_t));
    memcpy(dataBuffer2 + 1, &deviceID, sizeof(uint8_t));
    this->_ws->binary(clientID, dataBuffer2, sizeof(uint8_t) + sizeof(uint8_t));
};

void GameManager::webSocketClientEvent(WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
        break;
    case WStype_CONNECTED:
        break;
    case WStype_BIN:
    {
        if (payload[0] == C_NEW_GAMEMODE)
        {
            drawToScreen("Cambio gioco...");
            uint8_t newGameID;
            memcpy(&newGameID, payload + 1, sizeof(uint8_t));
            this->initializeGame(newGameID);
        }
        else if (payload[0] == C_PAUSE)
        {
            bool pause;
            memcpy(&pause, payload + 1, sizeof(bool));
            this->setPaused(pause);
        }
        else
        {
            _game->manageMessages(0, payload, length);
        }
    }
    break;
    case WStype_ERROR:
        break;
    }
}

void GameManager::webSocketServerEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    uint8_t deviceID = client->id() + 1;
    if (type == WS_EVT_CONNECT) // CLIENT CONNECTED
    {
        connectedDevicesID.push_back(deviceID);
        this->sendDeviceID(client->id(), deviceID);
        this->sendChangeGame(this->getCurrentGame(), deviceID);
        _game->onNewDeviceConnected(deviceID);
    }
    else if (type == WS_EVT_DISCONNECT) // CLIENT DISCONNECTED
    {
        connectedDevicesID.erase(remove(connectedDevicesID.begin(), connectedDevicesID.end(), deviceID), connectedDevicesID.end());
        _game->onDeviceDisconnected(deviceID);
    }
    else if (type == WS_EVT_DATA) // RECEIVED DATA
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_BINARY)
        {
            _game->manageMessages(deviceID, data, len);
        }
    }
    else if (type == WS_EVT_ERROR)
    {
        // error was received from the other end
    }
}