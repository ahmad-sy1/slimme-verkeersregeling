// MASTER: the brain. It has no lights of its own; it only times the phases
// and broadcasts the aspect of all 8 signal heads over ESP-NOW to both slaves.
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <VriConfig.h>
#include <VriProtocol.h>

// ---- Phases: which signal heads may be green together ----
// Each phase is one road, both directions (A+B) of the same lane -> no conflicts.
struct Phase {
  const char *name;
  uint8_t signalHeads;  // bitmask of SignalHeadId
};

#define HEAD_BIT(id) (1u << (id))

const Phase PHASES[] = {
  {"Main road right (A+B)", HEAD_BIT(MAIN_A_RIGHT) | HEAD_BIT(MAIN_B_RIGHT)},
  {"Main road left (A+B)",  HEAD_BIT(MAIN_A_LEFT)  | HEAD_BIT(MAIN_B_LEFT)},
  {"Side road right (A+B)", HEAD_BIT(SIDE_A_RIGHT) | HEAD_BIT(SIDE_B_RIGHT)},
  {"Side road left (A+B)",  HEAD_BIT(SIDE_A_LEFT)  | HEAD_BIT(SIDE_B_LEFT)},
};
const int PHASE_COUNT = sizeof(PHASES) / sizeof(PHASES[0]);

enum Step { STEP_GREEN, STEP_ORANGE, STEP_CLEARANCE };

const uint8_t BROADCAST_MAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

int phase = PHASE_COUNT - 1;  // the first clearance moves on to phase 0
Step step = STEP_CLEARANCE;
unsigned long stepStartedAt = 0;
unsigned long stepDurationMs = STARTUP_ALL_RED_MS;
unsigned long lastSentAt = 0;

void sendSignals() {
  SignalMessage message;
  message.magic = MESSAGE_MAGIC;
  message.phase = phase;
  for (int i = 0; i < SIGNAL_HEAD_COUNT; i++) {
    bool active = step != STEP_CLEARANCE && (PHASES[phase].signalHeads & HEAD_BIT(i));
    message.aspects[i] = !active ? ASPECT_RED : (step == STEP_GREEN ? ASPECT_GREEN : ASPECT_ORANGE);
  }
  esp_now_send(BROADCAST_MAC, (const uint8_t *)&message, sizeof(message));
  lastSentAt = millis();
}

void enterStep(Step next, unsigned long durationMs) {
  step = next;
  stepStartedAt = millis();
  stepDurationMs = durationMs;
  const char *STEP_NAMES[] = {"GREEN", "ORANGE", "ALL RED"};
  Serial.printf("[%7lu] %-22s %s\n", stepStartedAt, PHASES[phase].name, STEP_NAMES[step]);
  sendSignals();
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n=== Intersection MASTER (fixed timing) ===");

  WiFi.mode(WIFI_STA);
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

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

  Serial.println("Startup: all red");
  stepStartedAt = millis();
  sendSignals();
}

void loop() {
  if (millis() - stepStartedAt >= stepDurationMs) {
    switch (step) {
      case STEP_GREEN:
        enterStep(STEP_ORANGE, ORANGE_MS);
        break;
      case STEP_ORANGE:
        enterStep(STEP_CLEARANCE, CLEARANCE_MS);
        break;
      case STEP_CLEARANCE:
        phase = (phase + 1) % PHASE_COUNT;
        enterStep(STEP_GREEN, FIXED_GREEN_MS);
        break;
    }
  }

  // Keep repeating, so a slave that (re)starts joins in straight away
  if (millis() - lastSentAt >= SEND_INTERVAL_MS) sendSignals();
}
