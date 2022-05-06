#include <multiplayer.h>

void Multiplayer::manageCurrentWinner(uint8_t id, unsigned short pressingTime)
{
    if (Winner.time > pressingTime)
    {
        Winner.time = pressingTime;
        Winner.id = id;
    }
}

void Multiplayer::startGame()
{
    Serial.println("Gioco Iniziato!");
    Winner.time = SHRT_MAX;
    Winner.id = 0;
    this->resetButtonPressDelay();
    sendSwitchLightOn();
    matchStartTime = millis();
    setLightOn();
}

void Multiplayer::endGame()
{
    Serial.println("Gioco Finito!");
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
    drawDashboard(this->getDeviceID(), this->getPlayerPoints());
}
void Multiplayer::onWinnerResultsRecieved(uint8_t winnerID)
{
    checkResults(winnerID);
}
void Multiplayer::onDeviceIDRecieved()
{
    drawDashboard(this->getDeviceID(), this->getPlayerPoints());
}
void Multiplayer::onNewDeviceConnected(uint8_t deviceID)
{
    playerPoints[deviceID] = 0;
}
void Multiplayer::onTimeRecieved(uint8_t deviceID, short time)
{
    this->setDeviceButtonPressDelay(deviceID, time);
    manageCurrentWinner(deviceID, time);
}