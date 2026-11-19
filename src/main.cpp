#include "xboxlab/format_probe.hpp"
#include "xboxlab/pe_inspector.hpp"

#include <iomanip>
#include <iostream>
#include <string>

namespace {
void print_help() {
    std::cout
        << "Xbox Compatibility Research Lab 0.3.0\n"
        << "Experimental research scaffold — not an Xbox emulator.\n\n"
        << "Usage:\n"
        << "  xboxlab --help             Show this help\n"
        << "  xboxlab --inspect <file>   Inspect a file signature\n"
        << "  xboxlab --inspect-pe <file> Read basic PE/COFF header metadata\n\n"
        << "PE metadata does not identify an Xbox title or prove compatibility.\n"
        << "No game files are modified, decrypted, or executed.\n";
}

int inspect_pe(const char* filename) {
    const auto result = xboxlab::inspect_pe_file(filename);
    if (!result.valid) {
        std::cerr << "PE inspection failed: " << result.error << "\n";
        return 2;
    }

    std::cout << "PE/COFF header: valid (basic structural checks)\n"
              << "Machine: " << result.machine_name << " (0x"
              << std::hex << result.machine << std::dec << ")\n"
              << "Sections: " << result.section_count << "\n"
              << "COFF timestamp: " << result.timestamp << "\n"
              << "Optional header: " << result.optional_header_name << "\n"
              << "Characteristics: 0x" << std::hex << result.characteristics
              << std::dec << "\n";
    if (result.has_subsystem) {
        std::cout << "Subsystem value: " << result.subsystem << "\n";
    } else {
        std::cout << "Subsystem value: not present in supplied optional header\n";
    }
    std::cout << "Target note: PE metadata alone cannot distinguish Xbox One / Series X|S "
                 "software from ordinary Windows software.\n";
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
        std::cout << "Result: " << result.label << "\n"
                  << result.note << "\n";
        return result.label == "Unable to read file" ? 2 : 0;
    }
    if (argc == 3 && std::string(argv[1]) == "--inspect-pe") {
        return inspect_pe(argv[2]);
    }
    print_help();
    return argc == 1 ? 0 : 1;
}
