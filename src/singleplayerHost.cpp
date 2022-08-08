#include <singleplayerHost.h>

SingleplayerHost::SingleplayerHost(Device *device) : Game(device)
{
    this->_canRestart = false;
    this->_byAttempting = false;
    this->_resetPlayersButtonDelaysVector();
    this->_nextRestartTime = millis() + 5000; // Due secondo e incomincia il gioco
    this->device->socket()->on(DSM_GAME, WSHM_BIN, [&](WSH_Message msgType, uint8_t from, SocketDataMessage *message)
                               {
                                   //Serial.printf("Message from: %i\ncode: %i\n\n", from,message->code);
        switch (message->code)
        {
        case C_TIME:
        {
            unsigned short time;
            memcpy(&time, message->payload, sizeof(short));
            this->_playerButtonPressDelays[this->_currentPlayer][this->_numberOfAttempts] = MAX_LIGHT_ON_TIME - time;
            break;
        }
        default:
            break;
        }; });
    this->_playersNumberHandler = this->device->webServer()->on("/players", HTTP_GET, std::bind(&SingleplayerHost::_handlePlayersNumberEndpointRequest, this, std::placeholders::_1));
}

SingleplayerHost::~SingleplayerHost()
{
    this->device->webServer()->removeHandler(&this->_playersNumberHandler);
}

void SingleplayerHost::initalize()
{
    this->device->display()->drawToScreen("Giocatore " + String(this->_currentPlayer));
    this->device->socket()->sendMessageAll(C_LIGHTS_OFF);
    Winner.meanTime = SHRT_MAX;
    Winner.id = 0;
    this->_numberOfAttempts = 0;
}

void SingleplayerHost::start()
{
    this->_canRestart = false;
    this->_startAttempt();
};

void SingleplayerHost::end()
{
    this->_endAttempt();
    this->_checkIfCurrentWinner(this->_currentPlayer);
    const int rescaleIndex = MAX_LIGHT_ON_TIME / 1000;
    short totalPoints = this->_arrayTimeMean(this->_playerButtonPressDelays[this->_currentPlayer], MAX_NUMBER_OF_ATTEMPTS) / rescaleIndex;
    this->_playerButtonPressDelays[this->_currentPlayer][this->_numberOfAttempts];
    this->device->display()->drawTwoToScreen("Punteggio: ", String(totalPoints));
    delay(2000);
    if (this->_currentPlayer == this->_numberOfPlayers)
    {
        this->_currentPlayer = 1;
        this->incrementPlayerPoints(Winner.id, 1);
        this->device->display()->drawTwoToScreen("Vincitore: ", "Giocatore " + String(Winner.id));
        delay(2000);
        this->_resetPlayersButtonDelaysVector();
    }
    else
    {
        this->_currentPlayer += 1;
    }
    this->_canRestart = true;
    this->_nextRestartTime = millis() + 4000;
}

void SingleplayerHost::_endAttempt()
{
    (this->_deviceWithLightON == 0) ? this->device->setLightOn(false) : this->device->socket()->sendMessage(this->_deviceWithLightON, C_LIGHTS_OFF);
    this->_nextAttemptDelay = random(500, 4000);
    this->_numberOfAttempts += 1;
    this->_byAttempting = false;
}

void SingleplayerHost::_startAttempt()
{
    this->_deviceWithLightON = random(0, this->device->socket()->getConnectedDevicesIDVector().size() + 1);
    if (this->_deviceWithLightON == 0)
    {
        this->device->setLightOn(true);
        this->_lightOnTime = millis();
    }
    else
    {
        this->device->socket()->sendMessage(this->_deviceWithLightON, C_LIGHTS_ON);
    }
    this->_timeSinceLastDeviceLightOn = millis();
    this->_byAttempting = true;
}

