// PIN SCAN: trace the wiring. All output pins are actively driven LOW
// (nothing floats), and over serial you pick up to 4 pins at a time:
//   "s <steady> <slow> <fast> <flash>"   e.g. "s 12 13 14 15"   (-1 = none)
//   steady = always on, slow = blinks 1x/sec, fast = blinks 4x/sec,
//   flash = 2 short flashes, then a pause
//   "b <pin> <pin> ..."         all these pins blink slowly together
//   "o"                         all off
#include <Arduino.h>

// Every output-capable GPIO that could be wired to an LED, not only the ones
// in VriConfig.h: the point of this tool is to find out which pin is which.
const uint8_t PINS[] = {2, 4, 5, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33};
const int PIN_COUNT = sizeof(PINS) / sizeof(PINS[0]);

int steady = -1, slow = -1, fast = -1, flash = -1;
int group[12];
int groupSize = 0;
String line;

void allOff() {
  for (int i = 0; i < PIN_COUNT; i++) digitalWrite(PINS[i], LOW);
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < PIN_COUNT; i++) pinMode(PINS[i], OUTPUT);
  allOff();
  delay(200);
  Serial.println("\n=== PIN SCAN ready ===");
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      line.trim();
      if (line.startsWith("s")) {
        allOff();
        groupSize = 0;
        steady = slow = fast = flash = -1;
        sscanf(line.c_str() + 1, "%d %d %d %d", &steady, &slow, &fast, &flash);
        Serial.printf("OK steady=%d slow=%d fast=%d flash=%d\n", steady, slow, fast, flash);
      } else if (line.startsWith("b")) {
        // "b 13 14 18" -> all these pins blink slowly together
        allOff();
        steady = slow = fast = flash = -1;
        groupSize = 0;
        char buf[128];
        line.substring(1).toCharArray(buf, sizeof(buf));
        for (char *t = strtok(buf, " "); t && groupSize < 12; t = strtok(nullptr, " ")) group[groupSize++] = atoi(t);
        Serial.printf("OK group of %d pins\n", groupSize);
      } else if (line == "o") {
        groupSize = 0;
        steady = slow = fast = flash = -1;
        allOff();
        Serial.println("OK off");
      }
      line = "";
    } else {
      line += c;
    }
  }

  unsigned long t = millis();
  if (steady >= 0) digitalWrite(steady, HIGH);
  if (slow >= 0) digitalWrite(slow, (t / 500) % 2);
  if (fast >= 0) digitalWrite(fast, (t / 125) % 2);
  for (int i = 0; i < groupSize; i++) digitalWrite(group[i], (t / 500) % 2);
  unsigned long f = t % 1600;
  if (flash >= 0) digitalWrite(flash, f < 100 || (f >= 250 && f < 350));
}
