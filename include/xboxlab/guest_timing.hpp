#pragma once

#include <cstdint>

namespace xboxlab {

// Deterministic virtual time measured in abstract guest ticks.
// These ticks are not calibrated to Xbox hardware or wall-clock time.
class GuestTiming {
public:
    [[nodiscard]] std::uint64_t ticks() const noexcept;
    // Returns false on overflow and leaves the clock unchanged.
    [[nodiscard]] bool advance(std::uint64_t delta) noexcept;
    void reset() noexcept;

private:
    std::uint64_t ticks_{0};
};

} // namespace xboxlab
