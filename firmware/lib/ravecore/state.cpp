#include "state.h"

DisplayMode computeMode(double distanceM, uint32_t msSinceLastFrame,
                        bool haveOwnFix, bool haveAnchorFrame) {
    // Priority 1: NO_FIX — missing own fix or never heard anchor.
    // Beats everything — never show an arrow we can't compute.
    if (!haveOwnFix || !haveAnchorFrame) return DisplayMode::NO_FIX;

    // Priority 2: STALE — last frame is old (strictly > STALE_MS).
    // Exactly STALE_MS is treated as live (see header comment).
    // STALE beats PROXIMITY: if data is stale we don't know the true distance.
    if (msSinceLastFrame > STALE_MS) return DisplayMode::STALE;

    // Priority 3: PROXIMITY — within GPS accuracy floor.
    // Inclusive boundary: distanceM <= PROXIMITY_M -> PROXIMITY.
    if (distanceM <= PROXIMITY_M) return DisplayMode::PROXIMITY;

    // Priority 4: LIVE_ARROW — normal operation.
    return DisplayMode::LIVE_ARROW;
}
