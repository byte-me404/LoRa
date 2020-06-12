#include <Arduino.h>


//Libraries for LoRa
#include <SPI.h>
#include <LoRa.h>

//Libraries for BME280
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

//define the pins used by the LoRa transceiver module
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 23
#define DIO0 26

//866E6 for Europe
#define BAND 866E6


#define SEALEVELPRESSURE_HPA (929.35)

float temperatur = 0;
float druck = 0;
float hoehe = 0;
float feuchte = 0;

Adafruit_BME280 bme;

void setup() {

  bool status;

  //Initialize Serial Monitor
  Serial.begin(115200);

  //Initialize BME280 
  status = bme.begin(0x76);  
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  
  Serial.println("LoRa Sender");

  //SPI LoRa pins
  SPI.begin(SCK, MISO, MOSI, SS);
  //setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);
  
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initializing OK!");
  delay(2000);
}

void loop() {
   
  Serial.print("Sending packet ");


  Serial.print("Temperature = ");
  temperatur = bme.readTemperature();
  Serial.print(temperatur);
  Serial.println(" *C");

  Serial.print("Pressure = ");
  druck = bme.readPressure() / 100.0F;
  Serial.print(druck);
  Serial.println(" hPa");

  Serial.print("Approx. Altitude = ");
  hoehe = bme.readAltitude(SEALEVELPRESSURE_HPA)+665;
  Serial.print(hoehe);
  Serial.println(" m");

  Serial.print("Humidity = ");
  feuchte = bme.readHumidity();
  Serial.print(feuchte);
  Serial.println(" %");



  //Send LoRa packet to receiver
  LoRa.beginPacket();
  LoRa.print(temperatur);
  LoRa.print("%");
  LoRa.print(druck);
  LoRa.print("%");
  LoRa.print(hoehe);
  LoRa.print("%");
  LoRa.print(feuchte);
  LoRa.endPacket();
  
  delay(2000);
}