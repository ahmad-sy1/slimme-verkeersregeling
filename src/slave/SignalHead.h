#pragma once
#include <stdint.h>
#include <VriProtocol.h>

// Drives the LEDs of the signal heads on this slave.

// Picks the pin table that belongs to this board's MAC address and switches
// its LEDs off. Returns the role title ("MAIN ROAD" / "SIDE ROAD"), or nullptr
// for an unknown MAC; then every LED pin is driven LOW and stays off.
const char *signalHeadsBegin(const uint8_t *mac);

// Shows the aspect the master commands for each signal head of this slave.
void signalHeadsShow(const SignalMessage &message);

// Fail-safe: all signal heads flash orange. Call it every loop.
void signalHeadsFlashOrange();
