/*
 * test_main.cpp — ravecore unit tests
 *
 * Run with: pio test -e native
 * Framework: Unity (bundled with PlatformIO native test runner)
 *
 * All tests use cardinal-direction / known-geometry inputs so failures
 * point immediately to the broken formula rather than floating-point drift.
 */

#include <unity.h>
#include <cmath>
#include <cstring>

#include "../../lib/ravecore/geo.h"
#include "../../lib/ravecore/heading.h"
#include "../../lib/ravecore/arrow.h"
#include "../../lib/ravecore/protocol.h"
#include "../../lib/ravecore/state.h"
#include "../../lib/ravecore/crypto.h"
#include "../../lib/ravecore/magcal.h"
#include "../../lib/ravecore/smooth.h"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void assertNearDeg(double expected, double actual, double tol,
                           const char* msg) {
    double diff = std::fabs(actual - expected);
    // Wrap diff to [0, 180] to handle the 359/1 case.
    if (diff > 180.0) diff = 360.0 - diff;
    if (diff > tol) {
        
        TEST_FAIL_MESSAGE(msg);
    }
}

// ---------------------------------------------------------------------------
// SECTION 1: bearingDeg
// ---------------------------------------------------------------------------

void test_bearing_due_north(void) {
    double b = bearingDeg(-33.87, 151.20, -33.86, 151.20);
    assertNearDeg(0.0, b, 1.0, "bearing due north should be 0 deg");
}

void test_bearing_due_east(void) {
    double b = bearingDeg(-33.87, 151.20, -33.87, 151.21);
    assertNearDeg(90.0, b, 1.0, "bearing due east should be 90 deg");
}

void test_bearing_due_south(void) {
    double b = bearingDeg(-33.87, 151.20, -33.88, 151.20);
    assertNearDeg(180.0, b, 1.0, "bearing due south should be 180 deg");
}

void test_bearing_due_west(void) {
    double b = bearingDeg(-33.87, 151.20, -33.87, 151.19);
    assertNearDeg(270.0, b, 1.0, "bearing due west should be 270 deg");
}

void test_bearing_cos_lat_effect(void) {
    // 0.01 deg north + 0.01 deg east at Sydney (lat ~-33.87).
    // cos(-33.87 deg) ~= 0.8307, so the east component is shrunk.
    // Pure atan2(dLon, dLat) would give 45 deg. With cos-lat correction
    // atan2(0.01*0.8307, 0.01) ~ atan2(0.8307, 1) ~ 39.7 deg.
    // Spec: result should be in [35, 45) — specifically not 45.
    double b = bearingDeg(-33.87, 151.20, -33.86, 151.21);
    // Must be less than 45 (cos-lat shrinks east component)
    TEST_ASSERT_TRUE_MESSAGE(b > 35.0 && b < 45.0,
        "cos-lat: NE bearing at Sydney should be ~39-40 deg, not 45");
}

void test_bearing_antimeridian(void) {
    // 179.9 E to 179.9 W: the short path is just 0.2 deg east across the antimeridian.
    // Without normalisation, dLon = -179.9 - 179.9 = -359.8 -> ~270 (wrong).
    // With normalisation, dLon = +0.2 -> bearing ~90 (correct).
    double b = bearingDeg(0.0, 179.9, 0.0, -179.9);
    assertNearDeg(90.0, b, 5.0,
        "antimeridian crossing: 179.9E->179.9W (0.2 deg east) should give ~90 deg");
}

// ---------------------------------------------------------------------------
// SECTION 2: haversineMeters
// ---------------------------------------------------------------------------

void test_haversine_lat_step(void) {
    // 0.01 deg latitude = 1111.95 m with R = 6371000 (spherical model).
    double d = haversineMeters(-33.87, 151.20, -33.86, 151.20);
    TEST_ASSERT_TRUE_MESSAGE(std::fabs(d - 1111.95) < 1.0,
        "0.01 deg lat should be ~1111.95 m (within 1 m)");
}

void test_haversine_zero(void) {
    double d = haversineMeters(-33.87, 151.20, -33.87, 151.20);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(0.001, 0.0, d,
        "zero-distance haversine should return 0.0");
}

void test_haversine_symmetric(void) {
    double ab = haversineMeters(-33.87, 151.20, -33.86, 151.21);
    double ba = haversineMeters(-33.86, 151.21, -33.87, 151.20);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(0.001, ab, ba,
        "haversine should be symmetric");
}

