# bearing-and-distance Specification

## Purpose
Geometry from the token's position and heading to the LED-ring pixel that points at the anchor (`firmware/lib/ravecore/geo.h`, `arrow.h`). Pure functions; the token loop in `firmware/src/main.cpp` is a stub and does not yet call them.

## Requirements

### Requirement: Bearing to the anchor
The system SHALL compute `bearingDeg(lat1, lon1, lat2, lon2)` as the flat-earth bearing in degrees clockwise from true north in [0, 360), scaling the longitude difference by the cosine of the mean latitude and normalising the longitude difference to [-180, 180].

#### Scenario: Cardinal directions
- **WHEN** the target is 0.01 degrees north, east, south or west of (-33.87, 151.20)
- **THEN** the bearing is 0, 90, 180 or 270 respectively (within 1 degree)

#### Scenario: Latitude correction
- **WHEN** the target is 0.01 degrees north and 0.01 degrees east of (-33.87, 151.20)
- **THEN** the bearing is about 39.7 degrees, not 45

#### Scenario: Antimeridian crossing
- **WHEN** the bearing is taken from (0, 179.9) to (0, -179.9)
- **THEN** the result is 90
- **AND** the reverse bearing from (0, -179.9) to (0, 179.9) is 270

### Requirement: Distance to the anchor
The system SHALL compute `haversineMeters` as the great-circle distance in metres on a sphere of radius 6,371,000 m.

#### Scenario: One hundredth of a degree of latitude
- **WHEN** the distance is taken between (-33.87, 151.20) and (-33.86, 151.20)
- **THEN** the result is 1111.95 m (within 1 m)

#### Scenario: Coincident points
- **WHEN** both points are identical
- **THEN** the distance is 0

#### Scenario: Symmetry
- **WHEN** the distance is taken A to B and B to A
- **THEN** the two results are equal (within 0.001 m)

### Requirement: Relative angle
The system SHALL compute `relativeAngleDeg(bearing, heading)` as `bearing - heading` normalised to [0, 360).

#### Scenario: Wrap-around
- **WHEN** bearing 10 and heading 350
- **THEN** the result is 20
- **WHEN** bearing 350 and heading 10
- **THEN** the result is 340

#### Scenario: Anchor to the left
- **WHEN** bearing 0 and heading 90
- **THEN** the result is 270

### Requirement: Arrow pixel
The system SHALL map the relative angle to the nearest of `numPixels` equal sectors with `arrowPixel(bearing, heading, numPixels)`, where pixel 0 is device-forward, indices increase clockwise, each pixel is centred on `index * 360 / numPixels`, and an angle exactly on a sector boundary goes to the higher pixel. A `numPixels` of 0 or less SHALL return 0.

#### Scenario: Facing the anchor
- **WHEN** bearing equals heading on an 8-pixel ring
- **THEN** the pixel is 0

#### Scenario: Anchor to the right and behind
- **WHEN** an 8-pixel ring has bearing 45 and heading 0
- **THEN** the pixel is 1
- **WHEN** an 8-pixel ring has bearing 180 and heading 0
- **THEN** the pixel is 4

#### Scenario: Anchor to the left
- **WHEN** an 8-pixel ring has bearing 0 and heading 90
- **THEN** the pixel is 6

#### Scenario: Sector boundary
- **WHEN** an 8-pixel ring has relative angle 22.4
- **THEN** the pixel is 0
- **WHEN** an 8-pixel ring has relative angle 22.5
- **THEN** the pixel is 1

#### Scenario: Wrap to pixel 0
- **WHEN** an 8-pixel ring has relative angle 359
- **THEN** the pixel is 0

#### Scenario: Empty ring
- **WHEN** `numPixels` is 0
- **THEN** the pixel is 0
