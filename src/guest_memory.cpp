#include "xboxlab/guest_memory.hpp"

#include <algorithm>
#include <limits>

namespace xboxlab {

bool GuestMemory::map_region(std::uint64_t guest_base,
                             std::size_t size,
                             bool readable,
                             bool writable,
                             bool executable) {
    if (size == 0 || size > std::numeric_limits<std::uint64_t>::max() - guest_base) {
        return false;
    }

    const auto end = guest_base + static_cast<std::uint64_t>(size);
    auto next = regions_.lower_bound(guest_base);

    if (next != regions_.end() && next->first < end) {
        return false;
    }
    if (next != regions_.begin()) {
        const auto previous = std::prev(next);
        const auto previous_end = previous->first +
            static_cast<std::uint64_t>(previous->second.bytes.size());
        if (previous_end > guest_base) {
            return false;
        }
    }

    Region region;
    region.bytes.resize(size, 0);
    region.readable = readable;
    region.writable = writable;
    region.executable = executable;
    regions_.emplace(guest_base, std::move(region));
    return true;
}

auto GuestMemory::find_region(std::uint64_t address) const {
    auto it = regions_.upper_bound(address);
    if (it == regions_.begin()) return regions_.end();
    --it;
    const auto offset = address - it->first;
    if (offset >= it->second.bytes.size()) return regions_.end();
    return it;
}

auto GuestMemory::find_region(std::uint64_t address) {
    auto it = regions_.upper_bound(address);
    if (it == regions_.begin()) return regions_.end();
    --it;
    const auto offset = address - it->first;
    if (offset >= it->second.bytes.size()) return regions_.end();
    return it;
}

bool GuestMemory::protect_region(std::uint64_t guest_base,
                                  bool readable,
                                  bool writable,
                                  bool executable) noexcept {
    const auto it = regions_.find(guest_base);
    if (it == regions_.end()) return false;
    it->second.readable = readable;
    it->second.writable = writable;
    it->second.executable = executable;
    return true;
}

bool GuestMemory::read(std::uint64_t guest_address,
                       std::span<std::uint8_t> destination) const {
    if (destination.empty()) return true;
    const auto it = find_region(guest_address);
    if (it == regions_.end() || !it->second.readable) return false;

    const auto offset = static_cast<std::size_t>(guest_address - it->first);
    if (destination.size() > it->second.bytes.size() - offset) return false;
    std::copy_n(it->second.bytes.begin() + static_cast<std::ptrdiff_t>(offset),
                destination.size(), destination.begin());
    return true;
}

bool GuestMemory::write(std::uint64_t guest_address,
                        std::span<const std::uint8_t> source) {
    if (source.empty()) return true;
    const auto it = find_region(guest_address);
    if (it == regions_.end() || !it->second.writable) return false;

    const auto offset = static_cast<std::size_t>(guest_address - it->first);
    if (source.size() > it->second.bytes.size() - offset) return false;
    std::copy(source.begin(), source.end(),
              it->second.bytes.begin() + static_cast<std::ptrdiff_t>(offset));
    return true;
}

bool GuestMemory::read_u32_le(std::uint64_t guest_address,
                              std::uint32_t& value) const {
    std::uint8_t bytes[4]{};
    if (!read(guest_address, bytes)) return false;
    value = static_cast<std::uint32_t>(bytes[0]) |
            (static_cast<std::uint32_t>(bytes[1]) << 8U) |
            (static_cast<std::uint32_t>(bytes[2]) << 16U) |
            (static_cast<std::uint32_t>(bytes[3]) << 24U);
    return true;
}

bool GuestMemory::fetch(std::uint64_t guest_address,
                         std::span<std::uint8_t> destination) const {
    if (destination.empty()) return true;
    const auto it = find_region(guest_address);
    if (it == regions_.end() || !it->second.executable) return false;

    const auto offset = static_cast<std::size_t>(guest_address - it->first);
    if (destination.size() > it->second.bytes.size() - offset) return false;
    std::copy_n(it->second.bytes.begin() + static_cast<std::ptrdiff_t>(offset),
                destination.size(), destination.begin());
    return true;
}

bool GuestMemory::fetch_u32_le(std::uint64_t guest_address,
                               std::uint32_t& value) const {
    std::uint8_t bytes[4]{};
    if (!fetch(guest_address, bytes)) return false;
    value = static_cast<std::uint32_t>(bytes[0]) |
            (static_cast<std::uint32_t>(bytes[1]) << 8U) |
            (static_cast<std::uint32_t>(bytes[2]) << 16U) |
            (static_cast<std::uint32_t>(bytes[3]) << 24U);
    return true;
}

std::size_t GuestMemory::region_count() const noexcept {
    return regions_.size();
}

} // namespace xboxlab