// ---------------------------------------------------------------------------
// SECTION 3: relativeAngleDeg + arrowPixel
// ---------------------------------------------------------------------------

void test_relative_angle_zero(void) {
    double r = relativeAngleDeg(90.0, 90.0);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(0.001, 0.0, r,
        "bearing == heading -> relative angle 0");
}

void test_relative_angle_positive(void) {
    double r = relativeAngleDeg(90.0, 45.0);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(0.001, 45.0, r,
        "relative angle 45 deg");
}

void test_relative_angle_wraparound(void) {
    // bearing=5, heading=350 -> relative = 5 - 350 = -345 -> +15
    double r = relativeAngleDeg(5.0, 350.0);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(0.001, 15.0, r,
        "relative angle should wrap negative to positive [0,360)");
}

void test_relative_angle_always_non_negative(void) {
    // Various negative intermediate values should all normalise to [0, 360).
    for (int bearing = 0; bearing < 360; bearing += 7) {
        for (int heading = 0; heading < 360; heading += 7) {
            double r = relativeAngleDeg(static_cast<double>(bearing),
                                        static_cast<double>(heading));
            TEST_ASSERT_TRUE_MESSAGE(r >= 0.0 && r < 360.0,
                "relativeAngleDeg must always return [0, 360)");
        }
    }
}

void test_arrow_pixel_forward(void) {
    // bearing == heading -> relative 0 -> pixel 0 (device forward)
    int p = arrowPixel(45.0, 45.0, 8);
    TEST_ASSERT_EQUAL_MESSAGE(0, p, "bearing==heading -> pixel 0");
}

void test_arrow_pixel_45deg(void) {
    // relative 45 deg on 8-pixel ring -> pixel 1 (each pixel = 45 deg)
    int p = arrowPixel(90.0, 45.0, 8);
    TEST_ASSERT_EQUAL_MESSAGE(1, p, "relative 45 deg -> pixel 1");
}

void test_arrow_pixel_wraparound(void) {
    // relative angle just below 360 (e.g. 337.5 deg or more) wraps to pixel 0
    // 337.5 deg = 7.5 * 45 — midpoint of pixel-7 / pixel-0 boundary.
    // At exactly 337.5 + epsilon it rounds to pixel 0.
    // Test 350 deg -> (350 + 22.5) / 45 = 372.5/45 = 8.27 -> floor = 8 -> 8%8 = 0
    int p = arrowPixel(350.0, 0.0, 8);
    TEST_ASSERT_EQUAL_MESSAGE(0, p, "relative 350 deg on 8-px ring -> pixel 0 (wraparound)");
}

void test_arrow_pixel_all_pixels_8ring(void) {
    // All 8 pixels should be reachable on an 8-pixel ring.
    for (int px = 0; px < 8; px++) {
        double rel = px * 45.0;  // exact pixel centres
        int result = arrowPixel(rel, 0.0, 8);
        TEST_ASSERT_EQUAL_MESSAGE(px, result, "each pixel centre maps to that pixel");
    }
}

// ---------------------------------------------------------------------------
// SECTION 4: tiltCompensatedHeadingDeg + applyCalibration
// ---------------------------------------------------------------------------

void test_heading_flat_north(void) {
    // Flat device (az = 1g down, ax = ay = 0).
    // Mag pointing north: mx = 1, my = 0 -> heading = 0.
    double h = tiltCompensatedHeadingDeg(1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f);
    assertNearDeg(0.0, h, 1.0, "flat, mx=1 -> heading 0 (north)");
}

void test_heading_flat_east(void) {
    // Mag pointing east: mx = 0, my = -1 -> heading = 90.
    // (my is negative for east in NED compass convention: atan2(-(-1), 0) = 90)
    double h = tiltCompensatedHeadingDeg(0.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f);
    assertNearDeg(90.0, h, 1.0, "flat, my=-1 -> heading 90 (east)");
}

void test_heading_flat_south(void) {
    double h = tiltCompensatedHeadingDeg(-1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f);
    assertNearDeg(180.0, h, 1.0, "flat, mx=-1 -> heading 180 (south)");
}

void test_heading_flat_west(void) {
    double h = tiltCompensatedHeadingDeg(0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f);
    assertNearDeg(270.0, h, 1.0, "flat, my=+1 -> heading 270 (west)");
}

