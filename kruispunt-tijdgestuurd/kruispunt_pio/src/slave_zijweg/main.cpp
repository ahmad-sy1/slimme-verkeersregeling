// SLAVE 2 - ZIJWEG: stuurt 4 lichten aan, kleuren komen van de master.
// Pinnen per lampje gecontroleerd op het kruispunt (pinscan, 08-10-2026).
// A links rood (32) en A rechts geel (13) lichtten in de test niet zichtbaar op;
// het zijn de enige overgebleven pinnen en de LED-spanningsmeting past erbij.
#include "slave_common.h"

//                id           naam         rood geel groen
const LichtPins LICHTEN[] = {
  {ZW_A_LINKS,  "A links",   32,  33,  27},
  {ZW_A_RECHTS, "A rechts",  14,  13,  26},
  {ZW_B_LINKS,  "B links",   25,  23,  22},
  {ZW_B_RECHTS, "B rechts",  21,  18,  19},
};

void setup() { slaveSetup("ZIJWEG", LICHTEN, sizeof(LICHTEN) / sizeof(LICHTEN[0])); }
void loop() { slaveLoop(); }
