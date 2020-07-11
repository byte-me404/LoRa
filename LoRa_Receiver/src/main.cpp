#include "inc/main.h"

//Libaries für MQTT
#include <WiFi.h>
#include <PubSubClient.h>

//Libraries for LoRa
#include <SPI.h>
#include <LoRa.h>

//Libraries for OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Libraries für WiFi und MQTT
#include <WiFi.h>
#include <PubSubClient.h>


// Variablen für WiFi
const char* ssid = "SSID";
const char* password = "PW";
WiFiClient espClient;

// Variablen für MQTT
const char* mqtt_server = "10.0.0.155";
const char* humidity_topic = "lora32/humidity";
const char* temperatur_topic = "lora32/temperatur";
const char* pressure_topic = "lora32/pressure";
const char* akku_topic = "lora32/akku";
const char* windspeed_topic = "lora32/windspeed";
const char* winddirection_topic = "lora32/winddirection";
const char* counter_topic = "lora32/counter";
PubSubClient client(espClient);
long lastMsg = 0;
long now = 0;
char msg[50];

// Variablen für OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);


// Variablen für LoRa
String LoRaData;
float temperatur;
float druck;
float akku;
float feuchte;
float temperatur_temp;
float druck_temp;
float akku_temp;
float feuchte_temp;
float windgeschwindigkeit;
float windrichtung;
bool temp = true;





void setup() { 
  
  // Seriellen Monitor initialisieren
  Serial.begin(115200);

  // Wifi und MQTT-Verbindung initialisieren
  setup_wifi();
  client.setServer(mqtt_server, 1883);

  // OLED initialisieren
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);
  
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) {
    Serial.println(F("Display konnte nicht initialisiert werden!"));
    for(;;);
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("LoRa Wetterstation ");
  display.display();
  
  Serial.println("LoRa Wetterstation");
  
  // LoRa-Modul initialisieren
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Init OK!");
  display.setCursor(0,10);
  display.println("LoRa Init OK!");
  display.display();  

  // LED initialisieren
  delay(500);
  pinMode(25,OUTPUT);
  digitalWrite(25,LOW);
}


void loop() {

  int packetSize = LoRa.parsePacket();
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  digitalWrite(25,LOW);


  // Überprüfen ob neue Daten angekommen sind
  if (packetSize) {

    digitalWrite(25,HIGH);
    Serial.println("Daten wurden Empfangen!");

    uint8_t i = 0;

    // Daten lesen
    while (LoRa.available()) {

      LoRaData = LoRa.readStringUntil('%');

      switch(i){
        case 0: temperatur_temp = LoRaData.toFloat(); break;
        case 1: druck_temp = LoRaData.toFloat(); break;
        case 2: feuchte_temp = LoRaData.toFloat(); break;
        case 3: akku_temp = LoRaData.toFloat(); break;
        case 4: windrichtung = LoRaData.toFloat(); break;
        case 5: windgeschwindigkeit = LoRaData.toFloat(); break;
      }
      i++;
    }

    if(temp){

      temperatur = temperatur_temp;
      druck = druck_temp;
      feuchte = feuchte_temp;
      akku = akku_temp;

      temp = false;
    }

    // Überprüfen ob Werte korrekt sind
    if(isnan(windgeschwindigkeit)) windgeschwindigkeit = 0;
    if((temperatur_temp > (temperatur * 0.75)) && (temperatur_temp < (temperatur * 1.25))) temperatur = temperatur_temp;
    if((druck_temp > (druck * 0.75)) && (druck_temp < (druck * 1.25))) druck = druck_temp;
    if((feuchte_temp > (feuchte * 0.75)) && (feuchte_temp < (feuchte * 1.25))) feuchte= feuchte_temp;
    if((akku_temp > (akku * 0.75)) && (akku_temp < (akku * 1.25))) akku = akku_temp;

    int rssi = LoRa.packetRssi();
    Serial.print("mit einer RSSI von: ");    
    Serial.print(rssi);
    Serial.println("");

    client_publish();
    display_OLED();
  }
}

// WiFi Setup-Funktion
void setup_wifi() {
  delay(10);
  
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


// Funktion zum Verbinden
void reconnect() {
  
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      
      client.subscribe("lora32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      
      delay(5000);
    }
  }
}

// Daten an Broker senden
void client_publish(){

  Serial.println("Daten werden gesendet!!!");

  char tempString[8];
  char humString[8];
  char presString[8];
  char akkuString[8];
  char speedString[8];
  char dirString[8];

  // Daten in Strings konvertieren
  dtostrf(temperatur, 1, 2, tempString);
  dtostrf(feuchte, 1, 2, humString);
  dtostrf(druck, 1, 2, presString);
  dtostrf(akku, 1, 2, akkuString);
  dtostrf(windgeschwindigkeit, 1, 2, speedString);
  dtostrf(windrichtung, 1, 2, dirString);

  // Senden
  client.publish(temperatur_topic, tempString);
  client.publish(humidity_topic, humString);
  client.publish(pressure_topic, presString);
  client.publish(akku_topic, akkuString);
  client.publish(windspeed_topic, speedString);
  client.publish(winddirection_topic, dirString);
}

// Funktion um die Wetter-Daten am OLED anzuzeigen
void display_OLED(){

  display.clearDisplay();

  display.setCursor(0,5);
  display.print("Temperatur");
  display.setCursor(70,5);
  display.print(temperatur);

  display.setCursor(0,15);
  display.print("Luftfeuche:");
  display.setCursor(70,15);
  display.print(feuchte);

  display.setCursor(0,25);
  display.print("Druck:");
  display.setCursor(70,25);
  display.print(druck);

  display.setCursor(0,35);
  display.print("Bat. Span.:");
  display.setCursor(70,35);
  display.print(akku);

  display.setCursor(0,45);
  display.print("Windricht.:");
  display.setCursor(70,45);
  display.print(windrichtung);

  display.setCursor(0,55);
  display.print("Windgesch.:");
  display.setCursor(70,55);
  display.print(windgeschwindigkeit);

  display.display();
}