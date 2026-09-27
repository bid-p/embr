#pragma once

// Filters for sampled inputs

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
 * Debounces an input sampled at a fixed rate: returns raw once it has differed from debounced for requiredSamples
 * calls in a row, and debounced until then. A bouncing input reads its old value now and then, which restarts the
 * count. changedSamples keeps the count between calls: one per input, starting at 0.
 *
 * ```cpp
 * static bool vescPresentDebounced = false;
 * static uint8_t vescPresentChangedSamples = 0;
 *
 * const bool vescPresentRaw = Board::VescPresent::read();
 * vescPresentDebounced = embr::debounce(vescPresentDebounced, vescPresentRaw, vescPresentChangedSamples, 20);
 * ```
 */
inline uint8_t debounce(uint8_t debounced, uint8_t raw, uint8_t& changedSamples, uint8_t requiredSamples)
{
    uint8_t result = debounced;
    if (raw == debounced)
    {
        changedSamples = 0;
    }
    else
    {
        changedSamples++;
        if (changedSamples >= requiredSamples)
        {
            changedSamples = 0;
            result = raw;
        }
    }
    return result;
}

}  // namespace embr
