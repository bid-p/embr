#pragma once

/*******************************************************************************
 *                               I N C L U D E S
 ******************************************************************************/

#include <cstddef>
#include <cstdint>

namespace embr
{

/*******************************************************************************
 *                                D E F I N E S
 ******************************************************************************/

inline constexpr size_t RATE_COUNT = 5;

/// Frequency of each rate in Hz, indexed by index(rate)
inline constexpr uint32_t RATE_FREQUENCY_HZ[RATE_COUNT] = {1, 10, 100, 1'000, 5'000};

/*******************************************************************************
 *                                  T Y P E S
 ******************************************************************************/

/// The fixed set of rates a module's update functions run at
enum class Rate : uint8_t
{
    k1Hz,
    k10Hz,
    k100Hz,
    k1kHz,
    k5kHz
};

/**
 * A module's functions, which the Scheduler calls. A module sets only the ones it needs, in this order; the others
 * stay null and are never called:
 *
 * ```cpp
 * const embr::PeriodicModule& module() {
 *     static constexpr embr::PeriodicModule periodicModule = {
 *         .initialize = initialize,
 *         .update10Hz = update10Hz,
 *         .update1kHz = update1kHz,
 *     };
 *     return periodicModule;
 * }
 * ```
 */
struct PeriodicModule
{
    /// Called once by Scheduler::initialize(), before any update function
    void (*initialize)() = nullptr;

    void (*update1Hz)() = nullptr;
    void (*update10Hz)() = nullptr;
    void (*update100Hz)() = nullptr;
    void (*update1kHz)() = nullptr;
    void (*update5kHz)() = nullptr;
};

/*******************************************************************************
 *            P U B L I C   F U N C T I O N   D E F I N I T I O N S
 ******************************************************************************/

inline constexpr size_t index(Rate rate)
{
    return static_cast<size_t>(rate);
}

inline constexpr uint32_t frequencyHz(Rate rate)
{
    return RATE_FREQUENCY_HZ[index(rate)];
}

}  // namespace embr
