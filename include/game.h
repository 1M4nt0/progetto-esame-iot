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
    virtual void gameLoop() = 0;
    virtual void onTimeRecieved(uint8_t deviceID, short time) = 0;
    virtual void onButtonPressed() = 0;

    void sendSwitchLightOn();
    void sendSwitchLightOn(uint8_t deviceID);
    void sendSwitchLightOff();
    void sendSwitchLightOff(uint8_t deviceID);
    bool getIsLightOn();
    void sendTime(short time);
    void sendWinner(uint8_t winnerID, bool broadcast = true);
    void incrementPlayerPoints(uint8_t playerID, int increment);
    void resetAllPlayersPoints();
    void resetPlayerPoints(uint8_t playerID);
    std::map<uint8_t, int> getPlayersPoints();
    void initCommunication(bool isHost);

public:
    Game(AsyncWebServer *_server, std::vector<uint8_t> *connectedDevicesID);
    ~Game(){};
    void init(bool isHost);
    virtual void startGame() = 0;
    virtual void endGame() = 0;
    void initClient(WebSocketsClient *socketClient);
    void initServer(AsyncWebSocket *socketServer);
    virtual void configServerEndpoints(AsyncWebServer *_server) = 0;
    void loop();
    bool getIsHost();
    bool setIsHost();
    void setLightOn();
    long setLightOff();
    virtual void onSwitchLightOn() = 0;
    virtual void onSwitchLightOff() = 0;
    void setDeviceID(uint8_t deviceID);
    uint8_t getDeviceID();
    virtual void onDeviceIDRecieved() = 0;
    int getPlayerPoints(uint8_t playerID);
    virtual void onWinnerResultsRecieved(uint8_t winnerID) = 0;
    virtual void manageMessages(uint8_t fromID, uint8_t *payload, size_t length);
    virtual void onNewDeviceConnected(uint8_t deviceID) = 0;
    virtual void onDeviceDisconnected(uint8_t deviceID) = 0;
    std::vector<uint8_t> *getConnectedDevicesID();

private:
    AsyncWebServer *_server;
    WebSocketsClient *_webSocketClient;
    AsyncWebSocket *_ws;
    std::vector<uint8_t> *connectedDevicesID;
    std::map<uint8_t, int> playersPoints; // Device id to player points
    uint8_t deviceID;
    bool isLightOn;
    bool isHost;
    unsigned long lightOnTime;
};