#include "xboxlab/format_probe.hpp"

#include <iostream>
#include <string>

namespace {
void print_help() {
    std::cout
        << "Xbox Compatibility Research Lab 0.1.0\n"
        << "Experimental research scaffold — not a game emulator.\n\n"
        << "Usage:\n"
        << "  xboxlab --help             Show this help\n"
        << "  xboxlab --inspect <file>   Inspect a file's first bytes\n\n"
        << "No game files are modified, decrypted, or executed.\n";
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
    print_help();
    return argc == 1 ? 0 : 1;
}
