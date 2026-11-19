#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace xboxlab {

enum class FileKind {
    PeOrMzExecutable,
    Xex2Container,
    ZipPackage,
    ElfExecutable,
    Unknown
};

struct ProbeResult {
    FileKind kind{FileKind::Unknown};
    std::string label{"Unknown file"};
    std::string note{"No recognised signature was found."};
};

[[nodiscard]] ProbeResult probe_bytes(const std::vector<std::uint8_t>& bytes);
[[nodiscard]] ProbeResult probe_file(const std::filesystem::path& path);

} // namespace xboxlab
