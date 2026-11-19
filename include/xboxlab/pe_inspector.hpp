#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>

namespace xboxlab {

struct PeInspection {
    bool valid{false};
    std::string error{"Not inspected"};
    std::uint16_t machine{0};
    std::string machine_name{"Unknown"};
    std::uint16_t section_count{0};
    std::uint32_t timestamp{0};
    std::uint16_t optional_header_magic{0};
    std::string optional_header_name{"Unknown"};
    std::uint16_t characteristics{0};
    std::uint16_t subsystem{0};
    bool has_subsystem{false};
};

// Parses only bounded, public PE/COFF header fields. It does not load or execute code.
[[nodiscard]] PeInspection inspect_pe_bytes(const std::uint8_t* data, std::size_t size);
[[nodiscard]] PeInspection inspect_pe_file(const std::filesystem::path& path);

} // namespace xboxlab
