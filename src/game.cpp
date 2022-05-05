#include <vector>
#include <map>
#include <algorithm>
#include <Arduino.h>

class Game
{
public:
    Game(int LED_PIN, int BUTTON_PIN)
    {
        this->BUTTON_PIN = BUTTON_PIN;
        this->LED_PIN = LED_PIN;
        pinMode(this->BUTTON_PIN, INPUT);
        pinMode(this->LED_PIN, OUTPUT);
        this->isLightOn = false;
    }
    virtual void onStartMatch() = 0;
    virtual void onEndMatch() = 0;
    virtual void onTimeRecieved() = 0;
    virtual void onButtonPressed() = 0;
    virtual void sendTime() = 0;
    virtual void sendPlayerID() = 0;
    virtual void sendSwitchLightOn() = 0;
    virtual void sendSwitchLightOff() = 0;
    virtual void sendWinner() = 0;

    int getPlayerPoints(int playerID)
    {
        return playersPoints[playerID];
    }
    void incrementPlayerPoints(int playerID, int increment)
    {
        playersPoints[playerID] += increment;
    }
    bool getIsLightOn()
    {
        return isLightOn;
    }
    void setPlayerID(int playerID)
    {
        this->playerID = playerID;
    }
    void onNewPlayerConnected(int playerID, void (*callback)())
    {
        connectedPlayersID.push_back(playerID);
        callback();
    }
    void onPlayerDisconnected(int playerID, void (*callback)())
    {
        connectedPlayersID.erase(remove(connectedPlayersID.begin(), connectedPlayersID.end(), playerID), connectedPlayersID.end());
        callback();
    }
    void onPlayerIDRecieved(int playerID)
    {
        setPlayerID(playerID);
    }
    void onSwitchLightOnReciveved(void (*callback)())
    {
        digitalWrite(LED_PIN, HIGH);
        callback();
    }
    void onSwitchLightOffReciveved(void (*callback)())
    {
        digitalWrite(LED_PIN, LOW);
        callback();
    }
    void onWinnerResultsRecieved(int winnerID, void (*callbackWinner)(), void (*callbackLooser)())
    {
        if (winnerID == this->playerID)
        {
            callbackWinner();
        }
        else
        {
            callbackLooser();
        }
    }

private:
    std::vector<uint8_t> connectedPlayersID;
    std::map<int, int> playersPoints;
    int LED_PIN;
    int BUTTON_PIN;
    int playerID;
    bool isLightOn;
};