#pragma once

#include <cstdint>

namespace xboxlab {

// Normalized exception categories for the early guest-runtime model.
// These are not a complete x86 or Xbox exception/interrupt specification.
enum class GuestExceptionCode : std::uint8_t {
    InvalidOpcode,
    GeneralProtection,
    PageFault,
    AccessViolation,
    DivideByZero,
    Breakpoint,
    Unknown
};

struct GuestException {
    GuestExceptionCode code{GuestExceptionCode::Unknown};
    std::uint64_t address{0};
};

class GuestExceptionState {
public:
    // Only one exception may be pending. A second raise fails until cleared.
    [[nodiscard]] bool raise(GuestExceptionCode code,
                             std::uint64_t address) noexcept;
    [[nodiscard]] bool has_pending() const noexcept;
    [[nodiscard]] bool pending(GuestException& exception) const noexcept;
    void clear() noexcept;

private:
    GuestException exception_{};
    bool pending_{false};
};

} // namespace xboxlab
