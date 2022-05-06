#include "board_conf.h"
#include "WiFi.h"
#include "SPI.h"
#include "screen_utils.h"
#include <ArduinoOTA.h>
#include <multiplayer.h>

const char *WIFI_SSID = "IoTGame";
const char *WIFI_PASSWORD = "testpassword";
bool gamemode = 0;
Game *_game;

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
    _game->init(true);
    drawToScreen("Sei l'host!");
    delay(2000);
    drawDashboard(_game->getDeviceID(), _game->getPlayerPoints(_game->getDeviceID()));
  }
  else
  {
    drawToScreen("Connesso!");
    _game->init(false);
  }
}

void setup()
{
  initDisplay();
  drawToScreen("Avvio...");
  Serial.begin(115200);
  if (gamemode == 0)
  {
    _game = new Multiplayer();
  }
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  manageConnection();
  startOTA();
}

void loop()
{
  ArduinoOTA.handle();
  if (WiFi.status() != WL_CONNECTED && _game->getIsHost() != true)
  {
    drawTwoToScreen("Connessione", "persa :(");
    delay(random(1000, 10000));
    manageConnection();
  }
  else
  {
    _game->loop();
  }
}
