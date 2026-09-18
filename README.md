# Smart Traffic Control Oranjesingel

Prototype of a smart traffic light system (VRI) for the Oranjesingel intersection in
Nijmegen, built on an ESP32 master-slave setup. School project, MBO4 Software
Development, ROC Nijmegen.

---

## Contents

- [Context](#context)
- [Problem and goal](#problem-and-goal)
- [Team](#team)
- [Functional design (overview)](#functional-design-overview)
- [Technical design (overview)](#technical-design-overview)
- [Repository structure](#repository-structure)
- [Getting started](#getting-started)
- [Way of working](#way-of-working)
- [Conventions](#conventions)
- [Documentation](#documentation)

---

## Context

We act as software developers hired by the municipality of Nijmegen to design and build
a prototype traffic light system that reduces congestion on the Oranjesingel.

| | |
|---|---|
| **Client** | Municipality of Nijmegen |
| **Contact person** | *(UNKOWN)* |
| **Period** | *29 January 2027* |
| **Number of sprints** | 4 |

## Problem and goal

The traffic lights on the Oranjesingel run on fixed timings. As a result the main road
is sometimes held at red for no reason, while the side road gets a green phase with no
traffic waiting. During rush hour this causes queues, long waiting times and additional
CO2 emissions. On top of that, shop and restaurant owners want the intersection to keep
allowing traffic to turn from the Oranjesingel into the city centre, without creating
new bottlenecks.

The system must:

- improve traffic flow and demonstrate this with measured data;
- extend the green phase on the main road when it is busy, and give the side road green
  only when vehicles are actually waiting;
- still guarantee the side road a maximum waiting time;
- be able to give priority to emergency vehicles;
- record traffic data and display it in real time on a dashboard.

## Team

| Name | Student number | Role |
|---|---|---|
| *Ahmad Alasmi* | 1206993 | Software developer |
| *Stijn Reits* | 1207024 | Software developer |
| *Osama Alasmi* | 1206995 | Software developer |
| *Vic Theunissen* | 1205893 | Software developer |

## Functional design (overview)

The full breakdown including acceptance criteria lives in
[`docs/functional-design.md`](docs/functional-design.md). The backlog itself is managed
in Trello.

The user stories are grouped into six epics:

| Epic | Subject | Stories |
|---|---|---|
| E1 | Base control | Safe default cycle, fail-safe on failure |
| E2 | Detection | Side road vehicle detection, no green for an empty side road, queue length, repeatable test scenarios |
| E3 | Adaptive timing | Extended green under load, guaranteed maximum waiting time, peak and off-peak programs |
| E4 | Communication | Master drives intersection logic, wireless link between controllers |
| E5 | Special movements | Emergency vehicle priority, turning into the city centre |
| E6 | Data and dashboard | Record traffic data, real-time dashboard, historical overview, before/after comparison |

Prioritisation follows MoSCoW. Together, the Must stories form the minimum viable
control system.

## Technical design (overview)

The full breakdown lives in [`docs/technical-design.md`](docs/technical-design.md).

### Architecture

The control system runs on a master-slave setup. The master owns all intersection
logic; the slaves execute and report back.

- **Master (ESP32)** — maintains the state machine, determines timing, processes
  detection events and priority requests, and feeds data to the dashboard.
- **Slave (ESP32 / Arduino)** — switches the red, yellow and green LEDs for a single
  approach and reports to the master whether vehicles are waiting.

This split is deliberate: a single decision-maker makes it structurally impossible for
two conflicting directions to be green at the same time. The slaves contain no
intersection logic of their own.

### State machine

The control system is built as a `millis()`-based state machine rather than using
`delay()`. This keeps the master responsive to detection events and priority requests
while a green phase is running. The state diagram and the timing constants are
documented in [`docs/technical-design.md`](docs/technical-design.md).

### Hardware

| Component | Represents |
|---|---|
| LED (red / yellow / green) | traffic light |
| LDR | vehicle detection |
| Push button | emergency vehicle request and manual test input |
| Serial input | repeatable test scenarios |

No simulation software is used; the prototype is a physical test setup.

### Technology

| Area | Choice |
|---|---|
| Microcontrollers | ESP32 (master), ESP32 or Arduino (slave) |
| Firmware language | C++ (Arduino framework) |
| Master ↔ slave communication | *(to be decided — see `docs/decisions.md`)* |
| Dashboard | *(to be decided — see `docs/decisions.md`)* |

## Repository structure

```
src/master/       master controller firmware   (pio run -e master)
src/slave/        slave controller firmware    (pio run -e slave)
lib/VriConfig/    shared hardware config (board MAC addresses)
include/          project-wide headers
test/             PlatformIO unit tests
esp32-basics/     standalone Arduino IDE sketches (blink_test, mac_address)
docs/             functional design, decisions, client feedback
platformio.ini    build environments (shared [env] + master/slave)
```

## Getting started

*(expanded once the first firmware is in place)*

```bash
git clone https://github.com/ahmad-sy1/slimme-verkeersregeling.git
cd slimme-verkeersregeling
```

Wi-Fi credentials live in `secrets.h`. That file is listed in `.gitignore` and is never
committed. Copy `secrets.example.h` to `secrets.h` and fill in your own values.

## Way of working

The project runs on Scrum across four sprints. Every sprint delivers a working
increment that is demonstrated to the client.

- Daily stand-up at the start of every project day
- Scrum board in Trello: Backlog, Current Sprint, In Progress, Review / Testing, Done
- Every sprint closes with a review and a demonstration
- Client feedback is recorded in
  [`docs/client-feedback.md`](docs/client-feedback.md)

## Conventions

### Branches

One branch per user story, branched off `main`:

```
feature/US-03-side-road-detection
```

### Commits

Start the commit message with the story number, so the history shows which commits
belong to which story and sprint:

```
US-03: read LDR value on slave controller
```

### Definition of Done

NOT DISCUSSED YET

## Documentation

| Document | Contents |
|---|---|
| [`docs/functional-design.md`](docs/functional-design.md) | User stories with acceptance criteria |
| [`docs/technical-design.md`](docs/technical-design.md) | Architecture, state machine, data model, schematics |
| [`docs/test-plan.md`](docs/test-plan.md) | Test approach and test scenarios |
| [`docs/tests/`](docs/tests/) | Test reports per sprint |
| [`docs/decisions.md`](docs/decisions.md) | Decisions made and the reasoning behind them |
| [`docs/client-feedback.md`](docs/client-feedback.md) | Client feedback and how it was addressed |