#pragma once

// Hardware configuration for the smart traffic control prototype.
// MAC addresses are fixed per board and read with the mac_address sketch.
// The label in each comment matches the sticker on the physical board.

// Master - controller (determines timing, drives the slaves)
#define MAC_MASTER          { 0x94, 0xB9, 0x7E, 0xDA, 0xE2, 0x14 }

// Slave 1 - main road (Oranjesingel)
#define MAC_SLAVE_MAIN_ROAD { 0x94, 0xB9, 0x7E, 0xC4, 0x98, 0x68 }

// Slave 2 - side road
#define MAC_SLAVE_SIDE_ROAD { 0x94, 0xB9, 0x7E, 0xD9, 0xE3, 0xD4 }

// Pin scan (src/basics/pin_scan): output-capable GPIOs that are safe to blink.
// Left out on purpose: strapping and flash pins (0, 2, 5, 12, 15, 6-11), the serial
// pins (1, 3) and the input-only pins (34-39).
#define PIN_SCAN_GPIOS      { 4, 13, 14, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33 }
#define PIN_SCAN_BLINKS     5      // blinks per GPIO
#define PIN_SCAN_BLINK_MS   300    // LED on and off time per blink
#define PIN_SCAN_PAUSE_MS   3000   // pause between two GPIOs, time to note the pole

// LED test (src/basics/led_test): blinks all PIN_SCAN_GPIOS together.
#define LED_TEST_BLINK_MS   500    // LED on and off time
