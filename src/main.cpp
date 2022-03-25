#include "board_conf.h"
#include "WiFi.h"
#include "SPI.h"
#include "WiFiUdp.h"
#include "screen_utils.h"

#define BUTTON_PIN 14
#define LED_PIN 12
#define GAME_PORT 2022

/*
  00000001 = 1  ACCENDI
  00000010 = 2  TEMPO
  00000011 = 3  VINCITORE
*/

IPAddress GameHost = IPAddress(192, 168, 4, 1);
const char *WIFI_SSID = "IoTGame";
const char *WIFI_PASSWORD = "testpassword";
bool master = false;

int playerId = 0;

int matchStart = 0;
int match_vinti = 0;
int arrived_accendi = 0;
bool acceso = 0;

struct
{
  IPAddress ip;
  int time;
} winner;

byte packetBuffer[3]; // buffer to hold incoming packet
byte replyBuffer[3];  // buffer to hold incoming packet
WiFiUDP Udp;

void manageConnection()
{
  drawToScreen("Connecting...");
  Serial.println();
  Serial.print("Attempting to connect to open SSID: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 7);
  int status = WiFi.status();
  while (status != WL_CONNECTED && status != WL_NO_SSID_AVAIL)
  {
    delay(2000);
    status = WiFi.status();
  }
  if (status == WL_NO_SSID_AVAIL)
  {
    WiFi.mode(WIFI_MODE_AP);
    WiFi.softAP(WIFI_SSID, WIFI_PASSWORD, 7, 0);
    Serial.println();
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
    master = true;
    drawToScreen("Player 1");
  }
  else
  {
    String IP = WiFi.localIP().toString();
    playerId = IP.charAt(IP.length() - 1) - '0';
    char text[9];
    Serial.print("Connected : ");
    Serial.println(WiFi.SSID());
    Serial.print("Player ");
    Serial.println(playerId);
    Serial.print("IP : ");
    Serial.println(IP);
    master = false;
    snprintf(text, 9, "Player %i", playerId);
    drawToScreen(text);
  }
}

void send_accendi()
{
  byte code = 1;
  Udp.beginPacket(IPAddress(192, 168, 4, 255), GAME_PORT);
  Udp.write(&code, 1);
  Udp.endPacket();
}

void send_tempo(short millis)
{
  byte code = 2;
  memcpy(replyBuffer, &code, 1);
  memcpy(replyBuffer + 1, &millis, 2);
  Udp.beginPacket(GameHost, GAME_PORT);
  Udp.write(replyBuffer, 3);
  Udp.endPacket();
}

void send_vincitore(IPAddress vincitore)
{
  byte code = 3;
  Udp.beginPacket(vincitore, GAME_PORT);
  Udp.write(&code, 1);
  Udp.endPacket();
}

void manage_messages(byte payload[], IPAddress from)
{
  if (payload[0] == 1)
  {
    arrived_accendi = millis();
    acceso = true;
  }
  else if (payload[0] == 2)
  {
    short newTime;
    memcpy(&newTime, payload + 1, 2);
    if (winner.time > newTime)
    {
      winner.time = newTime;
      winner.ip = from;
    }
  }
  else if (payload[0] == 3)
  {
    match_vinti += 1;
    Serial.printf("Partite vinte: %i\n", match_vinti);
  }
}

void manage_packets()
{
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    if (Udp.available())
    {
      Udp.read(packetBuffer, 3);
    }
    manage_messages(packetBuffer, Udp.remoteIP());
  }
}

void setup()
{
  // initialize Serial Monitor
  Serial.begin(9600);
  while (!Serial)
    ;
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  initDisplay();
  drawToScreen("Avvio...");
  delay(1000);
  manageConnection();
  Udp.begin(GAME_PORT);
  matchStart = millis();
  winner.time = 10000;
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED && master != true)
  {
    Serial.println();
    Serial.println("Disconnesso");
    manageConnection();
  }
  else
  {
    if (master)
    {
      // MASTER
      if (millis() > matchStart + 5000)
      {
        send_vincitore(winner.ip);
        Serial.print("Vincitore: ");
        Serial.print(winner.ip);
        Serial.println();
        Serial.print("Tempo: ");
        Serial.print(winner.time);
        Serial.println();
        delay(random(2000, 10000));
        matchStart = millis();
        winner.time = 10000;
        Serial.println("Match start!");
        send_accendi();
      }
      else
      {
        manage_packets();
      }
      delay(1000);
    }
    else
    {
      // CLIENT
      manage_packets();
      if (acceso)
      {
        delay(random(300));
        send_tempo(millis() - arrived_accendi);
        acceso = false;
        /*
        if (digitalRead(LED_PIN) == LOW)
        {
          digitalWrite(LED_PIN, HIGH);
        }
        if (digitalRead(BUTTON_PIN) == LOW)
        {
          digitalWrite(LED_PIN, LOW);
          send_tempo(millis() - arrived_accendi);
          acceso = false;
        }
        */
      }
    }
  }
}
