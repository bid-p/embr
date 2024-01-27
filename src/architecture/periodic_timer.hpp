/*
 * This file is part of the Embr project, but was significantly derived from/inspired by the similarly named file in the
 * Taproot project, which is linked here: https://gitlab.com/aruw/controls/taproot
 */

#pragma once

#include "timeout.hpp"

namespace embr::arch::time {

/**
 * A timer class which unlike the `Timeout` class, will restart when execute
 * is called and returns that the timer has expired. Keeps its expire times
 * aligned with the given timeout period (i.e.: the timeout time will always be a
 * multiple of the period it is constructed to timeout in +/- some constant due to
 * program startup time).
 */
template <typename T>
class PeriodicTimer {
public:
    PeriodicTimer() : period(0) {}

    explicit PeriodicTimer(uint32_t period) : period(period), timeout(period) {}

    /**
     * Sets the timer to expire in `period` units of time from the time at which this function was called.
     */
    inline void restart() { this->timeout.restart(period); }

    /**
     * Similar to `restart()` but allows for the redefinition of the timer's period.
     *
     * @param[in] period: the new period to use for this `PeriodicTimer`
     */
    inline void restart(uint32_t period) {
        this->period = period;
        this->timeout.restart(period);
    }

    /**
     * Stops the timer.
     */
    inline void stop() { this->timeout.stop(); }

    /**
     * @brief Returns true on the first call of execute after the timer has expired. If the timer is checked and has expired,
     * the next expiration time is set to the closest multiple of the period after the current time.
     *
     * @return true the first time execute is called after the timer has expired, otherwise
     * @return false
     */
    inline bool execute() {
        if (this->timeout.execute()) {
            uint32_t now = T::TimeFunction();

            do {
                this->timeout.expireTime += this->period;
            } while (this->timeout.expireTime <= now);

            this->timeout.isExecuted = false;
            this->timeout.isRunning = true;
            return true;
        }
        return false;
    }

    /**
     * @return `true` if the timer is stopped.
     */
    inline bool isStopped() const { return this->timeout.isStopped(); }

private:
    uint32_t period;
    T timeout;
};

using PeriodicMicroTimeout = PeriodicTimer<MicroTimeout>;
using PeriodicMilliTimeout = PeriodicTimer<MilliTimeout>;

}  // namespace embr::arch::time