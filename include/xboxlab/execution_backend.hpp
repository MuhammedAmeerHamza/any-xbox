#pragma once

#include "xboxlab/guest_cpu.hpp"
#include "xboxlab/guest_memory.hpp"

#include <cstdint>
#include <string_view>

namespace xboxlab {

// A backend reports why a bounded guest execution request stopped.
// These values describe a contract; they do not imply that a backend exists.
enum class ExecutionStopReason : std::uint8_t {
    InstructionBudgetExhausted,
    GuestHalted,
    UnsupportedInstruction,
    GuestMemoryFault,
    GuestException,
    BackendUnavailable,
    InvalidRequest
};

struct ExecutionResult {
    ExecutionStopReason reason{ExecutionStopReason::BackendUnavailable};
    std::uint64_t instructions_executed{0};
    std::uint32_t exception_code{0};
};

// Future execution engines must implement this interface. Implementations
// must enforce the instruction budget and report guest faults explicitly.
// The current project provides no production execution backend.
class ExecutionBackend {
public:
    virtual ~ExecutionBackend() = default;
    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    // A zero instruction budget is invalid. A backend must never silently
    // treat a host exception or unsupported guest operation as success.
    [[nodiscard]] virtual ExecutionResult run(
        GuestCpuState& cpu,
        GuestMemory& memory,
        std::uint64_t instruction_budget) = 0;
};

} // namespace xboxlab