void test_heading_near_zero_accel_guarded(void) {
    // Freefall / bad sensor read: accel magnitude ~0 must hit the guard and
    // return 0.0, never NaN or a division blowup.
    double h = tiltCompensatedHeadingDeg(1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 0.0f);
    TEST_ASSERT_TRUE_MESSAGE(h == 0.0, "near-zero accel must return 0.0 (guard)");
    TEST_ASSERT_FALSE_MESSAGE(std::isnan(h), "near-zero accel must not produce NaN");
}

void test_heading_tilted_30deg_pitch_still_north(void) {
    // Construct a "true north" mag field in the world frame: mWorld = (1, 0, 0).
    // Tilt the device 30 deg nose-up (pitch = +30 deg, ax positive in NED).
    // After pitch rotation, mag components in device frame:
    //   mx_device = mWorld_x * cos(30) = cos(30) ~= 0.866
    //   mz_device = -mWorld_x * sin(30) = -0.5   (NED: pitched-up nose moves mz negative)
    // Accel under 30 deg nose-up pitch:
    //   ax = -sin(30) = -0.5  (gravity component along forward axis, negative = nose up)
    //   ay = 0
    //   az = cos(30) ~= 0.866
    //
    // The tilt-compensation code should recover heading ~0 (north).

    const float pitch_rad = static_cast<float>(30.0 * M_PI / 180.0);
    float mxD = std::cos(pitch_rad);
    float myD = 0.0f;
    float mzD = -std::sin(pitch_rad);

    float axD = -std::sin(pitch_rad);
    float ayD = 0.0f;
    float azD =  std::cos(pitch_rad);

    double h = tiltCompensatedHeadingDeg(mxD, myD, mzD, axD, ayD, azD);
    assertNearDeg(0.0, h, 2.0,
        "30-deg pitch with north mag field: tilt-comp heading should still be ~0 deg");
}

void test_heading_calibration_subtracts_offsets(void) {
    float mx = 100.0f, my = -50.0f, mz = 20.0f;
    MagCal cal{10.0f, -5.0f, 2.0f};
    applyCalibration(mx, my, mz, cal);
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, 90.0f,  mx, "cal mx");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, -45.0f, my, "cal my");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, 18.0f,  mz, "cal mz");
}

// ---------------------------------------------------------------------------
// SECTION 5: pack / unpack (protocol)
// ---------------------------------------------------------------------------

void test_protocol_roundtrip(void) {
    PositionFrame f;
    f.id      = 42;
    f.latE7   = -338688000;  // -33.8688 deg (Sydney) * 1e7
    f.lonE7   =  1512080000; //  151.208 deg
    f.seq     = 12345;
    f.battPct = 87;

    uint8_t buf[FRAME_LEN];
    size_t written = pack(f, buf);
    TEST_ASSERT_EQUAL_MESSAGE(FRAME_LEN, written, "pack should return FRAME_LEN");

    PositionFrame f2{};
    bool ok = unpack(buf, FRAME_LEN, f2);
    TEST_ASSERT_TRUE_MESSAGE(ok, "unpack of valid frame should return true");
    TEST_ASSERT_EQUAL_MESSAGE(f.id,      f2.id,      "roundtrip: id");
    TEST_ASSERT_EQUAL_MESSAGE(f.latE7,   f2.latE7,   "roundtrip: latE7");
    TEST_ASSERT_EQUAL_MESSAGE(f.lonE7,   f2.lonE7,   "roundtrip: lonE7");
    TEST_ASSERT_EQUAL_MESSAGE(f.seq,     f2.seq,     "roundtrip: seq");
    TEST_ASSERT_EQUAL_MESSAGE(f.battPct, f2.battPct, "roundtrip: battPct");
}

void test_protocol_negative_lat_encoding(void) {
    // -33.8688 * 1e7 must encode as -338688000 exactly.
    int32_t expected = -338688000;
    PositionFrame f{1, expected, 1512080000, 0, 100};

    uint8_t buf[FRAME_LEN];
    pack(f, buf);

    PositionFrame f2{};
    unpack(buf, FRAME_LEN, f2);
    TEST_ASSERT_EQUAL_INT32_MESSAGE(expected, f2.latE7,
        "negative latitude must survive pack/unpack without sign corruption");
}

