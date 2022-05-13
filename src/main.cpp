#include <device.h>
#include <multiplayer.h>

Device *device;
Game *game;

void setup()
{
  Serial.begin(115200);
  Serial.println("Avvio");
  delay(1000);
  device = new Device();
  game = new Multiplayer(device);
  game->initalize();
  game->start();
  Serial.println("Ready!");
}

void loop()
{
  device->loop();
  game->loop();
}
