# Hardware & prior-art research — September 2026

Research sweep: commercial products, DIY prior art, Meshtastic hardware landscape, and honest radio physics for a zero-signal friend compass. AU (Sydney) context — AU915/ANZ band, local suppliers.

## TL;DR

This exact device already exists as a community project: **[Meshtastic Friend Finder Edition](https://github.com/LeapYeet/Meshtastic-Firmware-Friend-Finder-Edition)** — a Meshtastic firmware fork adding pairing, live distance, and a magnetometer-driven arrow pointing at your friend, tested at a crowded outdoor concert. The pragmatic path: consumer-finished Meshtastic hardware (Seeed T1000-E card trackers as beacons, ~AUD $60–93 each), plus one or two screen-equipped pointer units running Friend Finder firmware. Region **ANZ (915–928 MHz)** is legal under the ACMA LIPD class licence — no permit needed. ~AUD $60–100/head, almost no soldering.

## Candidate hardware paths

| Board / kit | Cost (AUD approx.) | Finished case? | GPS? | Screen? | Battery | Effort | Verdict |
|---|---|---|---|---|---|---|---|
| **Seeed SenseCAP T1000-E** ([Seeed](https://www.seeedstudio.com/SenseCAP-Card-Tracker-T1000-E-for-Meshtastic-p-5913.html), [IoT Store AU $85 ex GST](https://iot-store.com.au/products/sensecap-tracker-t1000-e-meshtastic), [Core Electronics](https://core-electronics.com.au/sensecap-card-tracker-t1000-e-meshtastic-lora-bluetooth.html)) | $60–93 (USD $39.90 direct) | **Yes — credit-card size, IP65, looks like a commercial tracker** | Yes (AG3335) | No — buzzer + LED; compass via phone app | 700 mAh, ~2 days typical ([review](https://nodakmesh.org/blog/seeed-sensecap-t1000-e-review)) | Near zero — pre-flashed Meshtastic | **Best beacon.** nRF52 → NOT compatible with Friend Finder firmware |
| **LilyGO T-Echo** ([LilyGO USD $48.49](https://lilygo.cc/en-us/products/t-echo-meshtastic), [docs](https://meshtastic.org/docs/hardware/devices/lilygo/techo/)) | ~$75–95 + ship | Yes-ish — enclosed shell ([review](https://hamradiotherapy.com/lora-mesh-networks/lilygo-t-echo-review/)) | Yes (L76K) | 1.54" e-ink (stock firmware shows distance/bearing) | 850 mAh, several days (nRF52) | Zero build | Good all-in-one; e-ink slow for live compass; nRF52 → no Friend Finder fork |
| **T-Echo Lite** ([CNX](https://www.cnx-software.com/2025/04/25/lilygo-t-echo-lite-a-1-22-inch-e-paper-display-board-with-nrf52840-soc-lora-transceiver-gps/)) | ~$25–65 | Partial; GPS variants often out of stock | Optional | 1.22" e-ink | Very low draw | Low-med | Cheapest nRF52+GPS+e-ink but stock issues; skip |
| **Heltec WiFi LoRa 32 V3** + QMC5883L + GPS + printed case ([muzi.works H1 case](https://www.printables.com/model/741974-h1-case-for-heltec-v3-running-meshtastic)) | ~$60–70 all-in | No — print one (good designs exist) | External (wire in) | 0.96" OLED | LiPo; ESP32 ≈ 1 day realistic | **High** — solder, calibrate, print | **The Friend Finder reference build.** Confirmed supported board |
| **Heltec Wireless Tracker V1.1/V2** ([Heltec](https://heltec.org/project/wireless-tracker-v2/)) | ~$50 | No (cases printable) | **Onboard** | 0.96" TFT | External LiPo | Medium | GPS built-in, ESP32-S3 so Friend Finder *may* run — **unconfirmed** (only Heltec V3 + T-LoRa T3-S3 validated) |
| **RAK WisMesh Pocket V2** ([RAK USD $99](https://store.rakwireless.com/products/wismesh-pocket), [review](https://adrelien.com/wismesh-pocket-v2-review/)) | ~$150–160 | **Yes — injection-moulded consumer handheld** | Yes | 1.3" OLED | 3200 mAh, multi-day | Zero build | Most polished handheld; pricey ×6; nRF52 (no FF fork) |
| **LilyGO T-Beam** (classic) | ~$45–60 | No — bare board | Yes | OLED | 18650 | High | Superseded ([Burning Man build](https://yak.ventures/2022/08/11/finding-my-friend-at-burning-man/) used these); skip in 2026 |
| **Full scratch**: ESP32-C3/S3 + SX1262 + GPS + QMC5883L + GC9A01 round LCD | ~$40–60 parts | Print your own puck | Yes | 1.28" round LCD — genuinely compass-looking | Your LiPo | **Very high** | Coolest result, slowest path ([Micro Compass](https://abidcg.blogspot.com/2026/08/micro-compass.html) for the display half) |

## Projects to crib from

1. **[LeapYeet/Meshtastic-Firmware-Friend-Finder-Edition](https://github.com/LeapYeet/Meshtastic-Firmware-Friend-Finder-Edition)** + [web flasher](https://leapyeet.github.io/Meshtastic-Firmware-Friend-Finder-Edition/) — the crown jewel. Mutual pairing, position exchange every 20 s, live distance, big directional arrow (needs QMC5883L), saved locations ("car", "campsite"), friend map. ESP32-S3 boards only (Heltec V3, T-LoRa T3-S3). Figure-8 + flat-spin calibration. GPL, actively maintained.
2. **[seanhlewis/friendcompass](https://github.com/seanhlewis/friendcompass)** — RF/RSSI pair-compass experiment (SX1280 ToF, haptic "heartbeat" as you close in). Good haptics ideas; RSSI ranging not production-reliable.
3. **[YAK Ventures — Finding My Friend at Burning Man](https://yak.ventures/2022/08/11/finding-my-friend-at-burning-man/)** — T-Beam pair, printed cases; ~975 m verified; dust/heat sealing lessons.
4. **[meshtastic/burntastic](https://github.com/meshtastic/burntastic/)** — official Burning Man variant; reference for surviving ~1,000-node meshes ([thread](https://meshtastic.discourse.group/t/meshtastic-burning-man-2024/14293)).
5. **ESP32 Festival Tracker** ([video](https://www.youtube.com/watch?v=i7K-_zALYdg)) — scratch build of exactly this concept.
6. **[Micro Compass](https://abidcg.blogspot.com/2026/08/micro-compass.html)** + **[QMC5883LCompass lib](https://github.com/mprograms/QMC5883LCompass)** — pointer-UI building blocks.
7. Cases: **[NodakMesh 3D-print index](https://nodakmesh.org/3d-printing)** (70+ curated), **[muzi.works H1](https://www.printables.com/model/741974-h1-case-for-heltec-v3-running-meshtastic)**, [T1000-E print-in-place wallet](https://www.printables.com/model/1269619-sensecap-t-1000-print-in-place-wallet).

### Getting positions into custom UI (Meshtastic API surface)

Three ways: (1) fork the firmware (LeapYeet's approach); (2) stock firmware already draws per-node distance + bearing on GPS devices — but [firmware #9928](https://github.com/meshtastic/firmware/issues/9928): without a magnetometer the "compass" orients from GPS course, i.e. wrong while standing still — this is why the QMC5883L matters; (3) phone apps over BLE — official apps show node maps/distance, and [MeshWave (iOS)](https://apps.apple.com/us/app/meshwave/id6760241031) has an explicit bearing-navigation compass to any node. Phone-as-display + T1000-E-as-radio is fully solder-free.

## Radio physics — honest numbers

- **Legality (AU):** Meshtastic region **ANZ = 915.0–928.0 MHz**, licence-free under ACMA LIPD class licence ([wireless.org.au](https://wireless.org.au/meshtastic/), [AU analysis](https://is-this-legal.com/is-meshtastic-legal-in-australia/)). Set region at flash time.
- **LoRa range:** km-scale line-of-sight is real (~1 km verified casually). In dense crowds expect **hundreds of metres, not km** — bodies attenuate strongly ([festival report](https://wire.extrachill.com/festival-wire/how-meshtastic-mesh-radios-are-changing-festival-camp-communication/)). 915 MHz penetrates crowds far better than 2.4 GHz, and mesh hops via other friends extend reach. A 4–6 node private mesh across a 1–2 km festival site: realistically fine.
- **Why BLE/WiFi fail (confirmed):** human bodies strongly attenuate 2.4 GHz — one body eats 5–10 dB ([measured study](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC6211019/)); band also jammed by thousands of phones. (Note: the popular "2.4 GHz = water resonance" framing is a myth — the attenuation is real, the resonance isn't.) Tens of metres at best in a crowd.
- **ESP-NOW:** decent in the open (~250 m reliable, LR mode ~500 m — [Espressif tests](https://developer.espressif.com/blog/esp-now-for-outdoor-applications/), [atomic14](https://www.atomic14.com/videos/posts/oz0a7Ur7nko)) but still 2.4 GHz → crowd-killed, and no mesh. **Verdict: LoRa or nothing.**
- **Congestion:** LongFast preset ≈ 50–80 active nodes before the channel chokes ([preset blog](https://meshtastic.org/blog/why-your-mesh-should-switch-from-longfast/)); Burning Man 2024 hit ~1,000 and needed custom firmware. Private 6-node group trivial — use own private channel/PSK, consider non-default frequency slot if the festival has a big public mesh.
- **Battery:** nRF52 devices (T1000-E ~2 days; T-Echo several days) outlast a festival day easily; ESP32 boards burn 5–10× more — ~1 day per charge with 1,500–3,000 mAh + power bank. Position interval = main battery knob (20 s fine; stretch to 60 s).
- **GPS accuracy:** 3–10 m; bearing to a friend <30 m away swings wildly — switch from "arrow" to "you're basically there" logic up close (Friend Finder does this). Magnetometers need calibration; mount away from battery/speaker magnets.

## Recommended configurations (crew of 4–6)

### Config 1 — "Zero solder, passes any bag check": T1000-E fleet + phone compass
6 × Seeed T1000-E (AU915): USD $39.90 ea direct (≈AUD $62, 10% off ≥2) or AUD $85 ex GST local ([IoT Store](https://iot-store.com.au/products/sensecap-tracker-t1000-e-meshtastic), [Core Electronics](https://core-electronics.com.au/sensecap-card-tracker-t1000-e-meshtastic-lora-bluetooth.html)). Private channel + PSK, 30–60 s beacons. Compass = each phone over BLE (official app map, or MeshWave bearing compass on iOS). Phones in flight mode — BLE to your own pocket device always works; LoRa does the long haul.
**Total ~AUD $370–560 for 6.** IP65, card-sized, looks exactly like the commercial product it is. Downside: no phone-free arrow.

### Config 2 — "The actual friend compass" (recommended sweet spot): FF pointer units + T1000-E beacons
2–3 × pointer units: Heltec V3 (~$35) + QMC5883L (~$4) + ATGM336H/NEO-6M GPS (~$12) + 1,500 mAh LiPo (~$12) + H1-style printed case (~$2) ≈ **AUD $65/unit**; flash via [FF web flasher](https://leapyeet.github.io/Meshtastic-Firmware-Friend-Finder-Edition/), figure-8 calibrate. Plus 3–4 × T1000-E beacons for the less nerdy friends.
**Interop caveat:** FF's arrow works between its own paired nodes; verify stock-firmware beacon positions render on the pointer's friend map *before* the event.
**Total ~AUD $400–450 for 6.** H1 case + stub antenna reads as "commercial GPS walkie", not a bomb.

### Config 3 — "Full custom consumer-look puck" (the portfolio piece)
Per unit: ESP32-S3 mini + SX1262 + L76K GPS + QMC5883L + GC9A01 round LCD + DRV2605 haptic + 1,000 mAh LiPo ≈ AUD $45–60 parts, custom round printed puck. Firmware: port FF or thin custom protocol; crib UI from Micro Compass. **Total ~AUD $300–360 + many weekends.** Only if the build *is* the hobby.

## Security-optics notes

No first-hand "Meshtastic confiscated at festival" reports found — but organisers are twitchy about hobbyist boards (NYC banned Raspberry Pis/Flipper Zeros from an event outright — [The Register](https://www.theregister.com/2025/12/31/zohran_mamdani_raspberry_pi_ban/)); AU banned-items lists give guards discretion over "suspicious electronics". Rules of thumb: fully enclosed case, no visible wires/cells, a printed brand-ish label, stubby antenna not a 20 cm whip. (QR code idea rejected — guards don't scan, it just invites questions; boring beats explainable.) The T1000-E is indistinguishable from a Tile/AirTag-class product.

## Marketing vs verified

- T1000-E "2-day battery": corroborated by independent review; heavy 20 s beaconing will cut it — "70 days standby" is marketing.
- LoRa "kilometres": LOS only; crowd range = hundreds of metres (community reports).
- FF "performed exceptionally at a crowded concert": single-dev claim, plausible, unreplicated — **test at a park meetup before the festival**.
- Core Electronics "$21.20" search snippet for T1000-E is wrong/accessory pricing; verified AU street price $85 ex GST.
