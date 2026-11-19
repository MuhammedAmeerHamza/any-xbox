#include "xboxlab/pe_inspector.hpp"

#include <array>
#include <fstream>
#include <iterator>
#include <vector>

namespace xboxlab {
namespace {
constexpr std::size_t kMaximumHeaderRead = 1024U * 1024U;

bool has_range(std::size_t size, std::size_t offset, std::size_t length) {
    return offset <= size && length <= size - offset;
}

std::uint16_t read_u16(const std::uint8_t* data, std::size_t offset) {
    return static_cast<std::uint16_t>(data[offset]) |
           static_cast<std::uint16_t>(static_cast<std::uint16_t>(data[offset + 1]) << 8U);
}

std::uint32_t read_u32(const std::uint8_t* data, std::size_t offset) {
    return static_cast<std::uint32_t>(data[offset]) |
           (static_cast<std::uint32_t>(data[offset + 1]) << 8U) |
           (static_cast<std::uint32_t>(data[offset + 2]) << 16U) |
           (static_cast<std::uint32_t>(data[offset + 3]) << 24U);
}

std::string machine_name(std::uint16_t machine) {
    switch (machine) {
    case 0x014C: return "x86";
    case 0x8664: return "x86-64";
    case 0xAA64: return "ARM64";
    case 0x01C0: return "ARM";
    default: return "Unknown";
    }
}
} // namespace

PeInspection inspect_pe_bytes(const std::uint8_t* data, std::size_t size) {
    PeInspection result;
    if (data == nullptr || !has_range(size, 0, 64)) {
        result.error = "Input is too small for a DOS header.";
        return result;
    }
    if (data[0] != 0x4D || data[1] != 0x5A) {
        result.error = "Missing MZ DOS signature.";
        return result;
    }

    const std::uint32_t pe_offset_u32 = read_u32(data, 0x3C);
    if (pe_offset_u32 > size) {
        result.error = "PE header offset is outside the supplied data.";
        return result;
    }
    const auto pe_offset = static_cast<std::size_t>(pe_offset_u32);
    if (!has_range(size, pe_offset, 4 + 20)) {
        result.error = "Truncated PE signature or COFF header.";
        return result;
    }
    if (data[pe_offset] != 0x50 || data[pe_offset + 1] != 0x45 ||
        data[pe_offset + 2] != 0x00 || data[pe_offset + 3] != 0x00) {
        result.error = "Missing PE signature at the declared offset.";
        return result;
    }

    const std::size_t coff = pe_offset + 4;
    result.machine = read_u16(data, coff);
    result.machine_name = machine_name(result.machine);
    result.section_count = read_u16(data, coff + 2);
    result.timestamp = read_u32(data, coff + 4);
    const std::uint16_t optional_size = read_u16(data, coff + 16);
    result.characteristics = read_u16(data, coff + 18);

    const std::size_t optional = coff + 20;
    if (optional_size > 0) {
        if (!has_range(size, optional, optional_size)) {
            result.error = "Optional header extends beyond the supplied data.";
            return result;
        }
        if (optional_size >= 2) {
            result.optional_header_magic = read_u16(data, optional);
            if (result.optional_header_magic == 0x010B) {
                result.optional_header_name = "PE32";
            } else if (result.optional_header_magic == 0x020B) {
                result.optional_header_name = "PE32+";
            } else {
                result.optional_header_name = "Unknown optional-header magic";
            }
        }
        if (optional_size >= 70) {
            result.subsystem = read_u16(data, optional + 68);
            result.has_subsystem = true;
        }
    }

    result.valid = true;
    result.error.clear();
    return result;
}

PeInspection inspect_pe_file(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        PeInspection result;
        result.error = "Unable to open file.";
        return result;
    }

    std::vector<std::uint8_t> bytes(kMaximumHeaderRead + 1);
    input.read(reinterpret_cast<char*>(bytes.data()),
               static_cast<std::streamsize>(bytes.size()));
    const auto count = static_cast<std::size_t>(input.gcount());
    if (count > kMaximumHeaderRead) {
        PeInspection result;
        result.error = "Header inspection limit exceeded (1 MiB).";
        return result;
    }
    bytes.resize(count);
    return inspect_pe_bytes(bytes.data(), bytes.size());
}

} // namespace xboxlab
