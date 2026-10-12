# Decision Log

Technical and functional decisions made during the project, with the reasoning behind
them. Every entry is written at the moment the decision is made, not reconstructed
afterwards.

**How to use this file:** add a new entry at the top. Never edit or delete an old
entry — if a decision is reversed, add a new entry that supersedes it and update the
status of the old one. The history is the point.

**Template**

```markdown
## DEC-00 Title of the decision

| | |
|---|---|
| **Date** | YYYY-MM-DD |
| **Status** | Proposed / Accepted / Superseded by DEC-XX |
| **Decided by** | Names |
| **Affects** | US-XX, US-YY |

**Context**
What problem or question prompted this decision? What constraints applied?

**Options considered**
| Option | Pros | Cons |
|---|---|---|
| A | | |
| B | | |

**Decision**
What was chosen.

**Reasoning**
Why this option and not the others.

**Consequences**
What follows from this — what becomes easier, what becomes harder, what needs
revisiting later.
```

---

## DEC-11 Which signal heads may be green together

| | |
|---|---|
| **Date** | 2026-10-09 |
| **Status** | Proposed |
| **Decided by** | *(names)* |
| **Affects** | US-18.04, US-12 |

**Context**
The conflict guard (US-18.04) needs a table of signal heads that may be green at the
same time. An earlier design had such a table, but it uses its own names for the side
road approaches ("right" and "left" instead of A and B), its mapping onto the numbered
poles is unconfirmed, and it is inconsistent: Side "left" left lists Side "right" right
as allowed, but not the other way round. The current fixed-timing cycle does not use a
table; each phase gives green to one road in both directions.

**Options considered**

| Option | Pros | Cons |
|---|---|---|
| A. Correct the earlier table and map it onto the pole numbers 1–8 | Builds on existing work; allows more signal heads green at once | Its side road names have to be matched to A/B on the intersection first |
| B. Draw up a new table from the pole numbers on the intersection | Starts from the confirmed numbering | More work; may repeat the earlier analysis |

**Decision**
Not decided yet (open decision 9 in [`technical-design.md`](technical-design.md)).

**Reasoning**
*(to be filled in when decided)*

**Consequences**
US-18.04 cannot check conflicts against a table until this is decided.

---

## DEC-10 Slave aspect before the first message from the master

| | |
|---|---|
| **Date** | 2026-10-09 |
| **Status** | Proposed |
| **Decided by** | *(names)* |
| **Affects** | US-02, US-18.04 |

**Context**
A slave that boots before the master has not received any aspect yet. The technical
design first said it should show red; the fixed-timing prototype flashes orange, the
same as after a lost connection.

**Options considered**

| Option | Pros | Cons |
|---|---|---|
| A. Flash orange (current behaviour) | Same as the fail-safe after a timeout; one simple rule in the slave | The intersection does not start in all red when the slaves boot first |
| B. Show red until the first message | Matches "the system starts in all red" | A slave whose master never comes up stays red instead of flashing orange |

**Decision**
Not decided yet (open decision 8 in [`technical-design.md`](technical-design.md)). The
code keeps option A until the team decides.

**Reasoning**
*(to be filled in when decided)*

**Consequences**
If option B is chosen, the slave needs a separate state before first contact, and the
timeout only applies after first contact.

---

## DEC-09 Orange instead of yellow

| | |
|---|---|
| **Date** | 2026-10-09 |
| **Status** | Proposed |
| **Decided by** | *(names)* |
| **Affects** | All documentation and source code |

**Context**
The documentation used "yellow" for the middle light. The ported prototype and
`CLAUDE.md` use "orange".

**Options considered**

| Option | Pros | Cons |
|---|---|---|
| A. Orange | Matches the Dutch "oranje" used on the intersection and by the client | Differs from the English "yellow" common in traffic engineering |
| B. Yellow | Common English traffic term | Code and part of the documentation already say orange |

**Decision**
Orange: `ASPECT_ORANGE`, `ORANGE_MS`, "flashing orange".

**Reasoning**
One term everywhere avoids confusion between code and documents; the code already uses
orange.

**Consequences**
`functional-design.md`, section 1 of `technical-design.md` and the README overview still
say yellow and have to be updated.

---

## DEC-08 Fixed-timing cycle values

