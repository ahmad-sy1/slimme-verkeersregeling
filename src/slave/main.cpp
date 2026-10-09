// Slave firmware, shared by the main road and the side road slave. The board
// reads its own MAC address at start-up and picks the matching pin table.
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_arduino_version.h>
#include <VriConfig.h>
#include <VriProtocol.h>

struct LichtPins {
  LichtId id;
  const char *naam;
  uint8_t rood, geel, groen;
};

// Pinnen per lampje gecontroleerd op het kruispunt (pinscan, 08-10-2026).
//                id           naam         rood geel groen
const LichtPins LICHTEN_HOOFDWEG[] = {
  {HW_A_LINKS,  "A links",   18,  13,  14},
  {HW_A_RECHTS, "A rechts",  19,  21,  22},
  {HW_B_LINKS,  "B links",   27,  32,  26},
  {HW_B_RECHTS, "B rechts",  25,  33,  23},
};

// A links rood (32) en A rechts geel (13) lichtten in de test niet zichtbaar op;
// het zijn de enige overgebleven pinnen en de LED-spanningsmeting past erbij.
const LichtPins LICHTEN_ZIJWEG[] = {
  {ZW_A_LINKS,  "A links",   32,  33,  27},
  {ZW_A_RECHTS, "A rechts",  14,  13,  26},
  {ZW_B_LINKS,  "B links",   25,  23,  22},
  {ZW_B_RECHTS, "B rechts",  21,  18,  19},
};

struct SlaveRole {
  uint8_t mac[6];
  const char *title;
  const LichtPins *lights;
  int lightCount;
};

const SlaveRole ROLES[] = {
  {MAC_SLAVE_MAIN_ROAD, "HOOFDWEG", LICHTEN_HOOFDWEG, sizeof(LICHTEN_HOOFDWEG) / sizeof(LICHTEN_HOOFDWEG[0])},
  {MAC_SLAVE_SIDE_ROAD, "ZIJWEG",   LICHTEN_ZIJWEG,   sizeof(LICHTEN_ZIJWEG) / sizeof(LICHTEN_ZIJWEG[0])},
};

// Repeat the error, so it is also seen when the monitor is opened after boot.
const unsigned long UNKNOWN_MAC_REPEAT_MS = 5000;

static const LichtPins *gLichten = nullptr;
static int gAantal = 0;
static volatile unsigned long gLaatsteBericht = 0;
static volatile bool gOoitOntvangen = false;
static LichtBericht gBericht;
static uint8_t ownMac[6];

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

static const SlaveRole *findRole(const uint8_t *mac) {
  for (const SlaveRole &role : ROLES) {
    if (memcmp(role.mac, mac, 6) == 0) return &role;
  }
  return nullptr;
}

// Without a role it is unknown which pin is which LED, so every pin of every
// table is driven LOW. A floating pin can light an LED at random.
static void allLedsOff() {
  for (const SlaveRole &role : ROLES) {
    for (int i = 0; i < role.lightCount; i++) {
      pinMode(role.lights[i].rood, OUTPUT);
      pinMode(role.lights[i].geel, OUTPUT);
      pinMode(role.lights[i].groen, OUTPUT);
      zetKleur(role.lights[i], UIT);
    }
  }
}

static void printUnknownMac() {
  Serial.printf("ERROR: MAC %02X:%02X:%02X:%02X:%02X:%02X is not a slave in VriConfig.h, "
                "all LEDs stay off\n",
                ownMac[0], ownMac[1], ownMac[2], ownMac[3], ownMac[4], ownMac[5]);
}

#ifdef PINTEST
// Loopt elk licht af (rood, geel, groen) en print welke GPIO aan staat.
// Klopt de print niet met wat je ziet? Pas de pin-tabel aan.
static void slaveSetup(const char *titel, const LichtPins *lichten, int aantal) {
  gLichten = lichten;
  gAantal = aantal;
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
  Serial.printf("\n=== Kruispunt SLAVE %s ===\n", titel);

  for (int i = 0; i < aantal; i++) {
    pinMode(lichten[i].rood, OUTPUT);
    pinMode(lichten[i].geel, OUTPUT);
    pinMode(lichten[i].groen, OUTPUT);
    zetKleur(lichten[i], UIT);
  }

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

static const SlaveRole *role = nullptr;

void setup() {
  Serial.begin(115200);
  delay(200);

  // The station-mode MAC is the one stored in VriConfig.h.
  WiFi.mode(WIFI_STA);
  WiFi.macAddress(ownMac);
  role = findRole(ownMac);

  if (role == nullptr) {
    allLedsOff();
    printUnknownMac();
    return;
  }
  slaveSetup(role->title, role->lights, role->lightCount);
}

void loop() {
  if (role == nullptr) {
    static unsigned long lastErrorAt = 0;
    if (millis() - lastErrorAt >= UNKNOWN_MAC_REPEAT_MS) {
      lastErrorAt = millis();
      printUnknownMac();
    }
    return;
  }
  slaveLoop();
}
