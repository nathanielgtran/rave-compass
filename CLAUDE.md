# CLAUDE.md — rave-compass

Instructions for Claude Code (and any LLM assistant) working in this repo. Read this before touching anything.

## Objective

Build a physical device system so a crew of friends can find each other at raves/festivals with **zero mobile signal**, for free (no subscriptions, no cloud), and it must **pass a festival bag check** looking like a consumer gadget.

## Goal architecture (locked 2026-09-02)

**Single-beacon homing.** Not a mesh. Not per-person tracking.

- **1 beacon ("anchor")** — carried by one person or tied to the crew totem. Broadcasts its GPS position over LoRa (ANZ 915–928 MHz) every 1–2 s.
- **N pointer tokens** — small, RX-only, one per friend. GPS + magnetometer + an arrow that always points at the beacon. Lost → follow arrow → crew converges.

Accepted trade-off: you find the *beacon*, not individuals. That is the product: "get back to the crew", not "surveil 5 people".

## Operating constraints (non-negotiable)

- **Session scope:** one night rave, ~7 h. Battery must last that on the internal cell — **no powerbanks**.
- **Radio:** LoRa 915 MHz ANZ band, licence-free under ACMA LIPD class licence. Region must be set correctly in any firmware. 2.4 GHz (BLE/WiFi/ESP-NOW) is banned as the primary radio — bodies absorb it, crowds kill it (see docs/research-totem-competitors.md: Totem's failure mode).
- **Zero infra at the venue:** no cell, no internet, no cloud, no accounts. GPS is receive-only and always works; everything else travels over our own LoRa link.
- **Bag-check rule:** every unit ships fully enclosed — no exposed PCBs, no loose wires, no taped batteries. Branded label + stub antenna. If it looks like a bomb, it doesn't ship.
- **Budget:** cheap per token (~$30–60 AUD target); one-off beacon can cost more.

## Design principles

Defined in `PLAN.md` — refer to them in every design decision and PR description:

1. **LoRa or nothing** — 2.4 GHz dies in crowds.
2. **Ephemeral by design** — live positions only, private/encrypted, no trails, no cloud.
3. **Degrade loudly** — stale data is shown as stale (last-heard age), never presented as current.
4. **Consumer-finished or it doesn't ship** — every unit passes a bag check.

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
- Buy AU915/ANZ variants only. AU suppliers: IoT Store, Core Electronics; AliExpress/Seeed direct for modules.
- Key prior art (see docs/): LeapYeet Friend Finder Edition (Meshtastic fork), FeruzTopalov spoke/eleph (TDMA one-way beacons), natnafu/beacon (two-node pointer), reverse-geocache genre (arrow-to-coordinate UX), Totem FCC teardown (what not to do: 2.4 GHz).
