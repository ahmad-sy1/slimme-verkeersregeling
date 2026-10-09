#pragma once
#include <stdint.h>

// Gedeeld tussen master en slaves: wat er over ESP-NOW gaat.

enum Kleur : uint8_t { UIT = 0, ROOD = 1, GEEL = 2, GROEN = 3 };

// Alle 8 lichten van het kruispunt (namen zoals in "waarnemingen kruispunt.xlsx")
enum LichtId : uint8_t {
  HW_A_LINKS, HW_A_RECHTS, HW_B_LINKS, HW_B_RECHTS,  // slave hoofdweg
  ZW_A_LINKS, ZW_A_RECHTS, ZW_B_LINKS, ZW_B_RECHTS,  // slave zijweg
  AANTAL_LICHTEN
};

const uint8_t BERICHT_MAGIC = 0x4B;  // 'K' - negeer vreemde ESP-NOW pakketjes

struct __attribute__((packed)) LichtBericht {
  uint8_t magic;
  uint8_t fase;                   // alleen voor debug
  uint8_t kleur[AANTAL_LICHTEN];  // Kleur per licht
};

// Master stuurt dit elke VERSTUUR_INTERVAL ms opnieuw.
// Hoort een slave langer dan TIMEOUT_MS niets -> knipperend geel (storing).
const unsigned long VERSTUUR_INTERVAL = 100;
const unsigned long TIMEOUT_MS = 1500;
