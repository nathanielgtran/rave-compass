/*
 * rave-compass — main.cpp
 *
 * Role scaffold: compile-time role selection via ROLE_BEACON / ROLE_TOKEN.
 * Excluded from the native test build via src_filter = -<src/> in platformio.ini.
 *
 * This file is a STUB — pseudocode comments document the intended runtime
 * behaviour. Real implementation follows Phase 1 hardware arrival.
 */

#if !defined(NATIVE_TEST)

#include <Arduino.h>

// ---------------------------------------------------------------------------
// Shared init
// ---------------------------------------------------------------------------
void setup() {
    Serial.begin(115200);
    // TODO: init RadioLib SX1262 (SF8, BW 250 kHz, CR 4/5, 918.0 MHz)
    // TODO: init L76K GNSS (1 Hz, NMEA over UART)
    // TODO: init mag + accelerometer (I2C, e.g. LSM303 or QMC5883L + MPU6050)
}

// ---------------------------------------------------------------------------
// ROLE_BEACON — anchor loop
// ---------------------------------------------------------------------------
#ifdef ROLE_BEACON

void loop() {
    // 1. Wait for PPS pulse from L76K (rising edge on PPS GPIO).
    //    This aligns the TX burst to the top of each UTC second.
    //    Crib: FeruzTopalov/spoke PPS epoch handling.

    // 2. Read current GNSS fix (lat, lon, fix quality, satellite count).
    //    If no fix, transmit a "no-fix" frame with id + seq only
    //    so tokens can track staleness.

    // 3. Pack PositionFrame (see firmware/lib/ravecore/protocol.h):
    //       id      = device id (1 byte, set at flash time)
    //       latE7   = latitude  * 1e7 (int32, little-endian)
    //       lonE7   = longitude * 1e7 (int32, little-endian)
    //       seq     = rolling 16-bit sequence counter
    //       battPct = ADC-measured battery percentage (uint8)
    //    Append CRC8 (poly 0x07).

    // 4. TX via RadioLib: radio.transmit(buf, FRAME_LEN).
    //    +22 dBm, SF8, BW 250 kHz, CR 4/5, long preamble (16-24 symbols).
    //    Typical airtime: ~57 ms — completes well before next PPS.

    // 5. Sleep radio until next PPS to save power.
    //    TX energy per burst ~1.9 mAh/h — negligible vs GPS (41 mA continuous).
}

#endif  // ROLE_BEACON

// ---------------------------------------------------------------------------
// ROLE_TOKEN — pointer loop
// ---------------------------------------------------------------------------
#ifdef ROLE_TOKEN

void loop() {
    // 1. Each second: open a ~100 ms RX window aligned to PPS.
    //    RadioLib startReceiveDutyCycleAuto() or manual timed window.
    //    Sleep radio when window closes (sub-1 mA vs 5 mA continuous RX).

    // 2. If a PositionFrame received:
    //    a. Unpack + validate CRC8 (protocol::unpack). Drop on bad CRC.
    //    b. Record msSinceLastFrame = 0 (reset staleness timer).
    //    c. Store anchor lat/lon from latE7, lonE7.

    // 3. Read own GNSS fix (lat, lon).

    // 4. Compute display mode (state::computeMode):
    //       NO_FIX     — no own GPS fix OR never heard anchor
    //       STALE      — last frame > 10 s ago
    //       PROXIMITY  — distance < 30 m (GPS accuracy floor — "look up")
    //       LIVE_ARROW — normal operation

    // 5. Compute bearing + heading:
    //    a. bearing = geo::bearingDeg(ownLat, ownLon, anchorLat, anchorLon)
    //    b. Read mag + accel raw values; apply calibration (heading::applyCalibration)
    //    c. heading = heading::tiltCompensatedHeadingDeg(mx, my, mz, ax, ay, az)
    //    d. pixel  = arrow::arrowPixel(bearing, heading, 8)

    // 6. Drive 8-pixel LED ring:
    //    LIVE_ARROW  — one pixel lit at arrow::arrowPixel index, brightness = distance band
    //    PROXIMITY   — all pixels pulse (slow breathing) — "basically here, look up"
    //    STALE       — slow single-pixel blink at last-known arrow pixel, amber
    //    NO_FIX      — all pixels off, or single-red blink

    // 7. Mag calibration mode (button long-press):
    //    figure-8 motion, collect min/max on each axis, store offsets.
    //    LED feedback: spinning pixel while collecting.
}

#endif  // ROLE_TOKEN

#endif  // !NATIVE_TEST
