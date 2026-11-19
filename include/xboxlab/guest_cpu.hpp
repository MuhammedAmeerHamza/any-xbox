#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace xboxlab {

// Architectural general-purpose register order for the x86-64 guest context.
// This is state storage only; it does not execute guest instructions.
enum class X64Gpr : std::uint8_t {
    Rax, Rcx, Rdx, Rbx, Rsp, Rbp, Rsi, Rdi,
    R8, R9, R10, R11, R12, R13, R14, R15,
    Count
};

enum class X64Flag : std::uint8_t {
    Carry,
    Parity,
    AuxiliaryCarry,
    Zero,
    Sign,
    Trap,
    InterruptEnable,
    Direction,
    Overflow
};

class GuestCpuState {
public:
    static constexpr std::size_t kGeneralRegisterCount =
        static_cast<std::size_t>(X64Gpr::Count);
    static constexpr std::uint64_t kReservedRflagsBit = 1ULL << 1U;

    [[nodiscard]] bool read_gpr(X64Gpr reg, std::uint64_t& value) const noexcept;
    [[nodiscard]] bool write_gpr(X64Gpr reg, std::uint64_t value) noexcept;

    [[nodiscard]] bool flag_is_set(X64Flag flag, bool& value) const noexcept;
    [[nodiscard]] bool set_flag(X64Flag flag, bool value) noexcept;

    void set_rflags(std::uint64_t value) noexcept;
    void reset() noexcept;

    [[nodiscard]] std::uint64_t instruction_pointer() const noexcept;
    void set_instruction_pointer(std::uint64_t value) noexcept;

    [[nodiscard]] std::uint64_t rflags() const noexcept;

private:
    [[nodiscard]] static bool flag_mask(X64Flag flag,
                                        std::uint64_t& mask) noexcept;

    std::array<std::uint64_t, kGeneralRegisterCount> gpr_{};
    std::uint64_t rip_{0};
    std::uint64_t rflags_{kReservedRflagsBit};
};

} // namespace xboxlab
