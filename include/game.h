#pragma once
#include <ESPAsyncWebServer.h>
#include <WebSocketsClient.h>
#include <map>
#include <vector>

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
    virtual void startGame();
    virtual void endGame();
    virtual void gameLoop();
    virtual void onTimeRecieved(uint8_t deviceID, short time);
    virtual void onButtonPressed();
    virtual void onSwitchLightOnRecived();
    virtual void onSwitchLightOffRecived();
    virtual void onDeviceIDRecieved();
    virtual void onWinnerResultsRecieved(uint8_t winnerID);
    virtual void onNewDeviceConnected(uint8_t deviceID);
    virtual void onDeviceDisconnected(uint8_t deviceID);

    void setupClient();
    void setupServer();
    void sendSwitchLightOn();
    void sendSwitchLightOn(uint8_t deviceID);
    void sendSwitchLightOff();
    void sendSwitchLightOff(uint8_t deviceID);
    bool getIsLightOn();
    uint8_t getDeviceID();
    void setDeviceID(uint8_t deviceID);
    void sendDeviceID(uint32_t clientID, uint8_t deviceID);
    void sendTime(short time);
    void sendWinner(uint8_t winnerID, bool broadcast = true);

public:
    Game();
    void setup(bool isHost);
    void loop();
    bool getIsHost();
    bool setIsHost();

private:
    std::vector<uint8_t> connectedDevicesID;
    std::map<uint8_t, short> deviceResponseTime;
    uint8_t deviceID;
    bool isLightOn;
    bool isHost;
    void webSocketClientEvent(WStype_t type, uint8_t *payload, size_t length);
    void webSocketServerEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
};