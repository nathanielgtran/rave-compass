#pragma once
/*
 * protocol.h — rave-compass air-protocol v0 (pure C++17, no hardware deps)
 *
 * Frame layout (13 bytes, little-endian):
 *   Offset  Field    Type      Size  Description
 *   0       id       uint8     1     Device identifier (set at flash time)
 *   1       latE7    int32     4     Latitude  * 1e7 (degrees, signed)
 *   5       lonE7    int32     4     Longitude * 1e7 (degrees, signed)
 *   9       seq      uint16    2     Rolling sequence counter
 *   11      battPct  uint8     1     Battery percentage [0, 100]
 *   12      crc      uint8     1     CRC8 (poly 0x07, init 0) over bytes 0..11
 *
 * CRC8 covers bytes 0 to FRAME_LEN-2 (i.e. all fields except the CRC byte).
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
 *   Airtime    : ~57 ms (24 B payload, explicit header, CRC)
 *   Preamble   : 16-24 symbols (TODO: bench minimum for reliable 100 ms window)
 *
 * Security note (PROMINENT):
 *   Frame v0 is PLAINTEXT for bench / protocol work only.
 *   Payload encryption (e.g. AES-128-CTR with a pre-shared key and seq as
 *   a nonce component) is DEFERRED to a follow-up PR once the raw protocol
 *   is validated in the field. Do not ship to a real event without it.
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

// ---- Frame ------------------------------------------------------------------

struct PositionFrame {
    uint8_t  id;
    int32_t  latE7;    // degrees * 1e7  (e.g. -33.8688 deg -> -338688000)
    int32_t  lonE7;    // degrees * 1e7
    uint16_t seq;
    uint8_t  battPct;  // [0, 100]
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
