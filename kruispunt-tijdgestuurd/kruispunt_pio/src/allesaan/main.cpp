// ALLESAAN: test - alle 12 lampjes (rood, geel, groen van alle 4 lichten) continu aan.
#include <Arduino.h>

const int leds[] = {13, 14, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33};

void setup() {
  for (int pin : leds) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
  }
}

void loop() {}
