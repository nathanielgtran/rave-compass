# Contributing

Crew project, but outsiders welcome. Short version: read [PLAN.md](PLAN.md) first — architecture is locked (single-beacon homing, raw LoRa, no mesh) and the design principles at the bottom of that file govern every decision.

## Ways to help

- **Mechanical / CAD** — token puck (~50 mm, lanyard hole, LED diffuser face, GPS patch antenna needs sky view) and anchor box (belt-clip + zip-tie mounts). CAD lives in `hardware/`. We print on an Ender 3 in PETG; keep walls printable without supports where possible.
- **Electrical / RF** — antenna choice + placement (internal helical vs stub), LiPo charging/protection, magnetometer placement away from the battery, bench power measurements (see the open flags in [docs/research-single-beacon.md](docs/research-single-beacon.md) §6).
- **Firmware** — C++/PlatformIO, [RadioLib](https://github.com/jgromes/RadioLib), nRF52840 target. One codebase, `BEACON`/`TOKEN` roles. Cribs: [spoke](https://github.com/FeruzTopalov/spoke), [natnafu/beacon](https://github.com/natnafu/beacon).
- **Field testing** — walk-tests and (eventually) real-crowd range measurements. Log results to `docs/field-tests/` with date, location type, and measured numbers.

## Ground rules

- Branch + PR for everything; squash-merge. The repo owner merges.
- Every PR updates the docs it touches: CHANGELOG.md always (next `type-N` id), PLAN.md if architecture/BOM/phases change (diagrams must match reality), README.md if the pitch changes.
- Radio work must stay inside the ACMA LIPD class licence (915–928 MHz, ≤1 W EIRP). Firmware must set the region explicitly.
- No personal info in the repo (it's public): no names, addresses, phone numbers, or venue-specific plans.
- Jargon decoder for non-software folks: **walk-test** = two units, go for a walk, log where packets drop. **SF8/250** = LoRa spreading factor 8 on 250 kHz bandwidth (speed-vs-range knob). **PPS** = the GPS pulse-per-second line both ends use as a shared clock.