void test_protocol_corrupt_byte_rejected(void) {
    PositionFrame f{1, -338688000, 1512080000, 1, 90};
    uint8_t buf[FRAME_LEN];
    pack(f, buf);

    // Corrupt one data byte.
    buf[3] ^= 0xFF;

    PositionFrame f2{};
    bool ok = unpack(buf, FRAME_LEN, f2);
    TEST_ASSERT_FALSE_MESSAGE(ok, "corrupted byte should cause unpack to return false");
}

void test_protocol_short_buffer_rejected(void) {
    PositionFrame f{1, -338688000, 1512080000, 1, 90};
    uint8_t buf[FRAME_LEN];
    pack(f, buf);

    PositionFrame f2{};
    bool ok = unpack(buf, FRAME_LEN - 1, f2);
    TEST_ASSERT_FALSE_MESSAGE(ok, "short buffer should cause unpack to return false");
}

void test_protocol_all_ff_buffer_rejected(void) {
    // Plausible OTA garbage: a buffer of all 0xFF. CRC8(12 x 0xFF) = 0x71,
    // which does not equal the 0xFF in the CRC slot, so unpack must reject.
    uint8_t buf[FRAME_LEN];
    std::memset(buf, 0xFF, sizeof(buf));

    PositionFrame f{};
    bool ok = unpack(buf, FRAME_LEN, f);
    TEST_ASSERT_FALSE_MESSAGE(ok, "all-0xFF buffer must fail CRC and be rejected");
}

void test_protocol_lat_e7_exact(void) {
    // Verify the int32 encoding: -33.8688 * 1e7 = -338688000.
    // This is a compile-time sanity check on the value used in other tests.
    double raw = -33.8688 * 1e7;
    int32_t encoded = static_cast<int32_t>(raw);
    TEST_ASSERT_EQUAL_INT32_MESSAGE(-338688000, encoded,
        "latE7 encoding: -33.8688 deg * 1e7 should equal -338688000");
}

// ---------------------------------------------------------------------------
// SECTION 6: computeMode (state machine)
// ---------------------------------------------------------------------------

void test_state_live_arrow(void) {
    DisplayMode m = computeMode(50.0, 1000, true, true);
    TEST_ASSERT_EQUAL_MESSAGE(static_cast<int>(DisplayMode::LIVE_ARROW),
                              static_cast<int>(m),
                              "dist=50m, fresh, both fixes -> LIVE_ARROW");
}

void test_state_proximity(void) {
    DisplayMode m = computeMode(20.0, 1000, true, true);
    TEST_ASSERT_EQUAL_MESSAGE(static_cast<int>(DisplayMode::PROXIMITY),
                              static_cast<int>(m),
                              "dist=20m, fresh -> PROXIMITY");
}

void test_state_proximity_boundary_inclusive(void) {
    // Exactly PROXIMITY_M (30.0) -> PROXIMITY (inclusive boundary).
    DisplayMode m = computeMode(30.0, 1000, true, true);
    TEST_ASSERT_EQUAL_MESSAGE(static_cast<int>(DisplayMode::PROXIMITY),
                              static_cast<int>(m),
                              "dist==30.0m (exactly PROXIMITY_M) -> PROXIMITY (inclusive)");
}

void test_state_just_outside_proximity(void) {
    // 30.001 m -> LIVE_ARROW (exclusive: > PROXIMITY_M).
    DisplayMode m = computeMode(30.001, 1000, true, true);
    TEST_ASSERT_EQUAL_MESSAGE(static_cast<int>(DisplayMode::LIVE_ARROW),
                              static_cast<int>(m),
                              "dist=30.001m -> LIVE_ARROW (just outside proximity)");
}

void test_state_stale(void) {
    // msSinceLastFrame = 10001 (strictly > STALE_MS=10000).
    DisplayMode m = computeMode(50.0, 10001, true, true);
    TEST_ASSERT_EQUAL_MESSAGE(static_cast<int>(DisplayMode::STALE),
                              static_cast<int>(m),
                              "10001 ms since frame -> STALE");
}

void test_state_exactly_stale_ms_is_live(void) {
    // Exactly STALE_MS (10000) -> NOT stale. Boundary: strictly > triggers STALE.
    DisplayMode m = computeMode(50.0, 10000, true, true);
    TEST_ASSERT_EQUAL_MESSAGE(static_cast<int>(DisplayMode::LIVE_ARROW),
                              static_cast<int>(m),
                              "exactly 10000 ms -> LIVE_ARROW (not yet stale)");
}

