#pragma once
// Gedeelde slave-logica. Elke slave geeft alleen zijn eigen pin-tabel mee.
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_arduino_version.h>
#include "protocol.h"

struct LichtPins {
  LichtId id;
  const char *naam;
  uint8_t rood, geel, groen;
};

static const LichtPins *gLichten = nullptr;
static int gAantal = 0;
static volatile unsigned long gLaatsteBericht = 0;
static volatile bool gOoitOntvangen = false;
static LichtBericht gBericht;

static void zetKleur(const LichtPins &l, uint8_t k) {
  digitalWrite(l.rood, k == ROOD);
  digitalWrite(l.geel, k == GEEL);
  digitalWrite(l.groen, k == GROEN);
}

static void verwerk(const uint8_t *data, int len) {
  if (len != sizeof(LichtBericht) || data[0] != BERICHT_MAGIC) return;
  memcpy(&gBericht, data, sizeof(gBericht));
  gLaatsteBericht = millis();
  gOoitOntvangen = true;
}

#if ESP_ARDUINO_VERSION_MAJOR >= 3
static void onRecv(const esp_now_recv_info_t *, const uint8_t *data, int len) { verwerk(data, len); }
#else
static void onRecv(const uint8_t *, const uint8_t *data, int len) { verwerk(data, len); }
#endif

#ifdef PINTEST
// Loopt elk licht af (rood, geel, groen) en print welke GPIO aan staat.
// Klopt de print niet met wat je ziet? Pas de pin-tabel aan.
static void slaveSetup(const char *titel, const LichtPins *lichten, int aantal) {
  gLichten = lichten;
  gAantal = aantal;
  Serial.begin(115200);
  delay(200);
  Serial.printf("\n=== PIN TEST %s ===\n", titel);
  for (int i = 0; i < aantal; i++) {
    pinMode(lichten[i].rood, OUTPUT);
    pinMode(lichten[i].geel, OUTPUT);
    pinMode(lichten[i].groen, OUTPUT);
    zetKleur(lichten[i], UIT);
  }
}

static void slaveLoop() {
  const char *kleurNaam[] = {"", "ROOD ", "GEEL ", "GROEN"};
  for (int i = 0; i < gAantal; i++) {
    const LichtPins &l = gLichten[i];
    uint8_t pins[] = {0, l.rood, l.geel, l.groen};
    for (uint8_t k = ROOD; k <= GROEN; k++) {
      Serial.printf("%-10s %s  -> GPIO%d\n", l.naam, kleurNaam[k], pins[k]);
      zetKleur(l, k);
      delay(2000);
    }
    zetKleur(l, UIT);
  }
  Serial.println("--- opnieuw ---");
}

#else

static void slaveSetup(const char *titel, const LichtPins *lichten, int aantal) {
  gLichten = lichten;
  gAantal = aantal;
  Serial.begin(115200);
  delay(200);
  Serial.printf("\n=== Kruispunt SLAVE %s ===\n", titel);

  for (int i = 0; i < aantal; i++) {
    pinMode(lichten[i].rood, OUTPUT);
    pinMode(lichten[i].geel, OUTPUT);
    pinMode(lichten[i].groen, OUTPUT);
    zetKleur(lichten[i], UIT);
  }

  WiFi.mode(WIFI_STA);
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init mislukt, herstart...");
    delay(1000);
    ESP.restart();
  }
  esp_now_register_recv_cb(onRecv);
}

static void slaveLoop() {
  static bool wasStoring = true;
  static uint8_t vorigeFase = 0xFF;
  bool storing = !gOoitOntvangen || millis() - gLaatsteBericht > TIMEOUT_MS;

  if (storing) {
    // Geen master -> knipperend geel, zoals een echt kruispunt bij storing
    bool aan = (millis() / 500) % 2;
    for (int i = 0; i < gAantal; i++) zetKleur(gLichten[i], aan ? GEEL : UIT);
    if (!wasStoring) Serial.println("Geen signaal van master -> knipperend geel");
    wasStoring = true;
    return;
  }

  if (wasStoring) Serial.println("Master gevonden");
  wasStoring = false;

  LichtBericht b;
  noInterrupts();
  memcpy(&b, &gBericht, sizeof(b));
  interrupts();

  for (int i = 0; i < gAantal; i++) zetKleur(gLichten[i], b.kleur[gLichten[i].id]);

  if (b.fase != vorigeFase) {
    vorigeFase = b.fase;
    Serial.printf("Fase %d\n", b.fase);
  }
}
#endif
