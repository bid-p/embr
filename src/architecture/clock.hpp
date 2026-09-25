/*
 * This file is part of the Embr project, but was significantly derived from/inspired by the similarly named file in the
 * Taproot project, which is linked here: https://gitlab.com/aruw/controls/taproot
 */

#pragma once

#include "modm/architecture/interface/clock.hpp"
#include "modm/platform/device.hpp"

namespace embr::arch::time {

/**
 * CPU cycles from the DWT cycle counter, which modm enables at startup (modm::delay_us spins on it).
 *
 * @warning Wraps every 2^32 cycles (25 seconds at 170 MHz): compare timestamps by unsigned subtraction.
 */
inline uint32_t getCycles() { return DWT->CYCCNT; }

/// Converts a number of CPU cycles to microseconds at the current core clock, rounded to the nearest
inline uint32_t cyclesToMicroseconds(uint32_t cycles) {
    const uint32_t cyclesPerMicrosecond = SystemCoreClock / 1'000'000;
    return cycles / cyclesPerMicrosecond + (cycles % cyclesPerMicrosecond >= cyclesPerMicrosecond / 2 ? 1 : 0);
}

inline uint32_t getTimeMilliseconds() { return modm::Clock().now().time_since_epoch().count(); }

/**
 * @warning This clock time will wrap every 72 minutes.
 */
inline uint32_t getTimeMicroseconds() { return modm::PreciseClock::now().time_since_epoch().count(); }

}  // namespace embr::arch::time