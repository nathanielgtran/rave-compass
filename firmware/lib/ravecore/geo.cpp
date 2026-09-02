#include "geo.h"
#include <cmath>

static constexpr double DEG_TO_RAD = M_PI / 180.0;
static constexpr double RAD_TO_DEG = 180.0 / M_PI;
static constexpr double EARTH_RADIUS_M = 6371000.0;

/*
 * bearingDeg — flat-earth bearing from (lat1,lon1) to (lat2,lon2).
 *
 * Uses the cos-latitude scaling on dLon so the result is correct
 * at non-equatorial latitudes (~6 deg error at Sydney without it).
 *
 * Antimeridian handling: dLon is normalised to [-180, 180] so a crossing
 * from 179.9 E to 179.9 W (0.2 deg east) gives ~90 deg, not ~270 deg.
 *
 * Returns [0, 360).
 */
double bearingDeg(double lat1, double lon1, double lat2, double lon2) {
    double dLon = lon2 - lon1;

    // Normalise dLon to [-180, 180] — handles antimeridian crossings.
    while (dLon > 180.0)  dLon -= 360.0;
    while (dLon < -180.0) dLon += 360.0;

    double midLat = (lat1 + lat2) * 0.5 * DEG_TO_RAD;
    double dLat   = (lat2 - lat1) * DEG_TO_RAD;
    double dLonRad = dLon * DEG_TO_RAD;

    double x = dLonRad * std::cos(midLat);
    double y = dLat;

    double bearing = std::atan2(x, y) * RAD_TO_DEG;

    // Normalise to [0, 360).
    if (bearing < 0.0) bearing += 360.0;
    return bearing;
}

/*
 * haversineMeters — great-circle distance in metres.
 *
 * Symmetric: haversine(A, B) == haversine(B, A).
 * Returns 0.0 for coincident points.
 */
double haversineMeters(double lat1, double lon1, double lat2, double lon2) {
    double dLat = (lat2 - lat1) * DEG_TO_RAD;
    double dLon = (lon2 - lon1) * DEG_TO_RAD;

    double a = std::sin(dLat * 0.5) * std::sin(dLat * 0.5)
             + std::cos(lat1 * DEG_TO_RAD) * std::cos(lat2 * DEG_TO_RAD)
             * std::sin(dLon * 0.5) * std::sin(dLon * 0.5);

    double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
    return EARTH_RADIUS_M * c;
}
