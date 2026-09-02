# Air-protocol — frame spec and radio parameters

Two frame versions are defined. v0 is plaintext for bench and protocol validation.
v1 adds AES-128-CTR payload encryption for use at real events.

---

## Frame v0 — plaintext (bench / field validation)

**Do not use v0 at a real event.** See v1 section for encrypted frames.

### Frame layout

Total: 13 bytes, little-endian.

| Offset | Field    | Type    | Size (B) | Description                                              |
|--------|----------|---------|----------|----------------------------------------------------------|
| 0      | id\_ver  | uint8   | 1        | Top 2 bits = frame version (v0=00, v1=01). Bottom 6 bits = device identifier [0–63]. |
| 1      | latE7    | int32   | 4        | Latitude, degrees × 1e7 (decode: value / 1e7), signed little-endian (e.g. Sydney -33.8688° → -338688000) |
| 5      | lonE7    | int32   | 4        | Longitude, degrees × 1e7 (decode: value / 1e7), signed little-endian           |
| 9      | seq      | uint16  | 2        | Rolling sequence counter, little-endian; wraps at 65535  |
| 11     | battPct  | uint8   | 1        | Battery percentage [0, 100]                              |
| 12     | crc      | uint8   | 1        | CRC8 over bytes 0–11 (over PLAINTEXT for v0; over CIPHERTEXT for v1) |

### Version bits

The top 2 bits of byte 0 encode the frame version. Receivers must check this before processing:

| Top 2 bits | Version | Payload   |
|------------|---------|-----------|
| 00         | v0      | Plaintext |
| 01         | v1      | AES-128-CTR encrypted (see v1 section) |
| 10–11      | reserved | — |

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

Use frame v1 (see below) at real events.

---

## Frame v1 — AES-128-CTR encrypted

Frame v1 provides payload confidentiality. The 12-byte payload (bytes 0–11) is
encrypted with AES-128-CTR using a pre-shared crew key provisioned at flash time.
The CRC byte (byte 12) is computed over the ciphertext so receivers can reject
bad or colliding frames before attempting decryption.

### What v1 protects

**Confidentiality:** position data is encrypted; a passive observer with a LoRa
receiver cannot read the crew's location.

**What v1 does NOT provide:** authentication or spoofing resistance. The CRC is
an integrity check for radio errors, not a MAC. A wrong-key attacker sees garbage
coordinates after decryption and cannot track the crew (the actual threat model for
a one-night rave). Spoofing resistance is **explicitly out of scope for v1** —
13-byte frames have no room for a MAC. A future v2 frame could extend to 16+ bytes
for a truncated HMAC if this threat model changes.

### Encryption algorithm

AES-128-CTR (NIST SP 800-38A). Block cipher: AES-128 (FIPS-197).

### Key material

- `key[16]`: AES-128 pre-shared key (128 bits), provisioned at flash time.
  Exchange manually before the event — no cloud required.
- `salt[8]`: per-crew fixed salt included in every CTR nonce.

Together these form a `CrewKey` struct (see `firmware/lib/ravecore/crypto.h`).

### CTR nonce construction

The 16-byte CTR nonce block is:

```
salt[8] || extSeq[4, LE] || blockCtr[4, LE]
```

- `salt[8]`: crew salt from the pre-shared key struct.
- `extSeq[4]`: the wire `seq` field (uint16) zero-extended to uint32, little-endian.
  This is safe for a single rave session (seq wraps at 65535 = ~18 hours at 1 Hz,
  well beyond a 7-hour event). Full resync across a seq rollover requires coordinated
  state reset on both sides; the zero-extend approach works for seq < 65536.
- `blockCtr[4]`: AES block counter (0 for the first block). For 12-byte payloads,
  only block 0 is needed (one AES block = 16 bytes of keystream, covering 12).

### CRC rule

CRC8 (poly 0x07, init 0) is computed over the **ciphertext** bytes 0–11, then
stored in byte 12. Receivers check the CRC before decrypting. This is NOT a MAC
(CRC8 is not collision-resistant against an adversary), but it does catch radio
errors cheaply without a second key derivation step.

### Encryption flow (sender)

1. Pack the plaintext `PositionFrame` into a 13-byte buffer (pack function).
2. Set top 2 bits of buf[0] to `01` (v1 version flag).
3. Encrypt bytes 0–11 in-place with `encryptFrame(buf, seq, crewKey)`.
4. Recompute `buf[12] = crc8(buf, 12)` over the ciphertext.
5. Transmit all 13 bytes.

### Decryption flow (receiver)

1. Check `buf[12] == crc8(buf, 12)` — drop if CRC fails.
2. Check top 2 bits of buf[0] for version — drop if not v0 or v1.
3. For v1: decrypt bytes 0–11 in-place with `decryptFrame(buf, seq, crewKey)`.
   `seq` comes from the decrypted buf[9..10].
   **Bootstrapping note:** seq is needed to build the nonce, but seq is inside
   the encrypted payload. The approach: use the wire seq field from buf[9..10]
   before decryption. Since seq is XOR'd with keystream and the nonce already
   contains seq, this is self-consistent — the same seq value is used on both sides.
4. Call `unpack(buf, FRAME_LEN, frame)` to deserialise.

### Resync rule

Wire seq (uint16) is zero-extended to uint32 for the nonce. This works without
state for any session where seq stays below 65536. If a device is powered on for
multiple sessions or rebooted mid-event without incrementing the epoch, the same
nonce (salt || seq || 0) would recur — re-using a CTR nonce with the same key
breaks confidentiality. Prevention: increment the salt or epoch counter at provisioning
time for each distinct event.
