#include <device.h>

Device::Device()
{
    initDisplay();
    drawToScreen("Avvio...");
    pinMode(BUTTON_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    this->_deviceSocket = new DeviceSocket();
    initOTA();
    this->setDefaultHandlers();
    if (this->isHost())
    {
        this->_deviceID = 0;
    }
}

void Device::initOTA()
{
    ArduinoOTA
        .onStart([&]()
                 { this->setLightOn(false); })
        .onEnd([]()
               { Serial.println("Aggiornamento completato!"); })
        .onProgress([](unsigned int progress, unsigned int total)
                    { Serial.printf("Progresso: %i%", progress / (total / 100)); })
        .onError([](ota_error_t error)
                 { Serial.println("Errore di aggiornamento"); });
    ArduinoOTA.begin();
}

void Device::setDeviceID(uint8_t deviceID)
{
    this->_deviceID = deviceID;
}

uint8_t Device::getDeviceID()
{
    return this->_deviceID;
}

void Device::setLightOn(bool on)
{
    this->_lightON = on;
    digitalWrite(LED_PIN, (on) ? HIGH : LOW);
}

void Device::loop()
{
    ArduinoOTA.handle();
    this->_deviceSocket->loop();
    if (digitalRead(BUTTON_PIN) == LOW)
    {
        this->_buttonPressed = true;
    }
    else
    {
        this->_buttonPressed = false;
    }
}

bool Device::isHost()
{
    return this->_deviceSocket->isHost();
}

DeviceSocket *Device::socket()
{
    return this->_deviceSocket;
}

void Device::setDefaultHandlers()
{
    this->_deviceSocket->on(WSHM_BIN, [&](WSH_Message msgType, uint8_t from, SocketDataMessage *message)
                            {
        switch (message->code)
        {
        case C_LIGHTS_ON:
            this->setLightOn(true);
            break;
        case C_LIGHTS_OFF:
            this->setLightOn(false);
            break;    
        default:
            break;
        }; });
}

void Device::sendSwitchLightOn()
{
    this->socket()->sendMessageAll(C_LIGHTS_ON);
}
void Device::sendSwitchLightOn(uint8_t deviceID)
{
    this->socket()->sendMessage(deviceID, C_LIGHTS_ON);
}
void Device::sendSwitchLightOff()
{
    this->socket()->sendMessageAll(C_LIGHTS_OFF);
}
void Device::sendSwitchLightOff(uint8_t deviceID)
{
    this->socket()->sendMessage(deviceID, C_LIGHTS_OFF);
}