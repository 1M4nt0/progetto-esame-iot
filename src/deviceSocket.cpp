#include <deviceSocket.h>

DeviceSocket::DeviceSocket(bool *isHost)
{
    this->_isHost = isHost;
}

void DeviceSocket::initAsHost(AsyncWebSocket *_socketHost)
{
    this->_socketHost = _socketHost;
}

void DeviceSocket::initAsClient(WebSocketsClient *_socketClient)
{
    this->_socketClient = _socketClient;
}

void DeviceSocket::loop()
{
    if (!this->_isHost)
    {
        this->_socketClient->loop();
    }
    else
    {
        this->_socketHost->cleanupClients();
    }
}

void DeviceSocket::sendMessage(uint8_t deviceID, uint8_t messageCode)
{
    memset(&dataBuffer, messageCode, sizeof(uint8_t));
    if (deviceID == 0)
    {
        this->_socketClient->sendBIN(dataBuffer, sizeof(uint8_t));
    }
    else
    {
        this->_socketHost->binary(deviceID, dataBuffer, sizeof(uint8_t));
    }
}

void DeviceSocket::sendMessage(uint8_t deviceID, uint8_t messageCode, uint8_t *payload, int len)
{
    memset(dataBuffer, messageCode, sizeof(uint8_t));
    memcpy(dataBuffer + 1, &payload, len);
    if (deviceID == 0)
    {
        this->_socketClient->sendBIN(dataBuffer, sizeof(uint8_t));
    }
    else
    {
        this->_socketHost->binary(deviceID, dataBuffer, sizeof(uint8_t) + len);
    }
}

void DeviceSocket::sendMessageAll(uint8_t messageCode)
{
    memset(dataBuffer, messageCode, sizeof(uint8_t));
    if (this->_isHost)
    {
        this->_socketHost->binaryAll(dataBuffer, sizeof(uint8_t));
    }
}

void DeviceSocket::sendMessageAll(uint8_t messageCode, uint8_t *payload, int len)
{
    memset(dataBuffer, messageCode, sizeof(uint8_t));
    memcpy(dataBuffer + 1, &payload, len);
    if (this->_isHost)
    {
        this->_socketHost->binaryAll(dataBuffer, sizeof(uint8_t) + len);
    }
}

void DeviceSocket::addHandler(uint8_t messageCode, std::function<void()> onMessage)
{
    SocketMessageHandler *handler = new SocketMessageHandler();
    handler->setMessageCode(messageCode);
    handler->onMessageRecieved(onMessage);
    _handlers.push_back(handler);
}