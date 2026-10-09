// ALL ON: test - all 12 LEDs (red, orange and green of all 4 signal heads) stay on.
#include <Arduino.h>
#include <VriConfig.h>

void setup() {
  for (uint8_t pin : SLAVE_LED_PINS) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
  }
}

void loop() {}
