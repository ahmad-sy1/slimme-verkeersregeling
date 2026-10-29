#include <Arduino.h>
#include "VriConfig.h"

// Blinks every candidate GPIO at the same time. A pole LED that stays dark is not
// on any of these GPIOs, or is wired wrong. Use pin_scan to find out which GPIO is which.

static const uint8_t testPins[] = PIN_SCAN_GPIOS;
static const size_t testPinCount = sizeof(testPins) / sizeof(testPins[0]);

static bool ledsOn = false;
static unsigned long lastChangeMs = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  for (size_t i = 0; i < testPinCount; i++) {
    pinMode(testPins[i], OUTPUT);
    digitalWrite(testPins[i], LOW);
  }
  Serial.println("LED test: all candidate GPIOs blink together.");
}

void loop() {
  unsigned long now = millis();
  if (now - lastChangeMs < LED_TEST_BLINK_MS) return;
  lastChangeMs = now;

  ledsOn = !ledsOn;
  for (size_t i = 0; i < testPinCount; i++) {
    digitalWrite(testPins[i], ledsOn ? HIGH : LOW);
  }
}
