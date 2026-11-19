#include "xboxlab/format_probe.hpp"
#include "xboxlab/guest_memory.hpp"
#include "xboxlab/package_probe.hpp"
#include "xboxlab/pe_inspector.hpp"

#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdint>
#include <limits>
#include <vector>

#define CHECK(condition) do { \
    if (!(condition)) { \
        std::cerr << "CHECK failed: " #condition << " at " << __FILE__ \
                  << ':' << __LINE__ << std::endl; \
        std::abort(); \
    } \
} while (false)

namespace {
void put_u16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[offset + 1] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
}
void put_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value) {
    for (unsigned int i = 0; i < 4; ++i) {
        bytes[offset + i] = static_cast<std::uint8_t>((value >> (i * 8U)) & 0xFFU);
    }
}
} // namespace

int main() {
    using xboxlab::FileKind;
    using xboxlab::probe_bytes;

    CHECK(probe_bytes({0x4D, 0x5A}).kind == FileKind::PeOrMzExecutable);
    CHECK(probe_bytes({0x58, 0x45, 0x58, 0x32}).kind == FileKind::Xex2Container);
    CHECK(probe_bytes({0x50, 0x4B, 0x03, 0x04}).kind == FileKind::ZipPackage);
    CHECK(probe_bytes({0x7F, 0x45, 0x4C, 0x46}).kind == FileKind::ElfExecutable);
    CHECK(probe_bytes({0x00, 0x01, 0x02, 0x03}).kind == FileKind::Unknown);
    CHECK(probe_bytes({}).kind == FileKind::Unknown);

    // Microsoft documents _x.xvc for Xbox One and _xs.xvc for Series X|S.
    // These are filename-only checks using synthetic names, not game packages.
    using xboxlab::ConsolePackageFamily;
    const auto xbox_one = xboxlab::probe_console_package_name("Sample_x.xvc");
    CHECK(xbox_one.family == ConsolePackageFamily::XboxOne);
    const auto series = xboxlab::probe_console_package_name("Sample_xs.xvc");
    CHECK(series.family == ConsolePackageFamily::XboxSeriesXs);
    const auto uppercase = xboxlab::probe_console_package_name("SAMPLE_XS.XVC");
    CHECK(uppercase.family == ConsolePackageFamily::XboxSeriesXs);
    const auto generic_xvc = xboxlab::probe_console_package_name("Sample.xvc");
    CHECK(generic_xvc.family == ConsolePackageFamily::Unknown);
    const auto pc_package = xboxlab::probe_console_package_name("Sample.msixvc");
    CHECK(pc_package.family == ConsolePackageFamily::Unknown);
    const auto misleading = xboxlab::probe_console_package_name("Sample_xs.zip");
    CHECK(misleading.family == ConsolePackageFamily::Unknown);

    // Minimal synthetic PE32+ header; no copyrighted or executable fixture is used.
    std::vector<std::uint8_t> pe(0x180, 0);
    pe[0] = 0x4D; pe[1] = 0x5A;
    put_u32(pe, 0x3C, 0x40);
    pe[0x40] = 0x50; pe[0x41] = 0x45;
    const std::size_t coff = 0x44;
    put_u16(pe, coff, 0x8664);
    put_u16(pe, coff + 2, 3);
    put_u32(pe, coff + 4, 123456U);
    put_u16(pe, coff + 16, 112);
    put_u16(pe, coff + 18, 0x0022);
    put_u16(pe, coff + 20, 0x020B);
    put_u16(pe, coff + 20 + 68, 3);
    const std::size_t section_table = coff + 20 + 70;
    pe[section_table] = 't'; pe[section_table + 1] = 'e';
    pe[section_table + 2] = 'x'; pe[section_table + 3] = 't';
    put_u32(pe, section_table + 8, 0x200);
    put_u32(pe, section_table + 12, 0x1000);
    put_u32(pe, section_table + 16, 0x200);
    put_u32(pe, section_table + 20, 0x400);
    put_u32(pe, section_table + 36, 0x60000020U);

    const auto parsed = xboxlab::inspect_pe_bytes(pe.data(), pe.size());
    CHECK(parsed.valid);
    CHECK(parsed.machine == 0x8664);
    CHECK(parsed.machine_name == "x86-64");
    CHECK(parsed.section_count == 3);
    CHECK(parsed.timestamp == 123456U);
    CHECK(parsed.optional_header_name == "PE32+");
    CHECK(parsed.has_subsystem && parsed.subsystem == 3);
    CHECK(parsed.sections.size() == 3);
    CHECK(parsed.sections[0].name == "text");
    CHECK(parsed.sections[0].virtual_address == 0x1000);
    CHECK(parsed.sections[0].raw_offset == 0x400);
    CHECK(parsed.sections[0].characteristics == 0x60000020U);

    CHECK(!xboxlab::inspect_pe_bytes(pe.data(), 20).valid);
    auto bad_offset = pe;
    put_u32(bad_offset, 0x3C, 0xFFFFFFF0U);
    CHECK(!xboxlab::inspect_pe_bytes(bad_offset.data(), bad_offset.size()).valid);
    auto bad_signature = pe;
    bad_signature[0x40] = 0;
    CHECK(!xboxlab::inspect_pe_bytes(bad_signature.data(), bad_signature.size()).valid);
    auto overlapping_pe_offset = pe;
    put_u32(overlapping_pe_offset, 0x3C, 2);
    CHECK(!xboxlab::inspect_pe_bytes(overlapping_pe_offset.data(), overlapping_pe_offset.size()).valid);
    auto truncated_sections = pe;
    truncated_sections.resize(section_table + 20);
    CHECK(!xboxlab::inspect_pe_bytes(truncated_sections.data(), truncated_sections.size()).valid);
    auto excessive_sections = pe;
    put_u16(excessive_sections, coff + 2, 0xFFFF);
    CHECK(!xboxlab::inspect_pe_bytes(excessive_sections.data(), excessive_sections.size()).valid);
    auto truncated_optional = pe;
    put_u16(truncated_optional, coff + 16, 0xFFFF);
    CHECK(!xboxlab::inspect_pe_bytes(truncated_optional.data(), truncated_optional.size()).valid);
    auto missing_optional = pe;
    put_u16(missing_optional, coff + 16, 0);
    CHECK(!xboxlab::inspect_pe_bytes(missing_optional.data(), missing_optional.size()).valid);
    auto short_optional = pe;
    put_u16(short_optional, coff + 16, 70);
    CHECK(!xboxlab::inspect_pe_bytes(short_optional.data(), short_optional.size()).valid);
    auto unknown_optional_magic = pe;
    put_u16(unknown_optional_magic, coff + 20, 0x9999);
    CHECK(!xboxlab::inspect_pe_bytes(unknown_optional_magic.data(), unknown_optional_magic.size()).valid);
    CHECK(!xboxlab::inspect_pe_bytes(nullptr, pe.size()).valid);

    // Large executable payloads must not prevent bounded header inspection.
    const auto large_fixture_path =
        std::filesystem::temp_directory_path() / "any_xbox_large_pe_fixture.bin";
    auto large_fixture = pe;
    large_fixture.resize(1024U * 1024U + 128U, 0xAA);
    {
        std::ofstream fixture(large_fixture_path, std::ios::binary);
        CHECK(static_cast<bool>(fixture));
        fixture.write(reinterpret_cast<const char*>(large_fixture.data()),
                      static_cast<std::streamsize>(large_fixture.size()));
        CHECK(static_cast<bool>(fixture));
    }
    CHECK(xboxlab::inspect_pe_file(large_fixture_path).valid);
    std::filesystem::remove(large_fixture_path);

    // Deterministic malformed-input smoke corpus for parser robustness.
    const std::size_t minimum_pe_size = section_table + 3 * 40;
    for (std::size_t length = 0; length < minimum_pe_size; ++length) {
        CHECK(!xboxlab::inspect_pe_bytes(pe.data(), length).valid);
    }
    std::uint32_t fuzz_state = 0xA5C31E27U;
    for (std::size_t sample = 0; sample < 1024; ++sample) {
        const std::size_t length = sample % 257;
        std::vector<std::uint8_t> fuzz_bytes(length);
        for (auto& byte : fuzz_bytes) {
            fuzz_state ^= fuzz_state << 13U;
            fuzz_state ^= fuzz_state >> 17U;
            fuzz_state ^= fuzz_state << 5U;
            byte = static_cast<std::uint8_t>(fuzz_state & 0xFFU);
        }
        (void)xboxlab::inspect_pe_bytes(fuzz_bytes.data(), fuzz_bytes.size());
    }

    xboxlab::GuestMemory memory;
    CHECK(memory.region_count() == 0);
    CHECK(!memory.map_region(0x1000, 0));
    CHECK(memory.map_region(0x1000, 16));
    CHECK(!memory.map_region(0x1008, 16));
    CHECK(memory.map_region(0x2000, 8, true, false));
    CHECK(!memory.map_region(std::numeric_limits<std::uint64_t>::max() - 2, 8));

    const std::array<std::uint8_t, 4> input{0x78, 0x56, 0x34, 0x12};
    CHECK(memory.write(0x1004, input));
    std::array<std::uint8_t, 4> output{};
    CHECK(memory.read(0x1004, output));
    CHECK(output == input);

    std::uint32_t value = 0;
    CHECK(memory.read_u32_le(0x1004, value));
    CHECK(value == 0x12345678U);
    CHECK(!memory.read(0x100E, output));
    CHECK(!memory.write(0x2000, input));
    CHECK(!memory.read(0x3000, output));
    CHECK(memory.region_count() == 2);
    return 0;
}
