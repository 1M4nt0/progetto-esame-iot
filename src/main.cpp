#include <gameManager.h>

GameManager *_gameManager;

void setup()
{
  Serial.begin(115200);
  Serial.println("Avvio");
  delay(1000);
  _gameManager = new GameManager();
  Serial.println("Ready!");
}

void loop()
{
  _gameManager->loop();
}
