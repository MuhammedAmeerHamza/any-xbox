#pragma once

#include "xboxlab/execution_backend.hpp"

namespace xboxlab {

// A deliberately small interpreter for a documented subset of x86-64.
// It executes bytes from GuestMemory only; it never executes host code.
class X64InterpreterBackend final : public ExecutionBackend {
public:
    [[nodiscard]] std::string_view name() const noexcept override;
    [[nodiscard]] ExecutionResult run(
        GuestCpuState& cpu,
        GuestMemory& memory,
        std::uint64_t instruction_budget) override;
};

} // namespace xboxlab
