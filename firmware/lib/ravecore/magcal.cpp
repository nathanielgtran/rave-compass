#include "magcal.h"
#include <cfloat>
#include <cmath>

namespace ravecore {

MagCalSession::MagCalSession(float rangeThreshold)
    : minX_(FLT_MAX),  maxX_(-FLT_MAX)
    , minY_(FLT_MAX),  maxY_(-FLT_MAX)
    , minZ_(FLT_MAX),  maxZ_(-FLT_MAX)
    , threshold_(rangeThreshold)
    , hasData_(false)
{}

void MagCalSession::addSample(float x, float y, float z) {
    if (!hasData_) {
        minX_ = maxX_ = x;
        minY_ = maxY_ = y;
        minZ_ = maxZ_ = z;
        hasData_ = true;
        return;
    }
    if (x < minX_) minX_ = x;
    if (x > maxX_) maxX_ = x;
    if (y < minY_) minY_ = y;
    if (y > maxY_) maxY_ = y;
    if (z < minZ_) minZ_ = z;
    if (z > maxZ_) maxZ_ = z;
}

bool MagCalSession::isSufficient() const {
    if (!hasData_) return false;
    return (maxX_ - minX_) >= threshold_
        && (maxY_ - minY_) >= threshold_
        && (maxZ_ - minZ_) >= threshold_;
}

MagCal MagCalSession::result() const {
    MagCal cal;
    cal.offsetX = (minX_ + maxX_) * 0.5f;
    cal.offsetY = (minY_ + maxY_) * 0.5f;
    cal.offsetZ = (minZ_ + maxZ_) * 0.5f;
    return cal;
}

void MagCalSession::reset() {
    minX_ = maxX_ = 0.0f;
    minY_ = maxY_ = 0.0f;
    minZ_ = maxZ_ = 0.0f;
    hasData_ = false;
}

} // namespace ravecore
