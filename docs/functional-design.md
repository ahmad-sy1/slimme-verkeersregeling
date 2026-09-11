# Functional Design

What the system does, described from the perspective of its users and the client. How
it is built is covered in [`technical-design.md`](technical-design.md).

The backlog itself is maintained in Trello; this document holds the acceptance criteria
that each story is verified against.

---

## 1. Users and stakeholders

| Stakeholder | Interest |
|---|---|
| Main road driver | Reach the city centre without queuing longer than necessary |
| Side road driver | Get a green phase within a reasonable time, even when the main road is busy |
| Emergency services | Pass the intersection without delay |
| Traffic engineer (municipality) | Monitor the intersection and prove the control system performs better than fixed timings |
| City centre business owner | Keep the city centre reachable from the Oranjesingel |

---

## 2. Business rules

These rules apply across every story. A story that violates one of them is never Done,
regardless of its own acceptance criteria.

| ID | Rule |
|---|---|
| BR-01 | Two conflicting directions are never green at the same time. |
| BR-02 | Every green phase ends with yellow, followed by an all-red clearance period, before a conflicting direction turns green. |
| BR-03 | A direction never goes straight from red to green without passing through the defined sequence. |
| BR-04 | Green is never shorter than the minimum green time, not even under a priority request. |
| BR-05 | When the system cannot guarantee a safe state, it falls back to flashing yellow on all directions. |
| BR-06 | The side road is guaranteed a green phase within the maximum waiting time, regardless of how busy the main road is. |

---

## 3. Functional parameters

Values that determine the behaviour of the control system. These are configurable, not
hard-coded. The technical design documents how they are implemented.

| Parameter | Meaning | Starting value |
|---|---|---|
| Minimum green, main road | Shortest green phase on the main road | 10 s |
| Maximum green, main road | Longest green phase before the side road is served | 45 s |
| Minimum green, side road | Shortest green phase on the side road | 6 s |
| Maximum green, side road | Longest green phase on the side road | 20 s |
| Yellow time | Duration of the yellow phase | 3 s |
| Clearance time | All-red period between two conflicting green phases | 2 s |
| Maximum waiting time, side road | Guaranteed limit before the side road is served | *to be agreed with the client (US-07)* |

Starting values are for testing and are adjusted during sprint 1 based on measurements
and client feedback. Changes are recorded in [`decisions.md`](decisions.md).

---

## 4. Exception situations

| Situation | Expected behaviour |
|---|---|
| Startup | All directions red, then the main road turns green |
| Self test fails | Flashing yellow on all directions |
| A controller stops responding | Flashing yellow on all directions |
| Emergency vehicle request | Current green phase is ended safely, then the requested direction turns green |
| Recovery after a fault | Only by manual reset, never automatically |

---

## 5. User stories

The full cards, including priority ordering and acceptance criteria, are maintained on
the Trello board.

| ID | User story | Epic | Priority |
|---|---|---|---|
| US-01 | Safe default cycle | E1 Base control | Must |
| US-02 | Fail-safe on failure | E1 Base control | Must |
| US-03 | Side road vehicle detection | E2 Detection | Must |
| US-04 | No green for an empty side road | E2 Detection | Must |
| US-05 | Queue length measurement | E2 Detection | Should |
| US-06 | Extended green when the main road is busy | E3 Adaptive timing | Must |
| US-07 | Guaranteed maximum waiting time for the side road | E3 Adaptive timing | Must |
| US-08 | Automatic peak and off-peak programs | E3 Adaptive timing | Could |
| US-09 | Master drives the intersection logic | E4 Communication | Must |
| US-10 | Wireless link between the controllers | E4 Communication | Should |
| US-11 | Priority for ambulance and fire service | E5 Special movements | Must |
| US-12 | Turning into the city centre without blocking | E5 Special movements | Should |
| US-13 | Record traffic data | E6 Data and dashboard | Must |
| US-14 | Real-time dashboard for the control room | E6 Data and dashboard | Must |
| US-15 | Historical overview of waiting times | E6 Data and dashboard | Could |
| US-16 | Before/after comparison measurement | E6 Data and dashboard | Must |
| US-17 | Repeatable test scenarios | E2 Detection | Should |