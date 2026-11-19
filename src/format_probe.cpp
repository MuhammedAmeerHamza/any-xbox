#include "xboxlab/format_probe.hpp"

#include <array>
#include <fstream>
#include <iterator>

namespace xboxlab {
namespace {
bool starts_with(const std::vector<std::uint8_t>& bytes,
                 const std::array<std::uint8_t, 4>& signature) {
    return bytes.size() >= signature.size() &&
           std::equal(signature.begin(), signature.end(), bytes.begin());
}
} // namespace

ProbeResult probe_bytes(const std::vector<std::uint8_t>& bytes) {
    if (bytes.size() >= 2 && bytes[0] == 0x4D && bytes[1] == 0x5A) {
        return {FileKind::PeOrMzExecutable, "PE/DOS MZ executable",
                "MZ is a common executable header. This does not establish that the file is an Xbox title."};
    }
    if (starts_with(bytes, {0x58, 0x45, 0x58, 0x32})) {
        return {FileKind::Xex2Container, "XEX2 container signature",
                "Recognised the XEX2 magic only; no container parsing or execution is performed."};
    }
    if (starts_with(bytes, {0x50, 0x4B, 0x03, 0x04}) ||
        starts_with(bytes, {0x50, 0x4B, 0x05, 0x06}) ||
        starts_with(bytes, {0x50, 0x4B, 0x07, 0x08})) {
        return {FileKind::ZipPackage, "ZIP-based package",
                "Could be a generic ZIP archive or an application package; contents are not inspected."};
    }
    if (starts_with(bytes, {0x7F, 0x45, 0x4C, 0x46})) {
        return {FileKind::ElfExecutable, "ELF executable",
                "Recognised the ELF magic; architecture and validity have not been checked."};
    }
    return {};
}

ProbeResult probe_file(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return {FileKind::Unknown, "Unable to read file",
                "Check that the path exists and that the process has permission to read it."};
    }
    std::vector<std::uint8_t> header(16);
    input.read(reinterpret_cast<char*>(header.data()),
               static_cast<std::streamsize>(header.size()));
    header.resize(static_cast<std::size_t>(input.gcount()));
    return probe_bytes(header);
}

} // namespace xboxlab
