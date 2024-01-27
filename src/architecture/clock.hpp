/*
 * This file is part of the Embr project, but was significantly derived from/inspired by the similarly named file in the
 * Taproot project, which is linked here: https://gitlab.com/aruw/controls/taproot
 */

#pragma once

#include "modm/platform.hpp"

namespace embr::arch::time {

inline uint32_t getTimeMilliseconds() { return modm::Clock().now().time_since_epoch().count(); }

/**
 * @warning This clock time will wrap every 72 minutes.
 */
inline uint32_t getTimeMicroseconds() { return modm::PreciseClock::now().time_since_epoch().count(); }

}  // namespace embr::arch::time