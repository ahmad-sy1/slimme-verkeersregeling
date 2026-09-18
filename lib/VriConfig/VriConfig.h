#pragma once

// Hardware configuration for the smart traffic control prototype.
// MAC addresses are fixed per board and read with the mac_address sketch.
// The label in each comment matches the sticker on the physical board.

// Slave 1 - main road (Oranjesingel)
#define MAC_SLAVE1_MAIN_ROAD { 0x94, 0xB9, 0x7E, 0xC4, 0x98, 0x68 }

// Master - controller
#define MAC_MASTER { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }