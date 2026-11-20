#include "xboxlab/x64_interpreter.hpp"

#include <array>
#include <cstdint>
#include <limits>

namespace xboxlab {
namespace {

bool fetch_u8(const GuestMemory& memory, std::uint64_t address,
              std::uint8_t& value) {
    return memory.read(address, std::span<std::uint8_t>(&value, 1));
}

bool advance_ip(GuestCpuState& cpu, std::uint64_t amount) {
    const auto ip = cpu.instruction_pointer();
    if (amount > std::numeric_limits<std::uint64_t>::max() - ip) return false;
    cpu.set_instruction_pointer(ip + amount);
    return true;
}

bool relative_target(std::uint64_t next_ip, std::int8_t displacement,
                     std::uint64_t& target) {
    if (displacement >= 0) {
        const auto delta = static_cast<std::uint64_t>(displacement);
        if (delta > std::numeric_limits<std::uint64_t>::max() - next_ip) return false;
        target = next_ip + delta;
        return true;
    }
    const auto delta = static_cast<std::uint64_t>(
        -static_cast<std::int16_t>(displacement));
    if (delta > next_ip) return false;
    target = next_ip - delta;
    return true;
}

bool set_cmp_flags(GuestCpuState& cpu, std::uint32_t lhs,
                   std::uint32_t rhs) {
    const std::uint32_t result = lhs - rhs;
    const bool carry = lhs < rhs;
    const bool parity = (__builtin_popcount(static_cast<unsigned int>(result & 0xFFU)) % 2) == 0;
    const bool auxiliary = ((lhs ^ rhs ^ result) & 0x10U) != 0;
    const bool zero = result == 0;
    const bool sign = (result & 0x80000000U) != 0;
    const bool overflow = (((lhs ^ rhs) & (lhs ^ result)) & 0x80000000U) != 0;
    return cpu.set_flag(X64Flag::Carry, carry) &&
           cpu.set_flag(X64Flag::Parity, parity) &&
           cpu.set_flag(X64Flag::AuxiliaryCarry, auxiliary) &&
           cpu.set_flag(X64Flag::Zero, zero) &&
           cpu.set_flag(X64Flag::Sign, sign) &&
           cpu.set_flag(X64Flag::Overflow, overflow);
}

} // namespace

std::string_view X64InterpreterBackend::name() const noexcept {
    return "x86-64-subset-interpreter";
}

ExecutionResult X64InterpreterBackend::run(
    GuestCpuState& cpu, GuestMemory& memory,
    std::uint64_t instruction_budget) {
    if (instruction_budget == 0) {
        return {ExecutionStopReason::InvalidRequest, 0, 0};
    }

    std::uint64_t executed = 0;
    while (executed < instruction_budget) {
        const auto ip = cpu.instruction_pointer();
        std::uint8_t opcode = 0;
        if (!fetch_u8(memory, ip, opcode)) {
            return {ExecutionStopReason::GuestMemoryFault, executed, 0};
        }

        if (opcode >= 0xB8U && opcode <= 0xBFU) {
            std::uint32_t immediate = 0;
            if (ip > std::numeric_limits<std::uint64_t>::max() - 1U ||
                !memory.read_u32_le(ip + 1U, immediate)) {
                return {ExecutionStopReason::GuestMemoryFault, executed, 0};
            }
            const auto reg = static_cast<X64Gpr>(opcode - 0xB8U);
            if (!cpu.write_gpr(reg, immediate) || !advance_ip(cpu, 5U)) {
                return {ExecutionStopReason::GuestMemoryFault, executed, 0};
            }
            ++executed;
            continue;
        }

        if (opcode == 0x3DU) { // CMP EAX, imm32
            std::uint32_t immediate = 0;
            std::uint64_t rax = 0;
            if (ip > std::numeric_limits<std::uint64_t>::max() - 1U ||
                !memory.read_u32_le(ip + 1U, immediate) ||
                !cpu.read_gpr(X64Gpr::Rax, rax)) {
                return {ExecutionStopReason::GuestMemoryFault, executed, 0};
            }
            if (!set_cmp_flags(cpu, static_cast<std::uint32_t>(rax), immediate) ||
                !advance_ip(cpu, 5U)) {
                return {ExecutionStopReason::GuestException, executed, 0};
            }
            ++executed;
            continue;
        }

        if (opcode == 0x74U || opcode == 0x75U || opcode == 0xEBU) {
            std::uint8_t raw_displacement = 0;
            if (ip == std::numeric_limits<std::uint64_t>::max() ||
                !fetch_u8(memory, ip + 1U, raw_displacement)) {
                return {ExecutionStopReason::GuestMemoryFault, executed, 0};
            }
            const auto displacement = static_cast<std::int8_t>(raw_displacement);
            if (!advance_ip(cpu, 2U)) {
                return {ExecutionStopReason::GuestMemoryFault, executed, 0};
            }

            bool take_branch = opcode == 0xEBU;
            if (opcode == 0x74U || opcode == 0x75U) {
                bool zero = false;
                if (!cpu.flag_is_set(X64Flag::Zero, zero)) {
                    return {ExecutionStopReason::GuestException, executed, 0};
                }
                take_branch = opcode == 0x74U ? zero : !zero;
            }
            if (take_branch) {
                std::uint64_t target = 0;
                if (!relative_target(cpu.instruction_pointer(), displacement, target)) {
                    return {ExecutionStopReason::GuestMemoryFault, executed, 0};
                }
                cpu.set_instruction_pointer(target);
            }
            ++executed;
            continue;
        }

        if (opcode == 0x90U) { // NOP
            if (!advance_ip(cpu, 1U)) {
                return {ExecutionStopReason::GuestMemoryFault, executed, 0};
            }
            ++executed;
            continue;
        }

        if (opcode == 0xF4U) { // HLT
            if (!advance_ip(cpu, 1U)) {
                return {ExecutionStopReason::GuestMemoryFault, executed, 0};
            }
            ++executed;
            return {ExecutionStopReason::GuestHalted, executed, 0};
        }

        return {ExecutionStopReason::UnsupportedInstruction, executed, opcode};
    }

    return {ExecutionStopReason::InstructionBudgetExhausted, executed, 0};
}

} // namespace xboxlab
