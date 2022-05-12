#include <game.h>
#include <multiplayer.h>
#include <singleplayer.h>
#include <HTTPClient.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsClient.h>

#define C_NEW_GAMEMODE 6
#define C_PAUSE 7

class GameManager
{
private:
    Game *_game;
    AsyncWebServer *_server;
    WebSocketsClient *_webSocketClient;
    AsyncWebSocket *_ws;
    int _currentGameID = 0;
    std::vector<uint8_t> connectedDevicesID;
    bool _paused = true;
    bool _isHost;
    void initClient();
    void initServer();
    void webSocketClientEvent(WStype_t type, uint8_t *payload, size_t length);
    void webSocketServerEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
    void sendDeviceID(uint32_t clientID, uint8_t deviceID);
    void sendChangeGame(uint8_t gameID);
    void sendChangeGame(uint8_t gameID, uint8_t deviceID);
    void sendPause(bool pause);

public:
    GameManager(bool isHost);
    void loop();
    int getCurrentGame();
    void initializeGame(int gameID);
    void setPaused(bool pause);
    bool isPaused();
};