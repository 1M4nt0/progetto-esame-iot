#include <game.h>
#include <screen_utils.h>

uint8_t dataBuffer[3]; // buffer to hold packets

Game::Game(AsyncWebServer *_server, std::vector<uint8_t> *connectedDevicesID)
{
    this->connectedDevicesID = connectedDevicesID;
    this->_server = _server;
    pinMode(BUTTON_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    this->isLightOn = false;
    this->lightOnTime = 0;
}

void Game::loop()
{
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

void Game::resetPlayerPoints(uint8_t playerID)
{
    this->playersPoints[playerID] = 0;
}

int Game::getPlayerPoints(uint8_t playerID)
{
    return playersPoints[playerID];
}

std::map<uint8_t, int> Game::getPlayersPoints()
{
    return this->getPlayersPoints();
}

void Game::incrementPlayerPoints(uint8_t playerID, int increment)
{
    playersPoints[playerID] = playersPoints[playerID] + increment;
}

void Game::initClient(WebSocketsClient *socketClient)
{
    this->isHost = false;
    _webSocketClient = socketClient;
}

void Game::initServer(AsyncWebSocket *socketServer)
{
    this->isHost = true;
    _ws = socketServer;
    this->setDeviceID(1);
    // this->configServerEndpoints();
}

void Game::resetAllPlayersPoints()
{
    this->playersPoints.clear();
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

void Game::setLightOn()
{
    digitalWrite(LED_PIN, HIGH);
    this->isLightOn = true;
    this->lightOnTime = millis();
}

long Game::setLightOff()
{
    digitalWrite(LED_PIN, LOW);
    this->isLightOn = false;
    return millis() - this->lightOnTime;
}

bool Game::getIsLightOn()
{
    return this->isLightOn;
}
uint8_t Game::getDeviceID()
{
    return deviceID;
}

void Game::setDeviceID(uint8_t deviceID)
{
    this->deviceID = deviceID;
}

void Game::sendTime(short time)
{
    memset(dataBuffer, (uint8_t)C_TIME, sizeof(uint8_t));
    memcpy(dataBuffer + 1, &time, sizeof(short));
    _webSocketClient->sendBIN(dataBuffer, sizeof(uint8_t) + sizeof(short));
};

void Game::sendWinner(uint8_t winnerID, bool broadcast)
{
    memset(dataBuffer, (uint8_t)C_WINNER, 1);
    memcpy(dataBuffer + 1, &winnerID, sizeof(uint8_t));
    if (broadcast)
    {
        _ws->binaryAll(dataBuffer, sizeof(uint8_t) + sizeof(uint8_t));
    }
    else
    {
        _ws->binary(winnerID - 1, dataBuffer, sizeof(uint8_t) + sizeof(uint8_t));
    }
}

void Game::manageMessages(uint8_t fromID, uint8_t *payload, size_t length)
{
    int code = payload[0];
    if (code == C_PLAYER_ID) // Player ID ricevuto
    {
        uint8_t id;
        memcpy(&id, payload + 1, sizeof(uint8_t));
        this->setDeviceID(id);
        this->onDeviceIDRecieved();
    }
    else if (code == C_LIGHTS_ON) // Ricevuto comando accenditi
    {
        this->setLightOn();
        this->onSwitchLightOn();
    }
    else if (code == C_LIGHTS_OFF) // Ricevuto comando spegniti
    {
        this->setLightOff();
        this->onSwitchLightOff();
    }
    else if (code == C_WINNER) // Rievuto comando vincitore
    {
        uint8_t id;
        memcpy(&id, payload + 1, sizeof(uint8_t));
        this->onWinnerResultsRecieved(id);
    }
    else if (code == C_TIME) // Ricevuto tempo dal client
    {
        unsigned short time;
        memcpy(&time, payload + 1, 2);
        onTimeRecieved(fromID, time);
    }
}

std::vector<uint8_t> *Game::getConnectedDevicesID()
{
    return this->connectedDevicesID;
}