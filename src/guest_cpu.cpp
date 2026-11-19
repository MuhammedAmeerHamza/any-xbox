#include "xboxlab/guest_cpu.hpp"

namespace xboxlab {

bool GuestCpuState::read_gpr(X64Gpr reg, std::uint64_t& value) const noexcept {
    const auto index = static_cast<std::size_t>(reg);
    if (index >= gpr_.size()) {
        return false;
    }
    value = gpr_[index];
    return true;
}

bool GuestCpuState::write_gpr(X64Gpr reg, std::uint64_t value) noexcept {
    const auto index = static_cast<std::size_t>(reg);
    if (index >= gpr_.size()) {
        return false;
    }
    gpr_[index] = value;
    return true;
}

bool GuestCpuState::flag_mask(X64Flag flag, std::uint64_t& mask) noexcept {
    unsigned int bit = 0;
    switch (flag) {
    case X64Flag::Carry: bit = 0; break;
    case X64Flag::Parity: bit = 2; break;
    case X64Flag::AuxiliaryCarry: bit = 4; break;
    case X64Flag::Zero: bit = 6; break;
    case X64Flag::Sign: bit = 7; break;
    case X64Flag::Trap: bit = 8; break;
    case X64Flag::InterruptEnable: bit = 9; break;
    case X64Flag::Direction: bit = 10; break;
    case X64Flag::Overflow: bit = 11; break;
    default: return false;
    }
    mask = 1ULL << bit;
    return true;
}

bool GuestCpuState::flag_is_set(X64Flag flag, bool& value) const noexcept {
    std::uint64_t mask = 0;
    if (!flag_mask(flag, mask)) {
        return false;
    }
    value = (rflags_ & mask) != 0;
    return true;
}

bool GuestCpuState::set_flag(X64Flag flag, bool value) noexcept {
    std::uint64_t mask = 0;
    if (!flag_mask(flag, mask)) {
        return false;
    }
    if (value) {
        rflags_ |= mask;
    } else {
        rflags_ &= ~mask;
    }
    rflags_ |= kReservedRflagsBit;
    return true;
}

void GuestCpuState::set_rflags(std::uint64_t value) noexcept {
    // Bit 1 is architecturally reserved and reads as one.
    rflags_ = value | kReservedRflagsBit;
}

void GuestCpuState::reset() noexcept {
    gpr_.fill(0);
    rip_ = 0;
    rflags_ = kReservedRflagsBit;
}

std::uint64_t GuestCpuState::instruction_pointer() const noexcept {
    return rip_;
}

void GuestCpuState::set_instruction_pointer(std::uint64_t value) noexcept {
    rip_ = value;
}

std::uint64_t GuestCpuState::rflags() const noexcept {
    return rflags_;
}

} // namespace xboxlab