void test_state_no_fix_no_own_gps(void) {
    DisplayMode m = computeMode(50.0, 1000, false, true);
    TEST_ASSERT_EQUAL_MESSAGE(static_cast<int>(DisplayMode::NO_FIX),
                              static_cast<int>(m),
                              "no own GPS fix -> NO_FIX");
}

void test_state_no_fix_never_heard_anchor(void) {
    DisplayMode m = computeMode(50.0, 0, true, false);
    TEST_ASSERT_EQUAL_MESSAGE(static_cast<int>(DisplayMode::NO_FIX),
                              static_cast<int>(m),
                              "never heard anchor -> NO_FIX");
}

void test_state_stale_beats_proximity(void) {
    // close AND stale -> STALE (not PROXIMITY).
    // Degrade loudly: stale data should not be shown as an accurate position.
    DisplayMode m = computeMode(20.0, 15000, true, true);
    TEST_ASSERT_EQUAL_MESSAGE(static_cast<int>(DisplayMode::STALE),
                              static_cast<int>(m),
                              "dist=20m (proximity) but stale -> STALE beats PROXIMITY");
}

void test_state_no_fix_beats_stale(void) {
    // stale AND no own fix -> NO_FIX (not STALE).
    DisplayMode m = computeMode(50.0, 20000, false, true);
    TEST_ASSERT_EQUAL_MESSAGE(static_cast<int>(DisplayMode::NO_FIX),
                              static_cast<int>(m),
                              "no own fix + stale -> NO_FIX beats STALE");
}

// ---------------------------------------------------------------------------
// SECTION 7: AES-128 / crypto
// ---------------------------------------------------------------------------

void test_aes128_fips197_vector(void) {
    // FIPS-197 Appendix B known-answer test (verified against OpenSSL).
    // key       = 2b7e151628aed2a6abf7158809cf4f3c
    // plaintext = 3243f6a8885a308d313198a2e0370734
    // expected  = 3925841d02dc09fbdc118597196a0b32
    const uint8_t key[16] = {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c,
    };
    const uint8_t pt[16] = {
        0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d,
        0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34,
    };
    const uint8_t expected[16] = {
        0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc, 0x09, 0xfb,
        0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32,
    };
    uint8_t ct[16] = {};
    ravecore::aes128Encrypt(key, pt, ct);
    TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(expected, ct, 16,
        "AES-128 FIPS-197 Appendix B known-answer: ciphertext mismatch");
}

void test_ctr_roundtrip(void) {
    // Encrypt 12 bytes then decrypt — must recover original.
    const uint8_t original[12] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
        0xde, 0xad, 0xbe, 0xef,
    };
    ravecore::CrewKey ck{};
    ck.key[0] = 0xca; ck.key[1] = 0xfe; ck.key[15] = 0xba;
    ck.salt[0] = 0x11; ck.salt[7] = 0x22;
    uint16_t seq = 100;

    uint8_t buf[12];
    std::memcpy(buf, original, 12);

    ravecore::encryptFrame(buf, seq, ck);
    // Must differ from original after encryption
    TEST_ASSERT_FALSE_MESSAGE(
        std::memcmp(buf, original, 12) == 0,
        "encrypted frame must differ from plaintext");

    ravecore::decryptFrame(buf, seq, ck);
    TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(original, buf, 12,
        "CTR roundtrip: decrypt must recover original plaintext");
}

