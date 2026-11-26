// SIGNAL HEAD TEST: walks every signal head (red, orange, green) and prints
// which GPIO is on. Does the output not match what you see? Fix the pin table
// in VriConfig.h. The board picks its pin table by MAC address, like the slave.
#include <Arduino.h>
#include <WiFi.h>
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

static const SignalHeadPins *heads = nullptr;
static int headCount = 0;

static void setAspect(const SignalHeadPins &head, uint8_t aspect) {
  digitalWrite(head.red, aspect == ASPECT_RED);
  digitalWrite(head.orange, aspect == ASPECT_ORANGE);
  digitalWrite(head.green, aspect == ASPECT_GREEN);
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
      heads = role.heads;
      headCount = role.headCount;
      Serial.printf("\n=== PIN TEST %s ===\n", role.title);
      return;
    }
  }
  Serial.printf("ERROR: MAC %02X:%02X:%02X:%02X:%02X:%02X is not a slave in VriConfig.h, "
                "all LEDs stay off\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void loop() {
  if (heads == nullptr) return;

  const char *ASPECT_NAMES[] = {"", "RED   ", "ORANGE", "GREEN "};
  for (int i = 0; i < headCount; i++) {
    const SignalHeadPins &head = heads[i];
    uint8_t pins[] = {0, head.red, head.orange, head.green};
    for (uint8_t aspect = ASPECT_RED; aspect <= ASPECT_GREEN; aspect++) {
      Serial.printf("%-10s %s  -> GPIO%d\n", head.name, ASPECT_NAMES[aspect], pins[aspect]);
      setAspect(head, aspect);
      delay(2000);
    }
    setAspect(head, ASPECT_OFF);
  }
  Serial.println("--- again ---");
}
