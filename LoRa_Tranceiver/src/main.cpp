#include "inc/main.h"


//Libraries für LoRa
#include <SPI.h>
#include <LoRa.h>

//Libraries für BME280
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>




// Variablen für Windrichtungssensor
uint16_t windrichtung = 0;

// Variablen für Windgeschwindigkeitssensor
float windgeschwindigkeit = 0;
float windarray[256] = {0};
bool flag = false;
uint16_t counter = 0;
hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// Variablen für Temp.-Sensor
float temperatur = 0;
float druck = 0;
float feuchte = 0;
float akku = 0;
Adafruit_BME280 bme;



void setup() {

  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  // In- und Outputs definieren
  pinMode(BIT0, INPUT);
  pinMode(BIT1, INPUT);
  pinMode(BIT2, INPUT);
  pinMode(BIT3, INPUT);
  pinMode(BIT4, INPUT);
  pinMode(GND, OUTPUT);
  pinMode(IO_Wind, INPUT);
  pinMode(Vcc_Wind, OUTPUT);


  digitalWrite(Vcc_Wind, HIGH);


  // LoRa Modul initialiseren und starten
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(BAND)) {
    while (1);
  }


  // BME280 initialisieren
  bme.begin(0x76);  

  // Interrupt und Timer für Windgeschwindigkeitssensor konfigurieren
  attachInterrupt(digitalPinToInterrupt(IO_Wind), ISR_WIND, RISING);
  timer = timerBegin(0, 80, true);


  BME280();

  detachInterrupt(digitalPinToInterrupt(IO_Wind));
  timerEnd(timer);
  digitalWrite(Vcc_Wind, LOW);

  windspeed();

  digitalWrite(GND, HIGH);
  delay(500);
  winddirection();
  digitalWrite(GND, LOW);

  LoRa_send();


  LoRa.end();
  SPI.end();

  esp_deep_sleep_start();
}

void loop() {}



// Funktion zum bestimmen der Windrichtung
void winddirection(){

  uint8_t bit = 0;
  uint8_t bit_0, bit_1, bit_2, bit_3, bit_4;
  uint16_t loockup_array[31] = {0, 0, 144, 12, 72, 60, 132, 24, 288, 348, 156, 336, 300, 312, 120, 324, 216, 228, 204, 192, 84, 48, 96, 36, 276, 240, 168, 180, 264, 252, 108};

  bit_0 = digitalRead(BIT0);
  bit_1 = digitalRead(BIT1);
  bit_2 = digitalRead(BIT2);
  bit_3 = digitalRead(BIT3);
  bit_4 = digitalRead(BIT4);

  bit = bit_0 + (bit_1 << 1) + (bit_2 << 2) + (bit_3 << 3) + (bit_4 << 4);

  windrichtung = loockup_array[bit];
}



// ISR
void IRAM_ATTR ISR_WIND(){ //IRAM_ATTR führt dazu das sich die ISR im RAM befindet --> besser Perfomance

  float timersec = 0;

  if(flag){

    timersec = timerReadSeconds(timer); //Timer auslesen --> wird direkt in Sekunden konvertiert
    timerRestart(timer); //Timer wieder zurücksetzen

    windarray[counter] = timersec;

    counter++;
    if(counter == 265) counter = 0;
  }
  else {
    
    timerRestart(timer);
    flag = true;
  }
}



// Funktion zum berechnen der Windgeschwindigkeit
void windspeed(){

  float sum = 0;
  uint8_t x = 0;

  for(uint8_t i = 0; i < 255; i++){

    if(windarray[i] > 0.001){

      sum = sum + windarray[i];
      x++;
    }
  }

  windgeschwindigkeit = sum / x;
}



// Funktion zum lesen der Werte des BME280 Sensors
void BME280(){

  for(uint8_t i = 0; i <= 3; i++){

    temperatur = bme.readTemperature() + temperatur;
    druck = (bme.readPressure() / 100.0F) + druck;
    akku = ((analogRead(35) * 3.3 * 2 / 4096) + 0.6) + akku;
    feuchte = bme.readHumidity() + feuchte;
    delay(1000);
  }

  temperatur = temperatur / 4;
  druck = druck / 4;
  akku = akku / 4;
  feuchte = feuchte / 4;
}



// Funktion zum versenden der Daten via LoRa
void LoRa_send(){

  //Send LoRa packet to receiver
  LoRa.beginPacket();
  LoRa.print(temperatur);
  LoRa.print("%");
  LoRa.print(druck);
  LoRa.print("%");
  LoRa.print(feuchte);
  LoRa.print("%");
  LoRa.print(akku);
  LoRa.print("%");
  LoRa.print(windrichtung);
  LoRa.print("%");
  LoRa.print(windgeschwindigkeit);
  LoRa.endPacket();
}