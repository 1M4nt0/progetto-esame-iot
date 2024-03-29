#pragma once
#include <ESPAsyncWebServer.h>
#include <WebSocketsClient.h>
#include <socketMessageHandler.h>
#include "SPIFFS.h"
#include <list>
#include <iterator>

#define WIFI_SSID "IoTGame"
#define WIFI_PASSWORD "testpassword"
#define MAX_CONNECTED_DEVICES 7
#define WIFICONNECTIONRECHECKDELAY 2000

typedef enum
{
    DSM_CORE,
    DSM_GAME
} DSM_Type;

class DeviceSocket
{
protected:
    void handle(WSH_Event event);
    void handle(WSH_Event event, uint8_t from);
    void handle(WSH_Message messageType, uint8_t from, SocketDataMessage *message);
    virtual void _initSocket() = 0;
    std::vector<uint8_t> _connectedDevicesID;
    std::list<SocketMessageHandler *> _coreHandlers;
    std::list<SocketMessageHandler *> _handlers;
    AsyncWebServer *_server{nullptr};

public:
    ~DeviceSocket();
    AsyncWebServer *webServer() { return this->_server; };
    void clearCoreHandlers();
    void clearGameHandlers();
    virtual void sendMessage(uint8_t deviceID, uint8_t messageCode) = 0;
    virtual void sendMessage(uint8_t deviceID, uint8_t messageCode, uint8_t *payload, int len) = 0;
    virtual void sendMessageAll(uint8_t messageCode) = 0;
    virtual void sendMessageAll(uint8_t messageCode, uint8_t *payload, int len) = 0;
    std::vector<uint8_t> getConnectedDevicesIDVector() { return _connectedDevicesID; };
    void on(DSM_Type type, WSH_Event event, SocketEventCallback onEvent);
    void on(DSM_Type type, WSH_Event event, SocketEventFromCallback onEvent);
    void on(DSM_Type type, WSH_Message messageType, SocketMessageCallback onMessage);
    virtual void loop() = 0;

private:
    void _pushHandler(DSM_Type type, SocketMessageHandler *handler);
};