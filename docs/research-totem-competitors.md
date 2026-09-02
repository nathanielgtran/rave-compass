# Totem teardown + competitor & prior-art round 2 — September 2026

Follow-up research: how the commercial "Totem Compass" actually works (FCC filings + reviews), competitor landscape, and additional open-source prior art beyond [research-2026-09.md](research-2026-09.md).

## Totem Compass (Totem, Inc.)

Chattanooga TN startup, founded early 2024, $1.3M angel pre-seed (Feb 2025). **Real and shipping** — launched $69, now **$88 USD** ([totemlabs.com](https://totemlabs.com/products/totem-compass)). Deployed at EDC Las Vegas 2025 (~1% adoption), Lost Lands 2025 (~3,500 units), SXSW ([funding PR](https://www.businesswire.com/news/home/20250212385814/en/)). Adoption numbers are all from Totem's own press releases — treat as marketing.

### Tech stack (verified via FCC + their own docs)

| Component | Detail | Source |
|---|---|---|
| MCU/Radio | **ESP32-WROOM-32E** — FCC ID [2BMQS-1](https://fccid.io/2BMQS-1), granted Mar 2025 | FCC |
| Radio | **2.4 GHz, NOT LoRa.** Proprietary "peer-to-peer connectionless protocol", 2402–2480 MHz — given ESP32 + "connectionless" + 1 Hz updates, almost certainly **ESP-NOW or raw 802.11 frames** (inference; no teardown confirms firmware). TX up to 0.398 W | FCC + [their tech blog](https://totemlabs.com/blogs/posts/how-the-totem-compass-works) |
| Positioning | Onboard GNSS, ~1 m claimed, 1 Hz broadcast; direction = GPS delta + magnetometer, tilt-up "Compass Mode" | their blog |
| Mesh | "Unity Mesh Network" — every unit relays; range "boundless" *with density* | their blog |
| Display | No screen — **LED ring** with per-friend colours; capacitive "Touch Crystal"; mic for sound-reactive Vibe Mode | product page |
| Bonding | Touch-to-bond (~1 inch, simultaneous press), 4 bonds/unit, persists across power-off | their docs |
| Battery | Claimed 14 h Vibe / 20–24 h Eco, USB-C | product page |
| Phone app | Standalone; optional BLE companion app (Jul 2025), festival maps + waypoints (Nov 2025) | PRs |

### Claims vs reality

Independent [Trail & Kale review](https://trailandkale.com/totem-compass-review/) + user reports:

- **1,000 m+ range**: works in crowds, but "lagged once spread out too far" at Burning Man — the mesh only helps at density. 1 km at 2.4 GHz needs clear LOS.
- **Battery 10–24 h**: heavy users report **under 10 h**.
- **Setup**: finicky — hard resets, unpredictable outdoor bonding, day-one firmware updates at EDC.
- **Indoors**: doesn't work (they admit it).
- Overall sentiment genuinely positive at Bonnaroo/Lost Lands — but the top "Reddit review" search hit is hosted on totemlabs.com itself; astroturf-adjacent.

### What this means for rave-compass

Totem picked cheap 2.4 GHz ESP32 P2P + density-dependent mesh over LoRa — their weakness is exactly sparse/spread-out crews and body blockage, which is where 915 MHz LoRa wins. Their strengths worth copying: **1 Hz updates** (Meshtastic defaults are 20 s+), **touch-to-bond pairing UX**, and the **screen-less LED-ring** as a dead-simple/cheap form factor.

## Commercial competitors

| Product | Tech | Price/Status |
|---|---|---|
| [Lynq](https://trailandkale.com/lynq-review-smart-compass/) (2018 Indiegogo) | 900 MHz sub-GHz + GPS, display w/ distance+arrow, 12 users, no mesh | Still on Amazon/eBay 2026 but company looks dormant — old-stock resale (unverified) |
| [Crowd Compass](https://www.crowdcompass.io/products/crowd-compass) | **LoRa 915 MHz** + GPS + BLE app, magnetometer, mesh, canned messages | ~3 mi claimed, 24 h on 3×AA, perpetual "45% off" — closest architecture to ours; zero independent reviews (unverified) |
| [BuddyCompass](https://buddycompass.shop/) | LoRa + GPS + mesh, 8 devices, solo-dev German garage build | Pre-order only; 3 km / 5-day claims unverified ([Extra Chill](https://wire.extrachill.com/festival-wire/buddycompass-claims-lora-festival-compass-to-keep-groups-together-without-cell-service/)) |
| Flare | Phone app only | Not hardware — different game |

## Open-source prior art — round 2 finds

(Beyond Friend Finder Edition / friendcompass / YAK / burntastic from [round 1](research-2026-09.md).)

| Project | Hardware | Radio | Status | Worth stealing |
|---|---|---|---|---|
| [FeruzTopalov/spoke](https://github.com/FeruzTopalov/spoke) (+ predecessor [eleph](https://github.com/FeruzTopalov/eleph)) — GPL-3, active Mar 2026 | STM32F103, SX126x, GPS, LCD, optional mag | LoRa 433/868 | **Most complete find** — multiple PCB revs, full firmware, 5 users | **GPS-PPS-synchronised TDMA slots** (collision-free positions, no mesh overhead), 13-byte packed payload, geofence/timeout/SOS, radar UI. Best protocol design to crib |
| [ThePangel/LodeStone](https://github.com/ThePangel/LodeStone) — GPL-3, Aug 2026 | XIAO ESP32-S3, SX1262, L76K GNSS, MPU-9250, GC9A01 round TFT | LoRa P2P | Early WIP | On-demand location query (listen-idle, ask-on-raise = battery win), gesture pairing, TX power scaled to distance. BOM ≈ a DIY T1000-E |
| [natnafu/beacon](https://github.com/natnafu/beacon) | Feather M0 + LoRa + GPS + compass, points at its twin | LoRa | Complete-ish pair | Simplest two-node arrow architecture |
| [konradmh3/MiniMap](https://github.com/konradmh3/MiniMap) | Phone (React Native) + BLE to ESP32 LoRa/GPS dongle | LoRa backhaul, BLE to phone | Abandoned | Validates the "phone-as-screen, LoRa-as-radio" hybrid (our Config 1) |
| [ReinhardLenz/hot_and_cold_game](https://github.com/ReinhardLenz/hot_and_cold_game) | LED ring + mag + LoRa, 2 players | LoRa | Toy/WIP | LED-ring-only bearing UI — Totem's UX without a screen |
| [cmcoffman/FriendFinder](https://github.com/cmcoffman/FriendFinder) | ESP32 + LoRa + GPS | LoRa | Abandoned 2022 | Minimal reference |
| [Mikhail-Za/trail-mate-T-display-p4](https://github.com/Mikhail-Za/trail-mate-T-display-p4-) | LilyGo T-Display P4 | LoRa | Active, broader outdoor-OS scope | Offline maps + waypoints if we outgrow arrow-only |
| [Arduino forum thread](https://forum.arduino.cc/t/friend-compass-for-festivals/372031) | — | — | discussion | **SX1280 (2.4 GHz LoRa) Time-of-Flight ranging** — RF-only direction/distance, no GPS; fallback idea for GPS-denied dense pits |

## Design takeaways

1. **Radio choice validated**: Totem's user-reported failure mode (falls over when the crew spreads out) is precisely LoRa's strength. Stay 915 MHz.
2. **Crib spoke's TDMA**: PPS-synced slots + 13-byte payloads let a small crew beacon at ~1 Hz-like freshness without Meshtastic's airtime pain — worth considering if we outgrow stock Meshtastic intervals.
3. **Crib LodeStone's power model**: idle-listen, query-on-raise. Battery is the ESP32 path's weak point.
4. **LED-ring SKU**: Totem proves a screen-less LED-ring device is a viable, cheaper, simpler second form factor — could be the "beacon for the less-nerdy friend" instead of (or on) a T1000-E.
5. **Touch-to-bond UX** beats channel/PSK config screens for non-technical friends.
