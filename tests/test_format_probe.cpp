#include "xboxlab/format_probe.hpp"
#include "xboxlab/guest_memory.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <limits>

int main() {
    using xboxlab::FileKind;
    using xboxlab::probe_bytes;

    assert(probe_bytes({0x4D, 0x5A}).kind == FileKind::PeOrMzExecutable);
    assert(probe_bytes({0x58, 0x45, 0x58, 0x32}).kind == FileKind::Xex2Container);
    assert(probe_bytes({0x50, 0x4B, 0x03, 0x04}).kind == FileKind::ZipPackage);
    assert(probe_bytes({0x7F, 0x45, 0x4C, 0x46}).kind == FileKind::ElfExecutable);
    assert(probe_bytes({0x00, 0x01, 0x02, 0x03}).kind == FileKind::Unknown);
    assert(probe_bytes({}).kind == FileKind::Unknown);

    xboxlab::GuestMemory memory;
    assert(memory.region_count() == 0);
    assert(!memory.map_region(0x1000, 0));
    assert(memory.map_region(0x1000, 16));
    assert(!memory.map_region(0x1008, 16));
    assert(memory.map_region(0x2000, 8, true, false));
    assert(!memory.map_region(std::numeric_limits<std::uint64_t>::max() - 2, 8));

    const std::array<std::uint8_t, 4> input{0x78, 0x56, 0x34, 0x12};
    assert(memory.write(0x1004, input));
    std::array<std::uint8_t, 4> output{};
    assert(memory.read(0x1004, output));
    assert(output == input);

    std::uint32_t value = 0;
    assert(memory.read_u32_le(0x1004, value));
    assert(value == 0x12345678U);
    assert(!memory.read(0x100E, output));
    assert(!memory.write(0x2000, input));
    assert(!memory.read(0x3000, output));
    assert(memory.region_count() == 2);
    return 0;
}
