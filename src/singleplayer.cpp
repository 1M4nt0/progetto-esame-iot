#include <singleplayer.h>

short static arrayTimeMean(short *timeArray, int length)
{
    short currentPlayerMean = 0;
    for (int i = 0; i < length; i++)
    {
        currentPlayerMean += timeArray[i];
    }
    return currentPlayerMean / length;
}

void Singleplayer::startGame()
{
    if (this->getIsHost())
    {
        drawToScreen("Giocatore " + String(this->currentPlayer));
        Winner.meanTime = SHRT_MAX;
        Winner.id = 0;
        this->playerResponseDelays.clear();
        this->numberOfAttempts = 0;
        this->deviceWithLightON = random(1, this->getConnectedDevicesID()->size() + 1);
        delay(random(3000, 6500));
        (deviceWithLightON == 1) ? setLightOn() : this->sendSwitchLightOn(this->deviceWithLightON);
        this->timeSinceLastDeviceLightOn = millis();
    }
    else
    {
        drawToScreen("...");
    }
};
void Singleplayer::endGame()
{

    if (this->deviceWithLightON == 1)
    {
        this->setLightOff();
    }
    else
    {
        this->sendSwitchLightOff(this->deviceWithLightON);
    }
    this->checkIfCurrentWinner(this->currentPlayer);
    if (this->getIsHost())
    {
        short timeMean = arrayTimeMean(this->playerResponseDelays[this->currentPlayer], MAX_NUMBER_OF_ATTEMPTS);
        drawTwoToScreen("Punteggio: ", String(MAX_LIGHT_ON_TIME - timeMean));
        delay(2000);
    }

    if (this->currentPlayer == numberOfPlayers)
    {
        this->currentPlayer = 1;
        this->incrementPlayerPoints(Winner.id, 1);
        drawTwoToScreen("Vincitore: ", "Giocatore " + String(Winner.id));
        delay(2000);
    }
    else
    {
        this->currentPlayer += 1;
    }
};

void Singleplayer::gameLoop()
{
    if (this->getIsHost())
    {
        if (this->numberOfAttempts < MAX_NUMBER_OF_ATTEMPTS)
        {
            if (millis() > this->timeSinceLastDeviceLightOn + MAX_LIGHT_ON_TIME)
            {
                if (this->deviceWithLightON == 1)
                {
                    this->setLightOff();
                }
                else
                {
                    this->sendSwitchLightOff(this->deviceWithLightON);
                }
                if (this->playerResponseDelays[this->currentPlayer][this->numberOfAttempts] == 0)
                {
                    this->playerResponseDelays[this->currentPlayer][this->numberOfAttempts] = MAX_LIGHT_ON_TIME;
                }
                this->deviceWithLightON = random(1, this->getConnectedDevicesID()->size() + 1);
                this->numberOfAttempts += 1;
                delay(random(500, 4000));
                (deviceWithLightON == 1) ? setLightOn() : this->sendSwitchLightOn(this->deviceWithLightON);
                this->timeSinceLastDeviceLightOn = millis();
            }
        }
        else
        {
            this->endGame();
            delay(5000);
            this->startGame();
        }
    }
    if (this->getIsLightOn())
    {
        if (digitalRead(BUTTON_PIN) == LOW)
        {
            onButtonPressed();
        }
    }
};

void Singleplayer::onButtonPressed()
{
    long buttonPressDelay = setLightOff();
    (this->getIsHost()) ? managePressingTime(this->getDeviceID(), buttonPressDelay) : this->sendTime(buttonPressDelay);
};

void Singleplayer::onTimeRecieved(uint8_t deviceID, short time)
{
    managePressingTime(deviceID, time);
}

void Singleplayer::checkIfCurrentWinner(uint8_t playerID)
{
    short currentPlayerMean = arrayTimeMean(this->playerResponseDelays[playerID], MAX_NUMBER_OF_ATTEMPTS);
    if (Winner.meanTime > currentPlayerMean)
    {
        Winner.id = playerID;
        Winner.meanTime = currentPlayerMean;
    }
}

void Singleplayer::managePressingTime(uint8_t deviceID, short time)
{
    this->playerResponseDelays[this->currentPlayer][this->numberOfAttempts] = time;
}

void Singleplayer::configServerEndpoints(AsyncWebServer *_server)
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
            for(int i = 0; i < this->numberOfPlayers; i++){
                uint8_t playerID = i + 1;
                jsonData["players"][i]["id"] = playerID;
                jsonData["players"][index]["points"] = this->getPlayerPoints(playerID);
                if(playerID == this->currentPlayer && this->numberOfAttempts < MAX_NUMBER_OF_ATTEMPTS){
                    jsonData["players"][i]["time"] = arrayTimeMean(this->playerResponseDelays[playerID], this->numberOfAttempts);
                }else{
                    jsonData["players"][i]["time"] = arrayTimeMean(this->playerResponseDelays[playerID], MAX_NUMBER_OF_ATTEMPTS);
                }
            }
            response->setLength();
            request->send(response); });
}