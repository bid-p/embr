#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <concepts>

#include "emlib/architecture/clock.hpp"

#include "periodic_module.hpp"

namespace embr {

/// Timing of one rate's passes, readable over the debugger
struct RateStats {
    /// Completed passes
    uint32_t passes = 0;
    /// CPU cycles the latest pass took
    uint32_t lastCycles = 0;
    /// Most CPU cycles any pass has taken
    uint32_t worstCycles = 0;
    /// Passes that took longer than the rate's period
    uint32_t overruns = 0;
    /// Most CPU cycles between the starts of two consecutive passes: above the period when a pass started late,
    /// e.g. because another fiber did not yield in time
    uint32_t worstIntervalCycles = 0;
};

/**
 * Runs a fixed set of modules at their rates.
 *
 * ```cpp
 * embr::Scheduler scheduler{aux::module(), sbus::module()};
 *
 * scheduler.initialize();
 * // then, from whatever drives the 1 kHz rate (a fiber, a timer interrupt...):
 * scheduler.run<embr::Rate::k1kHz>();
 * ```
 *
 * Within a rate, modules run in registration order. There is no order across rates.
 *
 * The Scheduler does not decide what drives each rate. If a rate is run from an interrupt, its update functions must
 * never block, and state they share with other rates needs atomics.
 */
template <size_t N>
class Scheduler {
public:
    template <std::derived_from<PeriodicModule>... Modules>
        requires(sizeof...(Modules) == N)
    explicit Scheduler(Modules&... registered) : modules{&registered...} {}

    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;

    /**
     * Calls each module's initialize(), then builds each rate's list from the modules' rates(). Call it once, after the
     * system clock is configured and before the first run().
     */
    void initialize() {
        for (PeriodicModule* module : modules) {
            module->initialize();
        }
        for (PeriodicModule* module : modules) {
            const Rates rates = module->rates();
            for (size_t rate = 0; rate < kRateCount; rate++) {
                if (rates.contains(static_cast<Rate>(rate))) {
                    lists[rate].modules[lists[rate].count++] = module;
                }
            }
        }
        for (size_t rate = 0; rate < kRateCount; rate++) {
            periodCycles[rate] = SystemCoreClock / kRateFrequencyHz[rate];
        }
    }

    /// Runs one pass of rate R: calls updateR() on each module that overrides it, and records the pass's timing
    template <Rate R>
    void run() {
        const uint32_t start = arch::time::getCycles();
        const RateList& list = lists[index(R)];
        for (size_t i = 0; i < list.count; i++) {
            update<R>(*list.modules[i]);
        }
        const uint32_t cycles = arch::time::getCycles() - start;

        // The interval to the previous pass start, 0 on the first pass
        const uint32_t interval = stats[index(R)].passes > 0 ? start - lastStart[index(R)] : 0;
        lastStart[index(R)] = start;
        record(stats[index(R)], cycles, interval, periodCycles[index(R)]);
        record(windowStats[index(R)], cycles, interval, periodCycles[index(R)]);
    }

    /// The rate's timing since boot
    const RateStats& getStats(Rate rate) const { return stats[index(rate)]; }

    /**
     * Returns the rate's timing since the previous call (or boot), and starts a new window. For one reader that
     * takes it periodically, e.g. a logger; getStats() is unaffected.
     *
     * @warning Not safe against a rate run from an interrupt, which would need the copy and reset to be atomic.
     */
    RateStats takeWindowStats(Rate rate) {
        const RateStats window = windowStats[index(rate)];
        windowStats[index(rate)] = {};
        return window;
    }

    /// Number of modules that run at the rate
    size_t moduleCount(Rate rate) const { return lists[index(rate)].count; }

private:
    struct RateList {
        std::array<PeriodicModule*, N> modules{};
        size_t count = 0;
    };

    // Selected at compile time: exactly one virtual call per module per pass
    static void record(RateStats& rateStats, uint32_t cycles, uint32_t interval, uint32_t period) {
        rateStats.passes++;
        rateStats.lastCycles = cycles;
        if (cycles > rateStats.worstCycles) rateStats.worstCycles = cycles;
        if (cycles > period) rateStats.overruns++;
        if (interval > rateStats.worstIntervalCycles) rateStats.worstIntervalCycles = interval;
    }

    template <Rate R>
    static void update(PeriodicModule& module) {
        if constexpr (R == Rate::k1Hz) {
            module.update1Hz();
        } else if constexpr (R == Rate::k10Hz) {
            module.update10Hz();
        } else if constexpr (R == Rate::k100Hz) {
            module.update100Hz();
        } else if constexpr (R == Rate::k1kHz) {
            module.update1kHz();
        } else if constexpr (R == Rate::k5kHz) {
            module.update5kHz();
        }
    }

    std::array<PeriodicModule*, N> modules;
    std::array<RateList, kRateCount> lists{};
    std::array<RateStats, kRateCount> stats{};
    std::array<RateStats, kRateCount> windowStats{};
    std::array<uint32_t, kRateCount> periodCycles{};
    std::array<uint32_t, kRateCount> lastStart{};
};

template <typename... Modules>
Scheduler(Modules&...) -> Scheduler<sizeof...(Modules)>;

}  // namespace embr
