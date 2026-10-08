#pragma once

#include <Arduino.h>
#include <esp_idf_version.h>

// Shared by the espnow_master and espnow_slave sketches. ESP-NOW only moves
// raw bytes, so both sides must use this exact layout or the receiver reads
// the wrong bytes into the wrong fields.
struct EspNowMessage {
  char name[32];  // Sender's name, so the slave can greet the master by name
  char text[64];  // Full sentence to print, e.g. "I am Laura"
};

// ESP-NOW refuses payloads larger than 250 bytes.
static_assert(sizeof(EspNowMessage) <= 250, "EspNowMessage is too large for ESP-NOW");

// The ESP-NOW callback signatures come from ESP-IDF, not from Arduino itself.
// Arduino core 2.x ships IDF 4.4, core 3.x ships IDF 5.x, and IDF 5.5 (core
// 3.3+) changed the send callback again. Branching on the IDF version keeps
// the sketches compiling whichever core a team member's PlatformIO resolves.
#define ESPNOW_NEW_RECV_CB (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0))
#define ESPNOW_NEW_SEND_CB (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0))

// Prints a MAC address as AA:BB:CC:DD:EE:FF, matching the mac_address sketch
// so the output can be compared directly with VriConfig.h.
inline void printMac(const uint8_t *mac) {
  for (int i = 0; i < 6; i++) {
    if (mac[i] < 0x10) Serial.print("0");
    Serial.print(mac[i], HEX);
    if (i < 5) Serial.print(":");
  }
}
