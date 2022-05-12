#pragma once
#include <stdint.h>
#include <functional>

class SocketMessageHandler
{
public:
    void onMessageRecieved(std::function<void()> fn) { _onRequest = fn; };
    void setMessageCode(uint8_t messageCode) { _messageCode = messageCode; };
    void handleMessage() { _onRequest(); }

private:
    uint8_t _messageCode;
    std::function<void()> _onRequest;
};