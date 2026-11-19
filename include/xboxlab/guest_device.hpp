#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace xboxlab {

// A host-independent interface for a synthetic guest-mapped device.
// Implementations own their state; the bus stores non-owning pointers.
class GuestDevice {
public:
    virtual ~GuestDevice() = default;

    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    [[nodiscard]] virtual std::uint64_t base_address() const noexcept = 0;
    [[nodiscard]] virtual std::size_t size() const noexcept = 0;

    [[nodiscard]] virtual bool read(
        std::uint64_t offset, std::span<std::uint8_t> destination) = 0;
    [[nodiscard]] virtual bool write(
        std::uint64_t offset, std::span<const std::uint8_t> source) = 0;
};

class GuestDeviceBus {
public:
    // Registration rejects null/empty/overflowing/overlapping device ranges.
    [[nodiscard]] bool attach(GuestDevice* device);

    [[nodiscard]] bool read(
        std::uint64_t guest_address, std::span<std::uint8_t> destination);
    [[nodiscard]] bool write(
        std::uint64_t guest_address, std::span<const std::uint8_t> source);

    [[nodiscard]] std::size_t device_count() const noexcept;
    void clear() noexcept;

private:
    [[nodiscard]] GuestDevice* find_device(
        std::uint64_t address, std::size_t length) const noexcept;

    std::vector<GuestDevice*> devices_;
};

} // namespace xboxlab
