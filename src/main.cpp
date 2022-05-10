#include "board_conf.h"
#include "WiFi.h"
#include "SPI.h"
#include "screen_utils.h"
#include <ArduinoOTA.h>
#include <gameManager.h>

const char *WIFI_SSID = "IoTGame";
const char *WIFI_PASSWORD = "testpassword";
bool isHost;

GameManager *gameManager;

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

void initPins()
{
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
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
    isHost = true;
  }
  else
  {
    isHost = false;
  }
}

void setup()
{
  initDisplay();
  drawToScreen("Avvio...");
  delay(2000);
  Serial.begin(115200);
  initPins();
  manageConnection();
  gameManager = new GameManager(isHost);
  gameManager->initializeGame(0);
  startOTA();
}

void loop()
{
  ArduinoOTA.handle();
  if (WiFi.status() != WL_CONNECTED && isHost != true)
  {
    drawTwoToScreen("Connessione", "persa :(");
    delay(random(1000, 10000));
    manageConnection();
  }
  else
  {
    gameManager->loop();
  }
}
