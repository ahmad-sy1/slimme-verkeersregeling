// VOLGORDE: zet de 12 lampjes een voor een aan (zelfde pinnen als jullie testcode).
// Na stap 12 gaat alles 4 sec uit, dan begint hij weer bij stap 1.
#include <Arduino.h>

const int leds[] = {13, 14, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33};
const int aantalLeds = sizeof(leds) / sizeof(leds[0]);

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < aantalLeds; i++) {
    pinMode(leds[i], OUTPUT);
    digitalWrite(leds[i], LOW);
  }
}

void loop() {
  delay(6000);  // pauze = begin van de reeks
  for (int i = 0; i < aantalLeds; i++) {
    Serial.printf("stap %d -> GPIO%d\n", i + 1, leds[i]);
    digitalWrite(leds[i], HIGH);
    delay(3000);
    digitalWrite(leds[i], LOW);
    delay(500);
  }
}
