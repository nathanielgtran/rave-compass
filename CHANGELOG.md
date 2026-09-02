# Changelog

- ci-2: fix native-tests workflow — drop setup-python pip cache (hard-fails when repo has no requirements.txt; workflow was red since ci-1)
- feat-2: ravecore v1 — AES-128-CTR payload encryption (frame v1 via version bits in top 2 bits of id byte), streaming magnetometer hard-iron calibration (min/max figure-8 fit), circular bearing EMA + pixel hysteresis anti-flap; embedded l1lite_token + l1lite_beacon compile clean (board sub: adafruit_feather_nrf52840, same nRF52840 MCU — swap to Seeed BSP on hardware arrival); 49 tests green (was 40)
- ci-1: GitHub Actions CI — native unit tests on ubuntu-latest for every PR + master push (GitHub-hosted deliberately: public repo, fork PRs must not touch private infra)
- feat-1: firmware scaffold — PlatformIO native+nRF52 envs, ravecore pure-logic lib (geo/heading/arrow/protocol/state) + 38 unit tests all green (pio test -e native), protocol spec v0, hardware requirements doc

- docs-7: Phase 6 stretch — "become the anchor" token-promotion mode (long-press promote, listen-before-promote guard, new epoch/id)
- docs-6: single-beacon pivot — research-single-beacon.md (power budget, SF8/250 @ 1 Hz, raw RadioLib call, Wio Tracker L1 Lite pick); full PLAN.md + README rewrite w/ new mermaid diagrams; CONTRIBUTING.md; bearing formula fixed (cos-lat) + tilt-comp requirement added; review-agent fixes (water-resonance myth, QR removed, GPS overclaim)
- docs-5: CLAUDE.md — objective, locked architecture, doc rules, workflow (entry added retroactively)
- docs-4: mermaid diagrams in PLAN.md — system overview, position→arrow sequence, pointer-unit assembly, build phases + gates
- docs-3: Totem teardown + competitor round 2 research; PLAN.md — architecture, BOM, 5 build phases
- docs-2: hardware research report (Sept 2026) — prior art, board matrix, radio physics, 3 build configs; README findings summary
- docs-1: initial project brief — problem, physics, requirements, candidate architectures, roadmap
