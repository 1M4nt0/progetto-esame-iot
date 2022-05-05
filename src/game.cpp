#include <vector>
#include <map>
#include <algorithm>
#include <WebSocketsClient.h>
#include <ESPAsyncWebServer.h>

#define C_PLAYER_ID 1
#define C_LIGHTS_ON 2
#define C_LIGHTS_OFF 3
#define C_TIME 4
#define C_WINNER 5

#define BUTTON_PIN 16
#define LED_PIN 17

uint8_t dataBuffer[3]; // buffer to hold packets

class Game
{
protected:
    AsyncWebServer *_server;
    WebSocketsClient *_webSocketClient;
    AsyncWebSocket *_ws;
    virtual void startGame() = 0;
    virtual void endGame() = 0;
    virtual void gameLoop() = 0;
    virtual void onTimeRecieved(uint8_t deviceID, short time) = 0;
    virtual void onButtonPressed() = 0;
    virtual void onSwitchLightOnRecived();
    virtual void onSwitchLightOffRecived();
    virtual void onDeviceIDRecieved();
    virtual void onWinnerResultsRecieved(uint8_t winnerID);
    virtual void onNewDeviceConnected(uint8_t deviceID);
    virtual void onDeviceDisconnected(uint8_t deviceID);

    void setupClient()
    {
        _webSocketClient = new WebSocketsClient();
        _webSocketClient->begin("192.168.4.1", 80, "/ws");
        _webSocketClient->onEvent([&](WStype_t type, uint8_t *payload, size_t length)
                                  { webSocketClientEvent(type, payload, length); });
        _webSocketClient->setReconnectInterval(5000);
    }
    void setupServer()
    {
        _server = new AsyncWebServer(80);
        _ws = new AsyncWebSocket("/ws");
        _ws->onEvent([&](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
                     { webSocketServerEvent(server, client, type, arg, data, len); });
        _server->addHandler(_ws);
        _server->begin();
    }
    void sendSwitchLightOn()
    {
        memset(&dataBuffer, (uint8_t)C_LIGHTS_ON, sizeof(uint8_t));
        _ws->binaryAll(dataBuffer, sizeof(uint8_t));
    }
    void sendSwitchLightOn(uint8_t deviceID)
    {
        memset(&dataBuffer, (uint8_t)C_LIGHTS_ON, sizeof(uint8_t));
        _ws->binary(deviceID - 1, dataBuffer, sizeof(uint8_t));
    }
    void sendSwitchLightOff()
    {
        memset(&dataBuffer, (uint8_t)C_LIGHTS_OFF, sizeof(uint8_t));
        _ws->binaryAll(dataBuffer, sizeof(uint8_t));
    }
    void sendSwitchLightOff(uint8_t deviceID)
    {
        memset(&dataBuffer, (uint8_t)C_LIGHTS_OFF, sizeof(uint8_t));
        _ws->binary(deviceID - 1, dataBuffer, sizeof(uint8_t));
    }
    bool getIsLightOn()
    {
        return isLightOn;
    }
    uint8_t getDeviceID()
    {
        return deviceID;
    }
    void setDeviceID(uint8_t deviceID)
    {
        this->deviceID = deviceID;
    }
    void sendDeviceID(uint32_t clientID, uint8_t deviceID)
    {
        memset(dataBuffer, (uint8_t)C_PLAYER_ID, sizeof(uint8_t));
        memcpy(dataBuffer + 1, &deviceID, sizeof(uint8_t));
        _ws->binary(clientID, dataBuffer, sizeof(uint8_t) + sizeof(uint8_t));
    };
    void sendTime(short time)
    {
        memset(dataBuffer, (uint8_t)C_TIME, sizeof(uint8_t));
        memcpy(dataBuffer + 1, &time, sizeof(short));
        _webSocketClient->sendBIN(dataBuffer, sizeof(uint8_t) + sizeof(short));
    };
    void sendWinner(uint8_t winnerID, bool broadcast = true)
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

public:
    Game(bool isHost)
    {
        pinMode(BUTTON_PIN, INPUT);
        pinMode(LED_PIN, OUTPUT);
        this->isLightOn = false;
        this->isHost = isHost;
        (isHost) ? setupClient() : setupServer();
    }
    void loop()
    {
        _webSocketClient->loop();
        this->gameLoop();
    }
    bool getIsHost()
    {
        return this->isHost;
    }
    bool setIsHost()
    {
        return this->isHost;
    }

private:
    std::vector<uint8_t> connectedDevicesID;
    std::map<uint8_t, short> deviceResponseTime;
    uint8_t deviceID;
    bool isLightOn;
    bool isHost;
    void webSocketClientEvent(WStype_t type, uint8_t *payload, size_t length)
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
    void webSocketServerEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
    {
        if (type == WS_EVT_CONNECT) // CLIENT CONNECTED
        {
            connectedDevicesID.push_back(client->id());
            uint8_t id = client->id() + 1;
            sendDeviceID(client->id(), id);
            onNewDeviceConnected(id);
        }
        else if (type == WS_EVT_DISCONNECT) // CLIENT DISCONNECTED
        {
            connectedDevicesID.erase(remove(connectedDevicesID.begin(), connectedDevicesID.end(), client->id()), connectedDevicesID.end());
            onDeviceDisconnected(client->id());
        }
        else if (type == WS_EVT_DATA) // RECIEVED DATA
        {
            AwsFrameInfo *info = (AwsFrameInfo *)arg;
            if (info->final && info->index == 0 && info->len == len && info->opcode == WS_BINARY)
            {
                uint8_t code = data[0];
                if (code == C_TIME)
                {
                    unsigned short time;
                    memcpy(&time, data + 1, 2);
                    onTimeRecieved(client->id() + 1, time);
                }
            }
        }
        else if (type == WS_EVT_ERROR)
        {
            // error was received from the other end
        }
    }
};

#include <climits>
#include <screen_utils.h>

class Multiplayer : public Game
{
private:
    unsigned long matchStartTime = 0;
    unsigned short matchDuration = 10 * (10 ^ 3);
    std::map<uint8_t, uint8_t> playerID;
    std::map<uint8_t, int> playerPoints; // Device ID to playerID
    struct
    {
        uint8_t id = 0;
        unsigned short time = SHRT_MAX;
    } Winner;
    void manageCurrentWinner(uint8_t id, unsigned short pressingTime)
    {
        if (Winner.time > pressingTime)
        {
            Winner.time = pressingTime;
            Winner.id = id;
        }
    }

protected:
    uint8_t getPlayerID()
    {
        return this->playerID[this->getDeviceID()];
    }
    int getPlayerPoints()
    {
        return playerPoints[this->getPlayerID()];
    }
    void incrementPlayerPoints(uint8_t playerID, int increment)
    {
        playerPoints[this->getPlayerID()] = playerPoints[this->getPlayerID()] + increment;
    }
    uint8_t getPlayerIDFromDevice(uint8_t deviceID)
    {
        return playerID[deviceID];
    }
    void startGame()
    {
        Winner.time = SHRT_MAX;
        Winner.id = 0;
        sendSwitchLightOn();
        matchStartTime = millis();
        digitalWrite(LED_PIN, HIGH);
    }
    void endGame()
    {
        sendSwitchLightOff();
        digitalWrite(LED_PIN, LOW);
        if (Winner.id != 0)
        {
            sendWinner(Winner.id);
            checkResults(Winner.id);
        }
    }
    void gameLoop()
    {
        if (this->getIsHost() && millis() > matchStartTime + matchDuration)
        {
            this->endGame();
            delay(random(2000, 8000));
            this->startGame();
        }
        if (this->getIsLightOn())
        {
            if (digitalRead(BUTTON_PIN) == LOW)
            {
                digitalWrite(LED_PIN, LOW);
                onButtonPressed();
            }
        }
    }
    void onButtonPressed()
    {
        short pressingTime = millis() - matchStartTime;
        (this->getIsHost()) ? manageCurrentWinner(this->getPlayerID(), pressingTime) : this->sendTime(pressingTime);
    }
    void checkResults(uint8_t winnerID)
    {
        if (winnerID == this->getPlayerID())
        {
            drawToScreen("Hai vinto!");
            incrementPlayerPoints(this->getPlayerID(), 1);
        }
        else
        {
            drawToScreen("Hai perso!");
        }
        delay(2000);
        drawDashboard(this->getPlayerID(), this->getPlayerPoints());
    }
    void onWinnerResultsRecieved(uint8_t winnerID)
    {
        checkResults(winnerID);
    }
    void onDeviceIDRecieved()
    {
        playerID[this->getDeviceID()] = this->getDeviceID();
        drawDashboard(this->getPlayerID(), this->getPlayerPoints());
    }
    void onNewDeviceConnected(uint8_t deviceID)
    {
        playerID[deviceID] = deviceID;
        playerPoints[deviceID] = 0;
    }
    void onTimeRecieved(uint8_t deviceID, short time)
    {
        manageCurrentWinner(playerID[deviceID], time);
    }

public:
};
