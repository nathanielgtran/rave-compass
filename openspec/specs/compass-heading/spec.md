# compass-heading Specification

## Purpose
Turn raw magnetometer and accelerometer readings into a magnetic heading and calibrate the magnetometer for hard-iron offsets (`firmware/lib/ravecore/heading.h`, `magcal.h`). Pure functions; no sensor driver exists yet. Soft-iron correction is not implemented.

## Requirements

### Requirement: Tilt-compensated heading
The system SHALL compute `tiltCompensatedHeadingDeg(mx, my, mz, ax, ay, az)` as the heading of the device X axis from magnetic north in [0, 360), North 0 and East 90, using axes x forward, y right, z down, with pitch and roll taken from the accelerometer, whose magnitude is normalised away.

#### Scenario: Flat device, cardinal field
- **WHEN** the accelerometer reads (0, 0, 1) and the magnetometer reads (1, 0, 0), (0, -1, 0), (-1, 0, 0) or (0, 1, 0)
- **THEN** the heading is 0, 90, 180 or 270 respectively (within 1 degree)

#### Scenario: Pitched device
- **WHEN** the device is pitched nose-up 30 degrees facing north, with accelerometer (-sin 30, 0, cos 30) and magnetometer (cos 30, 0, -sin 30)
- **THEN** the heading is 0 (within 2 degrees)

#### Scenario: Rolled device
- **WHEN** the device is rolled right 30 degrees facing east, with accelerometer (0, sin 30, cos 30) and magnetometer (0, -cos 30, sin 30)
- **THEN** the heading is 90 (within 1 degree)

#### Scenario: Accelerometer scale
- **WHEN** the accelerometer reads (0, 0, 9.81) instead of (0, 0, 1) with magnetometer (0, -1, 0)
- **THEN** the heading is 90

### Requirement: Degenerate accelerometer input
The system SHALL return 0.0, never NaN, from `tiltCompensatedHeadingDeg` when the accelerometer magnitude is below 1e-9.

#### Scenario: Zero accelerometer
- **WHEN** the accelerometer reads (0, 0, 0)
- **THEN** the heading is exactly 0.0 and not NaN

### Requirement: Hard-iron offset removal
The system SHALL make `applyCalibration` subtract the `MagCal` offsets from the magnetometer readings in place.

#### Scenario: Offsets subtracted
- **WHEN** readings (100, -50, 20) are calibrated with offsets (10, -5, 2)
- **THEN** the readings become (90, -45, 18)

### Requirement: Streaming hard-iron calibration
The system SHALL accumulate raw magnetometer samples in `MagCalSession`, track per-axis minimum and maximum, and return offsets equal to the midpoint of minimum and maximum on each axis. `isSufficient` SHALL be true only when at least one sample has been added and every axis span is at least the range threshold (default 20.0, configurable at construction). `reset` SHALL discard all samples.

#### Scenario: Offsets are axis midpoints
- **WHEN** samples (10, -30, 5), (30, -10, 45) and (20, -20, 25) are added
- **THEN** `result` has offsets (20, -20, 25)
- **AND** `isSufficient` is true

#### Scenario: Span exactly at threshold
- **WHEN** samples (0, 0, 0) and (20, 20, 20) are added with the default threshold
- **THEN** `isSufficient` is true

#### Scenario: One axis below threshold
- **WHEN** samples (0, 0, 0) and (19.9, 100, 100) are added with the default threshold
- **THEN** `isSufficient` is false

#### Scenario: Only one axis moves
- **WHEN** 100 samples vary Z over a range of 120 while X and Y stay near constant
- **THEN** `isSufficient` is false

#### Scenario: Custom threshold
- **WHEN** samples (0, 0, 0) and (5, 5, 5) are added to a session with threshold 5.0
- **THEN** `isSufficient` is true

#### Scenario: No samples and reset
- **WHEN** no samples have been added, or `reset` was called after sufficient samples
- **THEN** `isSufficient` is false

#### Scenario: Recovering a known offset
- **WHEN** 360 samples trace a figure-8 of radius 60 centred on (50, 50, 50)
- **THEN** `result` offsets are each within 1 of 50
