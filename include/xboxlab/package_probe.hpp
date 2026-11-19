#pragma once

#include <filesystem>
#include <string>

namespace xboxlab {

enum class ConsolePackageFamily {
    XboxOne,
    XboxSeriesXs,
    Unknown
};

struct PackageProbeResult {
    ConsolePackageFamily family{ConsolePackageFamily::Unknown};
    std::string label{"Unknown console package family"};
    std::string note{"The filename does not match a documented generation suffix."};
};

// Classifies only the documented XVC filename suffix. It never reads package
// contents, decrypts data, or establishes that a package is valid.
[[nodiscard]] PackageProbeResult probe_console_package_name(
    const std::filesystem::path& path);

} // namespace xboxlab
