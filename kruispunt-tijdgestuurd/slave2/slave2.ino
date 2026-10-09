#include <esp_now.h>
#include <WiFi.h>

// Slave 2: zijweg links van de hoofdweg (lichten 7 en 8)
#define LICHT7_ROOD   18
#define LICHT7_GEEL   19
#define LICHT7_GROEN  21
#define LICHT7_LDR    35

#define LICHT8_ROOD   2
#define LICHT8_GEEL   4
#define LICHT8_GROEN  5
#define LICHT8_LDR    34

const uint8_t SLAVE_ID = 2;

enum Licht { ROOD, GEEL, GROEN };

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct SensorMelding {
  uint8_t slaveId;
  bool autoEen;  // licht7
  bool autoTwee; // licht8
} SensorMelding;

typedef struct LichtCommando {
  uint8_t combinatie; // bit6 = licht7, bit7 = licht8
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
  int pins[] = {LICHT7_ROOD, LICHT7_GEEL, LICHT7_GROEN, LICHT8_ROOD, LICHT8_GEEL, LICHT8_GROEN};
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
  bool licht7Actief = (ontvangen.combinatie >> 6) & 1;
  bool licht8Actief = (ontvangen.combinatie >> 7) & 1;

  zetLicht(LICHT7_ROOD, LICHT7_GEEL, LICHT7_GROEN, licht7Actief, ontvangen.status);
  zetLicht(LICHT8_ROOD, LICHT8_GEEL, LICHT8_GROEN, licht8Actief, ontvangen.status);

  if (millis() - laatsteVerzonden > 200) {
    SensorMelding m;
    m.slaveId = SLAVE_ID;
    m.autoEen = leesLDR(LICHT7_LDR);
    m.autoTwee = leesLDR(LICHT8_LDR);
    esp_now_send(broadcastAddress, (uint8_t *)&m, sizeof(m));
    laatsteVerzonden = millis();
  }
}
