// SIGNAL HEAD TEST: number the poles of the intersection. Send a signal head
// number over serial and all three LEDs of that signal head switch on until the
// next number. The board picks its pin table by MAC address, like the slave.
//   "1".."8"   signal head number = SignalHeadId + 1
//              (1 = MAIN_A_LEFT ... 4 = MAIN_B_RIGHT, 5 = SIDE_A_LEFT ... 8 = SIDE_B_RIGHT)
//              a slave only accepts the four numbers of its own signal heads
//   "0"        all off
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

static const SlaveRole *role = nullptr;
static String line;

// A floating pin can light an LED at random, so "off" drives every LED pin LOW,
// not only the pins of this slave's table.
static void allOff() {
  for (uint8_t pin : SLAVE_LED_PINS) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
}

static const SignalHeadPins *findHead(int number) {
  for (int i = 0; i < role->headCount; i++) {
    if (role->heads[i].id + 1 == number) return &role->heads[i];
  }
  return nullptr;
}

static void printOwnNumbers() {
  Serial.printf("This slave accepts:");
  for (int i = 0; i < role->headCount; i++) Serial.printf(" %d", role->heads[i].id + 1);
  Serial.println(", 0 = all off");
}

static void handleCommand(const String &command) {
  if (command == "0") {
    allOff();
    Serial.println("0: all off");
    return;
  }

  int number = command.toInt();
  const SignalHeadPins *head = command.length() > 0 ? findHead(number) : nullptr;
  if (head == nullptr) {
    Serial.printf("'%s' is not a signal head of this slave. ", command.c_str());
    printOwnNumbers();
    return;
  }

  allOff();
  digitalWrite(head->red, HIGH);
  digitalWrite(head->orange, HIGH);
  digitalWrite(head->green, HIGH);
  Serial.printf("%d: %s %s on - red GPIO%d, orange GPIO%d, green GPIO%d\n", number, role->title,
                head->name, head->red, head->orange, head->green);
}

void setup() {
  Serial.begin(115200);
  allOff();
  delay(200);

  uint8_t mac[6];
  WiFi.mode(WIFI_STA);
  WiFi.macAddress(mac);
  for (const SlaveRole &candidate : ROLES) {
    if (memcmp(candidate.mac, mac, 6) == 0) role = &candidate;
  }

  Serial.printf("\n=== SIGNAL HEAD TEST, MAC %02X:%02X:%02X:%02X:%02X:%02X ===\n", mac[0], mac[1],
                mac[2], mac[3], mac[4], mac[5]);
  if (role == nullptr) {
    Serial.println("ERROR: this MAC is not a slave in VriConfig.h, all LEDs stay off");
    return;
  }
  Serial.printf("This is the %s slave, all LEDs off. ", role->title);
  printOwnNumbers();
}

void loop() {
  if (role == nullptr) return;

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      line.trim();
      if (line.length() > 0) handleCommand(line);
      line = "";
    } else {
      line += c;
    }
  }
}
