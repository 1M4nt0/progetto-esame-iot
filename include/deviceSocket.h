#pragma once
#include <ESPAsyncWebServer.h>
#include <WebSocketsClient.h>
#include <socketMessageHandler.h>
#include "SPIFFS.h"
#include <list>
#include <iterator>

uint8_t dataBuffer[3]; // buffer to hold packets

class DeviceSocket
{
protected:
    void handle(HandlerMsgType type, uint8_t from, uint8_t *payload, size_t length);
    void handleNewDeviceConnected(uint8_t deviceID);

public:
    DeviceSocket(bool *isHost);
    void sendMessage(uint8_t deviceID, uint8_t messageCode);
    void sendMessage(uint8_t deviceID, uint8_t messageCode, uint8_t *payload, int len);
    void sendMessageAll(uint8_t messageCode);
    void sendMessageAll(uint8_t messageCode, uint8_t *payload, int len);
    void on(HandlerMsgType messageType, SocketHandlerFunction onMessage);
    void loop();

private:
    void webSocketClientEvent(WStype_t type, uint8_t *payload, size_t length);
    void webSocketServerEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
    std::vector<uint8_t> _connectedDevicesID;
    std::list<SocketMessageHandler *> _handlers;
    WebSocketsClient *_socketClient;
    AsyncWebSocket *_socketHost;
    AsyncWebServer *_server;
    bool *_isHost;
};