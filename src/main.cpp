#include <device.h>
#include <multiplayer.h>

Device *device;
Game *game;

void delayNB(unsigned long delayTime)
{
  unsigned long endDelay = millis() + delayTime;
  while (millis() < endDelay)
    ;
  return;
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Avvio");
  delay(1000);
  device = new Device();
  game = new Multiplayer(device);
  game->initalize();
  Serial.println("Ready!");
  game->start();
}

void loop()
{
  device->loop();
  game->loop();
}
