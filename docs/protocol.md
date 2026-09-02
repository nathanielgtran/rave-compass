# Air-protocol v0 — frame spec and radio parameters

Protocol version 0 is plaintext for bench and protocol validation work.
See the prominent security note below before using at a real event.

## Frame layout

Total: 13 bytes, little-endian.

| Offset | Field    | Type    | Size (B) | Description                                              |
|--------|----------|---------|----------|----------------------------------------------------------|
| 0      | id       | uint8   | 1        | Device identifier set at flash time (anchor only; 0 = unset) |
| 1      | latE7    | int32   | 4        | Latitude, degrees × 1e7 (decode: value / 1e7), signed little-endian (e.g. Sydney -33.8688° → -338688000) |
| 5      | lonE7    | int32   | 4        | Longitude, degrees × 1e7 (decode: value / 1e7), signed little-endian           |
| 9      | seq      | uint16  | 2        | Rolling sequence counter, little-endian; wraps at 65535  |
| 11     | battPct  | uint8   | 1        | Battery percentage [0, 100]                              |
| 12     | crc      | uint8   | 1        | CRC8 checksum over bytes 0–11                            |

### latE7 / lonE7 encoding example

```
-33.8688 deg  x  1e7  =  -338688000  (int32, little-endian bytes: 00 08 D0 EB)
151.208  deg  x  1e7  =  1512080000  (int32, little-endian bytes: 80 82 20 5A)
```

## CRC8 specification

- Polynomial : 0x07 (CRC-8/SMBUS)
- Initial value : 0x00
- Input/output reflection : none (straight feed)
- Covers : bytes 0 to 11 (all fields except the CRC byte itself)
- Table-free bit-loop implementation (see `firmware/lib/ravecore/protocol.cpp`)

## Radio parameters

| Parameter     | Value      | Notes                                                                 |
|---------------|------------|-----------------------------------------------------------------------|
| Frequency     | 918.0 MHz  | TODO: finalise offset from Meshtastic AU LongFast default to avoid collisions |
| Spreading factor | SF8     | ~-124 dBm sensitivity; balances range and airtime                    |
| Bandwidth     | 250 kHz    | Wider BW = lower sensitivity but half the airtime of 125 kHz         |
| Coding rate   | 4/5        |                                                                       |
| TX power      | +22 dBm    | 158 mW, legal under ACMA LIPD class licence (1 W EIRP max)           |
| Rate          | 1 Hz       | PPS-aligned anchor TX                                                 |
| Airtime       | ~57 ms     | 13 B payload, explicit header, CRC, SF8/250/CR4-5                    |
| Preamble      | 16–24 sym  | TODO: bench minimum for reliable 100 ms token RX window              |

## PPS timing

The anchor synchronises its TX burst to the top of each UTC second using the
L76K GPS PPS pulse (rising edge). This makes the TX deterministic so tokens
can duty-cycle their radio.

```
Anchor:  [--- GNSS fix + seq pack (< 1 ms) ---][TX ~57 ms][ radio sleep ][ next PPS ]
Token :  [ radio sleep ][ open 100 ms RX window ][ decode + display ][ radio sleep ]
```

- Token opens a ~100 ms RX window each second, aligned to the expected PPS epoch.
- Radio sleeps otherwise — average RX current drops from ~5 mA (continuous) to
  sub-1 mA (duty-cycled).
- Long preamble (16–24 symbols) gives the token time to lock on even if the RX
  window opens slightly before the TX preamble finishes.
- TODO: bench minimum preamble length at which the token reliably decodes
  across temperature and clock-drift margins.

## Security note (prominent)

**Frame v0 is PLAINTEXT. Do not use at a real event without adding encryption.**

The position data (lat/lon of the anchor person or crew totem) is private.
Adding payload encryption is straightforward — recommended approach:

- AES-128-CTR with a pre-shared key flashed at provisioning time.
- Nonce = `{id, seq}` (device id + sequence counter) — guarantees nonce
  uniqueness for the session lifetime (65535 seconds = ~18 hours at 1 Hz,
  well beyond a 7 h rave).
- Key exchange: manually via provisioning tool before the event; no cloud
  required.

Deferred to a follow-up PR once the raw protocol is validated in the field.
