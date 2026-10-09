#pragma once
#include <stdint.h>

// Shared by master and slaves: what goes over ESP-NOW.

enum Aspect : uint8_t { ASPECT_OFF = 0, ASPECT_RED = 1, ASPECT_ORANGE = 2, ASPECT_GREEN = 3 };

// All 8 signal heads of the intersection
enum SignalHeadId : uint8_t {
  MAIN_A_LEFT, MAIN_A_RIGHT, MAIN_B_LEFT, MAIN_B_RIGHT,  // main road slave
  SIDE_A_LEFT, SIDE_A_RIGHT, SIDE_B_LEFT, SIDE_B_RIGHT,  // side road slave
  SIGNAL_HEAD_COUNT
};

const uint8_t MESSAGE_MAGIC = 0x4B;  // 'K' - ignore other ESP-NOW packets

struct __attribute__((packed)) SignalMessage {
  uint8_t magic;
  uint8_t phase;                       // for debugging only
  uint8_t aspects[SIGNAL_HEAD_COUNT];  // Aspect per signal head
};
