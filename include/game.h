#pragma once
#include <ESPAsyncWebServer.h>
#include <WebSocketsClient.h>
#include <map>
#include <vector>
#include "SPIFFS.h"
#include "AsyncJson.h"
#include <ArduinoJson.h>

#define C_PLAYER_ID 1
#define C_LIGHTS_ON 2
#define C_LIGHTS_OFF 3
#define C_TIME 4
#define C_WINNER 5

#define BUTTON_PIN 16
#define LED_PIN 17

class Game
{
protected:
    AsyncWebServer *_server;
    WebSocketsClient *_webSocketClient;
    AsyncWebSocket *_ws;
    std::map<uint8_t, int> playerPoints; // Device id to player points
    virtual void startGame() = 0;
    virtual void endGame() = 0;
    virtual void gameLoop() = 0;
    virtual void onTimeRecieved(uint8_t deviceID, short time) = 0;
    virtual void onButtonPressed() = 0;
    virtual void onSwitchLightOnRecived() = 0;
    virtual void onSwitchLightOffRecived() = 0;
    virtual void onDeviceIDRecieved() = 0;
    virtual void onWinnerResultsRecieved(uint8_t winnerID) = 0;
    virtual void onNewDeviceConnected(uint8_t deviceID) = 0;
    virtual void onDeviceDisconnected(uint8_t deviceID) = 0;

    void setupClient();
    void setupServer();
    void sendSwitchLightOn();
    void sendSwitchLightOn(uint8_t deviceID);
    void sendSwitchLightOff();
    void sendSwitchLightOff(uint8_t deviceID);
    bool getIsLightOn();
    void setDeviceID(uint8_t deviceID);
    void sendDeviceID(uint32_t clientID, uint8_t deviceID);
    void sendTime(short time);
    void sendWinner(uint8_t winnerID, bool broadcast = true);
    void incrementPlayerPoints(uint8_t playerID, int increment);
    void setDeviceResponseTime(uint8_t deviceID, short time);

public:
    Game();
    ~Game(){};
    void setup(bool isHost);
    void loop();
    bool getIsHost();
    bool setIsHost();
    uint8_t getDeviceID();
    int getPlayerPoints();
    void resetResponseTimes();
    void resetPoints();

private:
    std::vector<uint8_t> connectedDevicesID;
    std::map<uint8_t, short> deviceResponseTime;
    uint8_t deviceID;
    bool isLightOn;
    bool isHost;
    void webSocketClientEvent(WStype_t type, uint8_t *payload, size_t length);
    void webSocketServerEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
};