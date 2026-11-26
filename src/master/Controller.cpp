#include "Controller.h"

#include <Arduino.h>
#include <VriConfig.h>

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

static int phase = PHASE_COUNT - 1;  // the first clearance moves on to phase 0
static Step step = STEP_CLEARANCE;
static unsigned long stepStartedAt = 0;
static unsigned long stepDurationMs = STARTUP_ALL_RED_MS;

static void enterStep(Step next, unsigned long durationMs) {
  step = next;
  stepStartedAt = millis();
  stepDurationMs = durationMs;
  const char *STEP_NAMES[] = {"GREEN", "ORANGE", "ALL RED"};
  Serial.printf("[%7lu] %-22s %s\n", stepStartedAt, PHASES[phase].name, STEP_NAMES[step]);
}

void controllerBegin() {
  Serial.println("Startup: all red");
  stepStartedAt = millis();
}

bool controllerUpdate() {
  if (millis() - stepStartedAt < stepDurationMs) return false;

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
  return true;
}

SignalMessage controllerMessage() {
  SignalMessage message;
  message.magic = MESSAGE_MAGIC;
  message.phase = phase;
  for (int i = 0; i < SIGNAL_HEAD_COUNT; i++) {
    bool active = step != STEP_CLEARANCE && (PHASES[phase].signalHeads & HEAD_BIT(i));
    message.aspects[i] = !active ? ASPECT_RED : (step == STEP_GREEN ? ASPECT_GREEN : ASPECT_ORANGE);
  }
  return message;
}
