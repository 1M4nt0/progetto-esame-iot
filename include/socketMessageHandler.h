#pragma once
#include <stdint.h>
#include <functional>

typedef enum
{
    T_ERROR,
    T_DISCONNECTED,
    T_CONNECTED,
    T_BIN_MASSAGE,
} HandlerMsgType;

typedef std::function<void(uint8_t from, uint8_t *payload, int len)> SocketHandlerFunction;

class SocketMessageHandler
{
public:
    void onMessageRecieved(SocketHandlerFunction fn) { _onRequest = fn; };
    void setMessageType(HandlerMsgType messageType) { _messageType = messageType; };
    void handleMessage(uint8_t from, uint8_t *payload, int len) { _onRequest(from, payload, len); };
    HandlerMsgType getMessageType() { return _messageType; };

private:
    HandlerMsgType _messageType;
    SocketHandlerFunction _onRequest;
};