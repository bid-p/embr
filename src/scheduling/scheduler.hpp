#pragma once

/*******************************************************************************
 *                               I N C L U D E S
 ******************************************************************************/

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>

#include "emlib/architecture/clock.hpp"

#include "periodic_module.hpp"

namespace embr
{

/*******************************************************************************
 *                                  T Y P E S
 ******************************************************************************/

/// Timing of one rate's passes
struct RateStats
{
    /// Completed passes
    uint32_t passes = 0;
    /// CPU cycles the latest pass took
    uint32_t lastCycles = 0;
    /// Most CPU cycles any pass has taken
    uint32_t worstCycles = 0;
    /// Passes that took longer than the rate's period
    uint32_t overruns = 0;
    /// Most CPU cycles a pass started late, i.e. after the previous pass's start plus the period, e.g. because
    /// another fiber did not yield in time
    uint32_t worstLateCycles = 0;
};

/**
 * Runs a fixed set of modules at their rates.
 *
 * ```cpp
 * embr::Scheduler scheduler{aux::getModule(), sbus::getModule()};
 *
 * scheduler.initialize();
 * // then, from whatever drives the 1 kHz rate (a fiber, a timer interrupt...):
 * scheduler.run(embr::Rate::k1kHz);
 * ```
 *
 * Within a rate, modules run in registration order. There is no order across rates.
 *
 * The Scheduler does not decide what drives each rate. If a rate is run from an interrupt, its update functions must
 * never block, and state they share with other rates needs atomics.
 */
template <size_t N>
class Scheduler
{
public:
    template <std::same_as<PeriodicModule>... Modules>
        requires(sizeof...(Modules) == N)
    explicit Scheduler(const Modules&... registered) : modules{&registered...}
    {}

    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;

    /**
     * Calls each module's initialize() and lists each rate's update functions. Call it once, after the system clock
     * is configured and before the first run().
     */
    void initialize()
    {
        for (const PeriodicModule* module : modules)
        {
            if (module->initialize != nullptr)
            {
                module->initialize();
            }
            add(Rate::k1Hz, module->update1Hz);
            add(Rate::k10Hz, module->update10Hz);
            add(Rate::k100Hz, module->update100Hz);
            add(Rate::k1kHz, module->update1kHz);
            add(Rate::k5kHz, module->update5kHz);
        }
    }

    /// Runs one pass of the rate, calling its update functions in registration order, and records its timing
    void run(Rate rate)
    {
        RateState& state = rates[index(rate)];
        const uint32_t start = getCycles();
        for (size_t i = 0; i < state.count; i++)
        {
            state.updates[i]();
        }
        const uint32_t cycles = getCycles() - start;

        const uint32_t period = SystemCoreClock / frequencyHz(rate);
        // The first pass has no previous start to be late against
        const uint32_t interval = state.stats.passes > 0 ? start - state.lastStart : 0;
        const uint32_t late = interval > period ? interval - period : 0;
        state.lastStart = start;
        record(state.stats, cycles, late, period);
        record(state.windowStats, cycles, late, period);
    }

    /**
     * Returns the rate's timing since the previous call (or boot), and starts a new window. For one reader that
     * takes it periodically, e.g. a logger. The timing since boot is in rates[].stats, for the debugger.
     *
     * @warning Not safe against a rate run from an interrupt, which would need the copy and reset to be atomic.
     */
    RateStats takeWindowStats(Rate rate)
    {
        const RateStats window = rates[index(rate)].windowStats;
        rates[index(rate)].windowStats = {};
        return window;
    }

    /// Number of modules that run at the rate
    size_t getModuleCount(Rate rate) const
    {
        return rates[index(rate)].count;
    }

private:
    using Update = void (*)();

    struct RateState
    {
        std::array<Update, N> updates{};
        size_t count = 0;
        uint32_t lastStart = 0;
        RateStats stats{};
        RateStats windowStats{};
    };

    void add(Rate rate, Update update)
    {
        if (update != nullptr)
        {
            RateState& state = rates[index(rate)];
            state.updates[state.count++] = update;
        }
    }

    static void record(RateStats& stats, uint32_t cycles, uint32_t late, uint32_t period)
    {
        stats.passes++;
        stats.lastCycles = cycles;
        if (cycles > stats.worstCycles)
        {
            stats.worstCycles = cycles;
        }
        if (cycles > period)
        {
            stats.overruns++;
        }
        if (late > stats.worstLateCycles)
        {
            stats.worstLateCycles = late;
        }
    }

    std::array<const PeriodicModule*, N> modules;
    std::array<RateState, RATE_COUNT> rates{};
};

template <typename... Modules>
Scheduler(const Modules&...) -> Scheduler<sizeof...(Modules)>;

}  // namespace embr
