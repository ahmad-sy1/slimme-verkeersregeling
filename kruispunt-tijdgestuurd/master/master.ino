#include <esp_now.h>
#include <WiFi.h>

// Lichten die dit board (hoofdweg) aanstuurt: 1, 2, 5, 6
#define LICHT1_GEEL   25
#define LICHT1_ROOD   26
#define LICHT1_GROEN  27
#define LICHT1_LDR    33

#define LICHT2_ROOD   12
#define LICHT2_GROEN  13
#define LICHT2_GEEL   14
#define LICHT2_LDR    32

#define LICHT5_ROOD   18
#define LICHT5_GEEL   19
#define LICHT5_GROEN  21
#define LICHT5_LDR    35

#define LICHT6_GEEL   2
#define LICHT6_GROEN  4
#define LICHT6_ROOD   5
#define LICHT6_LDR    34

enum Licht { ROOD, GEEL, GROEN };

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct SensorMelding {
  uint8_t slaveId; // 1 = zijweg rechts (licht 3,4), 2 = zijweg links (licht 7,8)
  bool autoEen;    // slave1: licht3 / slave2: licht7
  bool autoTwee;   // slave1: licht4 / slave2: licht8
} SensorMelding;

typedef struct LichtCommando {
  uint8_t combinatie; // bitmask: bit0=licht1 ... bit7=licht8
  uint8_t status;     // Licht: ROOD/GEEL/GROEN, geldt voor elk actief licht in de combinatie
} LichtCommando;

// index 0..7 = licht 1..8, true = auto aanwezig
bool autoAanwezig[8] = {false, false, false, false, false, false, false, false};

// LDR drempel: test met Serial.println(analogRead(pin)) en pas dit getal aan
const int LDR_DREMPEL = 2000;

bool leesLDR(int pin) {
  return analogRead(pin) < LDR_DREMPEL;
}

// Alle toegestane combinaties uit de opdracht, groot naar klein (licht 1..8 = bit 0..7)
const uint8_t MOGELIJKHEDEN[] = {
  0b10011001, // {1,4,5,8}
  0b10000011, // {1,2,8}
  0b00011001, // {1,4,5}
  0b10001001, // {1,4,8}
  0b10010001, // {1,5,8}
  0b10001100, // {3,4,8}
  0b00111000, // {4,5,6}
  0b10011000, // {4,5,8}
  0b11001000, // {4,7,8}
  0b00000011, // 1-2
  0b00001001, // 1-4
  0b00010001, // 1-5
  0b10000001, // 1-8
  0b00100010, // 2-6
  0b10000010, // 2-8
  0b00001100, // 3-4
  0b10000100, // 3-8
  0b00011000, // 4-5
  0b00101000, // 4-6
  0b01001000, // 4-7
  0b10001000, // 4-8
  0b00110000, // 5-6
  0b10010000, // 5-8
  0b11000000, // 7-8
  0b00000001, // 1 (los)
  0b00000010, // 2 (los)
  0b00000100, // 3 (los)
  0b00001000, // 4 (los)
  0b00010000, // 5 (los)
  0b00100000, // 6 (los)
  0b01000000, // 7 (los)
  0b10000000, // 8 (los)
};
const int AANTAL_MOGELIJKHEDEN = sizeof(MOGELIJKHEDEN) / sizeof(MOGELIJKHEDEN[0]);

uint8_t huidigeCombinatie = 0;
Licht status = ROOD;
unsigned long faseStart = 0;

const unsigned long ROOD_TIJD = 1000;
const unsigned long MIN_GROEN = 3000;
const unsigned long MAX_GROEN = 8000;
const unsigned long GEEL_TIJD = 2000;

bool comboVoldoet(uint8_t combo) {
  for (int bit = 0; bit < 8; bit++) {
    if ((combo >> bit) & 1 && !autoAanwezig[bit]) return false;
  }
  return true;
}

