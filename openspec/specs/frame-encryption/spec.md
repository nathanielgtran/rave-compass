# frame-encryption Specification

## Purpose
AES-128-CTR confidentiality for the position frame payload (`firmware/lib/ravecore/crypto.h`). Confidentiality only: there is no authentication. This is a host-tested library; nothing in `firmware/src/main.cpp` calls it yet, and no function combines `pack`, encryption and the CRC step, so callers compose them.

## Requirements

### Requirement: AES-128 block encryption
The system SHALL implement AES-128 single-block encryption (`aes128Encrypt`) per FIPS-197.

#### Scenario: FIPS-197 known answer
- **WHEN** the block `3243f6a8885a308d313198a2e0370734` is encrypted with key `2b7e151628aed2a6abf7158809cf4f3c`
- **THEN** the output is `3925841d02dc09fbdc118597196a0b32`

### Requirement: CTR nonce block
The system SHALL build the 16-byte CTR block as the 8 salt bytes, then the 32-bit `extSeq` little-endian, then the 32-bit block counter little-endian.

#### Scenario: Nonce layout
- **WHEN** `buildCtrBlock` is called with salt `a0 a1 a2 a3 a4 a5 a6 a7`, extSeq `0x01020304` and blockCtr `0x0A0B0C0D`
- **THEN** the block is `a0 a1 a2 a3 a4 a5 a6 a7 04 03 02 01 0d 0c 0b 0a`

### Requirement: Payload encryption
The system SHALL make `encryptFrame` and `decryptFrame` XOR bytes 0 to 11 of the frame, including the id/version byte, with the first 12 bytes of the AES-128 encryption (under the crew key) of the CTR block built from the crew salt, the wire `seq` zero-extended to 32 bits, and block counter 0. The two functions SHALL perform the same operation.

#### Scenario: Round trip
- **WHEN** 12 payload bytes are passed through `encryptFrame` and then `decryptFrame` with the same key, salt and seq
- **THEN** the original 12 bytes are recovered
- **AND** after `encryptFrame` alone the bytes differ from the original

#### Scenario: Keystream definition
- **WHEN** a packed frame is encrypted with `encryptFrame`
- **THEN** each of bytes 0 to 11 equals the plaintext byte XOR the matching keystream byte computed as above

#### Scenario: Id and version byte is encrypted
- **WHEN** a packed frame with id 7 and version `V1_ENCRYPTED` (byte 0 `0x47`) and seq `0x1234` is encrypted with key bytes `00 01 02 ... 0f` and salt `a0 a1 a2 a3 a4 a5 a6 a7`
- **THEN** byte 0 becomes `0x88`

### Requirement: CRC byte is outside the cipher
The system SHALL leave byte 12 of a 13-byte frame untouched in `encryptFrame` and `decryptFrame`; the caller recomputes the CRC8 over the ciphertext bytes 0 to 11.

#### Scenario: Encrypt then recompute CRC
- **WHEN** a packed frame is encrypted and byte 12 is set to `crc8` of bytes 0 to 11
- **THEN** `unpack` returns true on the result
- **AND** calling `decryptFrame` on bytes 0 to 11 restores the plaintext bytes packed originally

#### Scenario: Byte 12 unchanged by the cipher
- **WHEN** `encryptFrame` is called on a packed 13-byte frame
- **THEN** byte 12 has the value it had before the call

### Requirement: Keystream depends on key, salt and seq
The system SHALL derive the keystream only from the crew key, the crew salt and the wire seq, so identical inputs give identical ciphertext and changing the salt or seq changes it.

#### Scenario: Same inputs, same ciphertext
- **WHEN** the same plaintext is encrypted twice with the same key, salt and seq
- **THEN** both ciphertexts are identical

#### Scenario: Different seq or salt
- **WHEN** the same plaintext is encrypted with seq N and with seq N+1, or with two salts that differ in one byte
- **THEN** the ciphertexts differ

#### Scenario: Wrong seq on decrypt
- **WHEN** a ciphertext made with seq N is decrypted with seq N+1
- **THEN** the plaintext is not recovered

### Requirement: Confidentiality without authentication
The system SHALL provide no authentication of frames: `decryptFrame` returns no error signal, and a frame decrypted under the wrong key yields different bytes rather than a failure.

#### Scenario: Wrong key
- **WHEN** a CRC-valid encrypted frame is decrypted with a different key and salt
- **THEN** `decryptFrame` completes without error
- **AND** the decoded latitude and longitude are not both equal to the original coordinates
