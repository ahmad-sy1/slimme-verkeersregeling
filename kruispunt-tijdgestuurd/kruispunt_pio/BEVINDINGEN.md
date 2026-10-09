# Slim kruispunt – bevindingen (08-10-2026)

## Systeem

- **Master** – het brein, heeft zelf geen lichten. Bepaalt de fases op tijd en stuurt elke 100 ms
  de kleur van alle 8 lichten via ESP-NOW (broadcast) naar beide slaves.
- **Slave 1 – hoofdweg** – stuurt de 4 lichten van de hoofdweg aan.
- **Slave 2 – zijweg** – stuurt de 4 lichten van de zijweg aan.
- Geen sensoren, alleen vaste tijden.

| board | MAC | poort (laatst gezien) |
|---|---|---|
| master | 94:B9:7E:DA:E2:14 | COM7 |
| slave 1 hoofdweg | 94:B9:7E:D9:E3:D4 | COM9 (eerst COM8) |
| slave 2 zijweg | 7C:9E:BD:65:72:FC | COM7 |

De COM-poort verandert als je een kabel omwisselt of opnieuw insteekt. Controleer welk board
het is via de MAC (staat in de upload-output en in de Serial Monitor bij opstarten).

## Cyclus

Elke stap: groen 7 s → oranje 3 s → alles rood 2 s. Na opstarten eerst 3 s alles rood.

1. Hoofdweg rechts (A + B) groen
2. Hoofdweg links (A + B) groen
3. Zijweg rechts (A + B) groen
4. Zijweg links (A + B) groen

Tijden staan bovenaan `src/master/main.cpp`.

**Storing:** hoort een slave langer dan 1,5 s niets van de master, dan knipperen al zijn lichten
oranje. Komt de master terug, dan doet de slave binnen een seconde weer mee.

Gemeten: over 90 s geen enkel ESP-NOW-bericht verloren, beide slaves wisselen binnen dezelfde
0,1 s van fase. ESP-NOW zorgt dus **niet** voor vertraging.

## Pinnen (gecontroleerd op het kruispunt)

Gebruikte pinnen op beide slaves: 13, 14, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33.
Pin HIGH = lampje aan.

### Hoofdweg (slave 1)

| licht | rood | oranje | groen |
|---|---|---|---|
| A links | 18 | 13 | 14 |
| A rechts | 19 | 21 | 22 |
| B links | 27 | 32 | 26 |
| B rechts | 25 | 33 | 23 |

### Zijweg (slave 2)

| licht | rood | oranje | groen |
|---|---|---|---|
| A links | 32 | 33 | 27 |
| A rechts | 14 | 13 | 26 |
| B links | 25 | 23 | 22 |
| B rechts | 21 | 18 | 19 |

Hoe bepaald: met de `pinscan`-firmware steeds 3 pinnen per board tegelijk aan (vast / traag
knipperen / snel knipperen) en per licht genoteerd wat er brandde.

| ronde | pinnen (vast, traag, snel) | hoofdweg | zijweg |
|---|---|---|---|
| 1 | 13, 14, 18 | 13 A links oranje, 14 A links groen, 18 A links rood | 13 niets, 14 A rechts rood, 18 B rechts oranje |
| 2 | 19, 21, 22 | 19 niet genoemd (enige overgebleven = A rechts rood), 21 A rechts oranje, 22 A rechts groen | 19 B rechts groen, 21 B rechts rood, 22 B links groen |
| 3 | 23, 25, 26 | 23 B rechts groen, 25 B rechts rood, 26 B links groen | 23 B links oranje, 25 B links rood, 26 A rechts groen |
| 4 | 27, 32, 33 | 27 B links rood, 32 B links oranje, 33 B rechts oranje | 27 A links groen, 32 niets, 33 A links oranje |

### Aandachtspunten

