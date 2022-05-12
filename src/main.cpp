#include <device.h>

Device *device;

void setup()
{
  Serial.begin(115200);
  Serial.println("Avvio");
  delay(1000);
  device = new Device();
  Serial.println("Device Creato");
  delay(3000);
}

void loop()
{
  device->loop();
}
