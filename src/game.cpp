#include <game.h>

uint8_t dataBuffer[3]; // buffer to hold packets

Game::Game()
{
    pinMode(BUTTON_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    this->isLightOn = false;
}

void Game::setup(bool isHost)
{
    this->isHost = true;
    (isHost) ? setupServer() : setupClient();
}
void Game::loop()
{
    if (!this->getIsHost())
    {
        _webSocketClient->loop();
    }
    this->gameLoop();
}
bool Game::getIsHost()
{
    return this->isHost;
}
bool Game::setIsHost()
{
    return this->isHost;
}

int Game::getPlayerPoints()
{
    return playerPoints[this->getDeviceID()];
}

void Game::incrementPlayerPoints(uint8_t playerID, int increment)
{
    playerPoints[playerID] = playerPoints[playerID] + increment;
}

void Game::setupClient()
{
    _webSocketClient = new WebSocketsClient();
    _webSocketClient->begin("192.168.4.1", 80, "/ws");
    _webSocketClient->onEvent([&](WStype_t type, uint8_t *payload, size_t length)
                              { webSocketClientEvent(type, payload, length); });
    _webSocketClient->setReconnectInterval(5000);
}

void Game::resetPoints()
{
    this->playerPoints.clear();
}

void Game::setDeviceResponseTime(uint8_t deviceID, short time)
{
    this->deviceResponseTime[deviceID] = time;
}

void Game::resetResponseTimes()
{
    this->deviceResponseTime.clear();
}

void Game::setupServer()
{
    setDeviceID(1);
    connectedDevicesID.push_back(1);
    _server = new AsyncWebServer(80);
    _ws = new AsyncWebSocket("/ws");
    if (!SPIFFS.begin(true))
    {
        Serial.println("Errore SPIFFS");
    }
    _server->on("/", HTTP_ANY, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/index.html"); });
    _server->on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/style.css", "text/css"); });
    _server->on("/functions.js", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/functions.js", "text/js"); });
    _server->on("/points", HTTP_GET, [&](AsyncWebServerRequest *request)
                { 
              AsyncJsonResponse* response = new AsyncJsonResponse();
              response->addHeader("Server","ESP Async Web Server");
              const JsonObject& jsonData = response->getRoot();
              int index = 0;
              for (uint8_t deviceID : this->connectedDevicesID)
              {
                jsonData["players"][index]["id"] = deviceID;
                jsonData["players"][index]["points"] = this->playerPoints[deviceID];
                jsonData["players"][index]["time"] = this->deviceResponseTime[deviceID] ;
                index++;
              }
              response->setLength();
              request->send(response); });
    _ws->onEvent([&](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
                 { webSocketServerEvent(server, client, type, arg, data, len); });
    _server->addHandler(_ws);
    _server->begin();
}

void Game::sendSwitchLightOn()
{
    memset(&dataBuffer, (uint8_t)C_LIGHTS_ON, sizeof(uint8_t));
    _ws->binaryAll(dataBuffer, sizeof(uint8_t));
}
void Game::sendSwitchLightOn(uint8_t deviceID)
{
    memset(&dataBuffer, (uint8_t)C_LIGHTS_ON, sizeof(uint8_t));
    _ws->binary(deviceID - 1, dataBuffer, sizeof(uint8_t));
}
void Game::sendSwitchLightOff()
{
    memset(&dataBuffer, (uint8_t)C_LIGHTS_OFF, sizeof(uint8_t));
    _ws->binaryAll(dataBuffer, sizeof(uint8_t));
}
void Game::sendSwitchLightOff(uint8_t deviceID)
{
    memset(&dataBuffer, (uint8_t)C_LIGHTS_OFF, sizeof(uint8_t));
    _ws->binary(deviceID - 1, dataBuffer, sizeof(uint8_t));
}
bool Game::getIsLightOn()
{
    return isLightOn;
}
uint8_t Game::getDeviceID()
{
    return deviceID;
}
void Game::setDeviceID(uint8_t deviceID)
{
    this->deviceID = deviceID;
}
void Game::sendDeviceID(uint32_t clientID, uint8_t deviceID)
{
    memset(dataBuffer, (uint8_t)C_PLAYER_ID, sizeof(uint8_t));
    memcpy(dataBuffer + 1, &deviceID, sizeof(uint8_t));
    _ws->binary(clientID, dataBuffer, sizeof(uint8_t) + sizeof(uint8_t));
};
void Game::sendTime(short time)
{
    memset(dataBuffer, (uint8_t)C_TIME, sizeof(uint8_t));
    memcpy(dataBuffer + 1, &time, sizeof(short));
    _webSocketClient->sendBIN(dataBuffer, sizeof(uint8_t) + sizeof(short));
};
void Game::sendWinner(uint8_t winnerID, bool broadcast)
{
    memset(&dataBuffer, (uint8_t)C_WINNER, 1);
    memcpy(&dataBuffer + 1, &winnerID, sizeof(uint8_t));
    if (broadcast)
    {
        _ws->binaryAll(dataBuffer, sizeof(uint8_t) + sizeof(uint8_t));
    }
    else
    {
        _ws->binary(winnerID, dataBuffer, sizeof(uint8_t) + sizeof(uint8_t));
    }
}

void Game::webSocketClientEvent(WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
        break;
    case WStype_CONNECTED:
        break;
    case WStype_BIN:
    {
        int code = payload[0];
        if (code == C_PLAYER_ID)
        {
            uint8_t id;
            memcpy(&id, payload + 1, sizeof(uint8_t));
            setDeviceID(id);
            onDeviceIDRecieved();
        }
        else if (code == C_LIGHTS_ON)
        {
            digitalWrite(LED_PIN, HIGH);
            isLightOn = true;
            onSwitchLightOnRecived();
        }
        else if (code == C_LIGHTS_OFF)
        {
            digitalWrite(LED_PIN, LOW);
            isLightOn = false;
            onSwitchLightOffRecived();
        }
        else if (code == C_WINNER)
        {
            uint8_t id;
            memcpy(&id, payload + 1, sizeof(uint8_t));
            onWinnerResultsRecieved(id);
        }
    }
    break;
    case WStype_ERROR:
        break;
    }
}
void Game::webSocketServerEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    uint8_t deviceID = client->id() + 1;
    if (type == WS_EVT_CONNECT) // CLIENT CONNECTED
    {
        connectedDevicesID.push_back(deviceID);
        sendDeviceID(client->id(), deviceID);
        onNewDeviceConnected(deviceID);
    }
    else if (type == WS_EVT_DISCONNECT) // CLIENT DISCONNECTED
    {
        connectedDevicesID.erase(remove(connectedDevicesID.begin(), connectedDevicesID.end(), deviceID), connectedDevicesID.end());
        onDeviceDisconnected(deviceID);
    }
    else if (type == WS_EVT_DATA) // RECEIVED DATA
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_BINARY)
        {
            uint8_t code = data[0];
            if (code == C_TIME)
            {
                unsigned short time;
                memcpy(&time, data + 1, 2);
                onTimeRecieved(deviceID, time);
            }
        }
    }
    else if (type == WS_EVT_ERROR)
    {
        // error was received from the other end
    }
}