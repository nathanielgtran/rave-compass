#include "smooth.h"
#include <cmath>

namespace ravecore {

// ---------------------------------------------------------------------------
// BearingSmoother
// ---------------------------------------------------------------------------

BearingSmoother::BearingSmoother(float alpha)
    : alpha_(alpha), sinAcc_(0.0f), cosAcc_(0.0f), valid_(false)
{}

void BearingSmoother::addSample(float bearingDeg) {
    static constexpr float DEG_TO_RAD = static_cast<float>(M_PI) / 180.0f;
    float rad = bearingDeg * DEG_TO_RAD;
    float s = std::sin(rad);
    float c = std::cos(rad);

    if (!valid_) {
        sinAcc_ = s;
        cosAcc_ = c;
        valid_  = true;
        return;
    }

    sinAcc_ = (1.0f - alpha_) * sinAcc_ + alpha_ * s;
    cosAcc_ = (1.0f - alpha_) * cosAcc_ + alpha_ * c;
}

float BearingSmoother::value() const {
    static constexpr float RAD_TO_DEG = 180.0f / static_cast<float>(M_PI);
    float deg = std::atan2(sinAcc_, cosAcc_) * RAD_TO_DEG;
    if (deg < 0.0f) deg += 360.0f;
    return deg;
}

bool BearingSmoother::isValid() const {
    return valid_;
}

void BearingSmoother::reset() {
    sinAcc_ = 0.0f;
    cosAcc_ = 0.0f;
    valid_  = false;
}

// ---------------------------------------------------------------------------
// PixelHysteresis
// ---------------------------------------------------------------------------

PixelHysteresis::PixelHysteresis(int numPixels, float margin)
    : numPixels_(numPixels), margin_(margin), current_(0), valid_(false)
{}

/*
 * update — determine current pixel with hysteresis.
 *
 * Each pixel covers (360 / numPixels) degrees, centred on pixel_index * binSize.
 * We only commit to a new pixel when the bearing has moved >= margin degrees
 * past the boundary between the current pixel and the candidate pixel.
 *
 * Boundary between pixel P and P+1 is at: (P + 1) * binSize - binSize/2
 * = (P + 0.5) * binSize in unwrapped space.
 *
 * Implementation: compute the candidate pixel from raw bearing; commit only
 * if the bearing has cleared the current pixel's boundary by >= margin degrees.
 */
int PixelHysteresis::update(float bearingDeg) {
    float binSize = 360.0f / static_cast<float>(numPixels_);

    // Candidate pixel from raw bearing (same formula as arrowPixel but float)
    float shifted = bearingDeg + binSize * 0.5f;
    if (shifted >= 360.0f) shifted -= 360.0f;
    if (shifted < 0.0f)    shifted += 360.0f;
    int candidate = static_cast<int>(shifted / binSize) % numPixels_;

    if (!valid_) {
        current_ = candidate;
        valid_   = true;
        return current_;
    }

    if (candidate == current_) {
        return current_;
    }

    // Compute how far into the candidate's bin the bearing has moved.
    // The boundary between current_ and candidate is at:
    //   boundaryDeg = (candidate * binSize) - binSize/2  (unwrapped)
    // We compute the angular distance from that boundary to bearingDeg.
    float candidateCentre = static_cast<float>(candidate) * binSize;
    float boundary = candidateCentre - binSize * 0.5f;
    if (boundary < 0.0f) boundary += 360.0f;

    // Signed angular distance from boundary to bearingDeg (on the candidate side).
    float diff = bearingDeg - boundary;
    // Normalise to (-180, 180]
    while (diff >  180.0f) diff -= 360.0f;
    while (diff < -180.0f) diff += 360.0f;

    // Candidate is "forward" if diff > 0 (bearing is past the boundary in candidate direction)
    if (std::fabs(diff) >= margin_) {
        current_ = candidate;
    }

    return current_;
}

int PixelHysteresis::current() const {
    return valid_ ? current_ : -1;
}

void PixelHysteresis::reset() {
    current_ = 0;
    valid_   = false;
}

} // namespace ravecore
