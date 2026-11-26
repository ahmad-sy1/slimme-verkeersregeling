#include "Comms.h"

#include <Arduino.h>
#include <esp_now.h>
#include <esp_arduino_version.h>
#include <VriConfig.h>

// The receive callback runs in the Wi-Fi task. It only copies the message;
// loop() does the work. The spinlock stops loop() from reading a half-written
// message when a new one arrives on the other core.
static SignalMessage latestMessage;
static unsigned long lastMessageAt = 0;
static bool hasHeardMaster = false;
static portMUX_TYPE messageLock = portMUX_INITIALIZER_UNLOCKED;

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

void commsBegin() {
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed, restarting...");
    delay(1000);
    ESP.restart();
  }
  esp_now_register_recv_cb(onReceive);
}

bool commsLatestMessage(SignalMessage &message) {
  unsigned long messageAt;
  bool heardMaster;
  portENTER_CRITICAL(&messageLock);
  memcpy(&message, &latestMessage, sizeof(message));
  messageAt = lastMessageAt;
  heardMaster = hasHeardMaster;
  portEXIT_CRITICAL(&messageLock);

  return heardMaster && millis() - messageAt <= MASTER_TIMEOUT_MS;
}
