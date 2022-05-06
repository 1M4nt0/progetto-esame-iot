#include <multiplayer.h>

void Multiplayer::setup(bool isHost)
{
    if (isHost)
    {
        drawDashboard(this->getDeviceID(), this->getPlayerPoints(this->getDeviceID()));
    }
}

void Multiplayer::manageCurrentWinner(uint8_t id, unsigned short pressingTime)
{
    if (Winner.time > pressingTime)
    {
        Winner.time = pressingTime;
        Winner.id = id;
    }
}

void Multiplayer::configServerEndpoints(AsyncWebServer *_server)
{
    _server->on("/", HTTP_ANY, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/index.html"); });
    _server->on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/style.css", "text/css"); });
    _server->on("/functions.js", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/functions.js", "text/js"); });
    _server->on("/points", HTTP_GET, [&](AsyncWebServerRequest *request)
                { 
              AsyncJsonResponse* response = new AsyncJsonResponse();
              response->addHeader("Server","ESP Async Web Server");
              const JsonObject& jsonData = response->getRoot();
              int index = 0;
              for (uint8_t deviceID : this->getConnectedDevicesID())
              {
                jsonData["players"][index]["id"] = deviceID;
                jsonData["players"][index]["points"] = this->getPlayerPoints(deviceID);
                jsonData["players"][index]["time"] = this->getDeviceButtonPressDelay(deviceID);
                index++;
              }
              response->setLength();
              request->send(response); });
}

void Multiplayer::startGame()
{
    Winner.time = SHRT_MAX;
    Winner.id = 0;
    this->resetButtonPressDelay();
    sendSwitchLightOn();
    matchStartTime = millis();
    setLightOn();
}

void Multiplayer::endGame()
{
    sendSwitchLightOff();
    setLightOff();
    if (Winner.id > 0)
    {
        sendWinner(Winner.id, true);
        checkResults(Winner.id);
    }
}
void Multiplayer::gameLoop()
{
    if (this->getIsHost() && millis() > matchStartTime + matchDuration)
    {
        this->endGame();
        delay(random(2000, 8000));
        this->startGame();
    }
    if (this->getIsLightOn())
    {
        if (digitalRead(BUTTON_PIN) == LOW)
        {
            onButtonPressed();
        }
    }
}
void Multiplayer::onButtonPressed()
{
    long buttonPressDelay = setLightOff();
    this->setDeviceButtonPressDelay(this->getDeviceID(), buttonPressDelay);
    (this->getIsHost()) ? manageCurrentWinner(this->getDeviceID(), buttonPressDelay) : this->sendTime(buttonPressDelay);
}
void Multiplayer::checkResults(uint8_t winnerID)
{
    if (winnerID == this->getDeviceID())
    {
        drawToScreen("Hai vinto!");
        incrementPlayerPoints(this->getDeviceID(), 1);
    }
    else
    {
        drawToScreen("Hai perso!");
        incrementPlayerPoints(winnerID, 1);
    }
    delay(2000);
    drawDashboard(this->getDeviceID(), this->getPlayerPoints(this->getDeviceID()));
}
void Multiplayer::onWinnerResultsRecieved(uint8_t winnerID)
{
    checkResults(winnerID);
}
void Multiplayer::onDeviceIDRecieved()
{
    drawDashboard(this->getDeviceID(), this->getPlayerPoints(this->getDeviceID()));
}
void Multiplayer::onNewDeviceConnected(uint8_t deviceID)
{
    this->resetPlayerPoints(deviceID);
}
void Multiplayer::onTimeRecieved(uint8_t deviceID, short time)
{
    this->setDeviceButtonPressDelay(deviceID, time);
    manageCurrentWinner(deviceID, time);
}

void Multiplayer::setDeviceButtonPressDelay(uint8_t deviceID, short time)
{
    this->deviceButtonPressDelay[deviceID] = time;
}

short Multiplayer::getDeviceButtonPressDelay(uint8_t deviceID)
{
    return this->deviceButtonPressDelay[deviceID];
}

void Multiplayer::resetButtonPressDelay()
{
    this->deviceButtonPressDelay.clear();
}