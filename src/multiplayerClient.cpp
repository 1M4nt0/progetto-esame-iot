#include <multiplayerClient.h>

MultiplayerClient::MultiplayerClient(Device *device) : Game(device)
{
    this->device->socket()->on(DSM_GAME, WSHM_BIN, [&](WSH_Message msgType, uint8_t from, SocketDataMessage *message)
                               {
                                   Serial.printf("Message from: %i\ncode: %i\n\n", from,message->code);
        switch (message->code)
        {
        case C_PLAYER_ID:
        {
            uint8_t playerID = message->payload[0];
            this->_setPlayerID(playerID);
            drawDashboard(this->_playerID, this->getPlayerPoints(this->_playerID));
            break;
        }
        case C_WINNER:
        {
            this->incrementPlayerPoints(message->payload[0], 1);
            this->_displayResults(message->payload[0] == this->_playerID);
            break;
        }
        default:
            break;
        }; });
}

void MultiplayerClient::loop()
{
    if (this->device->isLightOn())
    {
        if (this->device->isButtonPressed())
        {
            this->onButtonPressed();
        }
    }
}

void MultiplayerClient::initalize()
{
}

void MultiplayerClient::start()
{
}

void MultiplayerClient::end()
{
}

void MultiplayerClient::_sendButtonPressDelay(unsigned short buttonPressDelay)
{
    uint8_t payload[2];
    memcpy(payload, &buttonPressDelay, 2);
    this->device->socket()->sendMessage(0, C_TIME, payload, sizeof(short));
}

void MultiplayerClient::onButtonPressed()
{
    unsigned long timeButtonPressed = millis();
    this->device->setLightOn(false);
    unsigned short buttonPressDelay = timeButtonPressed - this->_lightOnTime;
    this->_sendButtonPressDelay(buttonPressDelay);
}

void MultiplayerClient::onLightOn()
{
    if (this->_playerID > 0)
    {
        this->_lightOnTime = millis();
        drawDashboard(this->_playerID, this->getPlayerPoints(this->_playerID));
    }
}

void MultiplayerClient::_displayResults(bool isWinner)
{
    if (isWinner)
    {
        drawToScreen("Hai vinto!");
    }
    else
    {
        drawToScreen("Hai perso!");
    }
}