#pragma once
/*
 * geo.h — geodesic helpers (pure C++17, no Arduino/hardware deps)
 *
 * bearingDeg  : flat-earth bearing from point A to point B, degrees [0, 360)
 * haversineMeters : great-circle distance in metres
 *
 * Both functions use the cos-latitude correction on the longitude delta so
 * the bearing formula is correct (without it you get ~6 deg error at
 * Sydney latitude). haversine is used for distance — overkill under 1 km
 * but trivially cheap.
 */

double bearingDeg(double lat1, double lon1, double lat2, double lon2);
double haversineMeters(double lat1, double lon1, double lat2, double lon2);
