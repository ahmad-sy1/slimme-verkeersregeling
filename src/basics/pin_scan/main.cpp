#include <Arduino.h>
#include "VriConfig.h"

// Blinks one GPIO at a time and prints its number, so the LED that lights up can be
// matched to a pole by eye. Use it to find out which GPIO drives which LED.

static const uint8_t scanPins[] = PIN_SCAN_GPIOS;
static const size_t scanPinCount = sizeof(scanPins) / sizeof(scanPins[0]);

static size_t pinIndex = 0;
static uint8_t toggleCount = 0;  // 2 toggles (on, off) make one blink
static bool pausing = true;
static unsigned long lastChangeMs = 0;

static void startPin(size_t index) {
  pinIndex = index;
  toggleCount = 0;
  pausing = false;
  pinMode(scanPins[pinIndex], OUTPUT);
  digitalWrite(scanPins[pinIndex], LOW);
  Serial.print("Blinking GPIO ");
  Serial.println(scanPins[pinIndex]);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Pin scan: tell which pole and colour blinks for each GPIO.");
  startPin(0);
  lastChangeMs = millis();
}

void loop() {
  unsigned long now = millis();
  uint8_t pin = scanPins[pinIndex];

  if (pausing) {
    if (now - lastChangeMs >= PIN_SCAN_PAUSE_MS) {
      startPin((pinIndex + 1) % scanPinCount);
      lastChangeMs = now;
    }
    return;
  }

  if (now - lastChangeMs < PIN_SCAN_BLINK_MS) return;
  lastChangeMs = now;

  // Even toggles switch the LED on, odd toggles switch it off.
  digitalWrite(pin, toggleCount % 2 == 0 ? HIGH : LOW);
  toggleCount++;

  if (toggleCount >= PIN_SCAN_BLINKS * 2) {
    digitalWrite(pin, LOW);
    pinMode(pin, INPUT);  // release the pin so the next GPIO is the only one active
    pausing = true;
  }
}
