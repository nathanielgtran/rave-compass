#pragma once
/*
 * state.h — token display-mode state machine (pure C++17, no hardware deps)
 *
 * Display modes (precedence: NO_FIX > STALE > PROXIMITY > LIVE_ARROW):
 *
 *   NO_FIX     — own GPS has no valid fix, OR the anchor has never been heard.
 *                Shown as: all pixels off / single red blink.
 *                Takes priority over everything — showing an arrow without
 *                own position or anchor position is meaningless.
 *
 *   STALE      — a frame was heard before but the last one is > STALE_MS ago.
 *                Shown as: slow amber blink at last-known pixel.
 *                Degrade loudly: stale data is never presented as current.
 *                STALE beats PROXIMITY: if close AND stale, show STALE.
 *
 *   PROXIMITY  — anchor frame is recent and distance <= PROXIMITY_M.
 *                Shown as: all pixels breathing pulse ("basically here, look up")
 *                GPS accuracy floor (~3-5 m CEP) means "nearby" rather than "here".
 *                Boundary: exactly PROXIMITY_M is treated as PROXIMITY (inclusive).
 *
 *   LIVE_ARROW — normal operation; anchor frame recent, distance > PROXIMITY_M.
 *                Shown as: single pixel lit at arrow::arrowPixel index.
 *
 * Staleness boundary:
 *   msSinceLastFrame == STALE_MS (exactly 10000) is treated as LIVE (not STALE).
 *   STALE triggers at strictly > STALE_MS. Document this boundary here so
 *   future implementors don't flip it silently.
 */

#include <cstdint>

enum class DisplayMode {
    LIVE_ARROW,
    PROXIMITY,
    STALE,
    NO_FIX,
};

constexpr double   PROXIMITY_M  = 30.0;    // GPS accuracy floor — "look up"
constexpr uint32_t STALE_MS     = 10000;   // 10 seconds — degrade loudly

/*
 * computeMode — determine current display mode from sensor inputs.
 *
 * distanceM        : haversine distance to anchor in metres
 * msSinceLastFrame : milliseconds since the last valid anchor frame was decoded
 * haveOwnFix       : true if the token's own GNSS has a valid fix
 * haveAnchorFrame  : true if at least one anchor frame has ever been received
 *                    (even if now stale)
 *
 * Precedence (highest first):
 *   1. NO_FIX     — !haveOwnFix OR !haveAnchorFrame
 *   2. STALE      — msSinceLastFrame > STALE_MS
 *   3. PROXIMITY  — distanceM <= PROXIMITY_M
 *   4. LIVE_ARROW — otherwise
 */
DisplayMode computeMode(double distanceM, uint32_t msSinceLastFrame,
                        bool haveOwnFix, bool haveAnchorFrame);
