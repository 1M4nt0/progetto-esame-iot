#include "board_conf.h"
#include "WiFi.h"
#include "SPI.h"
#include "WiFiUdp.h"

/* CODICI
  A -> accenditi
  T,XXX -> tempo
  V -> vincitore

*/

#define button_pin 14
#define GAME_PORT 2022

IPAddress GameServerAddress(192, 168, 4, 1);

const char *WIFI_SSID = "IoTGame";
const char *WIFI_PASSWORD = "testpassword";
bool master = false;
int RTS;
int playerId = 0;
int localResult;

char packetBuffer[255]; // buffer to hold incoming packet
uint8_t ReplyBuffer[];  // a string to send back

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
  String message = str "T" + String(millis);
  memcpy(ReplyBuffer, message, 1);
  Udp.beginPacket(to, GAME_PORT);
  Udp.write(ReplyBuffer, 1);
  Udp.endPacket();
}

void send_vincitore(IPAddress vincitore)
{
  memcpy(ReplyBuffer, "V", 1);
  Udp.beginPacket(vincitore, GAME_PORT);
  Udp.write(ReplyBuffer, 1);
  Udp.endPacket();
}

void setup()
{
  // initialize Serial Monitor
  Serial.begin(9600);
  while (!Serial)
    ;
  pinMode(button_pin, INPUT);
  initDisplay();
  drawToScreen("Avvio...");
  delay(1000);
  manageConnection();
  Udp.begin(GAME_PORT);
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
      Udp.beginPacket(to, GAME_PORT);
      Udp.write(ReplyBuffer, 4);
      Udp.endPacket();
      delay(5000);
    }
    else
    {
      int packetSize = Udp.parsePacket();
      if (packetSize)
      {
        Serial.print("Received packet of size ");
        Serial.println(packetSize);
        Serial.print("From ");
        IPAddress remoteIp = Udp.remoteIP();
        Serial.print(remoteIp);
        Serial.print(", port ");
        Serial.println(Udp.remotePort());
        while (Udp.available())
        {
          Serial.print((char)Udp.read());
        }
        Serial.println("OK!");
      }
    }
  }
}
