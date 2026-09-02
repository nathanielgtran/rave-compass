#pragma once
/*
 * protocol.h — rave-compass air-protocol v0/v1 (pure C++17, no hardware deps)
 *
 * Frame layout (13 bytes, little-endian):
 *   Offset  Field    Type      Size  Description
 *   0       id_ver   uint8     1     Top 2 bits = version (v0=00, v1=01).
 *                                    Bottom 6 bits = device id [0, 63].
 *   1       latE7    int32     4     Latitude  * 1e7 (degrees, signed)
 *   5       lonE7    int32     4     Longitude * 1e7 (degrees, signed)
 *   9       seq      uint16    2     Rolling sequence counter
 *   11      battPct  uint8     1     Battery percentage [0, 100]
 *   12      crc      uint8     1     CRC8 (poly 0x07, init 0) over bytes 0..11
 *
 * CRC8 covers bytes 0 to FRAME_LEN-2 (i.e. all fields except the CRC byte).
 * For v1 frames, CRC is computed over the CIPHERTEXT (bytes 0..11 after encryption)
 * so receivers can reject garbage before attempting decryption.
 *
 * Radio parameters (locked — see docs/research-single-beacon.md):
 *   Frequency  : 918.0 MHz
 *                TODO: finalise channel offset from Meshtastic AU LongFast
 *                default to avoid collisions at events running Meshtastic
 *   SF         : 8
 *   Bandwidth  : 250 kHz
 *   Coding rate: 4/5
 *   TX power   : +22 dBm
 *   Rate       : 1 Hz (PPS-aligned anchor TX)
 *   Airtime    : ~57 ms (13 B payload, explicit header, CRC)
 *   Preamble   : 16-24 symbols (TODO: bench minimum for reliable 100 ms window)
 *
 * Security note (PROMINENT):
 *   Frame v0 is PLAINTEXT for bench / protocol work only.
 *   Frame v1 uses AES-128-CTR encryption (see crypto.h). CRC != MAC — v1 provides
 *   confidentiality, not authentication. A wrong-key attacker sees garbage coordinates
 *   and cannot track the crew (the actual threat model). Spoofing resistance is
 *   explicitly out of scope for v1 — 13-byte frames have no room for a MAC.
 *   Do not use v0 at a real event.
 */

#include <cstddef>
#include <cstdint>

// ---- Radio parameters (constexpr for use in both beacon and token) --------

// TODO: finalise offset from Meshtastic AU LongFast default before fleet buy
constexpr float  RADIO_FREQ_MHZ      = 918.0f;
constexpr int    RADIO_SF            = 8;
constexpr float  RADIO_BW_KHZ        = 250.0f;
constexpr int    RADIO_CR_DENOM      = 5;   // CR = 4/5
constexpr int    RADIO_TX_DBM        = 22;
constexpr int    RADIO_RATE_HZ       = 1;
constexpr int    RADIO_PREAMBLE_SYM  = 16;  // TODO: bench min for 100 ms window

// ---- Frame version ----------------------------------------------------------

// Version encoded in top 2 bits of the on-wire id byte.
// Bottom 6 bits carry the device identifier [0, 63].
//   V0_PLAINTEXT (0x00) — plaintext payload for bench / field validation
//   V1_ENCRYPTED (0x40) — AES-128-CTR encrypted payload (see crypto.h)
enum class FrameVersion : uint8_t {
    V0_PLAINTEXT = 0x00,
    V1_ENCRYPTED = 0x40,
};

// ---- Frame ------------------------------------------------------------------

struct PositionFrame {
    uint8_t  id;        // device identifier [0, 63] — bottom 6 bits of wire byte
    int32_t  latE7;    // degrees * 1e7  (e.g. -33.8688 deg -> -338688000)
    int32_t  lonE7;    // degrees * 1e7
    uint16_t seq;
    uint8_t  battPct;  // [0, 100]
    uint8_t  version;  // FrameVersion value (top 2 bits of wire id byte); 0 = V0_PLAINTEXT
                       // Placed last so existing aggregate initialisers {id, latE7, ...}
                       // zero-initialise it by default (trailing aggregate member rule).
};

// Total serialised length including CRC byte.
constexpr size_t FRAME_LEN = 13;  // 1 + 4 + 4 + 2 + 1 + 1

/*
 * pack — serialise a PositionFrame into buf (must be >= FRAME_LEN bytes).
 * Appends CRC8 at buf[12]. Returns FRAME_LEN on success.
 */
size_t pack(const PositionFrame& frame, uint8_t* buf);

/*
 * unpack — deserialise buf into frame. Validates CRC and minimum length.
 * Returns true on success, false if buf is too short or CRC fails.
 */
bool unpack(const uint8_t* buf, size_t len, PositionFrame& frame);

// ---- Internal (exposed for testing) ----------------------------------------

uint8_t crc8(const uint8_t* data, size_t len);