| | |
|---|---|
| **Date** | 2026-10-09 |
| **Status** | Proposed |
| **Decided by** | *(names)* |
| **Affects** | US-18.02, US-16 |

**Context**
The fixed-timing prototype is the baseline for the before/after comparison (US-16). Its
cycle values live in `VriConfig.h`.

**Options considered**

| Option | Pros | Cons |
|---|---|---|
| A. Four phases (main road right, main road left, side road right, side road left), each green 7 s → orange 3 s → all red 2 s; 3 s all red at start-up | Runs on the hardware now; one road and lane at a time cannot conflict | Differs from the order on the US-18.02 card (main green → side green) |
| B. Two phases (main road, side road), as on the US-18.02 card | Matches the card and the planned state machine | Not implemented yet |

**Decision**
Option A, as implemented in the prototype.

**Reasoning**
Orange 3 s and clearance 2 s match the functional design. The four-phase order comes
from the prototype that was tested on the intersection.

**Consequences**
The US-18.02 acceptance criterion on cycle order is not met as written; either the card
or the cycle has to change.

---

## DEC-07 Send interval 100 ms and master timeout 1500 ms

| | |
|---|---|
| **Date** | 2026-10-09 |
| **Status** | Proposed |
| **Decided by** | *(names)* |
| **Affects** | US-02, US-18.03 |

**Context**
The master repeats its message so a slave notices when it is gone. The technical design
proposed 200 ms and 500 ms; the prototype uses 100 ms and 1500 ms.

**Options considered**

| Option | Pros | Cons |
|---|---|---|
| A. 100 ms interval, 1500 ms timeout (prototype) | Measured on the intersection: no lost message in 90 s, both slaves switch within 0.1 s | Up to 1.5 s before a slave fails safe |
| B. 200 ms interval, 500 ms timeout (technical design) | Faster fail-safe | Only 2–3 missed messages trigger the fail-safe; not tested |

**Decision**
Option A: `SEND_INTERVAL_MS = 100`, `MASTER_TIMEOUT_MS = 1500`.

**Reasoning**
These values were tested on the hardware.

**Consequences**
The timeout can be shortened later once the link has been tested longer.

---

## DEC-06 Board roles

| | |
|---|---|
| **Date** | 2026-10-09 |
| **Status** | Proposed |
| **Decided by** | *(names)* |
| **Affects** | All hardware stories |

**Context**
The wiring check of 2026-10-08 found other boards in the setup than `VriConfig.h` listed.

**Decision**
Master `94:B9:7E:DA:E2:14`, main road slave `94:B9:7E:D9:E3:D4`, side road slave
`7C:9E:BD:65:72:FC`. The previous main road board (`94:B9:7E:C4:98:68`) is no longer part
of the setup.

**Reasoning**
These are the boards that drive the poles on the intersection: the pole numbering check
of 2026-10-09 found the main road poles on `94:B9:7E:D9:E3:D4` and the side road poles on
`7C:9E:BD:65:72:FC`.

**Consequences**
The stickers on the boards still have to be checked against these roles.

---

## DEC-05 One slave firmware, role chosen by MAC address

| | |
|---|---|
| **Date** | 2026-10-09 |
| **Status** | Proposed |
| **Decided by** | *(names)* |
| **Affects** | US-18.01 |

**Context**
The prototype had a separate firmware for each slave. Both slaves do the same work with a
different pin table.

**Options considered**

| Option | Pros | Cons |
|---|---|---|
| A. One build environment per slave | No role detection needed | Flashing the wrong environment gives a wrong pin table; duplicate code |
| B. One firmware; the slave picks its pin table by its own MAC address | One build; a slave cannot get the wrong pin table | A new board needs its MAC added to `VriConfig.h` |

**Decision**
Option B. A board with an unknown MAC keeps all LEDs off and prints an error.

**Reasoning**
It removes a way to flash the wrong firmware, and matches section 8 of the technical
design.

**Consequences**
Replacing a slave board means updating its MAC in `VriConfig.h`.

---

## DEC-04 One broadcast message with the aspects of all signal heads

| | |
|---|---|
| **Date** | 2026-10-09 |
| **Status** | Proposed |
| **Decided by** | *(names)* |
| **Affects** | US-18.03 |

