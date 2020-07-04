#include <Arduino.h>


// Defines für LoRa-Modul
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 23
#define DIO0 26
#define BAND 866E6

// Defines für OLED
#define OLED_SDA 21
#define OLED_SCL 22 
#define OLED_RST 16
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64



void display_OLED(void);
void setup_wifi(void);
void reconnect(void);
void client_publish(void);