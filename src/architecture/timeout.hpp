/*
 * This file is part of the Embr project, but was significantly derived from/inspired by the similarly named file in the
 * Taproot project, which is linked here: https://gitlab.com/aruw/controls/taproot
 */

#pragma once

#include "clock.hpp"

namespace embr::arch::time {

template <uint32_t (*T)()>
class Timeout {
    template <typename H>
    friend class PeriodicTimer;

private:
    bool isRunning;
    bool isExecuted;
    uint32_t expireTime;

public:
    static constexpr auto TimeFunction = T;  // allows for user to use either getTimeMicroseconds and getTimeMilliseconds

    Timeout() {
        stop();
        this->expireTime = 0;
    }

    explicit Timeout(uint32_t timeout) { restart(timeout); }

    /**
     * Sets the timer to expire in `timeout` units of time.
     *
     * @param[in] timeout: the amount of time from when this function
     * is called that the timer should expire.
     */
    inline void restart(uint32_t timeout) {
        this->expireTime = TimeFunction() + timeout;
        this->isRunning = true;
        this->isExecuted = false;
    }

    /**
     * Stops the timer. If expired, the expiration flags are cleared.
     */
    inline void stop() {
        this->isRunning = false;
        this->isExecuted = false;
    }

    /**
     * @return `true` if the timer is stopped.
     */
    inline bool isStopped() const { return !this->isRunning; }

    /**
     * @return `true` if the timer has expired (timeout has been reached) without being stopped.
     */
    inline bool isExpired() const { return this->isRunning && (TimeFunction() >= this->expireTime); }

    /**
     * Returns `true` on the first call when timer has expired since restart. Use to
     * only catch the timeout expiration once.
     *
     * @return `true` the first time the timer has expired (timeout has been reached)
     * since last `restart()`
     */
    inline bool execute() {
        if (!isExecuted && isExpired()) {
            this->isExecuted = true;
            return true;
        }
        return false;
    }
};

using MicroTimeout = Timeout<embr::arch::time::getTimeMicroseconds>;
using MilliTimeout = Timeout<embr::arch::time::getTimeMilliseconds>;

}  // namespace embr::arch::time