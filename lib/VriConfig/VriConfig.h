#pragma once

#include <stdint.h>
#include <VriProtocol.h>

// Hardware configuration for the smart traffic control prototype.
// MAC addresses are fixed per board and read with the mac_address sketch.
// Roles follow the wiring check of 2026-10-08 (docs/wiring-check.md).

// Master - controller (determines timing, drives the slaves)
#define MAC_MASTER          { 0x94, 0xB9, 0x7E, 0xDA, 0xE2, 0x14 }

// Slave 1 - main road (Oranjesingel)
#define MAC_SLAVE_MAIN_ROAD { 0x94, 0xB9, 0x7E, 0xD9, 0xE3, 0xD4 }

// Slave 2 - side road
#define MAC_SLAVE_SIDE_ROAD { 0x7C, 0x9E, 0xBD, 0x65, 0x72, 0xFC }

// ---- Pins -------------------------------------------------------------------

// The red, orange and green LED of one signal head. Pin HIGH = LED on.
struct SignalHeadPins {
  SignalHeadId id;
  const char *name;
  uint8_t red, orange, green;
};

// Checked per LED on the intersection with pin scan (2026-10-08).
// {id,           name,       red, orange, green}
const SignalHeadPins MAIN_ROAD_PINS[] = {
  {MAIN_A_LEFT,  "A left",    18,  13,  14},
  {MAIN_A_RIGHT, "A right",   19,  21,  22},
  {MAIN_B_LEFT,  "B left",    27,  32,  26},
  {MAIN_B_RIGHT, "B right",   25,  33,  23},
};

// Side road A left red (32) and A right orange (13) did not light up visibly in
// the test; they are the only pins left and the LED voltage measurement fits.
const SignalHeadPins SIDE_ROAD_PINS[] = {
  {SIDE_A_LEFT,  "A left",    32,  33,  27},
  {SIDE_A_RIGHT, "A right",   14,  13,  26},
  {SIDE_B_LEFT,  "B left",    25,  23,  22},
  {SIDE_B_RIGHT, "B right",   21,  18,  19},
};

// Every LED pin used on both slaves, for the test tools and for keeping all
// LEDs off on a board whose role is unknown.
const uint8_t SLAVE_LED_PINS[] = {13, 14, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33};

// ---- Timing (ms) ------------------------------------------------------------

// Fixed-timing cycle on the master.
const unsigned long FIXED_GREEN_MS     = 7000;
const unsigned long ORANGE_MS          = 3000;
const unsigned long CLEARANCE_MS       = 2000;  // all red between two phases
const unsigned long STARTUP_ALL_RED_MS = 3000;  // all red after start-up

// The master repeats its message every SEND_INTERVAL_MS. A slave that hears
// nothing for MASTER_TIMEOUT_MS flashes orange (fail-safe).
const unsigned long SEND_INTERVAL_MS  = 100;
const unsigned long MASTER_TIMEOUT_MS = 1500;

// A slave with an unknown MAC repeats its error, so it is also seen when the
// serial monitor is opened after boot.
const unsigned long UNKNOWN_MAC_REPEAT_MS = 5000;
