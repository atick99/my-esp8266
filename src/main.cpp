#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.println("ESP8266 Firmware Ready!");
  Serial.println("GitHub Auto Build Working ✅");
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);   // LED জ্বলবে
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);    // LED নিভবে
  delay(500);
}
