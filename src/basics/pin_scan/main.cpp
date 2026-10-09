// PINSCAN: bedrading uitzoeken. Alle uitgangspinnen worden actief LAAG gezet
// (niks zweeft), en via Serial kies je max 4 pinnen tegelijk:
//   "s <vast> <traag> <snel> <flits>"   bv. "s 12 13 14 15"   (-1 = geen)
//   vast  = continu aan, traag = knippert 1x/sec, snel = knippert 4x/sec,
//   flits = 2 korte flitsjes, dan pauze
//   "b <pin> <pin> ..."         al die pinnen knipperen samen traag
//   "o"                         alles uit
#include <Arduino.h>

const uint8_t PINS[] = {2, 4, 5, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33};
const int AANTAL = sizeof(PINS) / sizeof(PINS[0]);

int vast = -1, traag = -1, snel = -1, flits = -1;
int groep[12];
int aantalGroep = 0;
String regel;

void allesUit() {
  for (int i = 0; i < AANTAL; i++) digitalWrite(PINS[i], LOW);
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < AANTAL; i++) pinMode(PINS[i], OUTPUT);
  allesUit();
  delay(200);
  Serial.println("\n=== PINSCAN klaar ===");
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      regel.trim();
      if (regel.startsWith("s")) {
        allesUit();
        aantalGroep = 0;
        vast = traag = snel = flits = -1;
        sscanf(regel.c_str() + 1, "%d %d %d %d", &vast, &traag, &snel, &flits);
        Serial.printf("OK vast=%d traag=%d snel=%d flits=%d\n", vast, traag, snel, flits);
      } else if (regel.startsWith("b")) {
        // "b 13 14 18" -> al deze pinnen knipperen tegelijk traag
        allesUit();
        vast = traag = snel = flits = -1;
        aantalGroep = 0;
        char buf[128];
        regel.substring(1).toCharArray(buf, sizeof(buf));
        for (char *t = strtok(buf, " "); t && aantalGroep < 12; t = strtok(nullptr, " ")) groep[aantalGroep++] = atoi(t);
        Serial.printf("OK groep van %d pinnen\n", aantalGroep);
      } else if (regel == "o") {
        aantalGroep = 0;
        vast = traag = snel = flits = -1;
        allesUit();
        Serial.println("OK uit");
      }
      regel = "";
    } else {
      regel += c;
    }
  }

  unsigned long t = millis();
  if (vast >= 0) digitalWrite(vast, HIGH);
  if (traag >= 0) digitalWrite(traag, (t / 500) % 2);
  if (snel >= 0) digitalWrite(snel, (t / 125) % 2);
  for (int i = 0; i < aantalGroep; i++) digitalWrite(groep[i], (t / 500) % 2);
  unsigned long f = t % 1600;
  if (flits >= 0) digitalWrite(flits, f < 100 || (f >= 250 && f < 350));
}
