#pragma once
#include <stdint.h>
#include <functional>

typedef enum
{
    WSHM_CONNECTED,
    WSHM_DISCONNECTED,
    WSHM_TEXT,
    WSHM_BIN,
} WSH_Message;

typedef enum
{
    WSHE_WIFI_DISCONNECTED,
    WSHE_WIFI_CONNECTED,
    WSHE_SOCKET_DISCONNECTED,
    WSHE_SOCKET_CONNECTED,
    WSHE_ERROR,
    WSHE_MESSAGE
} WSH_Event;

typedef struct
{
    uint8_t code;
    uint8_t *payload;
    size_t length;
} SocketDataMessage;

typedef std::function<void(WSH_Event event)> SocketEventCallback;
typedef std::function<void(WSH_Message msgType, uint8_t from, SocketDataMessage *message)> SocketMessageCallback;

class SocketMessageHandler
{
public:
    void onMessage(SocketMessageCallback fn) { _onMessage = fn; };
    void onEvent(SocketEventCallback fn) { _onEvent = fn; };
    void setMessageType(WSH_Message messageType) { _messageType = messageType; };
    void setEventType(WSH_Event event) { _eventType = event; };
    void handle(WSH_Event event) { _onEvent(event); };
    void handle(WSH_Message msgType, uint8_t from, SocketDataMessage *message) { _onMessage(msgType, from, message); };
    WSH_Message getMessageType() { return _messageType; };
    WSH_Event getEventType() { return _eventType; };

private:
    WSH_Event _eventType;
    WSH_Message _messageType;
    SocketEventCallback _onEvent;
    SocketMessageCallback _onMessage;
};