#include "board_conf.h"
#include "WiFi.h"
#include "SPI.h"
#include "WiFiUdp.h"

#define button_pin 14
#define led_pin 12
#define GAME_PORT 2022

IPAddress GameServerAddress(192, 168, 4, 1);

const char *WIFI_SSID = "IoTGame";
const char *WIFI_PASSWORD = "testpassword";
bool master = false;
int RTS;
int playerId = 0;
int localResult;

int matchStart = 0;
int match_vinti = 0;
int arrived_accendi = 0;
bool acceso = 0;

struct
{
  IPAddress ip;
  int time;
} winner;

char packetBuffer[255];  // buffer to hold incoming packet
uint8_t ReplyBuffer[10]; // a string to send back

WiFiUDP Udp;

OLED_CLASS_OBJ display(OLED_ADDRESS, OLED_SDA, OLED_SCL);

void initDisplay()
{
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
}

void drawToScreen(String text)
{
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(display.getWidth() / 2, display.getHeight() / 2, text);
  display.display();
}

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
  memcpy(ReplyBuffer, "A", 1);
  Udp.beginPacket(IPAddress(192, 168, 4, 255), GAME_PORT);
  Udp.write(ReplyBuffer, 1);
  Udp.endPacket();
}

void send_tempo(int millis)
{
  IPAddress to = IPAddress(192, 168, 4, 1);
  String message = "T" + String(millis);
  memcpy(ReplyBuffer, &message, message.length());
  Udp.beginPacket(to, GAME_PORT);
  Udp.write(ReplyBuffer, message.length());
  Udp.endPacket();
}

void send_vincitore(IPAddress vincitore)
{
  memcpy(ReplyBuffer, "V", 1);
  Udp.beginPacket(vincitore, GAME_PORT);
  Udp.write(ReplyBuffer, 1);
  Udp.endPacket();
}

void manage_messages(String message, IPAddress from)
{
  char header = message[0];
  if (header == 'A')
  {
    arrived_accendi = millis();
    acceso = true;
  }
  else if (header == 'T')
  {
    int new_time = message.substring(1).toInt();
    if (winner.time > new_time)
    {
      winner.time = new_time;
      winner.ip = from;
    }
  }
  else if (header == 'V')
  {
    match_vinti += 1;
    Serial.printf("Partite vinte: %i", match_vinti);
  }
}

void manage_packets()
{
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    String message = "";
    while (Udp.available())
    {
      message += (char)Udp.read();
    }
    manage_messages(message, Udp.remoteIP());
  }
}

void setup()
{
  // initialize Serial Monitor
  Serial.begin(9600);
  while (!Serial)
    ;
  pinMode(button_pin, INPUT);
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);
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
      if (millis() > matchStart + 10000)
      {
        send_vincitore(winner.ip);
        Serial.print("Vincitore: ");
        Serial.print(winner.ip);
        Serial.println();
        Serial.print("Tempo: ");
        Serial.print(winner.time);
        Serial.println();
        delay(random(2000, 5000));
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
        if (digitalRead(led_pin) == LOW)
        {
          digitalWrite(led_pin, HIGH);
        }
        if (digitalRead(button_pin) == LOW)
        {
          digitalWrite(led_pin, LOW);
          send_tempo(millis() - arrived_accendi);
          acceso = false;
        }
      }
    }
  }
}
