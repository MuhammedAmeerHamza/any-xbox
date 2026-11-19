#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace xboxlab {

struct ManifestExecutable {
    std::string name;
    std::string id;
    std::string target_device_family;
    bool development_only{false};
};

struct GameManifest {
    bool valid{false};
    std::string error{"Not inspected"};
    std::string config_version;
    std::string identity_name;
    std::string publisher;
    std::string version;
    std::string display_name;
    std::string publisher_display_name;
    std::vector<ManifestExecutable> executables;
};

// Parses a conservative, bounded subset of the public MicrosoftGame.config XML
// manifest. It never opens an XVC package or resolves external entities.
[[nodiscard]] GameManifest inspect_game_manifest(std::string_view xml);

} // namespace xboxlab
