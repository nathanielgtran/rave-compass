#include "arrow.h"
#include <cmath>

double relativeAngleDeg(double bearingToAnchorDeg, double headingDeg) {
    double rel = bearingToAnchorDeg - headingDeg;

    // Normalise to [0, 360).
    rel = std::fmod(rel, 360.0);
    if (rel < 0.0) rel += 360.0;
    return rel;
}

int arrowPixel(double bearingToAnchorDeg, double headingDeg, int numPixels) {
    if (numPixels <= 0) return 0;

    double rel = relativeAngleDeg(bearingToAnchorDeg, headingDeg);

    // Each pixel covers (360 / numPixels) degrees.
    // Add half a pixel-width before dividing so we get nearest-integer
    // rounding rather than floor.
    double pixelWidth = 360.0 / numPixels;
    int pixel = static_cast<int>(std::floor((rel + pixelWidth * 0.5) / pixelWidth));

    // Wrap: 360 deg lands on pixel 0 after the shift.
    return pixel % numPixels;
}
