#include "protocol.h"
#include <cstring>

// ---------------------------------------------------------------------------
// CRC8 — polynomial 0x07, initial value 0x00.
// Covers an arbitrary byte buffer. Table-free (small code, no flash table).
// ---------------------------------------------------------------------------
uint8_t crc8(const uint8_t* data, size_t len) {
    uint8_t crc = 0x00;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            crc = (crc & 0x80) ? static_cast<uint8_t>((crc << 1) ^ 0x07)
                                : static_cast<uint8_t>(crc << 1);
        }
    }
    return crc;
}

// ---------------------------------------------------------------------------
// pack — little-endian serialisation + CRC append
// ---------------------------------------------------------------------------
size_t pack(const PositionFrame& f, uint8_t* buf) {
    // Top 2 bits = version (FrameVersion value shifted left 6), bottom 6 = id.
    buf[0] = static_cast<uint8_t>((f.version & 0xC0) | (f.id & 0x3F));

    // int32 little-endian
    buf[1] = static_cast<uint8_t>(f.latE7 & 0xFF);
    buf[2] = static_cast<uint8_t>((f.latE7 >> 8)  & 0xFF);
    buf[3] = static_cast<uint8_t>((f.latE7 >> 16) & 0xFF);
    buf[4] = static_cast<uint8_t>((f.latE7 >> 24) & 0xFF);

    buf[5] = static_cast<uint8_t>(f.lonE7 & 0xFF);
    buf[6] = static_cast<uint8_t>((f.lonE7 >> 8)  & 0xFF);
    buf[7] = static_cast<uint8_t>((f.lonE7 >> 16) & 0xFF);
    buf[8] = static_cast<uint8_t>((f.lonE7 >> 24) & 0xFF);

    // uint16 little-endian
    buf[9]  = static_cast<uint8_t>(f.seq & 0xFF);
    buf[10] = static_cast<uint8_t>((f.seq >> 8) & 0xFF);

    buf[11] = f.battPct;

    // CRC over bytes 0..11
    buf[12] = crc8(buf, FRAME_LEN - 1);

    return FRAME_LEN;
}

// ---------------------------------------------------------------------------
// unpack — validate length + CRC, then deserialise
// ---------------------------------------------------------------------------
bool unpack(const uint8_t* buf, size_t len, PositionFrame& f) {
    if (len < FRAME_LEN) return false;

    uint8_t expected = crc8(buf, FRAME_LEN - 1);
    if (buf[FRAME_LEN - 1] != expected) return false;

    // Decode version from top 2 bits; device id from bottom 6 bits.
    f.version = buf[0] & 0xC0;
    f.id      = buf[0] & 0x3F;

    f.latE7 = static_cast<int32_t>(
        static_cast<uint32_t>(buf[1])
      | (static_cast<uint32_t>(buf[2]) << 8)
      | (static_cast<uint32_t>(buf[3]) << 16)
      | (static_cast<uint32_t>(buf[4]) << 24));

    f.lonE7 = static_cast<int32_t>(
        static_cast<uint32_t>(buf[5])
      | (static_cast<uint32_t>(buf[6]) << 8)
      | (static_cast<uint32_t>(buf[7]) << 16)
      | (static_cast<uint32_t>(buf[8]) << 24));

    f.seq = static_cast<uint16_t>(
        static_cast<uint16_t>(buf[9])
      | (static_cast<uint16_t>(buf[10]) << 8));

    f.battPct = buf[11];

    return true;
}
