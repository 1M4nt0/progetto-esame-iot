#include <gameManager.h>
#include <HTTPClient.h>

GameManager::GameManager(bool isHost)
{
    this->_server = new AsyncWebServer(80);
    this->_isHost = isHost;
    if (this->_isHost)
    {
        http = new HTTPClient();
        this->_server->on("/gamemode", HTTP_GET, [&](AsyncWebServerRequest *request)
                          { 
                if(request->hasParam("id")){
                int gameID = request->getParam("id")->value().toInt();
                if(gameID != this->_currentGameID){
                  drawToScreen("Cambio gioco...");
                  this->initializeGame(gameID);
                }}else{
                  request->send(200, "text", String(this->_currentGameID));
                } });
    }
    this->_server->begin();
};

int GameManager::getCurrentGame()
{
    return this->_currentGameID;
}

void GameManager::initializeGame(int gameID)
{
    this->_currentGameID = gameID;
    if (this->_currentGameID == 0)
    {
        _game = new Multiplayer(this->_server);
    }
    else if (this->_currentGameID == 1)
    {
        _game = new Singleplayer(this->_server);
    }
    else
    {
        Serial.println("NON ESISTE QUESTA MODALITA");
    }
    (this->_isHost) ? _game->init(true) : _game->init(false);
    if (this->_isHost)
    {
        _game->startGame();
    }
}

void GameManager::loop()
{
    if (!this->_paused)
    {
        _game->loop();
    }
}

void GameManager::setPaused(bool pause)
{
    this->_paused = pause;
}

bool GameManager::isPaused()
{
    return this->_paused;
}

void GameManager::getConfigFromHost()
{
    String serverPath = "http://192.168.4.1/gamemode";
    http->begin(serverPath);
    int httpResponseCode = http->GET();
    if (httpResponseCode > 0)
    {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http->getString();
        Serial.println(payload);
    }
    else
    {
        delay(2000);
        getConfigFromHost();
    }
}