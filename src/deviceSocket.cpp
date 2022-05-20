#include <deviceSocket.h>

void DeviceSocket::on(WSH_Event event, SocketEventCallback onEvent)
{
    SocketMessageHandler *handler = new SocketMessageHandler();
    handler->setEventType(event);
    handler->onEvent(onEvent);
    _handlers.push_back(handler);
}

void DeviceSocket::on(WSH_Event event, SocketEventFromCallback onEventFrom)
{
    SocketMessageHandler *handler = new SocketMessageHandler();
    handler->setEventType(event);
    handler->onEventFrom(onEventFrom);
    _handlers.push_back(handler);
}

void DeviceSocket::on(WSH_Message messageType, SocketMessageCallback onMessage)
{
    SocketMessageHandler *handler = new SocketMessageHandler();
    handler->setEventType(WSHE_MESSAGE);
    handler->setMessageType(messageType);
    handler->onMessage(onMessage);
    _handlers.push_back(handler);
}

void DeviceSocket::handle(WSH_Event event)
{
    for (SocketMessageHandler *handler : _handlers)
    {
        if (handler->getEventType() == event)
        {
            handler->handle(event);
        }
    }
}

void DeviceSocket::handle(WSH_Event event, uint8_t from)
{
    for (SocketMessageHandler *handler : _handlers)
    {
        if (handler->getEventType() == event)
        {
            handler->handle(event, from);
        }
    }
}

void DeviceSocket::handle(WSH_Message messageType, uint8_t from, SocketDataMessage *message)
{
    for (SocketMessageHandler *handler : _handlers)
    {
        if (handler->getEventType() == WSHE_MESSAGE && handler->getMessageType() == messageType)
        {
            handler->handle(messageType, from, message);
        }
    }
}
