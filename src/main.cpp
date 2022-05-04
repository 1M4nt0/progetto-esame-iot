#include "board_conf.h"
#include "WiFi.h"
#include "SPI.h"
#include "screen_utils.h"
#include <ArduinoOTA.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsClient.h>
#include <vector>
#include <string>
#include "SPIFFS.h"
#include <map>
#include "AsyncJson.h"
#include <ArduinoJson.h>

#define C_PLAYER_ID 1
#define C_BEGIN_MATCH 2
#define C_END_MATCH 3
#define C_TIME 4
#define C_WINNER 5
#define MATCH_DURATION 7000
#define MAX_CONNECTED_DEVICES 8

#define BUTTON_PIN 16
#define LED_PIN 17

const char *WIFI_SSID = "IoTGame";
const char *WIFI_PASSWORD = "testpassword";
uint8_t packetBuffer[3]; // buffer to hold packets

bool master = false;
int playerID = 0;
int matchStartTime = 0;
int playerPoints = 0;
bool light = false;

AsyncWebServer server(80);
WebSocketsClient webSocketClient;
AsyncWebSocket ws("/ws");

std::vector<uint8_t> connectedPlayersID;
std::map<int, int> playersPoints;

struct
{
  uint8_t id = 0;
  unsigned short time = MATCH_DURATION;
} Winner;

void manageResults(uint8_t winnerID)
{
  if (winnerID == playerID)
  {
    drawToScreen("Hai vinto!");
    playerPoints++;
  }
  else
  {
    drawToScreen("Hai perso!");
  }
  playersPoints[winnerID] = playersPoints[winnerID] + 1;
  delay(2000);
  drawDashboard(playerID, playerPoints);
}

void handlePlayerConnected(uint8_t clientID)
{
  connectedPlayersID.push_back(clientID);
  memset(packetBuffer, (uint8_t)C_PLAYER_ID, sizeof(uint8_t));
  memcpy(packetBuffer + 1, &clientID, sizeof(uint8_t));
  ws.binary(clientID, packetBuffer, sizeof(uint8_t) + sizeof(uint8_t));
}

void handlePlayerDisconnected(uint8_t clientID)
{
  connectedPlayersID.erase(remove(connectedPlayersID.begin(), connectedPlayersID.end(), clientID), connectedPlayersID.end());
}

void manageWinner(uint8_t id, unsigned short pressingTime)
{
  if (Winner.time > pressingTime)
  {
    Winner.time = pressingTime;
    Winner.id = id;
  }
}

void handleButtonPress(unsigned short pressingTime)
{
  if (master)
  {
    manageWinner(0, pressingTime);
  }
  else
  {
    memset(packetBuffer, (uint8_t)C_TIME, sizeof(uint8_t));
    memcpy(packetBuffer + 1, &pressingTime, sizeof(short));
    webSocketClient.sendBIN(packetBuffer, sizeof(uint8_t) + sizeof(short));
  }
}

void handleTimeRecieved(uint8_t clientID, uint8_t *payload)
{
  unsigned short newTime;
  memcpy(&newTime, payload + 1, 2);
  manageWinner(clientID, newTime);
}

void handlePlayerIDRecieved(uint8_t *payload)
{
  uint8_t clientID;
  memcpy(&clientID, payload + 1, sizeof(uint8_t));
  playerID = clientID;
  drawDashboard(playerID, playerPoints);
}

void handleSendAccendi()
{
  digitalWrite(LED_PIN, HIGH);
  matchStartTime = millis();
}

void handleSendSpegni()
{
  digitalWrite(LED_PIN, LOW);
}

void handleResultsRecieved(uint8_t *payload)
{
  uint8_t winnerID;
  memcpy(&winnerID, payload + 1, sizeof(uint8_t));
  manageResults(winnerID);
}

void sendResults()
{
  memset(&packetBuffer, (uint8_t)C_WINNER, 1);
  memcpy(&packetBuffer + 1, &Winner.id, sizeof(uint8_t));
  ws.binaryAll(packetBuffer, sizeof(uint8_t) + sizeof(uint8_t));
}

void webSocketClientEvent(WStype_t type, uint8_t *payload, size_t length)
{

  switch (type)
  {
  case WStype_DISCONNECTED:
    break;
  case WStype_CONNECTED:
    break;
  case WStype_BIN:
  {
    int code = payload[0];
    if (code == C_PLAYER_ID)
    {
      handlePlayerIDRecieved(payload);
    }
    else if (code == C_BEGIN_MATCH)
    {
      Serial.println("Ricevuto accendi!");
      handleSendAccendi();
    }
    else if (code == C_END_MATCH)
    {
      handleSendSpegni();
    }
    else if (code == C_WINNER)
    {
      handleResultsRecieved(payload);
    }
  }
  break;
  case WStype_ERROR:
    break;
  }
}

void webSocketServerEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  if (type == WS_EVT_CONNECT)
  {
    // client connected
    handlePlayerConnected(client->id());
    ws.text(client->id(), String(client->id()));
    Serial.println("Client connesso");
  }
  else if (type == WS_EVT_DISCONNECT)
  {
    handlePlayerDisconnected(client->id());
    Serial.println("Client disconnesso");
  }
  else if (type == WS_EVT_ERROR)
  {
    // error was received from the other end
  }
  else if (type == WS_EVT_DATA)
  {
    // data packet
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len)
    {
      if (info->opcode == WS_BINARY)
      {
        uint8_t code = data[0];
        if (code == C_TIME)
        {
          handleTimeRecieved(client->id(), data);
        }
      }
    }
  }
}

void initServerSocket()
{
  if (!SPIFFS.begin(true))
  {
    Serial.println("Errore SPIFFS");
  }
  server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html"); });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/style.css", "text/css"); });
  server.on("/functions.js", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/functions.js", "text/js"); });
  server.on("/points", HTTP_GET, [](AsyncWebServerRequest *request)
            { 
              AsyncJsonResponse* response = new AsyncJsonResponse();
              response->addHeader("Server","ESP Async Web Server");
              const JsonObject& jsonPoints = response->getRoot();
              for (uint8_t playerID : connectedPlayersID)
              {
                jsonPoints["playerID"] = playerID;
                jsonPoints["points"] = playersPoints[playerID];
              }
              response->setLength();
              request->send(response); });

  ws.onEvent(webSocketServerEvent);
  server.addHandler(&ws);
  server.begin();
}

void initClientSocket()
{
  webSocketClient.begin("192.168.4.1", 80, "/ws");
  webSocketClient.onEvent(webSocketClientEvent);
  webSocketClient.setReconnectInterval(5000);
}

void startOTA()
{
  ArduinoOTA
      .onStart([]()
               { digitalWrite(LED_PIN, LOW); })
      .onEnd([]()
             {
                drawToScreen("Aggiornato! :)");
                delay(2000);
                drawToScreen("Riavvio...");
                delay(2000); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { drawToScreen("Progresso: " + String(progress / (total / 100)) + "%"); })
      .onError([](ota_error_t error)
               { drawToScreen("Errore :("); });
  ArduinoOTA.begin();
}

void manageConnection()
{
  drawToScreen("Connessione...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 7);
  int status = WiFi.status();
  String IP;
  while (status != WL_CONNECTED && status != WL_NO_SSID_AVAIL)
  {
    delay(2000);
    status = WiFi.status();
  }
  if (status == WL_NO_SSID_AVAIL)
  {
    WiFi.mode(WIFI_MODE_AP);
    WiFi.softAP(WIFI_SSID, WIFI_PASSWORD, 7, 0);
    drawToScreen("Sei l'host!");
    delay(2000);
    master = true;
    playerID = 0;
    initServerSocket();
    playersPoints[0] = 0;
    connectedPlayersID.push_back(0);
    drawDashboard(playerID, playerPoints);
  }
  else
  {
    drawToScreen("Connesso!");
    initClientSocket();
  }
}

void beginMultiplayer()
{
  Winner.time = MATCH_DURATION;
  Winner.id = MAX_CONNECTED_DEVICES + 1;
  memset(&packetBuffer, (uint8_t)C_BEGIN_MATCH, sizeof(uint8_t));
  ws.binaryAll(packetBuffer, sizeof(uint8_t));
  matchStartTime = millis();
  digitalWrite(LED_PIN, HIGH);
}

void endMultiplayer()
{
  memset(&packetBuffer, C_END_MATCH, 1);
  ws.binaryAll(packetBuffer, 1);
  digitalWrite(LED_PIN, LOW);
  if (Winner.id <= MAX_CONNECTED_DEVICES)
  {
    sendResults();
    manageResults(Winner.id);
  }
}

void setup()
{
  initDisplay();
  drawToScreen("Avvio...");
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  manageConnection();
  startOTA();
}

void loop()
{
  ArduinoOTA.handle();
  if (WiFi.status() != WL_CONNECTED && master != true)
  {
    drawTwoToScreen("Connessione", "persa :(");
    delay(random(1000, 10000));
    manageConnection();
  }
  else
  {
    if (!master)
    {
      webSocketClient.loop();
    }
    if (master && millis() > matchStartTime + MATCH_DURATION)
    {
      endMultiplayer();
      delay(random(2000, 8000));
      beginMultiplayer();
    }
    if (digitalRead(LED_PIN) == HIGH)
    {
      if (digitalRead(BUTTON_PIN) == LOW)
      {
        digitalWrite(LED_PIN, LOW);
        handleButtonPress(millis() - matchStartTime);
      }
    }
  }
}
