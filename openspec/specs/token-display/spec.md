# token-display Specification

## Purpose
How a token decides what its LED ring shows and steadies the arrow (`firmware/lib/ravecore/state.h`, `smooth.h`): display mode selection, circular bearing smoothing and pixel hysteresis. Pure logic; the token loop in `firmware/src/main.cpp` is a stub and does not yet drive the ring or call these.

## Requirements

### Requirement: Display mode precedence
The system SHALL make `computeMode(distanceM, msSinceLastFrame, haveOwnFix, haveAnchorFrame)` return the first matching mode in this order: `NO_FIX` when the token has no own fix or has never heard the anchor, `STALE` when the last frame is older than the staleness threshold, `PROXIMITY` when the distance is within the proximity radius, otherwise `LIVE_ARROW`.

#### Scenario: Normal operation
- **WHEN** distance 100 m, 500 ms since the last frame, own fix and an anchor frame heard
- **THEN** the mode is `LIVE_ARROW`

#### Scenario: Close and fresh
- **WHEN** distance 10 m, 500 ms since the last frame, own fix and an anchor frame heard
- **THEN** the mode is `PROXIMITY`

#### Scenario: No own fix
- **WHEN** the token has no own fix, whatever the other inputs
- **THEN** the mode is `NO_FIX`

#### Scenario: Anchor never heard
- **WHEN** the token has an own fix but has never received an anchor frame
- **THEN** the mode is `NO_FIX`

#### Scenario: No fix beats stale
- **WHEN** the token has no own fix and 99999 ms have passed since the last frame
- **THEN** the mode is `NO_FIX`

#### Scenario: Stale beats proximity
- **WHEN** distance 10 m, 10001 ms since the last frame, own fix and an anchor frame heard
- **THEN** the mode is `STALE`

### Requirement: Proximity radius
The system SHALL treat a distance of exactly `PROXIMITY_M` (30.0 m) as `PROXIMITY` and any greater distance as outside it.

#### Scenario: On the boundary
- **WHEN** distance 30.0 m with a fresh frame, own fix and an anchor frame heard
- **THEN** the mode is `PROXIMITY`

#### Scenario: Just outside
- **WHEN** distance 30.01 m with a fresh frame, own fix and an anchor frame heard
- **THEN** the mode is `LIVE_ARROW`

### Requirement: Staleness threshold
The system SHALL treat a last frame exactly `STALE_MS` (10000 ms) old as live and anything strictly older as `STALE`.

#### Scenario: On the boundary
- **WHEN** 10000 ms since the last frame at distance 100 m with own fix and an anchor frame heard
- **THEN** the mode is `LIVE_ARROW`

#### Scenario: Just past
- **WHEN** 10001 ms since the last frame at distance 100 m with own fix and an anchor frame heard
- **THEN** the mode is `STALE`

### Requirement: Circular bearing smoothing
The system SHALL smooth bearings with `BearingSmoother`, an exponential moving average (default alpha 0.2) over the sine and cosine of the bearing so values straddling the 0/360 seam average correctly. The first sample SHALL be taken as-is, `isValid` SHALL be false until a sample is added and after `reset`, and alpha 1.0 SHALL pass each sample through unsmoothed.

#### Scenario: Before any sample
- **WHEN** a smoother is constructed
- **THEN** `isValid` is false
- **WHEN** the first sample 123 is added
- **THEN** `isValid` is true and `value` is 123 (within 0.01)

#### Scenario: Average across the seam
- **WHEN** samples 350, 355, 0, 5 and 10 are added with alpha 0.5
- **THEN** `value` is within 30 degrees of north (below 30 or above 330), not near 180

#### Scenario: Blend of two bearings
- **WHEN** samples 0 then 90 are added with alpha 0.5
- **THEN** `value` is 45 (within 0.01)

#### Scenario: No smoothing at alpha 1
- **WHEN** samples 10 then 200 are added with alpha 1.0
- **THEN** `value` is 200 (within 0.01)

#### Scenario: Convergence
- **WHEN** a sample 0 is followed by twenty samples of 90 with alpha 0.3
- **THEN** `value` is between 80 and 100

#### Scenario: Reset
- **WHEN** `reset` is called on a valid smoother
- **THEN** `isValid` is false

### Requirement: Pixel hysteresis
The system SHALL hold the committed LED pixel in `PixelHysteresis` (default 12 pixels, margin 6.0 degrees) so that, when the bearing increases across a pixel boundary, the pixel changes only once the bearing is at least `margin` degrees past that boundary. The first update after construction or `reset` SHALL commit the nearest pixel immediately, and `current` SHALL return -1 until then.

#### Scenario: No pixel before first update
- **WHEN** a hysteresis object is constructed or reset
- **THEN** `current` is -1

#### Scenario: First update commits
- **WHEN** the first bearing 100 is fed to an 8-pixel ring with `update`
- **THEN** the returned pixel is 2 and `current` is 2

#### Scenario: Holds inside the margin
- **WHEN** a 12-pixel ring with margin 6 has committed pixel 0 at bearing 5 and the bearing rises to 15.0, then 20.9
- **THEN** the pixel stays 0

#### Scenario: Commits at the margin
- **WHEN** a 12-pixel ring with margin 6 has committed pixel 0 at bearing 5 and the bearing rises to 21.0
- **THEN** the pixel becomes 1

#### Scenario: Oscillation at a boundary
- **WHEN** a 12-pixel ring with margin 6 has committed pixel 0 at bearing 5 and the bearing alternates between 10 and 20 for 20 updates
- **THEN** the pixel never changes
