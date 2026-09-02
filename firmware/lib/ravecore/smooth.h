#pragma once
/*
 * smooth.h — circular bearing EMA + pixel hysteresis (pure C++17, no hardware deps)
 *
 * Two independent concerns:
 *
 * 1. BearingSmoother: exponential moving average on circular bearing.
 *    Handles the 0/360 degree seam correctly by accumulating sin/cos components
 *    and computing atan2 to recover the smoothed angle.
 *    Without this, naive EMA of 359 and 1 gives ~180 instead of ~0.
 *
 * 2. PixelHysteresis: prevents LED pixel flicker at bin boundaries.
 *    The smoothed bearing changes continuously; without hysteresis the pixel
 *    index flips repeatedly when the bearing sits near a bin boundary.
 *    PixelHysteresis requires the bearing to move >= margin degrees PAST the
 *    boundary before committing the flip.
 *
 * Integration note:
 *   When entering DisplayMode::PROXIMITY, bearing is meaningless (GPS accuracy
 *   floor). Call BearingSmoother::reset() and PixelHysteresis::reset() in the
 *   state machine when transitioning into PROXIMITY. The ravecore layer stays
 *   pure; the main loop wires this up.
 */

#include <cstdint>

namespace ravecore {

/*
 * BearingSmoother — EMA on circular bearing via sin/cos accumulation.
 *
 * alpha in (0, 1]: higher = faster response, less smoothing.
 * alpha = 1.0 means no smoothing (raw passthrough).
 */
class BearingSmoother {
public:
    explicit BearingSmoother(float alpha = 0.2f);

    void addSample(float bearingDeg);

    // Smoothed bearing in [0, 360). Undefined if !isValid().
    float value() const;

    // True after at least one sample has been fed.
    bool isValid() const;

    void reset();

private:
    float alpha_;
    float sinAcc_;
    float cosAcc_;
    bool  valid_;
};

/*
 * PixelHysteresis — commit pixel change only when bearing crosses margin degrees
 * past the bin boundary.
 *
 * numPixels: LED ring size (e.g. 12).
 * margin   : degrees past the adjacent bin boundary required to commit (default 6.0).
 */
class PixelHysteresis {
public:
    explicit PixelHysteresis(int numPixels = 12, float margin = 6.0f);

    // Feed smoothed bearing [0, 360). Returns current (possibly hysteresis-held) pixel.
    int update(float bearingDeg);

    // Current committed pixel index (0-based). -1 if not yet initialised.
    int current() const;

    void reset();

private:
    int   numPixels_;
    float margin_;
    int   current_;
    bool  valid_;
};

} // namespace ravecore
