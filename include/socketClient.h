#pragma once
#include <deviceSocket.h>

class SocketClient : public DeviceSocket
{
protected:
    void _initSocket() override;

public:
    SocketClient();
    ~SocketClient();
    void sendMessage(uint8_t deviceID, uint8_t messageCode) override;
    void sendMessage(uint8_t deviceID, uint8_t messageCode, uint8_t *payload, int len) override;
    void sendMessageAll(uint8_t messageCode) override{};
    void sendMessageAll(uint8_t messageCode, uint8_t *payload, int len) override{};
    void loop() override;

private:
    void _webSocketEvent(WStype_t type, uint8_t *payload, size_t length);
    WebSocketsClient *_socketClient{nullptr};
};