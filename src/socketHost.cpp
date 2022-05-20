#include <socketHost.h>

SocketHost::SocketHost()
{
    SPIFFS.begin(true);
    this->_server = new AsyncWebServer(80);
    this->_server->begin();
    this->_initSocket();
}

void SocketHost::_initSocket()
{
    this->_socketHost = new AsyncWebSocket("/ws");
    this->_socketHost->onEvent([&](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
                               { this->_webSocketEvent(server, client, type, arg, data, len); });
    this->_server->addHandler(this->_socketHost);
}

void SocketHost::loop()
{
    this->_socketHost->cleanupClients();
}

void SocketHost::sendMessage(uint8_t deviceID, uint8_t messageCode)
{
    uint8_t packetBuffer[3]; // buffer to hold packets
    memset(&packetBuffer, messageCode, sizeof(uint8_t));
    this->_socketHost->binary(deviceID, packetBuffer, sizeof(uint8_t));
}

void SocketHost::sendMessage(uint8_t deviceID, uint8_t messageCode, uint8_t *payload, int len)
{
    uint8_t packetBuffer[3]; // buffer to hold packets
    memset(packetBuffer, messageCode, sizeof(uint8_t));
    memcpy(packetBuffer + 1, payload, len);
    this->_socketHost->binary(deviceID, packetBuffer, sizeof(uint8_t) + len);
}

void SocketHost::sendMessageAll(uint8_t messageCode)
{
    uint8_t packetBuffer[3]; // buffer to hold packets
    memset(packetBuffer, messageCode, sizeof(uint8_t));
    this->_socketHost->binaryAll(packetBuffer, sizeof(uint8_t));
}

void SocketHost::sendMessageAll(uint8_t messageCode, uint8_t *payload, int len)
{
    uint8_t packetBuffer[3]; // buffer to hold packets
    memset(packetBuffer, messageCode, sizeof(uint8_t));
    memcpy(packetBuffer + 1, payload, len);
    this->_socketHost->binaryAll(packetBuffer, sizeof(uint8_t) + len);
}

void SocketHost::_webSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *payload, size_t length)
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
            this->handle(WSHE_SOCKET_CONNECTED, client->id());
        }
    }
    else if (type == WS_EVT_DISCONNECT) // CLIENT DISCONNECTED
    {
        _connectedDevicesID.erase(remove(_connectedDevicesID.begin(), _connectedDevicesID.end(), client->id()), _connectedDevicesID.end());
        this->handle(WSHE_SOCKET_DISCONNECTED, client->id());
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
        this->handle(WSHE_ERROR, client->id());
        Serial.println("Error in communication!");
    }
}