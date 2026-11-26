#pragma once
#include <VriProtocol.h>

// Fixed-timing cycle: four phases, each green -> orange -> all red clearance.

// Starts the cycle in all red.
void controllerBegin();

// Moves to the next step when the current one has lasted long enough and
// logs it. Returns true on a step change, so the new aspects go out at once.
bool controllerUpdate();

// The message for the current phase and step.
SignalMessage controllerMessage();
