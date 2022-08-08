#include <deviceSocket.h>

DeviceSocket::~DeviceSocket()
{
    delete this->_server;
    this->clearCoreHandlers();
    this->clearGameHandlers();
}

void DeviceSocket::clearGameHandlers()
{
    for (auto handler : this->_handlers)
    {
        delete handler;
    }
    this->_handlers.clear();
}

void DeviceSocket::clearCoreHandlers()
{
    for (auto handler : this->_coreHandlers)
    {
        delete handler;
    }
    this->_coreHandlers.clear();
}

void DeviceSocket::on(DSM_Type type, WSH_Event event, SocketEventCallback onEvent)
{
    SocketMessageHandler *handler = new SocketMessageHandler();
    handler->setEventType(event);
    handler->onEvent(onEvent);
    this->_pushHandler(type, handler);
}

void DeviceSocket::on(DSM_Type type, WSH_Event event, SocketEventFromCallback onEventFrom)
{
    SocketMessageHandler *handler = new SocketMessageHandler();
    handler->setEventType(event);
    handler->onEventFrom(onEventFrom);
    this->_pushHandler(type, handler);
}

void DeviceSocket::on(DSM_Type type, WSH_Message messageType, SocketMessageCallback onMessage)
{
    SocketMessageHandler *handler = new SocketMessageHandler();
    handler->setEventType(WSHE_MESSAGE);
    handler->setMessageType(messageType);
    handler->onMessage(onMessage);
    this->_pushHandler(type, handler);
}

void DeviceSocket::_pushHandler(DSM_Type type, SocketMessageHandler *handler)
{
    if (type == DSM_CORE)
    {
        _coreHandlers.push_back(handler);
    }
    else
    {
        _handlers.push_back(handler);
    }
}

void DeviceSocket::handle(WSH_Event event)
{
    for (SocketMessageHandler *handler : _coreHandlers)
    {
        if (handler->getEventType() == event)
        {
            handler->handle(event);
        }
    }
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
    for (SocketMessageHandler *handler : _coreHandlers)
    {
        if (handler->getEventType() == event)
        {
            handler->handle(event, from);
        }
    }
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
    for (SocketMessageHandler *handler : _coreHandlers)
    {
        if (handler->getEventType() == WSHE_MESSAGE && handler->getMessageType() == messageType)
        {
            handler->handle(messageType, from, message);
        }
    }
    for (SocketMessageHandler *handler : _handlers)
    {
        if (handler->getEventType() == WSHE_MESSAGE && handler->getMessageType() == messageType)
        {
            handler->handle(messageType, from, message);
        }
    }
}
