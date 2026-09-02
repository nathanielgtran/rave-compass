# rave-compass

An open-source "friend compass" for raves and festivals. One person (or the crew totem) carries an **anchor beacon**; everyone else wears a small **pointer token** whose LED arrow always points back to the crew — **with zero mobile signal**.

## The problem

Festivals kill phones: dense crowds + overloaded or absent cell towers mean every "find my friends" app is dead on arrival. Paid offline trackers are expensive, subscription-gated, or discontinued.

GPS itself works with zero cell signal — it's receive-only from satellites, so every device *knows where it is*. The missing piece is **sharing a position between devices with no internet**. rave-compass does that with a one-way LoRa broadcast: the anchor yells "here I am" every second; the tokens listen and point.

## How it works

- **1 anchor** — broadcasts its GPS position over raw LoRa (915 MHz) every second.
- **N tokens** — receive-only. Each knows its own GPS position + tilt-compensated compass heading → lights an LED-ring arrow toward the anchor, with distance. Lost? Follow the arrow. Crew converges.

No mesh, no pairing, no accounts, no cloud, no phone required. Tokens never transmit, so any number of friends can listen.

Full architecture, diagrams, BOM and build phases: **[PLAN.md](PLAN.md)**.

## Why this design

| Channel | Range in a packed crowd | Verdict |
|---|---|---|
| BLE / WiFi / 2.4 GHz | ~5–15 m — human bodies heavily attenuate 2.4 GHz | useless at festival scale (the commercial Totem Compass uses 2.4 GHz and its users report exactly this failure once the crew spreads out) |
| Cell / internet | assumes a working tower | the whole problem |
| **LoRa 915 MHz** | **hundreds of metres through crowds** | **the viable radio** |

One-way single-transmitter design means no mesh congestion, near-live 1 s updates (a mesh protocol like Meshtastic floors out at 30 s), and tokens that run 7+ hours on a small internal battery — no powerbanks.

## Requirements

1. **Zero-signal operation** — self-contained one-way LoRa broadcast, no venue infrastructure.
2. **1 anchor + a token per friend**, target ~$60–80 AUD per token.
3. **Compass UX** — LED-ring arrow + distance to the anchor; "basically here" mode under ~30 m (GPS accuracy floor).
4. **Must not look like a bomb.** Fully enclosed consumer-grade cases — no exposed PCBs, wires, or taped batteries. Sails through festival bag checks.
5. **7 h battery on the internal cell** — one night rave, no powerbanks.
6. **AU-legal** — 915–928 MHz under the ACMA LIPD class licence (licence-free; verified no duty-cycle limit in AU).

## Research

All in [docs/](docs/):

- [research-2026-09.md](docs/research-2026-09.md) — hardware landscape, Meshtastic ecosystem, radio physics, DIY prior art
- [research-totem-competitors.md](docs/research-totem-competitors.md) — FCC teardown of the commercial Totem Compass (ESP32 2.4 GHz — validates our band choice by counterexample), competitor status, more open-source prior art
- [research-single-beacon.md](docs/research-single-beacon.md) — **the load-bearing engineering doc**: measured power budgets (7 h feasibility), locked radio params (SF8/250 @ 1 Hz), raw-LoRa-vs-Meshtastic call, token hardware shortlist

Key prior art: [spoke](https://github.com/FeruzTopalov/spoke) (GPS-PPS-synced beacon epochs), [natnafu/beacon](https://github.com/natnafu/beacon) (bearing → LED ring), the LoRa-APRS ecosystem (one-way position beaconing at scale).

## Roadmap

1. **Protocol spike** — 2 boards, RadioLib, 1 Hz PPS-aligned beacon + scheduled RX windows
2. **Pointer maths** — tilt-compensated heading + bearing → LED arrow
3. **Power hardening** — bench-measure, size the battery for 7 h with margin
4. **Enclosure** — token puck + anchor box on the crew Ender 3 *(mech eng input wanted)*
5. **Fleet + field test** — 5 tokens + anchor at a real event; publish measured 915 MHz crowd-attenuation data (doesn't exist publicly)

## Repo layout

```
docs/       research, decisions, field-test notes
hardware/   schematics, BOMs, enclosure CAD
firmware/   beacon + token firmware (one codebase, two roles)
```

## Contributing

Open source (MIT) — the goal is anyone can build a set for their own crew. Open an issue or push a branch + PR. Mechanical (enclosure/CAD) and electrical (RF/antenna/power) eyes especially wanted. See [CONTRIBUTING.md](CONTRIBUTING.md).

## License

[MIT](LICENSE)