- **Zijweg A links rood (GPIO32) en A rechts oranje (GPIO13)** lichtten in geen enkele test
  zichtbaar op. Het zijn de enige overgebleven pinnen voor die lampjes, en de spanningsmeting
  (zie hieronder) laat zien dat er wel een LED op zit. Waarschijnlijk los draadje, LED verkeerd om
  of een heel zwak lampje: bedrading van die twee controleren.
- **Hoofdweg A rechts rood (GPIO19)** werd in de test niet genoemd (vast aan is makkelijk te
  missen), maar is de enige overgebleven pin voor dat lampje.
- "Links/rechts" volgt de namen zoals ze op het kruispunt gebruikt zijn. Gaan de verkeerde twee
  lichten tegelijk op groen (bv. A links met B rechts), dan moeten in de slave-tabel de regels van
  B links en B rechts omgewisseld worden.

## Wat er misging (lessen)

- **`waarnemingen kruispunt.xlsx` klopt niet met de huidige bedrading.** De pinnamen in kolom A
  (D12, D13, …) zijn van de oude pinout. Ook als je de rijen leest als stap 1..12 van de testcode
  zijn meerdere oranje/groen/rood-waarden fout. Gebruik de tabellen hierboven.
- De oude pinout (`pinout.xlsx`, pinnen 2, 4, 5, 12, …) is ook verouderd: GPIO 2, 4, 5 en 12
  laten op slave 1 niets branden.
- Een test waarbij niet-gebruikte pinnen niet expliciet LAAG gezet worden (zwevend) geeft
  willekeurig brandende lampjes. Zet altijd alle pinnen als OUTPUT LOW.
- 4 pinnen tegelijk laten knipperen met hetzelfde patroon zegt niet welke pin welk lampje is;
  elke pin moet een eigen herkenbaar patroon hebben.

## LED-spanningsmeting (`ledmeting`)

Spanning over elk lampje bij een heel klein stroompje (interne pull-up), in mV. Alleen pinnen
met ADC zijn meetbaar (18, 19, 21, 22, 23 niet). Rood en oranje/groen liggen dicht bij elkaar
(1,70–1,86 V), dus deze meting is alleen een hulpmiddel, geen vervanging voor kijken.
Rond 3000 mV zou betekenen: geen LED / los draadje (kwam niet voor).

| pin | 13 | 14 | 25 | 26 | 27 | 32 | 33 |
|---|---|---|---|---|---|---|---|
| hoofdweg | 1792 | 1797 | 1720 | 1801 | 1698 | 1824 | 1800 |
| zijweg | 1846 | 1744 | 1746 | 1848 | 1858 | 1726 | 1829 |

Op de zijweg hebben de rode lampjes (14, 25, 32) duidelijk de laagste spanning.

## PlatformIO

Project: deze map (`kruispunt_pio`). PlatformIO Core staat in `%USERPROFILE%\.platformio\penv`.

| env | wat |
|---|---|
| `master` | master (tijden + ESP-NOW) |
| `slave_hoofdweg` | slave 1 |
| `slave_zijweg` | slave 2 |
| `allesaan` | test: alle 12 lampjes continu aan |
| `volgorde` | test: lampjes een voor een aan (13, 14, 18, … 33), 3 s per stuk |
| `pinscan` | test: pinnen aansturen via Serial (`s vast traag snel flits`, `b pin pin …`, `o`) |
| `ledmeting` | test: LED-spanning per pin meten |
| `test_hoofdweg` / `test_zijweg` | test: loopt elk licht af volgens de pin-tabel |

Flashen naar een bepaalde poort:

```bash
pio run -e slave_zijweg -t upload --upload-port COM7
```

Let op: de upload-poorten in `platformio.ini` zijn de laatst gebruikte. Als twee boards
hetzelfde COM-nummer hebben gehad, altijd `--upload-port` meegeven en de MAC in de output checken.

## Voeding

Elk board heeft eigen voeding nodig: USB (lader, powerbank, PC) of 5–6 V op VIN + GND
(bv. 4× AA). Geen draden nodig tussen de boards. Het programma start vanzelf bij stroom.
