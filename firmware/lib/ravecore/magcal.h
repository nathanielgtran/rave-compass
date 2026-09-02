#pragma once
/*
 * magcal.h — streaming magnetometer hard-iron calibration (pure C++17, no hardware deps)
 *
 * Hard-iron distortion is a fixed offset on each axis from nearby permanent magnets
 * (battery, speaker, steel case). It shifts the mag sphere off-centre, causing a
 * constant heading error that soft-iron or tilt compensation cannot fix.
 *
 * Algorithm: min/max fit over a figure-8 motion.
 * - Collect raw samples during calibration (figure-8 sweeps all orientations).
 * - Track per-axis min and max.
 * - Offset = midpoint of (min, max) per axis — centres the sphere at the origin.
 * - isSufficient() gates commit: each axis must span >= rangeThreshold units,
 *   ensuring the figure-8 actually covered the field space.
 *
 * MagCal struct is defined in heading.h — include that rather than redefining here.
 */

#include "heading.h"  // provides MagCal struct

namespace ravecore {

/*
 * MagCalSession — stateful accumulator for hard-iron calibration.
 *
 * Usage:
 *   MagCalSession cal;
 *   // user performs figure-8 motion, app calls:
 *   while (collecting) cal.addSample(mx, my, mz);
 *   if (cal.isSufficient()) store(cal.result());
 *   else show_error();
 */
class MagCalSession {
public:
    // rangeThreshold: minimum per-axis span (raw sensor units) before isSufficient().
    // 20.0 is appropriate for typical magnetometers scaled to uT.
    explicit MagCalSession(float rangeThreshold = 20.0f);

    // Add a raw magnetometer sample.
    void addSample(float x, float y, float z);

    // True if each axis range exceeds rangeThreshold (figure-8 covered enough space).
    bool isSufficient() const;

    // Returns MagCal with offsets = midpoint of (min, max) per axis.
    // Precondition: hasData_ must be true. Caller should check isSufficient() first.
    MagCal result() const;

    // Reset to initial state (before a new calibration run).
    void reset();

private:
    float minX_, maxX_;
    float minY_, maxY_;
    float minZ_, maxZ_;
    float threshold_;
    bool  hasData_;
};

} // namespace ravecore
