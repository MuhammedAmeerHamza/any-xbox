#include "xboxlab/guest_memory.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <limits>

int main() {
    xboxlab::GuestMemory memory;
    assert(memory.region_count() == 0);
    assert(!memory.map_region(0x1000, 0));
    assert(memory.map_region(0x1000, 16));
    assert(!memory.map_region(0x1008, 16)); // overlap
    assert(memory.map_region(0x2000, 8, true, false)); // read-only
    assert(!memory.map_region(std::numeric_limits<std::uint64_t>::max() - 2, 8));

    const std::array<std::uint8_t, 4> input{0x78, 0x56, 0x34, 0x12};
    assert(memory.write(0x1004, input));

    std::array<std::uint8_t, 4> output{};
    assert(memory.read(0x1004, output));
    assert(output == input);

    std::uint32_t value = 0;
    assert(memory.read_u32_le(0x1004, value));
    assert(value == 0x12345678U);

    assert(!memory.read(0x100E, output)); // crosses region boundary
    assert(!memory.write(0x2000, input)); // read-only region
    assert(!memory.read(0x3000, output)); // unmapped address
    assert(memory.region_count() == 2);
    return 0;
}
