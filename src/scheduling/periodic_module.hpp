#pragma once

/*******************************************************************************
 *                               I N C L U D E S
 ******************************************************************************/

#include <chrono>
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

/// The fixed set of rates a module's update functions run at. The values are also available directly in
/// embr, e.g. embr::Freq10Hz.
enum class Rate : uint8_t
{
    Freq1Hz,
    Freq10Hz,
    Freq100Hz,
    Freq1kHz,
    Freq5kHz
};

using enum Rate;

/**
 * A module's functions, which the Scheduler calls. A module sets only the ones it needs, in this order; the others
 * stay null and are never called:
 *
 * ```cpp
 * const embr::PeriodicModule& getModule() {
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

/**
 * The number of passes of rate in time, rounded down, e.g. for a count of update calls:
 *
 * ```cpp
 * using namespace std::chrono_literals;
 * static constexpr uint32_t SHUTDOWN_DELAY_TICKS = embr::durationToTicks(embr::Freq10Hz, 5s);  // 50
 * ```
 */
inline constexpr uint32_t durationToTicks(Rate rate, std::chrono::milliseconds time)
{
    return time.count() * frequencyHz(rate) / 1000;
}

}  // namespace embr
