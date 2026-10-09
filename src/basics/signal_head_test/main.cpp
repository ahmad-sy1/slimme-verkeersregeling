// Loopt elk licht af (rood, geel, groen) en print welke GPIO aan staat.
// Klopt de print niet met wat je ziet? Pas de pin-tabel in VriConfig.h aan.
// The board picks its pin table by MAC address, like the slave firmware.
#include <Arduino.h>
#include <WiFi.h>
#include <VriConfig.h>
#include <VriProtocol.h>

struct SlaveRole {
  uint8_t mac[6];
  const char *title;
  const SignalHeadPins *lights;
  int lightCount;
};

const SlaveRole ROLES[] = {
  {MAC_SLAVE_MAIN_ROAD, "HOOFDWEG", MAIN_ROAD_PINS, sizeof(MAIN_ROAD_PINS) / sizeof(MAIN_ROAD_PINS[0])},
  {MAC_SLAVE_SIDE_ROAD, "ZIJWEG",   SIDE_ROAD_PINS, sizeof(SIDE_ROAD_PINS) / sizeof(SIDE_ROAD_PINS[0])},
};

static const SignalHeadPins *gLichten = nullptr;
static int gAantal = 0;

static void zetKleur(const SignalHeadPins &l, uint8_t k) {
  digitalWrite(l.red, k == ROOD);
  digitalWrite(l.orange, k == GEEL);
  digitalWrite(l.green, k == GROEN);
}

void setup() {
  Serial.begin(115200);
  delay(200);

  // A floating pin can light an LED at random, so every LED pin starts LOW.
  for (uint8_t pin : SLAVE_LED_PINS) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }

  uint8_t mac[6];
  WiFi.mode(WIFI_STA);
  WiFi.macAddress(mac);
  for (const SlaveRole &role : ROLES) {
    if (memcmp(role.mac, mac, 6) == 0) {
      gLichten = role.lights;
      gAantal = role.lightCount;
      Serial.printf("\n=== PIN TEST %s ===\n", role.title);
      return;
    }
  }
  Serial.printf("ERROR: MAC %02X:%02X:%02X:%02X:%02X:%02X is not a slave in VriConfig.h, "
                "all LEDs stay off\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void loop() {
  if (gLichten == nullptr) return;

  const char *kleurNaam[] = {"", "ROOD ", "GEEL ", "GROEN"};
  for (int i = 0; i < gAantal; i++) {
    const SignalHeadPins &l = gLichten[i];
    uint8_t pins[] = {0, l.red, l.orange, l.green};
    for (uint8_t k = ROOD; k <= GROEN; k++) {
      Serial.printf("%-10s %s  -> GPIO%d\n", l.name, kleurNaam[k], pins[k]);
      zetKleur(l, k);
      delay(2000);
    }
    zetKleur(l, UIT);
  }
  Serial.println("--- opnieuw ---");
}
