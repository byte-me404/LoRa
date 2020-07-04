#include <Arduino.h>

// Defines für LoRa Modul
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 23
#define DIO0 26
#define BAND 866E6

// Defines für Windrichtungssensor
#define BIT0 14
#define BIT1 12
#define BIT2 13
#define BIT3 15
#define BIT4 34
#define GND 2

// Defines für Windgeschwindigkeitssensor
#define IO_Wind 4
#define Vcc_Wind 25

// Defines für Deep-Sleep
#define uS_TO_S_FACTOR 1000000  
#define TIME_TO_SLEEP  60

// Prototypen
void winddirection(void);
void IRAM_ATTR ISR_WIND(void);
void windspeed(void);
void BME280(void);
void LoRa_send(void);