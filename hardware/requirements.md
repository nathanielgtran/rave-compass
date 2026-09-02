# Hardware requirements — token unit

Dimensioned requirements for the mechanical engineering collaborator.
All dimensions are targets / envelopes — verify against actual components
on arrival before finalising CAD. Board dimensions from Seeed wiki; verify
on arrival.

## Board of record

**Seeed Wio Tracker L1 Lite** (nRF52840 + SX1262 + L76K GPS on one board)
- Source: Core Electronics (AU stock), ~A$57.30
- Approximate PCB dimensions: ~50 mm × 23 mm (verify dims on arrival from Seeed wiki)
- Mounting: M2 standoffs or friction-fit pockets; confirm hole positions on arrival
- Connectors: JST-PH 2.0 (battery), USB-C (charging + flash), I2C header (IMU)

## Enclosure targets

| Requirement          | Target / constraint                                         |
|----------------------|-------------------------------------------------------------|
| Material             | PETG — heat-resistant (pocket/lanyard use), printable on Ender 3 |
| Outer envelope       | ~55 mm diameter puck, ~18 mm height — fits a belt pouch, passes bag check |
| Lanyard attachment   | Integrated loop or bar, minimum 4 mm slot width for standard nylon lanyard |
| Face / diffuser      | Translucent or frosted PETG over LED ring face; diffuses 8-pixel ring into glanceable glow |
| USB-C access         | Edge-recessed cutout; charging without opening enclosure    |
| Enclosure closure    | Snap-fit or 2× M2 screws; no exposed PCB or wires          |
| Label area           | Flat surface for branded adhesive label (~30 × 15 mm); no QR codes |

## GPS patch antenna keep-out

- L76K uses an onboard ceramic patch antenna.
- The antenna face must have **line-of-sight to the sky** in normal wearing orientation.
- Token hangs face-up on a lanyard: GPS patch faces up → keep the top face clear
  (no metal, no filled PETG above the antenna footprint).
- Keep-out zone: ≥5 mm clear radius around antenna footprint; no copper pour nearby.

## LED ring

- 8-pixel SK6805 / NeoPixel-compatible ring
- Approximate ring outer diameter: ~32 mm (typical 8-pixel NeoPixel ring — verify on arrival)
- Sits behind the diffuser face; ring plane parallel to the PCB
- MOSFET-gated: MCU GPIO controls power rail to ring (eliminates quiescent draw
  when ring is off; SK6805 draws ~0.6–1 mA/pixel doing nothing otherwise)
- Orientation: pixel 0 aligned to the board's "forward" marker (IMU X-axis)

## Battery

- **500 mAh LiPo**, JST-PH 2.0 connector
- Approximate cell envelope: 50 × 30 × 5 mm (varies by supplier — verify before CAD cutout)
- Mount on the opposite side of the PCB from the magnetometer; LiPo field
  interferes with magnetometer readings — **keep distance ≥ 15 mm**
- Bench-verify actual 7 h draw before committing to 500 mAh; 1000 mAh if
  measurements show margin is tight (see docs/research-single-beacon.md §1)

## IMU (magnetometer + accelerometer module)

- Separate module (e.g. LSM303-class combo or QMC5883L + MPU-6050)
- Mount away from the LiPo — **minimum 15 mm separation** from battery cell
- Axis alignment: module X-axis must align with the enclosure "forward" direction
  (same as pixel-0 of the LED ring) — mark alignment on the PCB footprint
- I2C connections to Wio Tracker header

## Anchor unit

Anchor uses the same Wio Tracker L1 Lite board with `ROLE_BEACON` firmware.
Enclosure differences:

- Larger LiPo (1000–2000 mAh) for extended TX duty
- Belt-clip or zip-tie channels for totem attachment
- Same antenna keep-out and bag-check requirements
- Recessed power switch (harder to accidentally cut TX mid-event)
