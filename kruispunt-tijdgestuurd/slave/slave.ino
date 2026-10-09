#include <esp_now.h>
#include <WiFi.h>

// Slave 1: zijweg rechts van de hoofdweg (lichten 3 en 4)
#define LICHT3_GROEN  25
#define LICHT3_GEEL   26
#define LICHT3_ROOD   27
#define LICHT3_LDR    33

#define LICHT4_ROOD   12
#define LICHT4_GEEL   13
#define LICHT4_GROEN  14
#define LICHT4_LDR    32

const uint8_t SLAVE_ID = 1;

enum Licht { ROOD, GEEL, GROEN };

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct SensorMelding {
  uint8_t slaveId;
  bool autoEen;  // licht3
  bool autoTwee; // licht4
} SensorMelding;

typedef struct LichtCommando {
  uint8_t combinatie; // bit2 = licht3, bit3 = licht4
  uint8_t status;
} LichtCommando;

LichtCommando ontvangen = {0, ROOD};

// LDR drempel: test met Serial.println(analogRead(pin)) en pas dit getal aan
const int LDR_DREMPEL = 2000;

bool leesLDR(int pin) {
  return analogRead(pin) < LDR_DREMPEL;
}

void zetLicht(int rood, int geel, int groen, bool actief, uint8_t s) {
  digitalWrite(rood, !actief || s == ROOD);
  digitalWrite(geel, actief && s == GEEL);
  digitalWrite(groen, actief && s == GROEN);
}

void onCommand(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  memcpy(&ontvangen, data, sizeof(ontvangen));
}

void onSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {}

void setup() {
  Serial.begin(115200);
  int pins[] = {LICHT3_ROOD, LICHT3_GEEL, LICHT3_GROEN, LICHT4_ROOD, LICHT4_GEEL, LICHT4_GROEN};
  for (int p : pins) pinMode(p, OUTPUT);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init mislukt");
    return;
  }
  esp_now_register_recv_cb(onCommand);
  esp_now_register_send_cb(onSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

unsigned long laatsteVerzonden = 0;

void loop() {
  bool licht3Actief = (ontvangen.combinatie >> 2) & 1;
  bool licht4Actief = (ontvangen.combinatie >> 3) & 1;

  zetLicht(LICHT3_ROOD, LICHT3_GEEL, LICHT3_GROEN, licht3Actief, ontvangen.status);
  zetLicht(LICHT4_ROOD, LICHT4_GEEL, LICHT4_GROEN, licht4Actief, ontvangen.status);

  if (millis() - laatsteVerzonden > 200) {
    SensorMelding m;
    m.slaveId = SLAVE_ID;
    m.autoEen = leesLDR(LICHT3_LDR);
    m.autoTwee = leesLDR(LICHT4_LDR);
    esp_now_send(broadcastAddress, (uint8_t *)&m, sizeof(m));
    laatsteVerzonden = millis();
  }
}
