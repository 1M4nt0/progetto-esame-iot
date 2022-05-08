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
    virtual void setup(bool isHost) = 0;
    virtual void endGame() = 0;
    virtual void gameLoop() = 0;
    virtual void onTimeRecieved(uint8_t deviceID, short time) = 0;
    virtual void onButtonPressed() = 0;
    virtual void onSwitchLightOn() = 0;
    virtual void onSwitchLightOff() = 0;
    virtual void onDeviceIDRecieved() = 0;
    virtual void onWinnerResultsRecieved(uint8_t winnerID) = 0;
    virtual void onNewDeviceConnected(uint8_t deviceID) = 0;
    virtual void onDeviceDisconnected(uint8_t deviceID) = 0;
    virtual void configServerEndpoints(AsyncWebServer *_server) = 0;

    void initClient();
    void initServer();
    void sendSwitchLightOn();
    void sendSwitchLightOn(uint8_t deviceID);
    void sendSwitchLightOff();
    void sendSwitchLightOff(uint8_t deviceID);
    void setLightOn();
    long setLightOff();
    bool getIsLightOn();
    void setDeviceID(uint8_t deviceID);
    void sendDeviceID(uint32_t clientID, uint8_t deviceID);
    void sendTime(short time);
    void sendWinner(uint8_t winnerID, bool broadcast = true);
    void incrementPlayerPoints(uint8_t playerID, int increment);
    void resetAllPlayersPoints();
    void resetPlayerPoints(uint8_t playerID);
    std::vector<uint8_t> getConnectedDevicesID();
    std::map<uint8_t, int> getPlayersPoints();
    void initCommunication(bool isHost);

public:
    Game();
    ~Game(){};
    void init(bool isHost);
    virtual void startGame() = 0;
    void loop();
    bool getIsHost();
    bool setIsHost();
    uint8_t getDeviceID();
    int getPlayerPoints(uint8_t playerID);

private:
    AsyncWebServer *_server;
    WebSocketsClient *_webSocketClient;
    AsyncWebSocket *_ws;
    std::vector<uint8_t> connectedDevicesID;
    std::map<uint8_t, int> playersPoints; // Device id to player points
    uint8_t deviceID;
    bool isLightOn;
    bool isHost;
    unsigned long lightOnTime;
    void webSocketClientEvent(WStype_t type, uint8_t *payload, size_t length);
    void webSocketServerEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
};