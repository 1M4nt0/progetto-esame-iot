#include <multiplayer.h>

void Multiplayer::manageCurrentWinner(uint8_t id, unsigned short pressingTime)
{
    if (Winner.time > pressingTime)
    {
        Winner.time = pressingTime;
        Winner.id = id;
    }
}

uint8_t Multiplayer::getPlayerID()
{
    return this->playerID[this->getDeviceID()];
}
int Multiplayer::getPlayerPoints()
{
    return playerPoints[this->getPlayerID()];
}
void Multiplayer::incrementPlayerPoints(uint8_t playerID, int increment)
{
    playerPoints[this->getPlayerID()] = playerPoints[this->getPlayerID()] + increment;
}
uint8_t Multiplayer::getPlayerIDFromDevice(uint8_t deviceID)
{
    return playerID[deviceID];
}
void Multiplayer::startGame()
{
    Winner.time = SHRT_MAX;
    Winner.id = 0;
    sendSwitchLightOn();
    matchStartTime = millis();
    digitalWrite(LED_PIN, HIGH);
}
void Multiplayer::endGame()
{
    sendSwitchLightOff();
    digitalWrite(LED_PIN, LOW);
    if (Winner.id != 0)
    {
        sendWinner(Winner.id);
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
            digitalWrite(LED_PIN, LOW);
            onButtonPressed();
        }
    }
}
void Multiplayer::onButtonPressed()
{
    short pressingTime = millis() - matchStartTime;
    (this->getIsHost()) ? manageCurrentWinner(this->getPlayerID(), pressingTime) : this->sendTime(pressingTime);
}
void Multiplayer::checkResults(uint8_t winnerID)
{
    if (winnerID == this->getPlayerID())
    {
        drawToScreen("Hai vinto!");
        incrementPlayerPoints(this->getPlayerID(), 1);
    }
    else
    {
        drawToScreen("Hai perso!");
    }
    delay(2000);
    drawDashboard(this->getPlayerID(), this->getPlayerPoints());
}
void Multiplayer::onWinnerResultsRecieved(uint8_t winnerID)
{
    checkResults(winnerID);
}
void Multiplayer::onDeviceIDRecieved()
{
    playerID[this->getDeviceID()] = this->getDeviceID();
    drawDashboard(this->getPlayerID(), this->getPlayerPoints());
}
void Multiplayer::onNewDeviceConnected(uint8_t deviceID)
{
    playerID[deviceID] = deviceID;
    playerPoints[deviceID] = 0;
}
void Multiplayer::onTimeRecieved(uint8_t deviceID, short time)
{
    manageCurrentWinner(playerID[deviceID], time);
}