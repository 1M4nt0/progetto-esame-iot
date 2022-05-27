#include <device.h>

Device::Device()
{
    this->_deviceDisplay = new DeviceDisplay();
    this->display()->drawToScreen("Avvio...");
    pinMode(BUTTON_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    this->connectToWifi();
    initOTA();
    this->setDefaultHandlers();
    if (this->isHost())
    {
        this->_deviceID = 0;
    }
    this->socket()->on(DSM_CORE, WSHE_WIFI_DISCONNECTED, [&](WSH_Event event)
                       { this->setLightOn(false); });
}

Device::~Device()
{
    delete this->_deviceDisplay;
    delete this->_deviceSocket;
}

void Device::connectToWifi()
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 7);
    int status = WiFi.status();
    if (this->_deviceSocket != nullptr)
    {
        delete this->_deviceSocket;
    }
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
        this->_deviceSocket = new SocketHost();
    }
    else
    {
        this->_isHost = false;
        this->_deviceSocket = new SocketClient();
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

DeviceDisplay *Device::display()
{
    return this->_deviceDisplay;
}

DeviceSocket *Device::socket()
{
    return this->_deviceSocket;
}

AsyncWebServer *Device::webServer()
{
    return this->_deviceSocket->webServer();
}

void Device::setDefaultHandlers()
{
    this->_deviceSocket->on(DSM_CORE, WSHM_BIN, [&](WSH_Message msgType, uint8_t from, SocketDataMessage *message)
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