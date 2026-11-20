#include "xboxlab/format_probe.hpp"
#include "xboxlab/package_probe.hpp"
#include "xboxlab/pe_inspector.hpp"
#include "xboxlab/manifest_probe.hpp"
#include "xboxlab/x64_interpreter.hpp"

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


int play_demo() {
    constexpr std::uint64_t code_base = 0x1000;
    constexpr std::array<std::uint8_t, 19> program{
        0x3D, 0x07, 0x00, 0x00, 0x00, // CMP EAX, 7
        0x74, 0x06,                   // JE +6
        0xB8, 0x00, 0x00, 0x00, 0x00, // MOV EAX, 0
        0xF4,                         // HLT
        0xB8, 0x01, 0x00, 0x00, 0x00, // MOV EAX, 1
        0xF4                          // HLT
    };

    xboxlab::GuestMemory memory;
    if (!memory.map_region(code_base, 64) || !memory.write(code_base, program)) {
        std::cerr << "Demo setup failed: unable to map guest program.\\n";
        return 2;
    }

    xboxlab::X64InterpreterBackend interpreter;
    xboxlab::GuestCpuState cpu;
    std::cout << "Any Xbox — Guest Execution Demo\\n"
              << "A tiny synthetic x86-64 guest program is running in our interpreter.\\n"
              << "Guess the secret number (1–10). You have 5 attempts.\\n";

    for (int attempt = 1; attempt <= 5; ++attempt) {
        std::cout << "Attempt " << attempt << "/5 — your guess: ";
        std::string line;
        if (!std::getline(std::cin, line)) {
            std::cout << "\\nInput ended. Demo stopped.\\n";
            return 0;
        }

        int guess = 0;
        try {
            std::size_t parsed = 0;
            guess = std::stoi(line, &parsed);
            if (parsed != line.size() || guess < 1 || guess > 10) {
                std::cout << "Enter a whole number from 1 to 10.\\n";
                --attempt;
                continue;
            }
        } catch (...) {
            std::cout << "Enter a whole number from 1 to 10.\\n";
            --attempt;
            continue;
        }

        cpu.reset();
        if (!cpu.write_gpr(xboxlab::X64Gpr::Rax,
                           static_cast<std::uint32_t>(guess))) {
            std::cerr << "Demo failed: could not set guest register.\\n";
            return 2;
        }
        cpu.set_instruction_pointer(code_base);
        const auto result = interpreter.run(cpu, memory, 16);
        if (result.reason != xboxlab::ExecutionStopReason::GuestHalted) {
            std::cerr << "Guest program stopped unexpectedly (reason "
                      << static_cast<unsigned int>(result.reason) << ").\\n";
            return 2;
        }

        std::uint64_t guest_result = 0;
        if (!cpu.read_gpr(xboxlab::X64Gpr::Rax, guest_result)) {
            std::cerr << "Demo failed: could not read guest result.\\n";
            return 2;
        }
        if (guest_result == 1) {
            std::cout << "Correct! The guest program accepted your answer.\\n"
                      << "Executed " << result.instructions_executed
                      << " guest instructions for this attempt.\\n";
            return 0;
        }
        std::cout << (guess < 7 ? "Too low.\\n" : "Too high.\\n")
                  << "Guest instructions executed: " << result.instructions_executed << "\\n";
    }

    std::cout << "No attempts left. The secret number was 7.\\n";
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
    if (argc == 2 && std::string(argv[1]) == "--play-demo") return play_demo();
    if (argc == 3 && std::string(argv[1]) == "--inspect-package") return inspect_package(argv[2]);
    print_help();
    return argc == 1 ? 0 : 1;
}
