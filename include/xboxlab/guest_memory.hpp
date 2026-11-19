#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <span>
#include <vector>

namespace xboxlab {

// A deliberately small, host-independent guest address-space model.
// It stores test data only; it does not execute guest code or map host memory.
class GuestMemory {
public:
    [[nodiscard]] bool map_region(std::uint64_t guest_base,
                                  std::size_t size,
                                  bool readable = true,
                                  bool writable = true);

    [[nodiscard]] bool read(std::uint64_t guest_address,
                            std::span<std::uint8_t> destination) const;
    [[nodiscard]] bool write(std::uint64_t guest_address,
                             std::span<const std::uint8_t> source);

    [[nodiscard]] bool read_u32_le(std::uint64_t guest_address,
                                   std::uint32_t& value) const;
    [[nodiscard]] std::size_t region_count() const noexcept;

private:
    struct Region {
        std::vector<std::uint8_t> bytes;
        bool readable{true};
        bool writable{true};
    };

    [[nodiscard]] auto find_region(std::uint64_t address) const;
    [[nodiscard]] auto find_region(std::uint64_t address);

    std::map<std::uint64_t, Region> regions_;
};

} // namespace xboxlab
