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
#define MAX_CONNECTED_DEVICES 7

class DeviceSocket
{
protected:
    void handle(WSH_Event event);
    void handle(WSH_Message messageType, uint8_t from, SocketDataMessage *message);
    void handleNewDeviceConnected(uint8_t deviceID);

public:
    DeviceSocket();
    bool isHost();
    void sendMessage(uint8_t deviceID, uint8_t messageCode);
    void sendMessage(uint8_t deviceID, uint8_t messageCode, uint8_t *payload, int len);
    void sendMessageAll(uint8_t messageCode);
    void sendMessageAll(uint8_t messageCode, uint8_t *payload, int len);
    std::vector<uint8_t> getConnectedDevicesIDVector() { return _connectedDevicesID; };
    void on(WSH_Event event, SocketEventCallback onEvent);
    void on(WSH_Message messageType, SocketMessageCallback onMessage);
    void loop();

private:
    void webSocketClientEvent(WStype_t type, uint8_t *payload, size_t length);
    void webSocketServerEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
    void _initSocket();
    std::vector<uint8_t> _connectedDevicesID;
    std::list<SocketMessageHandler *> _handlers;
    WebSocketsClient *_socketClient;
    AsyncWebSocket *_socketHost;
    AsyncWebServer *_server;
    bool _isHost;
    void connectToWifi();
};