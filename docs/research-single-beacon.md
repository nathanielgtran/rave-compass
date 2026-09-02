# Single-beacon architecture research — September 2026

Validation research for the pivoted architecture: 1 anchor beacon broadcasting GPS at 1–2 s + N RX-only pointer tokens, 7 h night rave, no powerbanks. This is the load-bearing engineering doc for radio params, power budget, and hardware selection.

## 1. Power budget (measured numbers, cited)

| Component | Mode | Current | Source |
|---|---|---|---|
| SX1262 | Continuous LoRa RX (DC-DC, 125 kHz) | **4.2 mA** (~5 mA @ 250 kHz) | [Semtech datasheet](https://cdn.sparkfun.com/assets/6/b/5/1/4/SX1262_datasheet.pdf) |
| SX1262 | Duty-cycled RX (RadioLib) | ~3.4 mA naive; **sub-1 mA** with scheduled windows | [RadioLib #585](https://github.com/jgromes/RadioLib/discussions/585), [Semtech AN1200.85](https://www.semtech.com/uploads/technology/LoRa/cad-ensuring-lora-packets.pdf) |
| SX1262 | Sleep | 0.6–1.2 µA | [Seeed forum measurements](https://forum.seeedstudio.com/t/xiao-nrf52840-wio-sx1262-kit-for-meshtastic-power-consumption/294956) |
| SX1262 | TX +22 dBm | ~118 mA | datasheet |
| L76K GNSS | Tracking | **41 mA** (standby 360 µA) | [Seeed datasheet](https://evelta.com/l76k-gnss-module-for-xiao-seeed-studio/) |
| ATGM336H GNSS | Tracking @ 1 Hz | ~38 mA measured (spec <25 mA — ⚠ gap unverified; budget 30–40) | [datasheet](https://www.tinytronics.nl/product_files/002176_ATGM336H.pdf) |
| ESP32-S3 | Light sleep | ~0.7 mA configured well (~2.9 mA sloppy) | [Qoitech measurements](https://www.qoitech.com/blog/esp32-s3-c3-c6-sleep-power-consumption/) |
| nRF52840 | Sleep / active | **2–16 µA** sleep; low-mA active | [Seeed forum](https://forum.seeedstudio.com/t/xiao-nrf52840-wio-sx1262-kit-for-meshtastic-power-consumption/294956), [RAK4631 datasheet](https://docs.rakwireless.com/product-categories/wisblock/rak4631/datasheet/) |
| SSD1306 OLED | Arrow on black | ~1.5–8 mA (scales with lit pixels) | [measured curve](https://hsel.co.uk/2018/12/04/ssd1306-power-consumption/) |
| WS2812 ring | idle quiescent | ~0.6–1 mA **per pixel** doing nothing — ⚠ bench-verify; use 8 px, MOSFET-gate, or low-idle SK6805 | [Adafruit NeoPixel guide](https://learn.adafruit.com/adafruit-neopixel-uberguide/powering-neopixels) |
| Magnetometer + accel | Continuous | <1 mA | negligible |

**GPS is the power boss; the radio is nearly free.**

### Token totals over 7 h

| Build | Avg draw | 7 h needs | 500 mAh? | 1000 mAh? |
|---|---|---|---|---|
| ESP32-S3 everything-on, 12-px ring | ~65–70 mA | ~470 mAh | ✗ marginal | ✓ |
| ESP32-S3 light-sleep + duty-RX + 8-px dim | ~40–45 mA | ~300 mAh | ✓ tight | ✓✓ |
| **nRF52840 + duty-RX + dim ring** | ~38–43 mA | ~280 mAh | **✓ comfortable** | overkill |
| nRF52840 + duty-cycled GPS | ~20–30 mA | ~150–210 mAh | ✓✓ | — |

**Verdict: 7 h on an internal 500 mAh cell is comfortable on nRF52; 1000 mAh = zero-thought on anything.** Cross-check: LoRa-APRS trackers (ESP32 + GPS + periodic TX) run 2–4 days on 4000 mAh ≈ 40–80 mA average ([M1HAX build](https://m1hax.uk/2025/12/lilygo-tracker-lora-aprs-tracker-project/)).

**The PPS trick (crib from [spoke](https://github.com/FeruzTopalov/spoke)):** both ends have GPS time. Anchor transmits on the top of each second (PPS-aligned); tokens open a ~50–100 ms RX window per second and sleep the radio otherwise. Deterministic duty-cycled RX instead of CAD guesswork.

## 2. Radio parameters (AU)

**Regulatory (verified):** ACMA LIPD Class Licence, digital-modulation item 915–928 MHz: max **1 W EIRP**, and **no duty-cycle limit in Australia** ([TTN AU thread](https://www.thethingsnetwork.org/forum/t/how-does-the-things-network-comply-with-australian-legislation/71036), [ACMA LIPD](https://www.acma.gov.au/licences/low-interference-potential-devices-lipd-class-licence)). 1 Hz beaconing at +22 dBm (158 mW) is legal with headroom. ⚠ LIPD 2025 instrument remake in progress — re-check final text before shipping; no change flagged for this band.

Time-on-air (AN1200.13; CR4/5, explicit header + CRC, 24 B payload):

| Config | ToA | Duty @ 1 Hz | Sensitivity |
|---|---|---|---|
| SF7/250 | 30.8 ms | 3.1% | ~-121 dBm |
| SF7/125 | 61.7 ms | 6.2% | ~-124 dBm |
| **SF8/250** | **56.6 ms** | **5.7%** | ~-124 dBm |
| SF9/250 | 102.9 ms | 10.3% | ~-127 dBm |
| SF10/125 | 370.7 ms | 37% | too slow @ 1 Hz |

**Locked recommendation: SF8 @ 250 kHz, CR4/5, +22 dBm, 1 Hz, 16–24 B payload** (id 1B + lat 4B + lon 4B + alt/seq 2B + batt 1B + CRC). SF7/125-class sensitivity at half the airtime; ~10 dB budget over SF7/250 to eat crowd bodies. Matches Meshtastic MediumFast-class real-world (~1–2 km urban — [radio settings](https://meshtastic.org/docs/overview/radio-settings/)). Fallback if the far fence drops: SF9/250 (still legal). Long preamble (16–24 symbols) widens token RX windows cheaply. Offset our channel from the Meshtastic AU LongFast default so a field of T1000-Es doesn't collide with us. TX energy at 1 Hz ≈ 1.9 mAh/hour — irrelevant next to GPS.

## 3. Firmware base: raw LoRa (RadioLib), NOT Meshtastic

- Stock Meshtastic cannot do 1–2 s: smart-broadcast floor is **30 s** ([position config](https://meshtastic.org/docs/configuration/radio/position/)). Mesh routing/ACKs/admin = dead weight for one-TX/N-silent-RX.
- Cribs:
  - [FeruzTopalov/spoke](https://github.com/FeruzTopalov/spoke) / [eleph](https://github.com/FeruzTopalov/eleph) — GPS-PPS-synced TX epochs, tiny payloads, radar UI.
  - [natnafu/beacon](https://github.com/natnafu/beacon) — payload `{uint8 id, float lat, float lon}`, 24-px NeoPixel ring, bearing = courseTo − heading. Literally our token, minus the RX-only split.
  - LoRa-APRS ecosystem ([richonguzman tracker](https://github.com/richonguzman/LoRa_APRS_Tracker)) — one-way position beaconing at scale, proven battery numbers.
  - [RadioLib](https://jgromes.github.io/RadioLib/class_s_x126x.html) — `startReceiveDutyCycleAuto()`, CAD, GPS-beacon examples.
- Commercial envelope check: [Totem Compass](https://totemlabs.com/products/totem-compass) does 1 s updates / 1000 m+ / 10–14 h on 2.4 GHz — our spec (1–2 s, hundreds of m, 7 h, 915 MHz) sits inside proven bounds with a better band.

## 4. Token hardware shortlist

| Board | Chips | AUD | 7 h battery | Verdict |
|---|---|---|---|---|
| **Seeed Wio Tracker L1 Lite** ← pick | nRF52840 + SX1262 + **L76K GPS onboard** | **$57.30** ([Core Electronics](https://core-electronics.com.au/seeed-wio-tracker-l1-lite-for-meshtastic.html)) | 500 mAh easy; JST + solar in | Single board, local stock, nRF52 battery. Add mag/accel + ring |
| XIAO nRF52840 + Wio-SX1262 ($10.05) + L76K | same chips, stacked | ~$35 | 500 mAh comfy | Best DIY value if happy soldering B2B combo |
| XIAO ESP32S3 + Wio-SX1262 kit + L76K | ESP32-S3 stack | ~$36 | 1000 mAh safe | Cheapest w/ kit pricing; worse battery margin |
| Heltec Wireless Tracker | ESP32-S3 + GNSS + TFT | ~$50–60 | 1000 mAh | Chunky for a token; good *prototype/debug* unit |
| RAK WisBlock (RAK4631 + RAK12500) | nRF52840 + ZOE-M8Q | ~$80+ | 500 mAh | Pricey per token; GPS stack has sleep gotchas |
| LilyGO T-Impulse wristband | STM32 + S76G | — | — | **Discontinued**, 433/868 only — dead end |

**Token display: 8-px SK6805/NeoPixel ring, dim, MOSFET-gated** (~20–40 mAh over the night). Glanceable in the dark, rave-appropriate. OLED = more precise, worse dancefloor UX. Haptic pulse-on-heading = cheap nice-to-have. Servo arrow: fragile, skip.

**Heading: tilt-compensated.** Bare QMC5883L is wrong unless held level — pair with an accelerometer (mag + accel module, or LSM303-class combo) and do tilt-compensated heading in firmware. Figure-8 calibration does not fix tilt.

## 5. Anchor hardware

**T1000-E rejected as anchor**: stock Meshtastic 30 s floor; custom firmware means LR1110 + pogo-pin friction. Keep one as a crew Meshtastic fallback node if we like.

**Anchor = same board as tokens (Wio Tracker L1 Lite), same firmware, `#ifdef BEACON` role**: raw RadioLib beacon @ 1 Hz PPS-aligned, +22 dBm. Draw ~45–60 mA → 7 h ≈ 350–450 mAh → any 1000–2000 mAh cell laughs. One firmware repo, two roles — huge simplification.

## 6. Open flags (bench before fleet-buy)

1. WS2812/SK6805 idle quiescent draw — measure, gate, or pick low-idle parts.
2. ATGM336H/L76K real tracking current (25-vs-40 mA discrepancy).
3. Measured 915 MHz body-attenuation in a dense crowd — **no quantitative public data found anywhere**; our Phase-1 crowd test is original data worth publishing back.
4. LIPD 2025 final instrument text.
