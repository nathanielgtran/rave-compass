# CLAUDE.md — rave-compass

Instructions for Claude Code (and any LLM assistant) working in this repo. Read this before touching anything.

## Objective

Build a physical device system so a crew of friends can find each other at raves/festivals with **zero mobile signal**, for free (no subscriptions, no cloud), and it must **pass a festival bag check** looking like a consumer gadget.

## Goal architecture (locked 2026-09-02)

**Single-beacon homing.** Not a mesh. Not per-person tracking.

- **1 beacon ("anchor")** — carried by one person or tied to the crew totem. Broadcasts its GPS position over LoRa (ANZ 915–928 MHz) every 1 s, PPS-aligned.
- **N pointer tokens** — small, RX-only, one per friend. GPS + tilt-compensated compass (mag + accelerometer — bare magnetometer is not enough) + an LED-ring arrow that always points at the beacon. Lost → follow arrow → crew converges.

Accepted trade-off: you find the *beacon*, not individuals. That is the product: "get back to the crew", not "surveil 5 people".

**Firmware base (decided): custom raw-LoRa protocol on RadioLib, target nRF52840 (Wio Tracker L1 Lite). NOT Meshtastic** — stock Meshtastic floors position broadcast at 30 s and its mesh is dead weight for one-TX/N-silent-RX. One codebase, `BEACON`/`TOKEN` roles. Radio params locked in docs/research-single-beacon.md (SF8/250, 1 Hz, +22 dBm).

## Operating constraints (non-negotiable)

- **Session scope:** one night rave, ~7 h. Battery must last that on the internal cell — **no powerbanks**.
- **Radio:** LoRa 915 MHz ANZ band, licence-free under ACMA LIPD class licence. Region must be set correctly in any firmware. 2.4 GHz (BLE/WiFi/ESP-NOW) is banned as the primary radio — bodies absorb it, crowds kill it (see docs/research-totem-competitors.md: Totem's failure mode).
- **Zero infra at the venue:** no cell, no internet, no cloud, no accounts. GPS is receive-only and works with zero cell signal (degraded under solid cover); everything else travels over our own LoRa link.
- **Bag-check rule:** every unit ships fully enclosed — no exposed PCBs, no loose wires, no taped batteries. Branded label (no QR codes — nobody scans, it just invites questions) + stub antenna. If it looks like a bomb, it doesn't ship.
- **Budget:** ~$60–80 AUD per token; one-off anchor can cost more.

## Design principles

Canonical list lives in `PLAN.md` (bottom) — single source of truth, do not duplicate it here or elsewhere. Refer to the principles by name in every design decision and PR description: *LoRa or nothing · Ephemeral by design · Degrade loudly · Consumer-finished or it doesn't ship.*

## Repo layout

```
README.md    public-facing pitch + findings summary — keep current for contributors
PLAN.md      architecture, diagrams, BOM, build phases, Current State section
CLAUDE.md    this file
CHANGELOG.md incrementing IDs (type-N), newest first
docs/        research reports, decisions, field-test notes
hardware/    schematics, BOMs, enclosure CAD (mech eng mate owns CAD)
firmware/    beacon + token firmware
```

## Documentation rules (ALWAYS)

Docs are a first-class deliverable — the collaborators (mechanical + electrical engineers, not Claude users) rely on them.

- **Every PR updates the relevant docs in the same PR**: README.md if user-facing behaviour/summary changes, PLAN.md if architecture/BOM/phases change, CHANGELOG.md always (next `type-N` id), docs/ for new research or field-test results.
- **PLAN.md `## Current State`** (3 lines: phase, next, blocked) must be updated after every work session.
- **Mermaid diagrams in PLAN.md must match reality** — if the architecture changes, the diagrams change in the same PR. No stale diagrams.
- Research goes in `docs/research-*.md` with cited URLs; flag marketing claims vs verified numbers.
- Field-test results go in `docs/field-tests/` with date, location type, and measured numbers (range, battery, arrow accuracy).

## Workflow

- Branch + PR for everything; never push to master. Squash-merge.
- **The `main/master` ruleset requires a PR review that same-account approval can't satisfy — the human merges via the GitHub web UI.** Don't attempt `--admin` merges.
- Public repo: no personal info (addresses, phone numbers, mates' names), no credentials, no venue-specific plans in committed files.
- Commit style: `type: description` (no ticket prefix — hobby repo). No Claude co-author footer, no emojis.
- Firmware code: C++/PlatformIO or ESP-IDF conventions once firmware/ exists; keep functions small, no swallowed errors.

## Hardware context

- Prototyping + enclosures: a collaborator has a **modded Ender 3** — printed cases are free and iterable (PETG for heat).
- Buy AU915/ANZ variants only. AU suppliers: IoT Store, Core Electronics; AliExpress/Seeed direct for modules. Board of record: Seeed Wio Tracker L1 Lite (nRF52840 + SX1262 + L76K GPS, ~A$57 at Core Electronics).
- Key prior art (see docs/): FeruzTopalov spoke/eleph (GPS-PPS-synced one-way beacons — the protocol crib), natnafu/beacon (bearing → LED-ring pointer — the token crib), LoRa-APRS ecosystem (one-way beaconing at scale), Totem FCC teardown (what not to do: 2.4 GHz). Meshtastic-era research (Friend Finder Edition etc.) is historical context in docs/research-2026-09.md — superseded by the raw-LoRa decision.
