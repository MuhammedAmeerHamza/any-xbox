#include "xboxlab/format_probe.hpp"
#include "xboxlab/package_probe.hpp"
#include "xboxlab/pe_inspector.hpp"
#include "xboxlab/manifest_probe.hpp"

#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>

namespace {
void print_help() {
    std::cout
        << "Any Xbox Research Lab 0.9.0\n"
        << "Experimental compatibility research scaffold — not an Xbox emulator.\n\n"
        << "Usage:\n"
        << "  xboxlab --help                  Show this help\n"
        << "  xboxlab --inspect <file>        Inspect a file signature\n"
        << "  xboxlab --inspect-pe <file>     Inspect PE headers and section table\n"
        << "  xboxlab --inspect-package <file> Classify documented XVC filename suffix\n"
        << "  xboxlab --inspect-manifest <file> Inspect bounded MicrosoftGame.config metadata\n\n"
        << "Package classification uses the filename only; package contents are not opened.\n"
        << "Manifest inspection supports a limited documented metadata subset, not full schema validation.\n"
        << "PE metadata does not identify an Xbox title or prove compatibility.\n"
        << "No game files are modified, decrypted, or executed.\n";
}

int inspect_pe(const char* filename) {
    const auto result = xboxlab::inspect_pe_file(filename);
    if (!result.valid) {
        std::cerr << "PE inspection failed: " << result.error << "\n";
        return 2;
    }
    std::cout << "PE/COFF headers and section table: structurally present\n"
              << "Machine: " << result.machine_name << " (0x"
              << std::hex << result.machine << std::dec << ")\n"
              << "Sections: " << result.section_count << "\n"
              << "COFF timestamp: " << result.timestamp << "\n"
              << "Optional header: " << result.optional_header_name << "\n"
              << "Characteristics: 0x" << std::hex << result.characteristics
              << std::dec << "\n";
    if (result.has_subsystem) {
        std::cout << "Subsystem value: " << result.subsystem << "\n";
    }
    for (std::size_t i = 0; i < result.sections.size(); ++i) {
        const auto& section = result.sections[i];
        std::cout << "Section " << i << ": " << section.name
                  << " RVA=0x" << std::hex << section.virtual_address
                  << " virtual-size=0x" << section.virtual_size
                  << " raw-offset=0x" << section.raw_offset
                  << " raw-size=0x" << section.raw_size
                  << " flags=0x" << section.characteristics << std::dec << "\n";
    }
    std::cout << "Target note: PE metadata alone cannot distinguish Xbox One / Series X|S "
                 "software from ordinary Windows software.\n";
    return 0;
}

int inspect_manifest(const char* filename) {
    std::ifstream input(filename, std::ios::binary);
    if (!input) {
        std::cerr << "Manifest inspection failed: unable to open file.\n";
        return 2;
    }
    std::array<char, 64U * 1024U + 1U> bytes{};
    input.read(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    const auto count = static_cast<std::size_t>(input.gcount());
    if (count > 64U * 1024U) {
        std::cerr << "Manifest inspection failed: file exceeds 64 KiB limit.\n";
        return 2;
    }
    const auto result = xboxlab::inspect_game_manifest(std::string_view(bytes.data(), count));
    if (!result.valid) {
        std::cerr << "Manifest inspection failed: " << result.error << "\n";
        return 2;
    }
    std::cout << "MicrosoftGame.config metadata (limited subset)\n"
              << "Config version: " << result.config_version << "\n"
              << "Identity name: " << result.identity_name << "\n"
              << "Publisher: " << result.publisher << "\n"
              << "MSA App ID: " << (result.msa_app_id.empty() ? "not specified" : result.msa_app_id) << "\n"
              << "Title ID: " << (result.title_id.empty() ? "not specified" : result.title_id) << "\n"
              << "Version: " << (result.version.empty() ? "not specified" : result.version) << "\n"
              << "Display name: " << (result.display_name.empty() ? "not specified" : result.display_name) << "\n"
              << "Executable entries: " << result.executables.size() << "\n";
    for (const auto& executable : result.executables) {
        std::cout << "  - " << executable.name
                  << " | ID: " << (executable.id.empty() ? "not specified" : executable.id)
                  << " | TargetDeviceFamily: "
                  << (executable.target_device_family.empty() ? "not specified" : executable.target_device_family)
                  << " | development-only: " << (executable.development_only ? "yes" : "no") << "\n";
    }
    std::cout << "Note: this does not validate an XVC package, signatures, or game compatibility.\n";
    return 0;
}

int inspect_package(const char* filename) {
    const auto result = xboxlab::probe_console_package_name(filename);
    std::cout << "Result: " << result.label << "\n"
              << result.note << "\n";
    return 0;
}
} // namespace

int main(int argc, char** argv) {
    if (argc == 2 && std::string(argv[1]) == "--help") {
        print_help();
        return 0;
    }
    if (argc == 3 && std::string(argv[1]) == "--inspect") {
        const auto result = xboxlab::probe_file(argv[2]);
        std::cout << "Result: " << result.label << "\n" << result.note << "\n";
        return result.label == "Unable to read file" ? 2 : 0;
    }
    if (argc == 3 && std::string(argv[1]) == "--inspect-pe") return inspect_pe(argv[2]);
    if (argc == 3 && std::string(argv[1]) == "--inspect-manifest") return inspect_manifest(argv[2]);
    if (argc == 3 && std::string(argv[1]) == "--inspect-package") return inspect_package(argv[2]);
    print_help();
    return argc == 1 ? 0 : 1;
}
