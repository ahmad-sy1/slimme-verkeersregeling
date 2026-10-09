#include "SignalHead.h"

#include <Arduino.h>
#include <VriConfig.h>

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

const char *signalHeadsBegin(const uint8_t *mac) {
  const SlaveRole *role = findRole(mac);
  if (role == nullptr) {
    allLedsOff();
    return nullptr;
  }

  heads = role->heads;
  headCount = role->headCount;
  for (int i = 0; i < headCount; i++) {
    pinMode(heads[i].red, OUTPUT);
    pinMode(heads[i].orange, OUTPUT);
    pinMode(heads[i].green, OUTPUT);
    setAspect(heads[i], ASPECT_OFF);
  }
  return role->title;
}

void signalHeadsShow(const SignalMessage &message) {
  for (int i = 0; i < headCount; i++) setAspect(heads[i], message.aspects[heads[i].id]);
}

void signalHeadsFlashOrange() {
  bool orangeOn = (millis() / 500) % 2;
  for (int i = 0; i < headCount; i++) setAspect(heads[i], orangeOn ? ASPECT_ORANGE : ASPECT_OFF);
}
