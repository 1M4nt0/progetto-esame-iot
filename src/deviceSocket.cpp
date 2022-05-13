#include <deviceSocket.h>

uint8_t packetBuffer[3]; // buffer to hold packets

DeviceSocket::DeviceSocket()
{
    connectToWifi();
    SPIFFS.begin(true);
    this->_server = new AsyncWebServer(80);
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

bool DeviceSocket::isHost()
{
    return this->_isHost;
}

void DeviceSocket::connectToWifi()
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 7);
    int status = WiFi.status();
    while (status != WL_CONNECTED && status != WL_NO_SSID_AVAIL)
    {
        delay(2000);
        status = WiFi.status();
    }
    if (status == WL_NO_SSID_AVAIL)
    {
        WiFi.mode(WIFI_MODE_AP);
        WiFi.softAP(WIFI_SSID, WIFI_PASSWORD, 7, 0);
        this->_isHost = true;
    }
    else
    {
        this->_isHost = false;
    }
}

void DeviceSocket::loop()
{
    if (WiFi.status() != WL_CONNECTED && !this->_isHost)
    {
        delay(random(1000, 10000));
        connectToWifi();
    }
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
    memset(&packetBuffer, messageCode, sizeof(uint8_t));
    if (deviceID == 0)
    {
        this->_socketClient->sendBIN(packetBuffer, sizeof(uint8_t));
    }
    else
    {
        this->_socketHost->binary(deviceID, packetBuffer, sizeof(uint8_t));
    }
}

void DeviceSocket::sendMessage(uint8_t deviceID, uint8_t messageCode, uint8_t *payload, int len)
{
    memset(packetBuffer, messageCode, sizeof(uint8_t));
    memcpy(packetBuffer + 1, payload, len);
    if (deviceID == 0)
    {
        this->_socketClient->sendBIN(packetBuffer, sizeof(uint8_t) + len);
    }
    else
    {
        this->_socketHost->binary(deviceID, packetBuffer, sizeof(uint8_t) + len);
    }
}

void DeviceSocket::sendMessageAll(uint8_t messageCode)
{
    memset(packetBuffer, messageCode, sizeof(uint8_t));
    if (this->_isHost)
    {
        this->_socketHost->binaryAll(packetBuffer, sizeof(uint8_t));
    }
}

void DeviceSocket::sendMessageAll(uint8_t messageCode, uint8_t *payload, int len)
{
    memset(packetBuffer, messageCode, sizeof(uint8_t));
    memcpy(packetBuffer + 1, payload, len);
    if (this->_isHost)
    {
        this->_socketHost->binaryAll(packetBuffer, sizeof(uint8_t) + len);
    }
}

void DeviceSocket::on(WSH_Event event, SocketEventCallback onEvent)
{
    SocketMessageHandler *handler = new SocketMessageHandler();
    handler->setEventType(event);
    handler->onEvent(onEvent);
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

void DeviceSocket::webSocketClientEvent(WStype_t type, uint8_t *payload, size_t length)
{
    if (type == WStype_DISCONNECTED)
    {
        this->handle(WSHE_SOCKET_DISCONNECTED);
    }
    else if (type == WStype_CONNECTED)
    {
        this->handle(WSHE_SOCKET_CONNECTED);
    }
    else if (type == WStype_BIN)
    {
        SocketDataMessage *data = new SocketDataMessage{payload[0], payload + 1, length - 1};
        this->handle(WSHM_BIN, 0, data);
    }
    else if (type == WStype_ERROR)
    {
        this->handle(WSHE_ERROR);
        Serial.println("Error in communication!s");
    }
}

void DeviceSocket::webSocketServerEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *payload, size_t length)
{
    if (type == WS_EVT_CONNECT) // CLIENT CONNECTED
    {
        if (_connectedDevicesID.size() == MAX_CONNECTED_DEVICES)
        {
            client->close();
        }
        else
        {
            _connectedDevicesID.push_back(client->id());
            this->handle(WSHM_CONNECTED, client->id(), nullptr);
        }
    }
    else if (type == WS_EVT_DISCONNECT) // CLIENT DISCONNECTED
    {
        _connectedDevicesID.erase(remove(_connectedDevicesID.begin(), _connectedDevicesID.end(), client->id()), _connectedDevicesID.end());
        this->handle(WSHM_DISCONNECTED, client->id(), nullptr);
    }
    else if (type == WS_EVT_DATA) // RECEIVED DATA
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->final && info->index == 0 && info->len == length && info->opcode == WS_BINARY)
        {
            SocketDataMessage *data = new SocketDataMessage{payload[0], payload + 1, length - 1};
            this->handle(WSHM_BIN, client->id(), data);
        }
    }
    else if (type == WS_EVT_ERROR)
    {
        this->handle(WSHE_ERROR);
        Serial.println("Error in communication!");
    }
}