**Context**
The master has to tell both slaves what to show. The technical design planned a
`SetSignal` message per slave with a `Status` reply.

**Options considered**

| Option | Pros | Cons |
|---|---|---|
| A. One broadcast `SignalMessage` with the aspect of all 8 signal heads (prototype) | One send reaches both slaves at the same moment; no peer list | ESP-NOW broadcast has no delivery confirmation |
| B. A `SetSignal` message per slave, sent to its MAC, with a `Status` reply | Delivery confirmation; the master knows what each slave shows | More messages and code; not implemented yet |

**Decision**
Option A for the fixed-timing prototype.

**Reasoning**
It works on the hardware and keeps both slaves in step.

**Consequences**
The US-18.03 criterion "slave confirms receipt; master logs a failed delivery" needs
option B or an extra reply message.

---

## DEC-03 ESP-NOW for master–slave communication

| | |
|---|---|
| **Date** | 2026-10-09 |
| **Status** | Proposed |
| **Decided by** | *(names)* |
| **Affects** | US-18.03, E4 |

**Context**
The master and the two slaves need a link. The technical design chose ESP-NOW and says
the reasoning is in this log, but no entry existed yet.

**Options considered**

| Option | Pros | Cons |
|---|---|---|
| A. ESP-NOW | No router or network needed; low latency; boards address each other by fixed MAC | ESP32 only; must share the Wi-Fi channel if the dashboard uses Wi-Fi |
| B. Wi-Fi through a router (UDP or MQTT) | Standard networking; easy to reach from a laptop | Needs a router; school Wi-Fi often blocks devices |
| C. Wires between the boards (UART) | No radio | Cables across the intersection; one link per slave |

**Decision**
Option A.

**Reasoning**
It needs no infrastructure and was tested on the intersection: no lost message in 90 s.

**Consequences**
Both slaves must be ESP32 boards. If the dashboard uses Wi-Fi on the master, the channel
is fixed in `VriConfig.h`.

---

## DEC-02 Documentation and naming in English

| | |
|---|---|
| **Date** | 2026-09-11 |
| **Status** | Accepted |
| **Decided by** | *Ahmad Alasmi* |
| **Affects** | All documentation and source code |

**Context**
The team needed one consistent language for file names, identifiers, commit messages
and documentation. A mixed codebase makes the Git history harder to read and invites
inconsistency as soon as a team member is unsure which language applies where.

**Decision**
File and folder names, identifiers, constants, commit messages, branch names and all
documentation in this repository are written in English. User stories on the Trello
board remain in Dutch, as they are shared with the client.

**Reasoning**
Established English terminology exists for traffic control (minimum green, clearance
time, demand, fail-safe). Using it makes the code readable to anyone familiar with the
domain and avoids inventing Dutch equivalents. Keeping the Trello stories in Dutch
keeps them accessible to the client.

**Consequences**
Comments in code follow the same rule. Any Dutch identifiers written before this date
are renamed when the surrounding file is next touched.

---

## DEC-01 State machine instead of delay()

| | |
|---|---|
| **Date** | 2026-09-11 |
| **Status** | Accepted |
| **Decided by** | *(names)* |
| **Affects** | US-01, US-03, US-04, US-09, US-11 |

**Context**
The controller has to keep responding to vehicle detection and emergency priority
requests while a green phase is running. The obvious approach for timing traffic light
phases is a sequence of `delay()` calls.

**Options considered**

| Option | Pros | Cons |
|---|---|---|
| `delay()` sequence | Simple to write and read | Blocks the loop entirely; no detection or priority handling during a phase |
| `millis()` state machine | Loop stays free; inputs handled at any moment | More code; requires explicit state tracking |

**Decision**
The control logic is implemented as a state machine driven by `millis()`.

**Reasoning**
With `delay()` the controller is deaf for the duration of a phase, which makes US-03,
US-04 and US-11 impossible to implement correctly. A state machine also maps directly
onto the phases the client understands, which makes the behaviour easier to explain and
to test against the acceptance criteria.

**Consequences**
Every state transition has an explicit condition, which makes the timing constants
configurable rather than hard-coded. The state diagram in
[`technical-design.md`](technical-design.md) must be kept in sync with the code.