void test_encrypted_frame_roundtrip(void) {
    // Pack a frame with Sydney coords, encrypt, decrypt, unpack — verify coords.
    PositionFrame orig{};
    orig.id      = 7;
    orig.version = static_cast<uint8_t>(FrameVersion::V1_ENCRYPTED);
    orig.latE7   = -338688000;   // -33.8688 deg (Sydney)
    orig.lonE7   =  1512080000;  // 151.208 deg
    orig.seq     = 500;
    orig.battPct = 75;

    ravecore::CrewKey ck{};
    ck.key[0] = 0xAB; ck.key[7] = 0xCD; ck.key[15] = 0xEF;
    ck.salt[0] = 0x55; ck.salt[3] = 0xAA;

    uint8_t wire[FRAME_LEN];
    pack(orig, wire);

    // Encrypt bytes 0..11 in-place, then recompute CRC over ciphertext.
    ravecore::encryptFrame(wire, orig.seq, ck);
    wire[12] = crc8(wire, FRAME_LEN - 1);

    // CRC over ciphertext must be valid before decryption.
    TEST_ASSERT_EQUAL_MESSAGE(crc8(wire, FRAME_LEN - 1), wire[12],
        "CRC over ciphertext must be consistent");

    // Receiver: validate CRC, then decrypt.
    uint8_t rx[FRAME_LEN];
    std::memcpy(rx, wire, FRAME_LEN);
    TEST_ASSERT_EQUAL_MESSAGE(crc8(rx, FRAME_LEN - 1), rx[12],
        "receiver must accept CRC-valid ciphertext frame");

    ravecore::decryptFrame(rx, orig.seq, ck);

    PositionFrame decoded{};
    // CRC was over ciphertext; unpack from plaintext (no CRC append).
    // Manually extract fields after decryption.
    decoded.version = rx[0] & 0xC0;
    decoded.id      = rx[0] & 0x3F;
    decoded.latE7 = static_cast<int32_t>(
        static_cast<uint32_t>(rx[1]) | (static_cast<uint32_t>(rx[2]) << 8)
      | (static_cast<uint32_t>(rx[3]) << 16) | (static_cast<uint32_t>(rx[4]) << 24));
    decoded.lonE7 = static_cast<int32_t>(
        static_cast<uint32_t>(rx[5]) | (static_cast<uint32_t>(rx[6]) << 8)
      | (static_cast<uint32_t>(rx[7]) << 16) | (static_cast<uint32_t>(rx[8]) << 24));
    decoded.seq     = static_cast<uint16_t>(rx[9] | (rx[10] << 8));
    decoded.battPct = rx[11];

    TEST_ASSERT_EQUAL_MESSAGE(orig.id,      decoded.id,      "encrypted roundtrip: id");
    TEST_ASSERT_EQUAL_MESSAGE(orig.latE7,   decoded.latE7,   "encrypted roundtrip: latE7");
    TEST_ASSERT_EQUAL_MESSAGE(orig.lonE7,   decoded.lonE7,   "encrypted roundtrip: lonE7");
    TEST_ASSERT_EQUAL_MESSAGE(orig.seq,     decoded.seq,     "encrypted roundtrip: seq");
    TEST_ASSERT_EQUAL_MESSAGE(orig.battPct, decoded.battPct, "encrypted roundtrip: battPct");
    TEST_ASSERT_EQUAL_MESSAGE(
        static_cast<uint8_t>(FrameVersion::V1_ENCRYPTED),
        decoded.version, "encrypted roundtrip: version bits");
}

void test_wrong_key_invalid_coords(void) {
    // CRC != MAC: a wrong-key attacker cannot track the crew.
    // The frame CRC is over ciphertext, so a correctly-keyed CRC check passes
    // even for a wrong-key receiver. The attacker sees garbage coordinates after
    // decryption with the wrong key — they cannot track the crew.
    // (Spoofing resistance is out of scope for v1 — no room for a MAC in 13 bytes.)
    PositionFrame orig{};
    orig.id      = 3;
    orig.latE7   = -338688000;
    orig.lonE7   =  1512080000;
    orig.seq     = 42;
    orig.battPct = 90;

    ravecore::CrewKey correctKey{};
    correctKey.key[0] = 0xDE; correctKey.key[15] = 0xAD;
    correctKey.salt[0] = 0xBE; correctKey.salt[7] = 0xEF;

    ravecore::CrewKey wrongKey{};
    wrongKey.key[0] = 0xFF; wrongKey.key[15] = 0x00;
    wrongKey.salt[0] = 0x12; wrongKey.salt[7] = 0x34;

    uint8_t wire[FRAME_LEN];
    pack(orig, wire);
    ravecore::encryptFrame(wire, orig.seq, correctKey);
    wire[12] = crc8(wire, FRAME_LEN - 1);

    // Decrypt with wrong key — garbage output.
    uint8_t rx[FRAME_LEN];
    std::memcpy(rx, wire, FRAME_LEN);
    ravecore::decryptFrame(rx, orig.seq, wrongKey);

    int32_t gotLat = static_cast<int32_t>(
        static_cast<uint32_t>(rx[1]) | (static_cast<uint32_t>(rx[2]) << 8)
      | (static_cast<uint32_t>(rx[3]) << 16) | (static_cast<uint32_t>(rx[4]) << 24));
    int32_t gotLon = static_cast<int32_t>(
        static_cast<uint32_t>(rx[5]) | (static_cast<uint32_t>(rx[6]) << 8)
      | (static_cast<uint32_t>(rx[7]) << 16) | (static_cast<uint32_t>(rx[8]) << 24));

    // Wrong-key output is not the original coordinates.
    TEST_ASSERT_FALSE_MESSAGE(
        gotLat == orig.latE7 && gotLon == orig.lonE7,
        "wrong key must produce garbage coordinates, not the original");
}

