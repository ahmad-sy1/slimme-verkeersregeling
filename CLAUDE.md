# CLAUDE.md

Rules for working in this repository. Read this before changing anything.

Smart traffic light prototype (VRI) for the Oranjesingel in Nijmegen: one ESP32 master
and two ESP32 slaves, built with PlatformIO (Arduino framework). School project, run
with Scrum. The work is graded on code quality and on collaboration, so small, clear
changes matter more than speed.

## Language

Everything in this repository is in English (DEC-02 in `docs/decisions.md`): file and
folder names, identifiers, constants, enum values, comments, serial output, commit
messages, branch names and documentation.

- Do not write Dutch identifiers or comments, also not when the user talks to you in
  Dutch. Answer in the user's language, write the repository in English.
- When you touch a file that still has Dutch names, rename them in the same change.
- Use the traffic control terms from `docs/functional-design.md` and
  `docs/technical-design.md`: main road, side road, signal head, phase, clearance,
  fail-safe, orange (not yellow/geel).
- The user stories on the Trello board stay in Dutch. Do not translate them.

## Where code goes

There is exactly one PlatformIO project: the repository root. Never create a second
project folder, a second `platformio.ini`, or `.ino` sketches.

```
lib/VriConfig/VriConfig.h       board MAC addresses, pin tables, timing parameters
lib/VriProtocol/VriProtocol.h   ESP-NOW message definitions shared by master and slave
src/master/                     master firmware    (pio run -e master)
src/slave/                      slave firmware     (pio run -e slave)
src/basics/<name>/              practice and test sketches, one environment each
docs/                           all documentation
```

- Master and slave each have one build environment. Both slaves run the same firmware;
  a slave finds its role (main road or side road) from its own MAC address and takes
  the matching pin table from `VriConfig.h`. Do not add an environment per slave.
- Each module is its own `.h`/`.cpp` pair in `src/master/` or `src/slave/`
  (see section 8 of `docs/technical-design.md`). `main.cpp` only wires modules together.
- A value that is hardware or timing (pin, MAC, duration, timeout, interval) lives in
  `VriConfig.h`. Never hard-code it in a `.cpp` file.
- Anything that goes over ESP-NOW is defined once, in `VriProtocol.h`.
- Hardware test tools (pin scan, LED test) go in `src/basics/<name>/` with their own
  `[env:<name>]`, in the same style as the existing ones.

## platformio.ini

- Keep the shared `[env]` section as it is. The platform version is pinned on purpose;
  do not remove or change the pin.
- Never commit `upload_port` or `monitor_port`. Ports differ per laptop and per
  operating system. Pass `--upload-port` on the command line instead.
- Identify a board by its MAC address (`pio run -e mac_address -t upload -t monitor`),
  never by its port.

## Firmware rules

- No `delay()` in `loop()` or in anything called from it. Timing is a `millis()` state
  machine (DEC-01). A short `delay()` in `setup()` is fine.
- ESP-NOW callbacks run in the Wi-Fi task: only copy data there and guard shared data
  with a `portMUX` spinlock, as in `src/basics/espnow_master/main.cpp`. Do the work and
  the `Serial` printing in `loop()`.
- Safety comes first: the intersection may never show conflicting greens, every change
  of direction goes through orange and an all-red clearance, and the system starts in
  all red.
- Comment the why, not the what.

## What does not belong in the repository

- Binary office files (`.xlsx`, `.pptx`, `.docx`). Put the content in a Markdown file
  in `docs/`; tables go in Markdown tables.
- Old or experimental sketches. If it is worth keeping, make it a `src/basics/` sketch;
  otherwise leave it out.
- `secrets.h`, build output, IDE folders (already in `.gitignore`).

## Documentation

- Findings, measurements and test results go in `docs/`, in English, linked from the
  README table.
- A decision (a choice between options, a changed value the team agreed on) gets an
  entry at the top of `docs/decisions.md` using the template there. Never edit an old
  entry.
- When the code changes something that `docs/technical-design.md` describes (pins,
  message format, states, timing), update that document in the same pull request.
- The pin mapping in the documentation must be the one confirmed on the hardware.
  Mark anything unconfirmed as unconfirmed.

## Git

- Never commit to `main`. One branch per task, named `<type>/<short-description>`,
  for example `feat/light-driver` or `docs/sprint-2-test-report`.
- Conventional commit messages in English: `feat:`, `fix:`, `docs:`, `chore:`,
  `refactor:`, `test:`. One logical change per commit; do not put a whole feature,
  its tests and its documentation in a single commit.
- Mention the Trello task in the pull request description (for example `US-18.01`) and
  list which acceptance criteria the pull request meets.
- A pull request is reviewed by a teammate other than the author before it is merged.
- Do not push or open a pull request without asking the user first.

## Before you say something is done

- `pio run` builds every environment without errors or new warnings.
- You cannot see the hardware. Never state that an LED, a pin or a link works unless
  the user confirmed it on the setup. Say what was built and what still has to be
  checked on the hardware.
- Check the work against the acceptance criteria of the Trello task and name the ones
  that are not met yet.
