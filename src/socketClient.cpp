#include <socketClient.h>

SocketClient::SocketClient() : DeviceSocket()
{
    SPIFFS.begin(true);
    this->_server = new AsyncWebServer(80);
    this->_server->begin();
    this->_initSocket();
}

void SocketClient::_initSocket()
{
    this->_socketClient = new WebSocketsClient();
    this->_socketClient->begin("192.168.4.1", 80, "/ws");
    this->_socketClient->onEvent([&](WStype_t type, uint8_t *payload, size_t length)
                                 { this->_webSocketEvent(type, payload, length); });
    this->_socketClient->setReconnectInterval(5000);
}

void SocketClient::loop()
{
    this->_socketClient->loop();
}

void SocketClient::sendMessage(uint8_t deviceID, uint8_t messageCode)
{
    uint8_t packetBuffer[3]; // buffer to hold packets
    memset(&packetBuffer, messageCode, sizeof(uint8_t));
    this->_socketClient->sendBIN(packetBuffer, sizeof(uint8_t));
}

void SocketClient::sendMessage(uint8_t deviceID, uint8_t messageCode, uint8_t *payload, int len)
{
    uint8_t packetBuffer[3]; // buffer to hold packets
    memset(packetBuffer, messageCode, sizeof(uint8_t));
    memcpy(packetBuffer + 1, payload, len);
    this->_socketClient->sendBIN(packetBuffer, sizeof(uint8_t) + len);
}

void SocketClient::_webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
    if (type == WStype_DISCONNECTED)
    {
        this->handle(WSHE_SOCKET_DISCONNECTED, 0);
    }
    else if (type == WStype_CONNECTED)
    {
        this->handle(WSHE_SOCKET_CONNECTED, 0);
    }
    else if (type == WStype_BIN)
    {
        SocketDataMessage *data = new SocketDataMessage{payload[0], payload + 1, length - 1};
        this->handle(WSHM_BIN, 0, data);
    }
    else if (type == WStype_ERROR)
    {
        this->handle(WSHE_ERROR, 0);
        Serial.println("Error in communication!s");
    }
}