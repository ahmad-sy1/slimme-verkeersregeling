#include "Comms.h"

#include <Arduino.h>
#include <esp_now.h>
#include <VriConfig.h>

const uint8_t BROADCAST_MAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static unsigned long lastSentAt = 0;

void commsBegin() {
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed, restarting...");
    delay(1000);
    ESP.restart();
  }
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, BROADCAST_MAC, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);
}

void commsSend(const SignalMessage &message) {
  esp_now_send(BROADCAST_MAC, (const uint8_t *)&message, sizeof(message));
  lastSentAt = millis();
}

bool commsResendDue() {
  return millis() - lastSentAt >= SEND_INTERVAL_MS;
}
