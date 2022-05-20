#pragma once
#include <deviceSocket.h>

class SocketHost : public DeviceSocket
{
protected:
    void _initSocket() override;

public:
    SocketHost();
    void sendMessage(uint8_t deviceID, uint8_t messageCode) override;
    void sendMessage(uint8_t deviceID, uint8_t messageCode, uint8_t *payload, int len) override;
    void sendMessageAll(uint8_t messageCode) override;
    void sendMessageAll(uint8_t messageCode, uint8_t *payload, int len) override;
    void loop() override;

private:
    void _webSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *payload, size_t length);
    AsyncWebSocket *_socketHost;
};