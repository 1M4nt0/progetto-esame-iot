#include <Wire.h>
#include "SSD1306Wire.h"

#define LORA_V2_0_OLED 1
#define LORA_SENDER 0
#define OLED_CLASS_OBJ SSD1306Wire
#define OLED_ADDRESS 0x3C
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST -1

#define LORA_PERIOD 868
#define BAND 868E6
#define CONFIG_MOSI 27
#define CONFIG_MISO 19
#define CONFIG_CLK 5
#define CONFIG_NSS 18
#define CONFIG_RST 23
#define CONFIG_DIO0 26

#define SDCARD_MOSI 15
#define SDCARD_MISO 2
#define SDCARD_SCLK 14
#define SDCARD_CS 13