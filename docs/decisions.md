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

## DEC-04 Change the master MAC address

| | |
|---|---|
| **Date** | 2026-10-29 |
| **Status** | Accepted |
| **Decided by** | Osama |
| **Affects** | All hardware stories |

**Context**
The board connected as master reports `7C:9E:BD:65:72:FC`. `VriConfig.h` listed
`94:B9:7E:DA:E2:14` as master. The slaves send to the master's MAC, so a wrong value
means no ESP-NOW link. The board is labelled "Master" and no LED blinks when `led_test`
runs on it. An earlier finding on Trello (US-18.09, 2026-10-09) listed
`94:B9:7E:DA:E2:14` as master and `7C:9E:BD:65:72:FC` as side road slave; the boards
were probably swapped when the wiring was changed.

**Options considered**
| Option | Pros | Cons |
|---|---|---|
| Change the master MAC in the config | Config matches the board that is in use | The board `94:B9:7E:DA:E2:14` is no longer used and not yet located |
| Look for the original master board | Config stays as it was | Time spent on a board the team may have swapped on purpose |

**Decision**
`MAC_MASTER` is `7C:9E:BD:65:72:FC`.

**Reasoning**
The config describes the hardware that is used.

**Consequences**
`94:B9:7E:DA:E2:14` is unaccounted for. If that board turns up, it is a spare.

---

## DEC-03 Swap the slave roles in the MAC table

| | |
|---|---|
| **Date** | 2026-10-29 |
| **Status** | Accepted |
| **Decided by** | Osama |
| **Affects** | All hardware stories |

**Context**
After the autumn break the board on `94:B9:7E:C4:98:68` turned out to be the one
standing next to the side road signal heads. It is labelled "slave 2 side road" and the
four side road LEDs blink when `led_test` runs on it. `VriConfig.h` listed it as the
main road slave. A slave finds its role from its own MAC address, so the table must
match the physical setup.

**Options considered**
| Option | Pros | Cons |
|---|---|---|
| Swap the two MAC addresses in the config | No rewiring, config matches the hardware | The main road board has to be re-checked the same way |
| Swap the boards on the breadboards | Config stays as it was | Rewiring work for no gain |

**Decision**
`MAC_SLAVE_SIDE_ROAD` is `94:B9:7E:C4:98:68` and `MAC_SLAVE_MAIN_ROAD` is
`94:B9:7E:D9:E3:D4`.

**Reasoning**
The config describes the hardware, so the config is what changes.

**Consequences**
The main road MAC (`D9:E3:D4`) follows from the swap and is not yet checked on the
main road board. Confirm it with the `mac_address` sketch when that board is connected.

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