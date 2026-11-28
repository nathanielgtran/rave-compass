# air-frame Specification

## Purpose
The 13-byte position frame the anchor broadcasts and tokens decode (`firmware/lib/ravecore/protocol.h`), and the shared radio parameter constants. This is a host-tested library: the device loop in `firmware/src/main.cpp` is a stub and does not yet transmit, receive or apply these parameters.

## Requirements

### Requirement: Frame layout
The system SHALL serialise a `PositionFrame` with `pack` into exactly 13 bytes and return 13: the id/version byte, `latE7` as little-endian int32, `lonE7` as little-endian int32, `seq` as little-endian uint16, `battPct`, then the CRC8 byte.

#### Scenario: Pack a position frame
- **WHEN** a frame with id 7, version 0, latE7 -338688000, lonE7 1512080000, seq 0x1234 and battPct 75 is packed
- **THEN** `pack` returns 13
- **AND** the bytes are `07 00 08 d0 eb 80 82 20 5a 34 12 4b 27`

#### Scenario: Signed coordinates round-trip
- **WHEN** a frame with negative latE7 and positive lonE7 is packed and then unpacked
- **THEN** `unpack` returns true and every field equals the original

### Requirement: Id and version byte
The system SHALL write the version into the top 2 bits and the device id into the bottom 6 bits of byte 0, masking `version` with `0xC0` and `id` with `0x3F`, and SHALL define `V0_PLAINTEXT` as `0x00` and `V1_ENCRYPTED` as `0x40`.

#### Scenario: Version and id share byte 0
- **WHEN** a frame with id 7 and version `V1_ENCRYPTED` is packed
- **THEN** byte 0 is `0x47`
- **AND** `unpack` returns id 7 and version `0x40`

#### Scenario: Id bits above 6 are masked
- **WHEN** a frame with id `0x47` and version 0 is packed
- **THEN** byte 0 is `0x07`

#### Scenario: Version bits below the top 2 are masked
- **WHEN** a frame with id 3 and version `0x01` is packed
- **THEN** byte 0 is `0x03`

### Requirement: CRC8 checksum
The system SHALL compute the frame checksum with `crc8`: polynomial `0x07`, initial value `0x00`, no reflection, no final XOR, over bytes 0 to 11, and store it in byte 12.

#### Scenario: Standard check value
- **WHEN** `crc8` is computed over the ASCII bytes `123456789`
- **THEN** the result is `0xF4`

#### Scenario: Empty input
- **WHEN** `crc8` is computed over zero bytes
- **THEN** the result is `0x00`

### Requirement: Unpack validation
The system SHALL make `unpack` return false, without modifying the output frame, when the buffer is shorter than 13 bytes or byte 12 does not equal the CRC8 of bytes 0 to 11, and SHALL ignore any bytes after the first 13.

#### Scenario: Short buffer
- **WHEN** `unpack` is given a valid frame with length 12
- **THEN** it returns false

#### Scenario: Corrupted byte
- **WHEN** one bit of byte 5 of a valid packed frame is flipped and the buffer is unpacked
- **THEN** `unpack` returns false
- **AND** the output frame is unchanged

#### Scenario: All 0xFF buffer
- **WHEN** `unpack` is given 13 bytes of `0xFF`
- **THEN** it returns false

#### Scenario: Trailing bytes ignored
- **WHEN** `unpack` is given a valid frame followed by 7 more bytes with length 20
- **THEN** it returns true and the fields are those of the first 13 bytes

### Requirement: Version reported, not enforced
The system SHALL report the top 2 bits of byte 0 in `version` on a CRC-valid frame and SHALL NOT reject a frame because of its version value.

#### Scenario: Unassigned version bits
- **WHEN** a frame packed with version `0x80` is unpacked
- **THEN** `unpack` returns true and `version` is `0x80`

### Requirement: Field values are not range-checked
The system SHALL pack and unpack field values as given and SHALL NOT validate `battPct`, latitude, longitude or `seq` ranges.

#### Scenario: Out-of-range battery percentage
- **WHEN** a frame with battPct 255 is packed and unpacked
- **THEN** `unpack` returns true and battPct is 255

#### Scenario: Extreme coordinates and sequence
- **WHEN** a frame with latE7 900000000, lonE7 -1800000000 and seq 65535 is packed and unpacked
- **THEN** `unpack` returns true and the three fields equal the originals

### Requirement: Radio parameter constants
The system SHALL expose the radio parameters as constants in `protocol.h`: `RADIO_FREQ_MHZ` 918.0, `RADIO_SF` 8, `RADIO_BW_KHZ` 250.0, `RADIO_CR_DENOM` 5 (coding rate 4/5), `RADIO_TX_DBM` 22, `RADIO_RATE_HZ` 1, `RADIO_PREAMBLE_SYM` 16, and `FRAME_LEN` 13.

#### Scenario: Constant values
- **WHEN** a build includes `protocol.h`
- **THEN** each constant has the value listed above
