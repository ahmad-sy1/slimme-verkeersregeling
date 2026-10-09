#pragma once

// Hardware configuration for the smart traffic control prototype.
// MAC addresses are fixed per board and read with the mac_address sketch.
// Roles follow the wiring check of 2026-10-08 (docs/wiring-check.md).

// Master - controller (determines timing, drives the slaves)
#define MAC_MASTER          { 0x94, 0xB9, 0x7E, 0xDA, 0xE2, 0x14 }

// Slave 1 - main road (Oranjesingel)
#define MAC_SLAVE_MAIN_ROAD { 0x94, 0xB9, 0x7E, 0xD9, 0xE3, 0xD4 }

// Slave 2 - side road
#define MAC_SLAVE_SIDE_ROAD { 0x7C, 0x9E, 0xBD, 0x65, 0x72, 0xFC }
