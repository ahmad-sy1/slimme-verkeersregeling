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

## DEC-03 ESP-NOW for communication between master and slaves

| | |
|---|---|
| **Date** | 2026-09-11 |
| **Status** | Accepted |
| **Decided by** | *Ahmad Alasmi, Stijn Reits, Osama Alasmi, Vic Theunissen* |
| **Affects** | US-09, US-10, US-18 |

**Context**
The master must send commands to the slaves and get their status back. The connection
must be wireless, fast, and work without the school network. If the connection is lost,
the system must notice it so it can switch to a safe state (BR-05).

**Options considered**

| Option | Pros | Cons |
|---|---|---|
| ESP-NOW | Built into the ESP32; no router needed; fast | Only works on ESP32; max 250 bytes per message |
| Wi-Fi via a router | Well-known; can also be used for the dashboard | Needs a router or the school network; slower; more can go wrong |
| Wires (UART / I2C) | Very reliable | Not wireless; extra cables between the boards |

**Decision**
Master and slaves communicate over ESP-NOW.

**Reasoning**
ESP-NOW is wireless and does not need any network, so it works the same in class and
at the demo. Our practice sketches showed that it works and that the sender can see
whether a message arrived.

**Consequences**
- All slaves must be ESP32 boards; an Arduino cannot be used.
- The MAC address of every board is stored in `lib/VriConfig/VriConfig.h`.
- Messages must be smaller than 250 bytes.
- A message can get lost, so both sides use a timeout to detect a lost connection.

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