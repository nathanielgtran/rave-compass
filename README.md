# rave-compass

A physical "friend compass" for raves and festivals. Each person in the crew carries one; it points an arrow + distance toward everyone else — **with zero mobile signal**.

## The problem

Festivals kill phones: dense crowds + overloaded or absent cell towers mean every "find my friends" app is dead on arrival. Paid offline trackers (Lynq etc.) are expensive, subscription-gated, or discontinued.

GPS itself still works offline — it's receive-only from satellites, so every device *knows where it is*. The missing piece is **sharing positions between devices with no internet**. That's what this project builds.

## Physics (why the obvious options fail)

| Channel | Range in a packed crowd | Verdict |
|---|---|---|
| BLE / WiFi-Direct | ~5–15m — human bodies absorb 2.4GHz | useless at festival scale |
| Cell / internet | assumes a working tower | the whole problem |
| **LoRa (915MHz AU ISM)** | **hundreds of m to km, off-grid** | **the viable radio** |

[Meshtastic](https://meshtastic.org) — open-source LoRa mesh firmware — already solves position broadcast, meshing, and congestion. Likely the radio layer regardless of which hardware path wins.

## Requirements

1. **Zero-signal operation** — self-contained LoRa mesh between units.
2. **One device per friend** (4–6 units), target ~$30–80 AUD per unit.
3. **Compass UX, not a map** — bearing arrow (GPS delta + magnetometer heading) + distance to each friend.
4. **Must not look like a bomb.** Clean consumer-grade enclosure — no exposed PCBs, loose wires, or taped-on batteries. Has to sail through festival security bag checks.
5. **12h+ battery** — a full rave day/night.
6. **AU-legal** — 915MHz ISM band (AU915).

## Candidate architectures

| Path | Shape | Trade-off |
|---|---|---|
| **A. Finished tracker + phone app** | Seeed T1000-E (card-size, IP65, stock Meshtastic, ~$60) per friend; custom compass app over BLE | Least hardware work, most consumer-looking; UI lives on phone |
| **B. Handheld with own display** | T-Echo (e-ink, cased) or T-Beam/Heltec + round LCD (GC9A01) + magnetometer (QMC5883L) in 3D-printed case | Real gadget feel, arrow on-device, no phone needed; enclosure work required |
| **C. Full custom** | ESP32 + LoRa + GPS + LED ring, custom firmware + case | Max fun, max effort, highest bomb-lookalike risk if the case is lazy |

## Open questions

- Meshtastic API surface: cleanest way to get friend positions into custom UI (serial / BLE / on-device plugin)?
- Real LoRa range in dense festival crowds (field reports, not marketing).
- Position-broadcast interval vs battery vs mesh congestion at 4–6 nodes.
- Magnetometer tilt-compensation (handhelds aren't level) — sensor fusion needed?
- Best cased-by-default boards in 2026 + AU pricing/suppliers.
- Enclosure design: 3D-print vs off-the-shelf case mods.

## Explicitly not doing

- Subscriptions, accounts, or any cloud dependency at the venue.
- Phone-only BLE mesh (physics says no).
- Persisted location trails — live positions only, ephemeral by design.

## Roadmap

1. **Research** — competitive landscape + prior DIY art (in progress; report → `docs/`)
2. **Radio prototype** — 2 boards exchanging positions over LoRa
3. **Compass prototype** — bearing math + heading on real hardware
4. **Enclosure pass** — the "doesn't look like a bomb" gate *(mech eng input wanted)*
5. **Crew build** — 4–6 units, field-test at an actual event

## Repo layout

```
docs/       research, decisions, field-test notes
hardware/   schematics, BOMs, enclosure CAD
firmware/   device code (or Meshtastic config + companion app)
```

## Contributing

Open source (MIT) — the goal is anyone can build one for their own crew. Open an issue or push a branch + PR. Mechanical (enclosure/CAD) and electrical (RF/antenna/power) eyes especially wanted.

## License

[MIT](LICENSE)
