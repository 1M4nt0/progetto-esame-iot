#include <deviceSocket.h>

DeviceSocket::DeviceSocket(bool *isHost)
{
    this->_isHost = isHost;
    this->_server = new AsyncWebServer(80);
    SPIFFS.begin(true);
    this->_server->begin();
    if (this->_isHost)
    {
        this->_socketHost = new AsyncWebSocket("/ws");
        this->_socketHost->onEvent([&](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
                                   { webSocketServerEvent(server, client, type, arg, data, len); });
        _server->addHandler(this->_socketHost);
    }
    else
    {
        this->_socketClient = new WebSocketsClient();
        this->_socketClient->begin("192.168.4.1", 80, "/ws");
        this->_socketClient->onEvent([&](WStype_t type, uint8_t *payload, size_t length)
                                     { webSocketClientEvent(type, payload, length); });
        this->_socketClient->setReconnectInterval(5000);
    }
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

void DeviceSocket::on(HandlerMsgType messageType, SocketHandlerFunction onMessage)
{
    SocketMessageHandler *handler = new SocketMessageHandler();
    handler->setMessageType(messageType);
    handler->onMessageRecieved(onMessage);
    _handlers.push_back(handler);
}

void DeviceSocket::handle(HandlerMsgType type, uint8_t from, uint8_t *payload, size_t len)
{
    for (SocketMessageHandler *handler : _handlers)
    {
        if (handler->getMessageType() == type)
        {
            handler->handleMessage(from, payload, len);
        }
    }
}

void DeviceSocket::webSocketClientEvent(WStype_t type, uint8_t *payload, size_t length)
{
    if (type == WStype_DISCONNECTED)
    {
        this->handle(T_DISCONNECTED, 0, payload, length);
    }
    else if (type == WStype_CONNECTED)
    {
        this->handle(T_CONNECTED, 0, payload, length);
    }
    else if (type == WStype_BIN)
    {
        this->handle(T_BIN_MASSAGE, 0, payload, length);
    }
    else if (type == WStype_ERROR)
    {
        Serial.println("Error in communication!s");
    }
}

void DeviceSocket::webSocketServerEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT) // CLIENT CONNECTED
    {
        _connectedDevicesID.push_back(client->id());
        this->handle(T_CONNECTED, client->id(), data, len);
    }
    else if (type == WS_EVT_DISCONNECT) // CLIENT DISCONNECTED
    {
        _connectedDevicesID.erase(remove(_connectedDevicesID.begin(), _connectedDevicesID.end(), client->id()), _connectedDevicesID.end());
        this->handle(T_DISCONNECTED, client->id(), data, len);
    }
    else if (type == WS_EVT_DATA) // RECEIVED DATA
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_BINARY)
        {
            this->handle(T_BIN_MASSAGE, client->id(), data, len);
        }
    }
    else if (type == WS_EVT_ERROR)
    {
        Serial.println("Error in communication!");
    }
}