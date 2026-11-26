#pragma once
#include <VriProtocol.h>

// ESP-NOW link to the slaves.

// Starts ESP-NOW with the broadcast address as peer. Restarts the board when
// ESP-NOW cannot start. WiFi must already be in station mode.
void commsBegin();

// Broadcasts the message to both slaves.
void commsSend(const SignalMessage &message);

// True when SEND_INTERVAL_MS has passed since the last send.
bool commsResendDue();
