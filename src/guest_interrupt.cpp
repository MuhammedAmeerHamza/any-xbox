#include "xboxlab/guest_interrupt.hpp"

namespace xboxlab {

bool GuestInterruptController::request(GuestInterrupt interrupt) noexcept {
    if (count_ == kCapacity) {
        return false;
    }
    const std::size_t tail = (head_ + count_) % kCapacity;
    queue_[tail] = interrupt;
    ++count_;
    return true;
}

bool GuestInterruptController::acknowledge(GuestInterrupt& interrupt) noexcept {
    if (count_ == 0) {
        return false;
    }
    interrupt = queue_[head_];
    head_ = (head_ + 1) % kCapacity;
    --count_;
    return true;
}

std::size_t GuestInterruptController::pending_count() const noexcept {
    return count_;
}

bool GuestInterruptController::empty() const noexcept {
    return count_ == 0;
}

void GuestInterruptController::reset() noexcept {
    head_ = 0;
    count_ = 0;
}

} // namespace xboxlab
