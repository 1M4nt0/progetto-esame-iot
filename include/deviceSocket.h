#pragma once
#include "WiFi.h"
#include <ESPAsyncWebServer.h>
#include <WebSocketsClient.h>
#include <socketMessageHandler.h>
#include "SPIFFS.h"
#include <list>
#include <iterator>

#define WIFI_SSID "IoTGame"
#define WIFI_PASSWORD "testpassword"

class DeviceSocket
{
protected:
    void handle(uint8_t from, HandlerMsgType type, SocketDataMessage *message);
    void handle(uint8_t from, HandlerMsgType type);
    void handleNewDeviceConnected(uint8_t deviceID);

public:
    DeviceSocket();
    bool getIsHost();
    void sendMessage(uint8_t deviceID, uint8_t messageCode);
    void sendMessage(uint8_t deviceID, uint8_t messageCode, uint8_t *payload, int len);
    void sendMessageAll(uint8_t messageCode);
    void sendMessageAll(uint8_t messageCode, uint8_t *payload, int len);
    void on(HandlerMsgType messageType, SocketMessageCallback onMessage);
    void on(HandlerMsgType messageType, SocketDataMessageCallback onMessage);
    void loop();

private:
    void webSocketClientEvent(WStype_t type, uint8_t *payload, size_t length);
    void webSocketServerEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
    std::vector<uint8_t> _connectedDevicesID;
    std::list<SocketMessageHandler *> _handlers;
    WebSocketsClient *_socketClient;
    AsyncWebSocket *_socketHost;
    AsyncWebServer *_server;
    bool _isHost;
    void connectToWifi();
};