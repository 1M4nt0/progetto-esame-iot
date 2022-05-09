#include <game.h>
#include <multiplayer.h>
#include <singleplayer.h>
#include <HTTPClient.h>

class GameManager
{
private:
    int _currentGameID = 0;
    Game *_game;
    AsyncWebServer *_server;
    HTTPClient *http;
    bool _paused = true;
    bool _isHost;

public:
    GameManager(bool isHost);
    void loop();
    int getCurrentGame();
    void initializeGame(int gameID);
    void setPaused(bool pause);
    bool isPaused();
    void getConfigFromHost();
};