#include "xboxlab/manifest_probe.hpp"
#include "xboxlab/pe_inspector.hpp"

#include <cstddef>
#include <cstdint>
#include <string_view>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    (void)xboxlab::inspect_pe_bytes(data, size);
    (void)xboxlab::inspect_game_manifest(
        std::string_view(reinterpret_cast<const char*>(data), size));
    return 0;
}
