#include <device.h>
#include <messageCodes.h>

Device::Device()
{
    pinMode(BUTTON_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    this->_deviceSocket = new DeviceSocket();
    initOTA();
    this->setDefaultHandlers();
    if (this->isHost())
    {
        this->_deviceID = 1;
    }
}

void Device::initOTA()
{
    ArduinoOTA
        .onStart([&]()
                 { this->setLight(false); })
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

void Device::setLight(bool on)
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
    this->_deviceSocket->on(T_BIN_MASSAGE, [&](uint8_t from, HandlerMsgType msgType, SocketDataMessage *message)
                            {
        switch (message->code)
        {
        case C_LIGHTS_ON:
            this->setLight(true);
            break;
        case C_LIGHTS_OFF:
            this->setLight(false);
            break;
        case C_DEVICE_ID:
        {
            if(!this->socket()->isHost()){
                this->_deviceID = message->payload[0];
            }
        }     
        default:
            break;
        }; });
}