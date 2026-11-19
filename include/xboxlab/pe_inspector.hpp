#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace xboxlab {

struct PeSection {
    std::string name;
    std::uint32_t virtual_size{0};
    std::uint32_t virtual_address{0};
    std::uint32_t raw_size{0};
    std::uint32_t raw_offset{0};
    std::uint32_t characteristics{0};
};

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
    std::vector<PeSection> sections;
};

// Parses bounded public PE/COFF headers and section-table metadata only.
// It does not load, modify, decrypt, or execute code.
[[nodiscard]] PeInspection inspect_pe_bytes(const std::uint8_t* data, std::size_t size);
[[nodiscard]] PeInspection inspect_pe_file(const std::filesystem::path& path);

} // namespace xboxlab
