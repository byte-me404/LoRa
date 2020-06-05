#include <Arduino.h>



void setup() {
  //pinMode(25, INPUT_PULLUP);
  Serial.begin(115200);
  //analogSetAttenuation(ADC_6db);
}

float potValue = 0;

void loop() {
  //Serial.println(digitalRead(25));
  delay(100);
  potValue = analogRead(14);
  Serial.println(potValue);
}