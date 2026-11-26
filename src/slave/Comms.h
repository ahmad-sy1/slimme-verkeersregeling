#pragma once
#include <VriProtocol.h>

// ESP-NOW link to the master.

// Starts ESP-NOW and listens for the master. Restarts the board when ESP-NOW
// cannot start. WiFi must already be in station mode.
void commsBegin();

// Copies the latest message from the master. Returns false when the master
// has not been heard yet or is silent for longer than MASTER_TIMEOUT_MS.
bool commsLatestMessage(SignalMessage &message);
