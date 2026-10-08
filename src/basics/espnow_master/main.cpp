// Practice assignment 4 - ESP-NOW master (sender).
// Sends "I am <name>" to the slave and prints the reply it gets back.

// ===== CHANGE THIS: your own name =====
const char *STUDENT_NAME = "Laura";
// ======================================

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <VriConfig.h>

#include "../espnow_common/espnow_message.h"

// Resend periodically so the slave still gets a message if it boots later
// than the master or a packet is lost.
const unsigned long SEND_INTERVAL_MS = 5000;

const uint8_t SLAVE_MAC[6] = MAC_SLAVE_MAIN_ROAD;

// The receive callback runs in the WiFi task. It only copies the data here;
// loop() does the slow Serial printing. The spinlock stops loop() from reading
// a half-written message when a new one arrives on the other core.
EspNowMessage incoming;
volatile bool replyReceived = false;
portMUX_TYPE incomingLock = portMUX_INITIALIZER_UNLOCKED;

unsigned long lastSendMs = 0;

#if ESPNOW_NEW_RECV_CB
void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
#else
void onReceive(const uint8_t *mac, const uint8_t *data, int len) {
#endif
  // Anything with a different size was not sent with EspNowMessage.
  if (len != sizeof(EspNowMessage)) return;

  portENTER_CRITICAL(&incomingLock);
  memcpy(&incoming, data, sizeof(incoming));
  replyReceived = true;
  portEXIT_CRITICAL(&incomingLock);
}

// Reports whether the slave's radio acknowledged the packet. A failure usually
// means the slave is off or its MAC in VriConfig.h is wrong. Like onReceive it
// runs in the WiFi task, so it only records the result for loop() to print.
volatile bool sendDone = false;
volatile bool sendOk = false;

#if ESPNOW_NEW_SEND_CB
void onSent(const esp_now_send_info_t *info, esp_now_send_status_t status) {
#else
void onSent(const uint8_t *mac, esp_now_send_status_t status) {
#endif
  sendOk = (status == ESP_NOW_SEND_SUCCESS);
  sendDone = true;
}

void sendGreeting() {
  EspNowMessage message = {};  // Zeroed so no stale bytes go over the air
  strncpy(message.name, STUDENT_NAME, sizeof(message.name) - 1);
  snprintf(message.text, sizeof(message.text), "I am %s", STUDENT_NAME);

  esp_err_t result = esp_now_send(SLAVE_MAC, (const uint8_t *)&message, sizeof(message));
  if (result == ESP_OK) {
    Serial.print("Sent: ");
    Serial.println(message.text);
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
  Serial.print("Master MAC: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  esp_now_register_recv_cb(onReceive);
  esp_now_register_send_cb(onSent);

  // ESP-NOW only sends to known peers. Channel 0 means "the current channel",
  // which is the same on both boards as long as neither joins a WiFi network.
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, SLAVE_MAC, 6);
  peer.channel = 0;
  peer.encrypt = false;
  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("Adding slave as peer failed");
    return;
  }

  Serial.print("Slave peer: ");
  printMac(SLAVE_MAC);
  Serial.println();
}

void loop() {
  if (millis() - lastSendMs >= SEND_INTERVAL_MS) {
    lastSendMs = millis();
    sendGreeting();
  }

  if (sendDone) {
    sendDone = false;
    Serial.println(sendOk ? "Delivery: success" : "Delivery: failed (is the slave on?)");
  }

  if (replyReceived) {
    EspNowMessage reply;
    portENTER_CRITICAL(&incomingLock);
    memcpy(&reply, &incoming, sizeof(reply));
    replyReceived = false;
    portEXIT_CRITICAL(&incomingLock);

    reply.text[sizeof(reply.text) - 1] = '\0';  // Never trust the sender to terminate
    Serial.print("Received: ");
    Serial.println(reply.text);
  }
}
