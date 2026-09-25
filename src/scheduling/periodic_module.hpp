#pragma once

#include <cstddef>
#include <cstdint>

namespace embr {

/// The fixed set of rates a module's update functions run at
enum class Rate : uint8_t { k1Hz, k10Hz, k100Hz, k1kHz, k5kHz };

inline constexpr size_t kRateCount = 5;

inline constexpr size_t index(Rate rate) { return static_cast<size_t>(rate); }

/// Frequency of each rate in Hz, indexed by index(rate)
inline constexpr uint32_t kRateFrequencyHz[kRateCount] = {1, 10, 100, 1'000, 5'000};

inline constexpr uint32_t frequencyHz(Rate rate) { return kRateFrequencyHz[index(rate)]; }

/// A set of rates
class Rates {
public:
    constexpr Rates() = default;
    constexpr Rates(Rate rate) : bits(bit(rate)) {}

    constexpr bool contains(Rate rate) const { return bits & bit(rate); }
    constexpr bool empty() const { return bits == 0; }

    constexpr Rates& operator|=(Rates other) {
        bits |= other.bits;
        return *this;
    }
    friend constexpr Rates operator|(Rates a, Rates b) { return a |= b; }
    friend constexpr bool operator==(Rates a, Rates b) = default;

private:
    static constexpr uint8_t bit(Rate rate) { return 1u << index(rate); }

    uint8_t bits = 0;
};

constexpr Rates operator|(Rate a, Rate b) { return Rates(a) | b; }

/**
 * Interface the Scheduler runs. Derive modules from Module<Derived> rather than from this class directly, so that
 * rates() is implemented for them.
 */
class PeriodicModule {
public:
    /// Called once by Scheduler::initialize(), before any update function
    virtual void initialize() {}

    virtual void update1Hz() {}
    virtual void update10Hz() {}
    virtual void update100Hz() {}
    virtual void update1kHz() {}
    virtual void update5kHz() {}

    /// The rates whose update function the module overrides: the Scheduler only calls those
    virtual Rates rates() const = 0;

protected:
    // Modules are never destroyed through this interface. Non-virtual keeps the destructor trivial, so modules with
    // static storage need no atexit registration.
    ~PeriodicModule() = default;
};

/**
 * Base class for modules: override the update functions for the rates the module needs, and nothing else.
 *
 * ```cpp
 * class AuxModule final : public embr::Module<AuxModule> {
 * public:
 *     void update1kHz() override;
 *     void update10Hz() override;
 * };
 * ```
 *
 * rates() is derived at compile time from which update functions Derived overrides: &Derived::update1kHz has type
 * void (Derived::*)() when Derived overrides it, but still void (PeriodicModule::*)() when it is inherited.
 *
 * Overrides must be public, since Module reads their addresses.
 */
template <typename Derived>
class Module : public PeriodicModule {
public:
    static constexpr Rates declaredRates() {
        Rates rates;
        if (overrides(&Derived::update1Hz)) rates |= Rate::k1Hz;
        if (overrides(&Derived::update10Hz)) rates |= Rate::k10Hz;
        if (overrides(&Derived::update100Hz)) rates |= Rate::k100Hz;
        if (overrides(&Derived::update1kHz)) rates |= Rate::k1kHz;
        if (overrides(&Derived::update5kHz)) rates |= Rate::k5kHz;
        return rates;
    }

    Rates rates() const final { return declaredRates(); }

protected:
    ~Module() = default;

private:
    static constexpr bool overrides(void (PeriodicModule::*)()) { return false; }

    template <typename Class>
    static constexpr bool overrides(void (Class::*)()) {
        return true;
    }
};

}  // namespace embr
