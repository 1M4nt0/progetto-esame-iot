#pragma once
#include <ESPAsyncWebServer.h>
#include <WebSocketsClient.h>
#include <socketMessageHandler.h>
#include <list>
#include <iterator>

uint8_t dataBuffer[3]; // buffer to hold packets

class DeviceSocket
{
public:
    DeviceSocket(bool *isHost);
    void initAsHost(AsyncWebSocket *_socketHost);
    void initAsClient(WebSocketsClient *_socketClient);
    void sendMessage(uint8_t deviceID, uint8_t messageCode);
    void sendMessage(uint8_t deviceID, uint8_t messageCode, uint8_t *payload, int len);
    void sendMessageAll(uint8_t messageCode);
    void sendMessageAll(uint8_t messageCode, uint8_t *payload, int len);
    void addHandler(uint8_t messageCode, std::function<void()> onMessage);
    void loop();

private:
    std::list<SocketMessageHandler *> _handlers;
    WebSocketsClient *_socketClient;
    AsyncWebSocket *_socketHost;
    bool *_isHost;
};