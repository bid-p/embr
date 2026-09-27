#pragma once

// Saturating counters, e.g. for how long a condition has held

/*******************************************************************************
 *                               I N C L U D E S
 ******************************************************************************/

#include <cstdint>

namespace embr
{

/*******************************************************************************
 *            P U B L I C   F U N C T I O N   D E F I N I T I O N S
 ******************************************************************************/

/**
 * Counts counter up by one, stopping at threshold, and returns whether it has reached threshold. It keeps returning
 * true while the counter stays there.
 *
 * ```cpp
 * static uint32_t vescAbsentTicks = 0;
 *
 * if (embr::counter_inc32(vescAbsentTicks, AUX_MANAGER_SHUTDOWN_DELAY_TICKS))
 * {
 *     // the VESC has been absent for the whole delay
 * }
 * ```
 */
constexpr bool counter_inc32(uint32_t& counter, uint32_t threshold)
{
    if (counter < threshold)
    {
        counter++;
    }
    else
    {
        counter = threshold;
    }
    return counter >= threshold;
}

/**
 * Counts counter down by one, stopping at 0, and returns whether it is at or below threshold. It keeps returning true
 * while the counter stays there. With a threshold of 0 it returns whether the counter has reached 0.
 */
constexpr bool counter_dec32(uint32_t& counter, uint32_t threshold)
{
    if (counter > 0)
    {
        counter--;
    }
    return counter <= threshold;
}

}  // namespace embr
