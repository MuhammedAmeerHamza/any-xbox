#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace xboxlab {

// A normalized interrupt request for the future guest runtime.
// Vector meanings and hardware priority are intentionally not assigned here.
struct GuestInterrupt {
    std::uint8_t vector{0};
    std::uint32_t source{0};
};

class GuestInterruptController {
public:
    static constexpr std::size_t kCapacity = 64;

    // Returns false when the bounded queue is full.
    [[nodiscard]] bool request(GuestInterrupt interrupt) noexcept;

    // Removes the oldest pending request. Returns false when empty.
    [[nodiscard]] bool acknowledge(GuestInterrupt& interrupt) noexcept;

    [[nodiscard]] std::size_t pending_count() const noexcept;
    [[nodiscard]] bool empty() const noexcept;
    void reset() noexcept;

private:
    std::array<GuestInterrupt, kCapacity> queue_{};
    std::size_t head_{0};
    std::size_t count_{0};
};

} // namespace xboxlab
