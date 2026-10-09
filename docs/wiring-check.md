# Wiring check

Findings from checking the wiring of the fixed-timing prototype on the physical
intersection, 2026-10-08. The pin tables below are the ones in
[`lib/VriConfig/VriConfig.h`](../lib/VriConfig/VriConfig.h).

- [System](#system)
- [Boards](#boards)
- [Cycle](#cycle)
- [Pins](#pins)
- [Pole numbering](#pole-numbering)
- [Points of attention](#points-of-attention)
- [Lessons learned](#lessons-learned)
- [LED voltage measurement](#led-voltage-measurement)
- [Firmware and environments](#firmware-and-environments)
- [Power supply](#power-supply)
- [Earlier material](#earlier-material)

---

## System

- **Master** – the brain, has no lights of its own. It times the phases and every
  100 ms sends the aspect of all 8 signal heads over ESP-NOW (broadcast) to both slaves.
- **Slave 1 – main road** – drives the 4 signal heads of the main road.
- **Slave 2 – side road** – drives the 4 signal heads of the side road.
- No sensors, fixed timing only.

## Boards

| Board | MAC |
|---|---|
| Master | `94:B9:7E:DA:E2:14` |
| Slave 1, main road | `94:B9:7E:D9:E3:D4` |
| Slave 2, side road | `7C:9E:BD:65:72:FC` |

- The board that was listed before as the main road slave (`94:B9:7E:C4:98:68`) is no
  longer part of the setup.
- `94:B9:7E:D9:E3:D4` was listed before as the side road slave and is now the main road
  slave. **Unconfirmed:** the roles above still have to be checked against the stickers
  on the boards.
- The port of a board changes when a cable is swapped or plugged in again. Identify a
  board by its MAC address: it is printed in the upload output and on the serial monitor
  at start-up.

## Cycle

Every phase: green 7 s → orange 3 s → all red 2 s (clearance). After start-up, first
3 s all red.

1. Main road right (A + B) green
2. Main road left (A + B) green
3. Side road right (A + B) green
4. Side road left (A + B) green

The durations are `FIXED_GREEN_MS`, `ORANGE_MS`, `CLEARANCE_MS` and
`STARTUP_ALL_RED_MS` in `VriConfig.h`.

**Fail-safe:** when a slave hears nothing from the master for more than 1.5 s
(`MASTER_TIMEOUT_MS`), all its lights flash orange. A slave also flashes orange from
start-up until it hears the master for the first time. When the master comes back, the
slave joins in again within a second.

Measured: over 90 s not a single ESP-NOW message was lost, and both slaves changed
phase within the same 0.1 s. ESP-NOW therefore does **not** cause a delay.

## Pins

Checked on the intersection. Pins used on both slaves: 13, 14, 18, 19, 21, 22, 23, 25,
26, 27, 32, 33. Pin HIGH = LED on.

### Main road (slave 1)

| Signal head | Red | Orange | Green |
|---|---|---|---|
| A left | 18 | 13 | 14 |
| A right | 19 | 21 | 22 |
| B left | 27 | 32 | 26 |
| B right | 25 | 33 | 23 |

### Side road (slave 2)

| Signal head | Red | Orange | Green |
|---|---|---|---|
| A left | 32 (unconfirmed) | 33 | 27 |
| A right | 14 | 13 (unconfirmed) | 26 |
| B left | 25 | 23 | 22 |
| B right | 21 | 18 | 19 |

### How the pins were found

With the pin scan firmware (`pin_scan`), 3 pins per board were switched on at a time
(steady / slow blink / fast blink), and for every signal head we noted what lit up.

| Round | Pins (steady, slow, fast) | Main road | Side road |
|---|---|---|---|
| 1 | 13, 14, 18 | 13 A left orange, 14 A left green, 18 A left red | 13 nothing, 14 A right red, 18 B right orange |
| 2 | 19, 21, 22 | 19 not reported (the only pin left = A right red), 21 A right orange, 22 A right green | 19 B right green, 21 B right red, 22 B left green |
| 3 | 23, 25, 26 | 23 B right green, 25 B right red, 26 B left green | 23 B left orange, 25 B left red, 26 A right green |
| 4 | 27, 32, 33 | 27 B left red, 32 B left orange, 33 B right orange | 27 A left green, 32 nothing, 33 A left orange |

## Pole numbering

Every pole of the intersection carries a number: the `SignalHeadId` order in
`VriProtocol.h` plus one.

| No. | `SignalHeadId` | No. | `SignalHeadId` |
|---|---|---|---|
| 1 | `MAIN_A_LEFT` | 5 | `SIDE_A_LEFT` |
| 2 | `MAIN_A_RIGHT` | 6 | `SIDE_A_RIGHT` |
| 3 | `MAIN_B_LEFT` | 7 | `SIDE_B_LEFT` |
| 4 | `MAIN_B_RIGHT` | 8 | `SIDE_B_RIGHT` |

The numbers were put on the poles with `signal_head_test`: a number sent over the serial
monitor switches on all three LEDs of that signal head. For every number we checked that
exactly one pole lit up and which colours it showed, then put the number on that pole.

### Main road (slave 1), 2026-10-09

Board `94:B9:7E:D9:E3:D4`.

| No. | Signal head | One pole only | Red | Orange | Green | Remark |
|---|---|---|---|---|---|---|
| 1 | Main road A left | yes | yes | yes | yes | Matches the pin table (red 18, orange 13, green 14) |
| 2 | Main road A right | yes | yes | yes | yes | Matches the pin table (red 19, orange 21, green 22). Red on GPIO 19 now confirmed |
| 3 | Main road B left | yes | yes | yes | yes | Matches the pin table (red 27, orange 32, green 26) |
| 4 | Main road B right | yes | yes | yes | yes | Matches the pin table (red 25, orange 33, green 23) |

### Side road (slave 2), 2026-10-09

Board `7C:9E:BD:65:72:FC`. Where a colour was missing, the three pins of that pole were
also switched on one at a time with `pin_scan` (steady, and blinking where nothing was
seen), while the whole intersection was checked.

| No. | Signal head | One pole only | Red | Orange | Green | Remark |
|---|---|---|---|---|---|---|
| 5 | Side road A left | yes | **no** | yes | yes | Red (GPIO 32) does not light up; GPIO 32 unreliable (once lit pole 6 red, not reproducible). Orange 33 and green 27 confirmed one by one |
| 6 | Side road A right | yes | yes | **no** | yes | Orange (GPIO 13) does not light up, steady or blinking. Red 14 and green 26 confirmed one by one |
| 7 | Side road B left | yes | yes | yes | yes | Matches the pin table (red 25, orange 23, green 22) |
| 8 | Side road B right | yes | yes | yes | yes | Matches the pin table (red 21, orange 18, green 19) |

## Points of attention

- **Side road A left red (GPIO 32) and A right orange (GPIO 13)** did not light up
  visibly in any test, including the pole numbering check of 2026-10-09. They are the
  only pins left for those LEDs, and the voltage measurement (below) shows that an LED
  is connected. Probably a loose wire, an LED the wrong way round or a very dim LED:
  check the wiring of these two.
- **GPIO 32 is unreliable.** With GPIO 32 on alone, side road A right red (pole 6, wired
  to GPIO 14) lit up once; this could not be reproduced. That points to a wiring fault
  around GPIO 32, for example a loose wire touching another one.
- **Main road A right red (GPIO 19)** was not reported in the pin scan (a steady LED is
  easy to miss). The pole numbering check of 2026-10-09 confirmed it.
- "Left/right" follows the names used on the intersection. If the wrong two signal heads
  turn green together (for example A left with B right), swap the B left and B right
  rows in the pin table.

## Lessons learned

- **`waarnemingen kruispunt.xlsx` did not match the current wiring.** The pin names in
  its first column (D12, D13, …) were from the old pinout. Even read as steps 1..12 of the
  test code, several orange/green/red values were wrong. Use the tables above.
- The old pinout (`pinout.xlsx`, pins 2, 4, 5, 12, …) was outdated as well: GPIO 2, 4, 5
  and 12 light nothing on slave 1.
- A test that does not explicitly drive unused pins LOW (floating) gives LEDs that light
  up at random. Always set all pins to OUTPUT LOW.
- Blinking 4 pins with the same pattern does not tell which pin is which LED; every pin
  needs its own recognisable pattern.

Both spreadsheets were removed from the repository; they are still in the Git history.

## LED voltage measurement

Measured with `led_voltage`: the voltage over each LED at a very small current (internal
pull-up), in mV. Only pins with an ADC can be measured (18, 19, 21, 22 and 23 cannot).
Red and orange/green are close together (1.70–1.86 V), so this measurement is only an
aid, not a replacement for looking. Around 3000 mV would mean: no LED or a loose wire
(did not occur).

| Pin | 13 | 14 | 25 | 26 | 27 | 32 | 33 |
|---|---|---|---|---|---|---|---|
| Main road | 1792 | 1797 | 1720 | 1801 | 1698 | 1824 | 1800 |
| Side road | 1846 | 1744 | 1746 | 1848 | 1858 | 1726 | 1829 |

On the side road the red LEDs (14, 25, 32) clearly have the lowest voltage.

## Firmware and environments

All firmware is built from the root PlatformIO project.

| Environment | What it does |
|---|---|
| `master` | Master: fixed-timing cycle and ESP-NOW broadcast |
| `slave` | Both slaves; the board picks its pin table by MAC address |
| `signal_head_test` | Test: walks every signal head of this slave (red, orange, green) following the pin table |
| `all_on` | Test: all 12 LEDs on continuously |
| `pin_scan` | Test: drives pins through serial commands (`s steady slow fast flash`, `b pin pin …`, `o`) |
| `led_voltage` | Test: measures the LED voltage per pin |

Flash to a specific port, then check the MAC in the output:

```bash
pio run -e slave -t upload --upload-port <port>
```

## Power supply

Every board needs its own power supply: USB (charger, power bank, PC) or 5–6 V on VIN
and GND (for example 4× AA). No wires are needed between the boards. The program starts
by itself when power is applied.

## Earlier material

Kept from the files of an earlier design that were removed from the repository
(`pinout.xlsx` and the Arduino sketches). None of it is confirmed on the current setup.

### Movement per signal head

The old pinout gave every approach two signal heads with a different movement:

| Road | One signal head | Other signal head |
|---|---|---|
| Main road | Straight on / right | Controlled left turn |
| Side road | Straight on / left | Controlled right turn |

**Unconfirmed:** which of the current "left" and "right" signal heads is which movement.
The pins in that pinout are outdated.

### LDRs

- The old pinout put the LDRs on GPIO 32, 33, 34 and 35. GPIO 32 and 33 now drive LEDs
  on both slaves, so at least two LDRs need other ADC1 pins (GPIO 34, 35, 36 or 39).
- Measured resistance range (min–max) of the side road LDRs:

  | LDR | Range |
  |---|---|
  | LDR 1 | 1.7 kΩ – 19.2 kΩ |
  | LDR 2 | 1.5 kΩ – 16.4 kΩ |
  | LDR 3 | 2.1 kΩ – 23.8 kΩ |
  | LDR 4 | 1.7 kΩ – 25.3 kΩ |

- The earlier sketches treated `analogRead(pin) < 2000` as "vehicle present". This
  threshold was never calibrated.