// ---------------------------------------------------------------------------
// SECTION 8: Magnetometer hard-iron calibration
// ---------------------------------------------------------------------------

// Helper: add synthetic figure-8 samples with a known hard-iron offset.
// The figure-8 spans a sphere of radius r centred at (offsetX, offsetY, offsetZ).
static void addFigureEightWithOffset(ravecore::MagCalSession& cal,
                                     float offsetX, float offsetY, float offsetZ,
                                     float r, int steps) {
    for (int i = 0; i < steps; i++) {
        float t = static_cast<float>(i) / static_cast<float>(steps) * 2.0f * static_cast<float>(M_PI);
        cal.addSample(offsetX + r * std::cos(t),
                      offsetY + r * std::sin(t),
                      offsetZ + r * std::sin(2.0f * t));
    }
}

void test_magcal_recovers_offset(void) {
    // Synthetic hard-iron: +50 uT on all axes. Radius 60 so range ~120 > threshold.
    ravecore::MagCalSession cal;
    const float OX = 50.0f, OY = 50.0f, OZ = 50.0f, R = 60.0f;
    addFigureEightWithOffset(cal, OX, OY, OZ, R, 360);

    TEST_ASSERT_TRUE_MESSAGE(cal.isSufficient(), "figure-8 coverage should be sufficient");

    MagCal result = cal.result();
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(1.0f, OX, result.offsetX, "recovered offsetX within 1 uT");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(1.0f, OY, result.offsetY, "recovered offsetY within 1 uT");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(1.0f, OZ, result.offsetZ, "recovered offsetZ within 1 uT");
}

void test_magcal_insufficient_coverage(void) {
    // Only vary Z — X and Y ranges stay below threshold (20.0 default).
    // No figure-8 on X/Y: isSufficient() must return false.
    ravecore::MagCalSession cal;
    for (int i = 0; i < 100; i++) {
        float t = static_cast<float>(i) / 100.0f * 2.0f * static_cast<float>(M_PI);
        cal.addSample(5.0f, 3.0f, 60.0f * std::sin(t));  // X/Y barely vary
    }
    TEST_ASSERT_FALSE_MESSAGE(cal.isSufficient(),
        "samples covering only Z axis should not be sufficient");
}

// ---------------------------------------------------------------------------
// SECTION 9: BearingSmoother + PixelHysteresis
// ---------------------------------------------------------------------------

void test_bearing_smoother_seam(void) {
    // Feed bearings straddling 0/360 seam: 350, 355, 0, 5, 10.
    // Correct circular average is near 0/360, NOT near 180.
    ravecore::BearingSmoother sm(0.5f);
    sm.addSample(350.0f);
    sm.addSample(355.0f);
    sm.addSample(0.0f);
    sm.addSample(5.0f);
    sm.addSample(10.0f);

    float v = sm.value();
    // Must be near 0 (within 30 deg), not near 180.
    bool nearZero = (v < 30.0f) || (v > 330.0f);
    TEST_ASSERT_TRUE_MESSAGE(nearZero,
        "circular smoother: average of 350,355,0,5,10 must be near 0, not 180");
}

void test_bearing_smoother_convergence(void) {
    // Step from 0 to 90 deg; with alpha=0.3 the smoother should
    // reach within 10 deg of 90 within 20 samples.
    ravecore::BearingSmoother sm(0.3f);
    sm.addSample(0.0f);  // prime with one sample at 0
    for (int i = 0; i < 20; i++) {
        sm.addSample(90.0f);
    }
    float v = sm.value();
    TEST_ASSERT_TRUE_MESSAGE(v > 80.0f && v < 100.0f,
        "smoother should converge within 10 deg of 90 after 20 samples (alpha=0.3)");
}

