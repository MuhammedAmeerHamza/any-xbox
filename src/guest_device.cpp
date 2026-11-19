#include "xboxlab/guest_device.hpp"

#include <limits>

namespace xboxlab {
namespace {
bool range_end(std::uint64_t base, std::size_t size, std::uint64_t& end) noexcept {
    if (size == 0 || size > std::numeric_limits<std::uint64_t>::max() - base) {
        return false;
    }
    end = base + static_cast<std::uint64_t>(size);
    return true;
}
} // namespace

bool GuestDeviceBus::attach(GuestDevice* device) {
    if (device == nullptr) {
        return false;
    }
    std::uint64_t new_end = 0;
    if (!range_end(device->base_address(), device->size(), new_end)) {
        return false;
    }
    for (const GuestDevice* existing : devices_) {
        std::uint64_t existing_end = 0;
        if (!range_end(existing->base_address(), existing->size(), existing_end)) {
            return false;
        }
        if (device->base_address() < existing_end &&
            existing->base_address() < new_end) {
            return false;
        }
    }
    devices_.push_back(device);
    return true;
}

GuestDevice* GuestDeviceBus::find_device(
    std::uint64_t address, std::size_t length) const noexcept {
    if (length == 0 ||
        length > std::numeric_limits<std::uint64_t>::max() - address) {
        return nullptr;
    }
    const std::uint64_t access_end = address + static_cast<std::uint64_t>(length);
    for (GuestDevice* device : devices_) {
        std::uint64_t device_end = 0;
        if (!range_end(device->base_address(), device->size(), device_end)) {
            continue;
        }
        if (address >= device->base_address() && access_end <= device_end) {
            return device;
        }
    }
    return nullptr;
}

bool GuestDeviceBus::read(
    std::uint64_t guest_address, std::span<std::uint8_t> destination) {
    GuestDevice* device = find_device(guest_address, destination.size());
    return device != nullptr &&
           device->read(guest_address - device->base_address(), destination);
}

bool GuestDeviceBus::write(
    std::uint64_t guest_address, std::span<const std::uint8_t> source) {
    GuestDevice* device = find_device(guest_address, source.size());
    return device != nullptr &&
           device->write(guest_address - device->base_address(), source);
}

std::size_t GuestDeviceBus::device_count() const noexcept {
    return devices_.size();
}

void GuestDeviceBus::clear() noexcept {
    devices_.clear();
}

} // namespace xboxlab
