#pragma once
/*
 * crypto.h — AES-128-CTR payload encryption (pure C++17, no hardware deps)
 *
 * Frame version bits live in the top 2 bits of the id byte:
 *   v0 (00) — plaintext, for bench / protocol validation
 *   v1 (01) — AES-128-CTR encrypted payload (this module)
 * The bottom 6 bits of id carry the device identifier (0-63).
 *
 * CTR nonce construction (16 bytes):
 *   salt[8] || extSeq[4, LE] || blockCtr[4, LE]
 *
 * extSeq: the wire seq field is uint16 (65535 frames = ~18 hours at 1 Hz,
 * well beyond a 7-hour rave). For this module, seq is zero-extended to uint32.
 * Full resync on seq rollover requires coordinated state reset on both sides;
 * the zero-extend approach is safe for a single-session rave where seq < 65536.
 *
 * Payload coverage:
 *   - Bytes 0..11 of the serialised frame are encrypted in-place.
 *   - Byte 12 (CRC8) is computed over the CIPHERTEXT bytes 0..11 and stays
 *     plaintext so receivers reject bad / colliding frames before decryption.
 *   - CRC != MAC. Wrong-key traffic produces garbage coordinates; an attacker
 *     cannot track the crew. Spoofing resistance is explicitly out of scope for
 *     v1 — 13-byte frames have no room for a MAC.
 */

#include <cstdint>

namespace ravecore {

struct CrewKey {
    uint8_t key[16];   // AES-128 pre-shared key (provisioned at flash time)
    uint8_t salt[8];   // per-crew fixed salt used in the CTR nonce
};

// AES-128 single-block encrypt (FIPS-197).
// Used internally for CTR keystream generation.
// key : 16-byte AES key
// in  : 16-byte plaintext block
// out : 16-byte ciphertext block (may alias in if same pointer)
void aes128Encrypt(const uint8_t key[16], const uint8_t in[16], uint8_t out[16]);

// Build the 16-byte CTR nonce block:
//   salt[8] || extSeq[4, LE] || blockCtr[4, LE]
void buildCtrBlock(uint8_t block[16], const uint8_t salt[8],
                   uint32_t extSeq, uint32_t blockCtr);

// CTR encrypt/decrypt 12-byte frame payload (CTR is symmetric — same call).
// buf12 : bytes 0..11 of the packed position frame (modified in-place)
// seq   : wire uint16 sequence counter (zero-extended to uint32 internally)
// ck    : crew key + salt
void encryptFrame(uint8_t* buf12, uint16_t seq, const CrewKey& ck);
void decryptFrame(uint8_t* buf12, uint16_t seq, const CrewKey& ck);

} // namespace ravecore
