// Slave firmware, shared by the main road and the side road slave. The board
// reads its own MAC address at start-up and picks the matching pin table.
#include <Arduino.h>
#include <WiFi.h>
#include <VriConfig.h>
#include <VriProtocol.h>

#include "Comms.h"
#include "SignalHead.h"

static bool hasRole = false;
static uint8_t ownMac[6];

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

  const char *title = signalHeadsBegin(ownMac);
  if (title == nullptr) {
    printUnknownMac();
    return;
  }
  hasRole = true;
  Serial.printf("\n=== Intersection SLAVE %s ===\n", title);

  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  commsBegin();
}

void loop() {
  if (!hasRole) {
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
  if (!commsLatestMessage(message)) {
    // No master -> flashing orange, like a real intersection on a fault
    signalHeadsFlashOrange();
    if (!wasFailSafe) Serial.println("No signal from master -> flashing orange");
    wasFailSafe = true;
    return;
  }

  if (wasFailSafe) Serial.println("Master found");
  wasFailSafe = false;

  signalHeadsShow(message);

  if (message.phase != lastPhase) {
    lastPhase = message.phase;
    Serial.printf("Phase %d\n", message.phase);
  }
}
