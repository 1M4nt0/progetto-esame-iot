#include "board_conf.h"
#include "WiFi.h"
#include "SPI.h"

const char *WIFI_SSID = "IoTGame";
const char *WIFI_PASSWORD = "testpassword";
bool master = false;
int RTS;
int playerId = 0;
IPAddress localServer(192, 168, 4, 1);
int localResult;

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
    Serial.print("Player ID: ");
    Serial.println(playerId);
    Serial.print("IP:");
    Serial.println(IP);
    snprintf(text, 9, "Player %i", playerId);
    drawToScreen(text);
  }
}

void setup()
{
  // initialize Serial Monitor
  Serial.begin(9600);
  while (!Serial)
    ;
  WiFi.mode(WIFI_MODE_APSTA);
  initDisplay();
  drawToScreen("Avvio...");
  delay(1000);
  manageConnection();
}

void loop()
{
}
