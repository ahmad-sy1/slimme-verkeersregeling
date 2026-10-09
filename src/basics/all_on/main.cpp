// ALLESAAN: test - alle 12 lampjes (rood, geel, groen van alle 4 lichten) continu aan.
#include <Arduino.h>
#include <VriConfig.h>

void setup() {
  for (uint8_t pin : SLAVE_LED_PINS) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
  }
}

void loop() {}
