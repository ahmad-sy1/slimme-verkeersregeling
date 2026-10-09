// MASTER: the brain. It has no lights of its own; it only times the phases
// and broadcasts the aspect of all 8 signal heads over ESP-NOW to both slaves.
#include <Arduino.h>
#include <WiFi.h>

#include "Comms.h"
#include "Controller.h"

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n=== Intersection MASTER (fixed timing) ===");

  WiFi.mode(WIFI_STA);
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

  commsBegin();
  controllerBegin();
  commsSend(controllerMessage());
}

void loop() {
  if (controllerUpdate()) commsSend(controllerMessage());

  // Keep repeating, so a slave that (re)starts joins in straight away
  if (commsResendDue()) commsSend(controllerMessage());
}
