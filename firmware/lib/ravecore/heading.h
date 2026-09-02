#pragma once
/*
 * heading.h — tilt-compensated compass heading (pure C++17, no hardware deps)
 *
 * Axis convention (NED — North-East-Down, standard for aerial/navigation):
 *   Accelerometer:  ax = forward (X), ay = right (Y), az = down (Z)
 *   Magnetometer:   mx = forward (X), my = right (Y), mz = down (Z)
 *
 * Mount the IMU so its X-axis aligns with the token's "forward" direction
 * (the pixel-0 face of the LED ring). Z-axis points down in normal
 * lanyard-hanging orientation.
 *
 * At rest (device flat, face up): ax~0, ay~0, az~1g (gravity down).
 *
 * Calibration removes hard-iron offsets collected during figure-8 motion.
 * Soft-iron correction is not implemented (deferred; gather data first).
 */

#include <cstdint>

struct MagCal {
    float offsetX;
    float offsetY;
    float offsetZ;
};

/*
 * applyCalibration — subtract hard-iron offsets from raw magnetometer readings.
 * Modifies mx, my, mz in-place.
 */
void applyCalibration(float& mx, float& my, float& mz, const MagCal& cal);

/*
 * tiltCompensatedHeadingDeg — heading of the device's X-axis relative to
 * magnetic north, in degrees [0, 360).
 *
 * mx, my, mz : calibrated magnetometer values (arbitrary units, same scale)
 * ax, ay, az : raw accelerometer values (arbitrary units, gravity direction
 *              is what matters — absolute magnitude is normalised internally)
 *
 * Returns heading in [0, 360). North = 0, East = 90.
 * Undefined (returns 0.0) if ax == ay == az == 0 (degenerate accel input).
 */
double tiltCompensatedHeadingDeg(float mx, float my, float mz,
                                  float ax, float ay, float az);
