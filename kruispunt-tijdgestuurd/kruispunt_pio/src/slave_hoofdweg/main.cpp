// SLAVE 1 - HOOFDWEG: stuurt 4 lichten aan, kleuren komen van de master.
// Pinnen per lampje gecontroleerd op het kruispunt (pinscan, 08-10-2026).
#include "slave_common.h"

//                id           naam         rood geel groen
const LichtPins LICHTEN[] = {
  {HW_A_LINKS,  "A links",   18,  13,  14},
  {HW_A_RECHTS, "A rechts",  19,  21,  22},
  {HW_B_LINKS,  "B links",   27,  32,  26},
  {HW_B_RECHTS, "B rechts",  25,  33,  23},
};

void setup() { slaveSetup("HOOFDWEG", LICHTEN, sizeof(LICHTEN) / sizeof(LICHTEN[0])); }
void loop() { slaveLoop(); }
