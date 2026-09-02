#pragma once
/*
 * arrow.h — bearing-to-pixel mapping for the LED ring (pure C++17)
 *
 * Pixel 0 = device-forward (same direction as the IMU X-axis / compass North
 * when heading=0). Pixels count clockwise: 1=45 deg, 2=90 deg, ... for an
 * 8-pixel ring.
 *
 * relativeAngleDeg : bearing minus heading, normalised to [0, 360)
 * arrowPixel       : converts relative angle to nearest pixel index
 */

double relativeAngleDeg(double bearingToAnchorDeg, double headingDeg);

/*
 * arrowPixel — map a relative angle to the nearest LED-ring pixel.
 *
 * bearingToAnchorDeg : absolute bearing to the anchor (0=north, 90=east, ...)
 * headingDeg         : device heading (0=north, 90=east, ...)
 * numPixels          : number of pixels in the ring (typically 8)
 *
 * Returns pixel index in [0, numPixels).
 * Pixel 0 = device-forward; pixels increase clockwise.
 */
int arrowPixel(double bearingToAnchorDeg, double headingDeg, int numPixels);