void SingleplayerHost::loop()
{
    if (this->_canRestart && millis() > this->_nextRestartTime)
    {
        this->initalize();
        this->start();
    }
    if (this->_numberOfAttempts == MAX_NUMBER_OF_ATTEMPTS && this->_canRestart == false && millis() > this->_timeSinceLastDeviceLightOn + MAX_LIGHT_ON_TIME)
    {
        this->end();
    }
    else if (!this->_canRestart)
    {
        if (millis() > this->_timeSinceLastDeviceLightOn + MAX_LIGHT_ON_TIME && this->_byAttempting)
        {
            this->_endAttempt();
        }
        if (millis() > this->_timeSinceLastDeviceLightOn + MAX_LIGHT_ON_TIME + this->_nextAttemptDelay && !this->_byAttempting)
        {
            this->_startAttempt();
        }
    }
    if (this->device->isLightOn())
    {
        if (digitalRead(BUTTON_PIN) == LOW)
        {
            onButtonPressed();
        }
    }
};

short SingleplayerHost::_arrayTimeMean(short *timeArray, int length)
{
    short currentPlayerMean = 0;
    for (int i = 0; i < length; i++)
    {
        currentPlayerMean += timeArray[i];
    }
    return currentPlayerMean / length;
}

void SingleplayerHost::_resetPlayersButtonDelaysVector()
{
    for (int i = 1; i <= this->_numberOfPlayers; i++)
    {
        for (int j = 0; j < MAX_NUMBER_OF_ATTEMPTS; j++)
        {
            this->_playerButtonPressDelays[i][j] = 0;
        }
    }
}

void SingleplayerHost::_resetPlayersButtonDelaysVector(uint8_t playerID)
{
    for (int j = 0; j < MAX_NUMBER_OF_ATTEMPTS; j++)
    {
        this->_playerButtonPressDelays[playerID][j] = 0;
    }
}

void SingleplayerHost::_checkIfCurrentWinner(uint8_t playerID)
{
    short currentPlayerMean = this->_arrayTimeMean(this->_playerButtonPressDelays[playerID], MAX_NUMBER_OF_ATTEMPTS);
    if (Winner.meanTime > currentPlayerMean)
    {
        Winner.id = playerID;
        Winner.meanTime = currentPlayerMean;
    }
}

void SingleplayerHost::onButtonPressed()
{
    this->device->setLightOn(false);
    long buttonPressDelay = millis() - this->_lightOnTime;
    this->_playerButtonPressDelays[this->_currentPlayer][this->_numberOfAttempts] = MAX_LIGHT_ON_TIME - buttonPressDelay;
};

void SingleplayerHost::servePointsEndpoint(AsyncWebServerRequest *request)
{
    AsyncJsonResponse *response = new AsyncJsonResponse();
    const JsonObject &jsonData = response->getRoot();
    const int rescaleIndex = MAX_LIGHT_ON_TIME / 1000;
    int index = 0;
    for (int i = 1; i <= this->_numberOfPlayers; i++)
    {
        jsonData["players"][index]["id"] = i;
        jsonData["players"][index]["points"] = this->getPlayerPoints(i);
        jsonData["players"][index]["time"] = this->_arrayTimeMean(this->_playerButtonPressDelays[i], MAX_NUMBER_OF_ATTEMPTS) / rescaleIndex;
        index++;
    }
    response->setLength();
    request->send(response);
}

void SingleplayerHost::_handlePlayersNumberEndpointRequest(AsyncWebServerRequest *request)
{
    if (request->hasParam("number"))
    {
        String action = request->getParam("number")->value();
        if (action == "increase")
        {
            request->send(200, "text", "OK!");
            this->_numberOfPlayers++;
            this->_resetPlayersButtonDelaysVector(this->_numberOfPlayers);
        }
        else if (action == "decrease")
        {
            request->send(200, "text", "OK!");
            this->_numberOfPlayers--;
        }
    }
    else
    {
        AsyncJsonResponse *response = new AsyncJsonResponse();
        const JsonObject &jsonData = response->getRoot();
        jsonData["players"] = this->_numberOfPlayers;
        response->setLength();
        request->send(response);
    }
}