#include "xboxlab/guest_timing.hpp"

#include <limits>

namespace xboxlab {

std::uint64_t GuestTiming::ticks() const noexcept {
    return ticks_;
}

bool GuestTiming::advance(std::uint64_t delta) noexcept {
    if (delta > std::numeric_limits<std::uint64_t>::max() - ticks_) {
        return false;
    }
    ticks_ += delta;
    return true;
}

void GuestTiming::reset() noexcept {
    ticks_ = 0;
}

} // namespace xboxlab
