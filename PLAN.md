# Build plan

Target architecture: **Config 2 from the research** — a hybrid fleet on one private Meshtastic-compatible LoRa mesh (ANZ 915–928 MHz):

- **Pointer units** (2–3): handheld compass gadgets with a real arrow — Heltec V3 + magnetometer + GPS + LiPo in a 3D-printed case, running [Friend Finder Edition](https://github.com/LeapYeet/Meshtastic-Firmware-Friend-Finder-Edition) firmware.
- **Beacon units** (3–4): Seeed T1000-E card trackers on stock Meshtastic for the rest of the crew — zero build, consumer-finished, phone app as their (optional) display.

Upgrade path later: custom round-LCD puck (Config 3) once the mesh + bearing logic is proven; LED-ring beacon SKU as a cheaper DIY alternative to the T1000-E.

## How it works

### System overview

```mermaid
flowchart TB
    subgraph mesh["915 MHz LoRa mesh — private channel + PSK, zero cell signal needed"]
        P1["Pointer unit #1<br/>(Heltec V3 + FF firmware)<br/>OLED arrow + distance"]
        P2["Pointer unit #2<br/>(Heltec V3 + FF firmware)"]
        B1["Beacon #1<br/>(T1000-E, stock Meshtastic)"]
        B2["Beacon #2<br/>(T1000-E)"]
        B3["Beacon #3<br/>(T1000-E)"]
        P1 <-->|position beacons 20-60s| B1
        P1 <-->|mesh hop| P2
        P2 <-->|position beacons| B2
        B1 <-->|"relay (extends range)"| B3
        P1 <-.->|out of direct range,<br/>reached via hops| B3
    end
    SAT(("GPS satellites<br/>(receive-only, always works)")) -.-> P1 & P2 & B1 & B2 & B3
    B1 -->|BLE, optional| APP["Phone app (flight mode OK)<br/>map / bearing view"]
```

### Position → arrow pipeline (on a pointer unit)

```mermaid
sequenceDiagram
    participant F as Friend's beacon (T1000-E)
    participant M as LoRa mesh
    participant R as Pointer radio (SX1262)
    participant C as Pointer MCU
    participant U as Display

    F->>F: GNSS fix (lat/long)
    F->>M: broadcast position (encrypted, private PSK)
    M->>R: direct or via mesh hops
    R->>C: friend position + timestamp
    C->>C: own GPS fix
    C->>C: bearing = atan2(Δlong, Δlat)
    C->>C: heading from QMC5883L magnetometer<br/>(figure-8 calibrated)
    C->>C: arrow angle = bearing − heading<br/>distance = haversine(Δ)
    alt distance > 30 m
        C->>U: rotate arrow + show distance + last-heard age
    else distance ≤ 30 m (GPS accuracy floor)
        C->>U: "basically here" proximity mode
    end
    Note over C,U: stale positions greyed out, never shown as current
```

1. **Position sensing**: every unit has GNSS — works with zero cell signal (GPS is receive-only).
2. **Position sharing**: units broadcast lat/long over LoRa on a private channel (own PSK, non-default frequency slot in case the festival has a public mesh). Mesh-hops via other crew members extend range.
3. **Bearing**: pointer unit computes bearing = atan2(Δlong, Δlat) from its GPS to the friend's last position, subtracts its own magnetometer heading (QMC5883L, figure-8 calibrated), rotates the arrow. Under ~30 m it switches to a "you're basically there" proximity mode (GPS accuracy floor).
4. **Distance**: haversine from GPS delta, shown next to the arrow.
5. **Staleness**: last-heard age shown per friend; stale positions greyed out, never silently trusted.

## Bill of materials (6-person crew, AUD approx.)

### Pointer unit ×3 (~$65 each)

| Part | Cost | Notes |
|---|---|---|
| Heltec WiFi LoRa 32 V3 (915 MHz) | ~$35 | validated Friend Finder board |
| GY-271 QMC5883L magnetometer | ~$4 | mount away from battery/speaker |
| ATGM336H or NEO-6M GPS module | ~$12 | |
| 1,500 mAh LiPo | ~$12 | ~1 day; power bank top-up at camp |
| 3D-printed case (muzi.works H1-style) | ~$2 | in-house Ender 3, PETG |

### Beacon unit ×3 (~$62–85 each)

| Part | Cost | Notes |
|---|---|---|
| Seeed T1000-E (AU915) | $62 direct / $85 local | pre-flashed Meshtastic, IP65, card-sized |

**Fleet total: ~AUD $400–450.**

## Build diagrams

### Pointer unit — hardware assembly

```mermaid
flowchart LR
    subgraph case["3D-printed case (Ender 3, PETG) — fully enclosed, branded label"]
        subgraph board["Heltec WiFi LoRa 32 V3"]
            MCU["ESP32-S3 MCU<br/>Friend Finder firmware"]
            LORA["SX1262 LoRa radio<br/>(915 MHz ANZ)"]
            OLED["0.96 in OLED<br/>arrow + distance"]
        end
        GPS["GPS module<br/>ATGM336H / NEO-6M"]
        MAG["QMC5883L magnetometer<br/>(mounted AWAY from battery)"]
        BAT["1,500 mAh LiPo"]
    end
    ANT["stub antenna<br/>(not a 20 cm whip)"]
    USB["USB-C port<br/>(accessible for charging)"]

    GPS -->|UART| MCU
    MAG -->|I2C| MCU
    MCU --- LORA
    MCU --- OLED
    BAT -->|JST| MCU
    LORA --- ANT
    USB --- MCU
```

Beacon units need no assembly — the T1000-E is a finished product; just flash region ANZ + private channel.

### Build phases + gates

```mermaid
flowchart TD
    P1["Phase 1 — Radio proof<br/>2x Heltec V3 + 1x T1000-E<br/>flash FF, private channel, walk-test"]
    G1{"GATE: stock T1000-E positions<br/>visible on FF pointer?"}
    ALT["Fallback: all-pointer fleet /<br/>firmware patch / phone-app display"]
    P2["Phase 2 — Compass proof<br/>wire QMC5883L + GPS, calibrate,<br/>park test arrow accuracy + battery"]
    P3["Phase 3 — Enclosure<br/>H1-style case on Ender 3<br/>GATE: passes the bag-check look test"]
    P4["Phase 4 — Fleet build + field test<br/>full crew, park rehearsal, real event"]
    P5["Phase 5 (stretch) — custom puck /<br/>LED-ring SKU / TDMA protocol"]

    P1 --> G1
    G1 -->|yes| P2
    G1 -->|no| ALT --> P2
    P2 --> P3 --> P4 --> P5
```

### Phase 1 — Radio proof (order → 1 weekend)
- Order 2 × Heltec V3 + modules + 1 × T1000-E (validate interop early).
- Flash Friend Finder via web flasher; set region ANZ, private channel + PSK.
- Two units exchanging positions on a desk, then a suburb walk-test: range, update latency, staleness behaviour.
- **Gate: stock-firmware T1000-E positions visible from a Friend Finder pointer.** If not, decide: all-pointer fleet vs firmware patch vs phone-app display for beacons.

### Phase 2 — Compass proof
- Wire QMC5883L + GPS to one pointer; figure-8 + flat-spin calibrate.
- Park test: two people, arrow accuracy while walking, close-range (<30 m) behaviour, magnetometer interference (phone in same pocket).
- Battery drain measurement at 20 s vs 60 s beacon interval.

### Phase 3 — Enclosure (mech eng + Ender 3)
- Print/adapt H1-style case; route GPS + magnetometer inside the shell, stub antenna, no visible wires.
- The "doesn't look like a bomb" gate: fully enclosed, branded label + QR code, reads as a commercial GPS gadget.
- PETG for heat; antenna port snug; USB-C accessible for charging.

### Phase 4 — Fleet build + field test
- Build remaining pointer units, buy remaining T1000-Es, bond/configure everything on the private channel.
- Full-crew park/park-run rehearsal, then a real event.
- Log: real crowd range, mesh-hop behaviour, battery over a full day, arrow usability in the dark.

### Phase 5 (stretch) — custom puck / LED-ring SKU
- Round GC9A01 LCD puck (crib LodeStone + Micro Compass) and/or LED-ring beacon (Totem-style UX, no screen).
- Crib spoke's PPS-synced TDMA + 13-byte payloads if stock Meshtastic intervals feel too stale.
- Touch-to-bond pairing UX instead of channel config.

## Design principles

- **LoRa or nothing** — 2.4 GHz dies in crowds (Totem's own users confirm: falls over when the crew spreads out).
- **Ephemeral by design** — live positions only, private PSK, no cloud, no accounts, no trails.
- **Degrade loudly** — stale data is shown as stale, never presented as current.
- **Consumer-finished or it doesn't ship** — every unit passes a bag check.

## Current State

- Phase: research complete (2 reports in docs/), plan drafted
- Next: order Phase 1 hardware (2× Heltec V3 + modules + 1× T1000-E)
- Blocked: nothing