void test_pixel_hysteresis_flicker(void) {
    // Bearing oscillates +/-5 deg around the boundary between pixel 0 and 1
    // on a 12-pixel ring (bin = 30 deg, boundary at 15 deg).
    // With margin=6 the pixel should not flicker; without it, it would.
    ravecore::PixelHysteresis hy(12, 6.0f);

    // First reading: squarely in pixel 0 territory (bearing = 5 deg).
    hy.update(5.0f);
    TEST_ASSERT_EQUAL_MESSAGE(0, hy.current(), "initial bearing 5 deg -> pixel 0");

    int changes = 0;
    int prev = hy.current();
    // Oscillate between 10 and 20 deg (straddles the 15 deg boundary).
    for (int i = 0; i < 20; i++) {
        float bearing = (i % 2 == 0) ? 10.0f : 20.0f;
        int p = hy.update(bearing);
        if (p != prev) { changes++; prev = p; }
    }
    // With 6 deg margin, oscillation of +/-5 deg around the boundary
    // should cause at most 1 committed change (settling into pixel 1).
    TEST_ASSERT_TRUE_MESSAGE(changes <= 1,
        "pixel hysteresis: oscillation at bin boundary should cause at most 1 pixel change");
}

// ---------------------------------------------------------------------------
// Unity boilerplate
// ---------------------------------------------------------------------------

int main(void) {
    UNITY_BEGIN();

    // bearing
    RUN_TEST(test_bearing_due_north);
    RUN_TEST(test_bearing_due_east);
    RUN_TEST(test_bearing_due_south);
    RUN_TEST(test_bearing_due_west);
    RUN_TEST(test_bearing_cos_lat_effect);
    RUN_TEST(test_bearing_antimeridian);

    // haversine
    RUN_TEST(test_haversine_lat_step);
    RUN_TEST(test_haversine_zero);
    RUN_TEST(test_haversine_symmetric);

    // arrow
    RUN_TEST(test_relative_angle_zero);
    RUN_TEST(test_relative_angle_positive);
    RUN_TEST(test_relative_angle_wraparound);
    RUN_TEST(test_relative_angle_always_non_negative);
    RUN_TEST(test_arrow_pixel_forward);
    RUN_TEST(test_arrow_pixel_45deg);
    RUN_TEST(test_arrow_pixel_wraparound);
    RUN_TEST(test_arrow_pixel_all_pixels_8ring);

    // heading
    RUN_TEST(test_heading_flat_north);
    RUN_TEST(test_heading_flat_east);
    RUN_TEST(test_heading_flat_south);
    RUN_TEST(test_heading_flat_west);
    RUN_TEST(test_heading_tilted_30deg_pitch_still_north);
    RUN_TEST(test_heading_calibration_subtracts_offsets);

    // protocol
    RUN_TEST(test_protocol_roundtrip);
    RUN_TEST(test_protocol_negative_lat_encoding);
    RUN_TEST(test_protocol_corrupt_byte_rejected);
    RUN_TEST(test_protocol_short_buffer_rejected);
    RUN_TEST(test_protocol_lat_e7_exact);
    RUN_TEST(test_protocol_all_ff_buffer_rejected);

    RUN_TEST(test_heading_near_zero_accel_guarded);

    // state machine
    RUN_TEST(test_state_live_arrow);
    RUN_TEST(test_state_proximity);
    RUN_TEST(test_state_proximity_boundary_inclusive);
    RUN_TEST(test_state_just_outside_proximity);
    RUN_TEST(test_state_stale);
    RUN_TEST(test_state_exactly_stale_ms_is_live);
    RUN_TEST(test_state_no_fix_no_own_gps);
    RUN_TEST(test_state_no_fix_never_heard_anchor);
    RUN_TEST(test_state_stale_beats_proximity);
    RUN_TEST(test_state_no_fix_beats_stale);

    // crypto / AES-128-CTR
    RUN_TEST(test_aes128_fips197_vector);
    RUN_TEST(test_ctr_roundtrip);
    RUN_TEST(test_encrypted_frame_roundtrip);
    RUN_TEST(test_wrong_key_invalid_coords);

    // mag calibration
    RUN_TEST(test_magcal_recovers_offset);
    RUN_TEST(test_magcal_insufficient_coverage);

    // bearing smoother + pixel hysteresis
    RUN_TEST(test_bearing_smoother_seam);
    RUN_TEST(test_bearing_smoother_convergence);
    RUN_TEST(test_pixel_hysteresis_flicker);

    return UNITY_END();
}
