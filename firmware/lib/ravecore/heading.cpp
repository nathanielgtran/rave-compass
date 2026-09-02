#include "heading.h"
#include <cmath>

static constexpr double RAD_TO_DEG = 180.0 / M_PI;

void applyCalibration(float& mx, float& my, float& mz, const MagCal& cal) {
    mx -= cal.offsetX;
    my -= cal.offsetY;
    mz -= cal.offsetZ;
}

/*
 * tiltCompensatedHeadingDeg
 *
 * Axis convention: x=forward, y=right, z=down (NED, gravity = +z at rest).
 *
 * Steps:
 *   1. Compute pitch and roll from accelerometer.
 *      pitch = asin(-ax / |a|)       — nose-up is negative pitch in NED
 *      roll  = atan2(ay, az)          — right-tilt is positive roll
 *   2. Rotate the magnetometer into the horizontal plane using pitch + roll.
 *      mx_h = mx*cos(pitch) + mz*sin(pitch)
 *      my_h = mx*sin(roll)*sin(pitch) + my*cos(roll) - mz*sin(roll)*cos(pitch)
 *   3. heading = atan2(-my_h, mx_h)  — standard compass convention
 *      (negative my_h: right=East=positive heading)
 *   4. Normalise to [0, 360).
 */
double tiltCompensatedHeadingDeg(float mx, float my, float mz,
                                  float ax, float ay, float az) {
    double gMag = std::sqrt((double)ax * ax + (double)ay * ay + (double)az * az);
    if (gMag < 1e-9) {
        return 0.0;  // degenerate input — return a safe default
    }

    // Normalise accel to unit vector so gravity magnitude doesn't matter.
    double axN = ax / gMag;
    double ayN = ay / gMag;
    double azN = az / gMag;

    // Clamp to [-1, 1] before asin to guard against tiny floating-point overrun.
    double sinPitch = -axN;
    if (sinPitch >  1.0) sinPitch =  1.0;
    if (sinPitch < -1.0) sinPitch = -1.0;

    double pitch = std::asin(sinPitch);
    double roll  = std::atan2(ayN, azN);

    double cosPitch = std::cos(pitch);
    double sinPitchVal = std::sin(pitch);
    double cosRoll  = std::cos(roll);
    double sinRoll  = std::sin(roll);

    // Tilt-compensated horizontal mag components.
    double mxH = (double)mx * cosPitch  + (double)mz * sinPitchVal;
    double myH = (double)mx * sinRoll * sinPitchVal
               + (double)my * cosRoll
               - (double)mz * sinRoll  * cosPitch;

    // atan2(-myH, mxH): standard formula — East is positive heading.
    double heading = std::atan2(-myH, mxH) * RAD_TO_DEG;

    if (heading < 0.0) heading += 360.0;
    return heading;
}
