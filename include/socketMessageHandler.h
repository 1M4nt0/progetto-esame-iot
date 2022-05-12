#pragma once
#include <stdint.h>
#include <functional>

typedef enum
{
    T_ERROR,
    T_DISCONNECTED,
    T_CONNECTED,
    T_TEXT_MESSAGE,
    T_BIN_MASSAGE,
} HandlerMsgType;

typedef struct
{
    uint8_t code;
    uint8_t *payload;
    size_t length;
} SocketDataMessage;

typedef std::function<void(uint8_t from, HandlerMsgType msgType)> SocketMessageCallback;
typedef std::function<void(uint8_t from, HandlerMsgType msgType, SocketDataMessage *message)> SocketDataMessageCallback;

class SocketMessageHandler
{
public:
    void onMessage(SocketMessageCallback fn) { _onMessage = fn; };
    void onDataMessage(SocketDataMessageCallback fn) { _onDataMessage = fn; };
    void setMessageType(HandlerMsgType messageType) { _messageType = messageType; };
    void handle(uint8_t from, HandlerMsgType msgType) { _onMessage(from, msgType); };
    void handle(uint8_t from, HandlerMsgType msgType, SocketDataMessage *message) { _onDataMessage(from, msgType, message); };
    HandlerMsgType getMessageType() { return _messageType; };

private:
    HandlerMsgType _messageType;
    SocketMessageCallback _onMessage;
    SocketDataMessageCallback _onDataMessage;
};