#include "xboxlab/guest_exception.hpp"

namespace xboxlab {

bool GuestExceptionState::raise(GuestExceptionCode code,
                                std::uint64_t address) noexcept {
    if (pending_) {
        return false;
    }
    exception_ = {code, address};
    pending_ = true;
    return true;
}

bool GuestExceptionState::has_pending() const noexcept {
    return pending_;
}

bool GuestExceptionState::pending(GuestException& exception) const noexcept {
    if (!pending_) {
        return false;
    }
    exception = exception_;
    return true;
}

void GuestExceptionState::clear() noexcept {
    exception_ = {};
    pending_ = false;
}

} // namespace xboxlab
