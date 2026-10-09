// Slave firmware, shared by the main road and the side road slave. The board
// reads its own MAC address at start-up and picks the matching pin table.
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_arduino_version.h>
#include <VriConfig.h>
#include <VriProtocol.h>

struct SlaveRole {
  uint8_t mac[6];
  const char *title;
  const SignalHeadPins *heads;
  int headCount;
};

const SlaveRole ROLES[] = {
  {MAC_SLAVE_MAIN_ROAD, "MAIN ROAD", MAIN_ROAD_PINS, sizeof(MAIN_ROAD_PINS) / sizeof(MAIN_ROAD_PINS[0])},
  {MAC_SLAVE_SIDE_ROAD, "SIDE ROAD", SIDE_ROAD_PINS, sizeof(SIDE_ROAD_PINS) / sizeof(SIDE_ROAD_PINS[0])},
};

static const SlaveRole *role = nullptr;
static const SignalHeadPins *heads = nullptr;
static int headCount = 0;

// The receive callback runs in the Wi-Fi task. It only copies the message;
// loop() does the work. The spinlock stops loop() from reading a half-written
// message when a new one arrives on the other core.
static SignalMessage latestMessage;
static unsigned long lastMessageAt = 0;
static bool hasHeardMaster = false;
static portMUX_TYPE messageLock = portMUX_INITIALIZER_UNLOCKED;
static uint8_t ownMac[6];

static void setAspect(const SignalHeadPins &head, uint8_t aspect) {
  digitalWrite(head.red, aspect == ASPECT_RED);
  digitalWrite(head.orange, aspect == ASPECT_ORANGE);
  digitalWrite(head.green, aspect == ASPECT_GREEN);
}

static void handleMessage(const uint8_t *data, int len) {
  if (len != sizeof(SignalMessage) || data[0] != MESSAGE_MAGIC) return;
  unsigned long now = millis();
  portENTER_CRITICAL(&messageLock);
  memcpy(&latestMessage, data, sizeof(latestMessage));
  lastMessageAt = now;
  hasHeardMaster = true;
  portEXIT_CRITICAL(&messageLock);
}

#if ESP_ARDUINO_VERSION_MAJOR >= 3
static void onReceive(const esp_now_recv_info_t *, const uint8_t *data, int len) { handleMessage(data, len); }
#else
static void onReceive(const uint8_t *, const uint8_t *data, int len) { handleMessage(data, len); }
#endif

static const SlaveRole *findRole(const uint8_t *mac) {
  for (const SlaveRole &candidate : ROLES) {
    if (memcmp(candidate.mac, mac, 6) == 0) return &candidate;
  }
  return nullptr;
}

// Without a role it is unknown which pin is which LED, so every LED pin is
// driven LOW. A floating pin can light an LED at random.
static void allLedsOff() {
  for (uint8_t pin : SLAVE_LED_PINS) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
}

static void printUnknownMac() {
  Serial.printf("ERROR: MAC %02X:%02X:%02X:%02X:%02X:%02X is not a slave in VriConfig.h, "
                "all LEDs stay off\n",
                ownMac[0], ownMac[1], ownMac[2], ownMac[3], ownMac[4], ownMac[5]);
}

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

  heads = role->heads;
  headCount = role->headCount;
  Serial.printf("\n=== Intersection SLAVE %s ===\n", role->title);

  for (int i = 0; i < headCount; i++) {
    pinMode(heads[i].red, OUTPUT);
    pinMode(heads[i].orange, OUTPUT);
    pinMode(heads[i].green, OUTPUT);
    setAspect(heads[i], ASPECT_OFF);
  }

  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed, restarting...");
    delay(1000);
    ESP.restart();
  }
  esp_now_register_recv_cb(onReceive);
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

  static bool wasFailSafe = true;
  static uint8_t lastPhase = 0xFF;

  SignalMessage message;
  unsigned long messageAt;
  bool heardMaster;
  portENTER_CRITICAL(&messageLock);
  memcpy(&message, &latestMessage, sizeof(message));
  messageAt = lastMessageAt;
  heardMaster = hasHeardMaster;
  portEXIT_CRITICAL(&messageLock);

  bool failSafe = !heardMaster || millis() - messageAt > MASTER_TIMEOUT_MS;

  if (failSafe) {
    // No master -> flashing orange, like a real intersection on a fault
    bool orangeOn = (millis() / 500) % 2;
    for (int i = 0; i < headCount; i++) setAspect(heads[i], orangeOn ? ASPECT_ORANGE : ASPECT_OFF);
    if (!wasFailSafe) Serial.println("No signal from master -> flashing orange");
    wasFailSafe = true;
    return;
  }

  if (wasFailSafe) Serial.println("Master found");
  wasFailSafe = false;

  for (int i = 0; i < headCount; i++) setAspect(heads[i], message.aspects[heads[i].id]);

  if (message.phase != lastPhase) {
    lastPhase = message.phase;
    Serial.printf("Phase %d\n", message.phase);
  }
}
