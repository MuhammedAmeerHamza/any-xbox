#include "xboxlab/package_probe.hpp"

#include <algorithm>
#include <string>

namespace xboxlab {
namespace {

std::string lowercase(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char ch) {
                       if (ch >= 'A' && ch <= 'Z') {
                           return static_cast<char>(ch - 'A' + 'a');
                       }
                       return static_cast<char>(ch);
                   });
    return value;
}

bool ends_with(const std::string& value, const std::string& suffix) {
    return value.size() >= suffix.size() &&
           value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

} // namespace

PackageProbeResult probe_console_package_name(const std::filesystem::path& path) {
    const std::string filename = lowercase(path.filename().string());

    if (!ends_with(filename, ".xvc")) {
        return {ConsolePackageFamily::Unknown,
                "Not a recognised Xbox console XVC filename",
                "The documented console package extension is .xvc; no content was opened."};
    }

    const std::string stem = filename.substr(0, filename.size() - 4);
    if (ends_with(stem, "_xs")) {
        return {ConsolePackageFamily::XboxSeriesXs,
                "Xbox Series X|S package naming convention",
                "The _xs.xvc suffix matches Microsoft's documented Series X|S package naming convention. "
                "This filename check does not validate package contents."};
    }
    if (ends_with(stem, "_x")) {
        return {ConsolePackageFamily::XboxOne,
                "Xbox One family package naming convention",
                "The _x.xvc suffix matches Microsoft's documented Xbox One package naming convention. "
                "This filename check does not validate package contents."};
    }

    return {ConsolePackageFamily::Unknown,
            "XVC package; console generation not identified",
            "The .xvc extension identifies the console package format, but the filename has no "
            "recognised _x or _xs generation suffix."};
}

} // namespace xboxlab
