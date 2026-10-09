// MASTER: het brein. Heeft zelf geen lichten, bepaalt alleen de fases op tijd
// en broadcast de kleur van alle 8 lichten via ESP-NOW naar beide slaves.
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <VriConfig.h>
#include <VriProtocol.h>

// ---- Fases: welke lichten tegelijk groen mogen ----
// Per fase steeds 1 weg, beide richtingen (A+B) van dezelfde baan -> geen conflicten.
struct Fase {
  const char *naam;
  uint8_t lichten;  // bitmask van LichtId
};

#define BIT(id) (1u << (id))

const Fase FASES[] = {
  {"Hoofdweg rechts (A+B)", BIT(HW_A_RECHTS) | BIT(HW_B_RECHTS)},
  {"Hoofdweg links (A+B)",  BIT(HW_A_LINKS)  | BIT(HW_B_LINKS)},
  {"Zijweg rechts (A+B)",   BIT(ZW_A_RECHTS) | BIT(ZW_B_RECHTS)},
  {"Zijweg links (A+B)",    BIT(ZW_A_LINKS)  | BIT(ZW_B_LINKS)},
};
const int AANTAL_FASES = sizeof(FASES) / sizeof(FASES[0]);

enum Stap { STAP_GROEN, STAP_GEEL, STAP_ALLROOD };

const uint8_t BROADCAST[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

int fase = AANTAL_FASES - 1;  // eerste ALLROOD gaat door naar fase 0
Stap stap = STAP_ALLROOD;
unsigned long stapStart = 0;
unsigned long stapDuur = STARTUP_ALL_RED_MS;
unsigned long laatsteVerzonden = 0;

void verstuur() {
  LichtBericht b;
  b.magic = BERICHT_MAGIC;
  b.fase = fase;
  for (int i = 0; i < AANTAL_LICHTEN; i++) {
    bool actief = stap != STAP_ALLROOD && (FASES[fase].lichten & BIT(i));
    b.kleur[i] = !actief ? ROOD : (stap == STAP_GROEN ? GROEN : GEEL);
  }
  esp_now_send(BROADCAST, (const uint8_t *)&b, sizeof(b));
  laatsteVerzonden = millis();
}

void naarStap(Stap nieuw, unsigned long duur) {
  stap = nieuw;
  stapStart = millis();
  stapDuur = duur;
  const char *namen[] = {"GROEN", "GEEL", "ALLES ROOD"};
  Serial.printf("[%7lu] %-22s %s\n", stapStart, FASES[fase].naam, namen[stap]);
  verstuur();
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n=== Kruispunt MASTER (tijdgestuurd) ===");

  WiFi.mode(WIFI_STA);
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init mislukt, herstart...");
    delay(1000);
    ESP.restart();
  }
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, BROADCAST, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  Serial.println("Opstarten: alles rood");
  stapStart = millis();
  verstuur();
}

void loop() {
  if (millis() - stapStart >= stapDuur) {
    switch (stap) {
      case STAP_GROEN:
        naarStap(STAP_GEEL, ORANGE_MS);
        break;
      case STAP_GEEL:
        naarStap(STAP_ALLROOD, CLEARANCE_MS);
        break;
      case STAP_ALLROOD:
        fase = (fase + 1) % AANTAL_FASES;
        naarStap(STAP_GROEN, FIXED_GREEN_MS);
        break;
    }
  }

  // Blijf herhalen zodat een (her)opstartende slave meteen weer meedoet
  if (millis() - laatsteVerzonden >= SEND_INTERVAL_MS) verstuur();
}
