// Practice assignment 4 - ESP-NOW slave (receiver).
// Waits for "I am <name>" from the master and replies "Hello <name>, I am <own name>".

// ===== CHANGE THIS: your own name =====
const char *STUDENT_NAME = "Janna";
// ======================================

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <VriConfig.h>

#include "../espnow_common/espnow_message.h"

const uint8_t MASTER_MAC[6] = MAC_MASTER;

// The receive callback runs in the WiFi task. It only copies the data here;
// loop() does the printing and replying. The spinlock stops loop() from
// reading a half-written message when a new one arrives on the other core.
EspNowMessage incoming;
volatile bool messageReceived = false;
portMUX_TYPE incomingLock = portMUX_INITIALIZER_UNLOCKED;

// Delivery status of the reply, recorded by onSent for loop() to print.
volatile bool sendDone = false;
volatile bool sendOk = false;

#if ESPNOW_NEW_RECV_CB
void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
#else
void onReceive(const uint8_t *mac, const uint8_t *data, int len) {
#endif
  // Anything with a different size was not sent with EspNowMessage.
  if (len != sizeof(EspNowMessage)) return;

  portENTER_CRITICAL(&incomingLock);
  memcpy(&incoming, data, sizeof(incoming));
  messageReceived = true;
  portEXIT_CRITICAL(&incomingLock);
}

#if ESPNOW_NEW_SEND_CB
void onSent(const esp_now_send_info_t *info, esp_now_send_status_t status) {
#else
void onSent(const uint8_t *mac, esp_now_send_status_t status) {
#endif
  sendOk = (status == ESP_NOW_SEND_SUCCESS);
  sendDone = true;
}

void sendReply(const char *masterName) {
  EspNowMessage reply = {};  // Zeroed so no stale bytes go over the air
  strncpy(reply.name, STUDENT_NAME, sizeof(reply.name) - 1);
  snprintf(reply.text, sizeof(reply.text), "Hello %s, I am %s", masterName, STUDENT_NAME);

  esp_err_t result = esp_now_send(MASTER_MAC, (const uint8_t *)&reply, sizeof(reply));
  if (result == ESP_OK) {
    Serial.print("Sent: ");
    Serial.println(reply.text);
  } else {
    Serial.print("Send error: ");
    Serial.println(esp_err_to_name(result));
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // ESP-NOW runs on top of the WiFi radio, which must be in station mode.
  // VriConfig.h stores station-mode MACs, so this also makes them match.
  WiFi.mode(WIFI_STA);

  Serial.println();
  Serial.print("Slave MAC: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  esp_now_register_recv_cb(onReceive);
  esp_now_register_send_cb(onSent);

  // Receiving works from anyone, but replying needs the master as a known
  // peer. Channel 0 means "the current channel", which is the same on both
  // boards as long as neither joins a WiFi network.
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, MASTER_MAC, 6);
  peer.channel = 0;
  peer.encrypt = false;
  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("Adding master as peer failed");
    return;
  }

  Serial.print("Master peer: ");
  printMac(MASTER_MAC);
  Serial.println();
  Serial.println("Waiting for the master...");
}

void loop() {
  if (messageReceived) {
    EspNowMessage message;
    portENTER_CRITICAL(&incomingLock);
    memcpy(&message, &incoming, sizeof(message));
    messageReceived = false;
    portEXIT_CRITICAL(&incomingLock);

    // Never trust the sender to terminate its strings.
    message.name[sizeof(message.name) - 1] = '\0';
    message.text[sizeof(message.text) - 1] = '\0';

    Serial.print("Received: ");
    Serial.println(message.text);
    sendReply(message.name);
  }

  if (sendDone) {
    sendDone = false;
    Serial.println(sendOk ? "Delivery: success" : "Delivery: failed (is the master on?)");
  }
}