bool comboHeeftNogAuto(uint8_t combo) {
  for (int bit = 0; bit < 8; bit++) {
    if ((combo >> bit) & 1 && autoAanwezig[bit]) return true;
  }
  return false;
}

uint8_t kiesCombinatie() {
  for (int i = 0; i < AANTAL_MOGELIJKHEDEN; i++) {
    if (comboVoldoet(MOGELIJKHEDEN[i])) return MOGELIJKHEDEN[i];
  }
  return 0; // nergens een auto
}

void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  SensorMelding m;
  memcpy(&m, data, sizeof(m));
  if (m.slaveId == 1) {
    autoAanwezig[2] = m.autoEen;  // licht 3
    autoAanwezig[3] = m.autoTwee; // licht 4
  } else if (m.slaveId == 2) {
    autoAanwezig[6] = m.autoEen;  // licht 7
    autoAanwezig[7] = m.autoTwee; // licht 8
  }
}

void onDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {}

void zetLicht(int rood, int geel, int groen, bool actief, Licht s) {
  digitalWrite(rood, !actief || s == ROOD);
  digitalWrite(geel, actief && s == GEEL);
  digitalWrite(groen, actief && s == GROEN);
}

void toonLichten() {
  zetLicht(LICHT1_ROOD, LICHT1_GEEL, LICHT1_GROEN, (huidigeCombinatie >> 0) & 1, status);
  zetLicht(LICHT2_ROOD, LICHT2_GEEL, LICHT2_GROEN, (huidigeCombinatie >> 1) & 1, status);
  zetLicht(LICHT5_ROOD, LICHT5_GEEL, LICHT5_GROEN, (huidigeCombinatie >> 4) & 1, status);
  zetLicht(LICHT6_ROOD, LICHT6_GEEL, LICHT6_GROEN, (huidigeCombinatie >> 5) & 1, status);
}

void stuurCommando() {
  LichtCommando cmd = {huidigeCombinatie, (uint8_t)status};
  esp_now_send(broadcastAddress, (uint8_t *)&cmd, sizeof(cmd));
}

void setup() {
  Serial.begin(115200);
  int pins[] = {LICHT1_ROOD, LICHT1_GEEL, LICHT1_GROEN, LICHT2_ROOD, LICHT2_GEEL, LICHT2_GROEN,
                LICHT5_ROOD, LICHT5_GEEL, LICHT5_GROEN, LICHT6_ROOD, LICHT6_GEEL, LICHT6_GROEN};
  for (int p : pins) pinMode(p, OUTPUT);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init mislukt");
    return;
  }
  esp_now_register_recv_cb(onDataRecv);
  esp_now_register_send_cb(onDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  faseStart = millis();
  toonLichten();
  stuurCommando();
}

void loop() {
  autoAanwezig[0] = leesLDR(LICHT1_LDR);
  autoAanwezig[1] = leesLDR(LICHT2_LDR);
  autoAanwezig[4] = leesLDR(LICHT5_LDR);
  autoAanwezig[5] = leesLDR(LICHT6_LDR);

  unsigned long nu = millis();
  uint8_t vorigeCombinatie = huidigeCombinatie;
  Licht vorigeStatus = status;

  switch (status) {
    case ROOD:
      if (nu - faseStart >= ROOD_TIJD) {
        huidigeCombinatie = kiesCombinatie();
        status = huidigeCombinatie ? GROEN : ROOD;
        faseStart = nu;
      }
      break;
    case GROEN:
      if ((nu - faseStart >= MIN_GROEN && !comboHeeftNogAuto(huidigeCombinatie)) || nu - faseStart >= MAX_GROEN) {
        status = GEEL;
        faseStart = nu;
      }
      break;
    case GEEL:
      if (nu - faseStart >= GEEL_TIJD) {
        huidigeCombinatie = 0;
        status = ROOD;
        faseStart = nu;
      }
      break;
  }

  if (status != vorigeStatus || huidigeCombinatie != vorigeCombinatie) {
    toonLichten();
    stuurCommando();
  }
}
