#include "xboxlab/format_probe.hpp"
#include "xboxlab/guest_memory.hpp"
#include "xboxlab/package_probe.hpp"
#include "xboxlab/pe_inspector.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <vector>

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

    assert(probe_bytes({0x4D, 0x5A}).kind == FileKind::PeOrMzExecutable);
    assert(probe_bytes({0x58, 0x45, 0x58, 0x32}).kind == FileKind::Xex2Container);
    assert(probe_bytes({0x50, 0x4B, 0x03, 0x04}).kind == FileKind::ZipPackage);
    assert(probe_bytes({0x7F, 0x45, 0x4C, 0x46}).kind == FileKind::ElfExecutable);
    assert(probe_bytes({0x00, 0x01, 0x02, 0x03}).kind == FileKind::Unknown);
    assert(probe_bytes({}).kind == FileKind::Unknown);

    // Microsoft documents _x.xvc for Xbox One and _xs.xvc for Series X|S.
    // These are filename-only checks using synthetic names, not game packages.
    using xboxlab::ConsolePackageFamily;
    const auto xbox_one = xboxlab::probe_console_package_name("Sample_x.xvc");
    assert(xbox_one.family == ConsolePackageFamily::XboxOne);
    const auto series = xboxlab::probe_console_package_name("Sample_xs.xvc");
    assert(series.family == ConsolePackageFamily::XboxSeriesXs);
    const auto uppercase = xboxlab::probe_console_package_name("SAMPLE_XS.XVC");
    assert(uppercase.family == ConsolePackageFamily::XboxSeriesXs);
    const auto generic_xvc = xboxlab::probe_console_package_name("Sample.xvc");
    assert(generic_xvc.family == ConsolePackageFamily::Unknown);
    const auto pc_package = xboxlab::probe_console_package_name("Sample.msixvc");
    assert(pc_package.family == ConsolePackageFamily::Unknown);
    const auto misleading = xboxlab::probe_console_package_name("Sample_xs.zip");
    assert(misleading.family == ConsolePackageFamily::Unknown);

    // Minimal synthetic PE32+ header; no copyrighted or executable fixture is used.
    std::vector<std::uint8_t> pe(0x180, 0);
    pe[0] = 0x4D; pe[1] = 0x5A;
    put_u32(pe, 0x3C, 0x40);
    pe[0x40] = 0x50; pe[0x41] = 0x45;
    const std::size_t coff = 0x44;
    put_u16(pe, coff, 0x8664);
    put_u16(pe, coff + 2, 3);
    put_u32(pe, coff + 4, 123456U);
    put_u16(pe, coff + 16, 70);
    put_u16(pe, coff + 18, 0x0022);
    put_u16(pe, coff + 20, 0x020B);
    put_u16(pe, coff + 20 + 68, 3);\n    const std::size_t section_table = coff + 20 + 70;\n    pe[section_table] = 't'; pe[section_table + 1] = 'e';\n    pe[section_table + 2] = 'x'; pe[section_table + 3] = 't';\n    put_u32(pe, section_table + 8, 0x200);\n    put_u32(pe, section_table + 12, 0x1000);\n    put_u32(pe, section_table + 16, 0x200);\n    put_u32(pe, section_table + 20, 0x400);\n    put_u32(pe, section_table + 36, 0x60000020U);

    const auto parsed = xboxlab::inspect_pe_bytes(pe.data(), pe.size());
    assert(parsed.valid);
    assert(parsed.machine == 0x8664);
    assert(parsed.machine_name == "x86-64");
    assert(parsed.section_count == 3);
    assert(parsed.timestamp == 123456U);
    assert(parsed.optional_header_name == "PE32+");
    assert(parsed.has_subsystem && parsed.subsystem == 3);\n    assert(parsed.sections.size() == 3);\n    assert(parsed.sections[0].name == "text");\n    assert(parsed.sections[0].virtual_address == 0x1000);\n    assert(parsed.sections[0].raw_offset == 0x400);\n    assert(parsed.sections[0].characteristics == 0x60000020U);

    assert(!xboxlab::inspect_pe_bytes(pe.data(), 20).valid);
    auto bad_offset = pe;
    put_u32(bad_offset, 0x3C, 0xFFFFFFF0U);
    assert(!xboxlab::inspect_pe_bytes(bad_offset.data(), bad_offset.size()).valid);
    auto bad_signature = pe;
    bad_signature[0x40] = 0;
    assert(!xboxlab::inspect_pe_bytes(bad_signature.data(), bad_signature.size()).valid);\n    auto truncated_sections = pe;\n    truncated_sections.resize(section_table + 20);\n    assert(!xboxlab::inspect_pe_bytes(truncated_sections.data(), truncated_sections.size()).valid);\n    auto excessive_sections = pe;\n    put_u16(excessive_sections, coff + 2, 0xFFFF);\n    assert(!xboxlab::inspect_pe_bytes(excessive_sections.data(), excessive_sections.size()).valid);
    auto truncated_optional = pe;
    put_u16(truncated_optional, coff + 16, 0xFFFF);
    assert(!xboxlab::inspect_pe_bytes(truncated_optional.data(), truncated_optional.size()).valid);

    xboxlab::GuestMemory memory;
    assert(memory.region_count() == 0);
    assert(!memory.map_region(0x1000, 0));
    assert(memory.map_region(0x1000, 16));
    assert(!memory.map_region(0x1008, 16));
    assert(memory.map_region(0x2000, 8, true, false));
    assert(!memory.map_region(std::numeric_limits<std::uint64_t>::max() - 2, 8));

    const std::array<std::uint8_t, 4> input{0x78, 0x56, 0x34, 0x12};
    assert(memory.write(0x1004, input));
    std::array<std::uint8_t, 4> output{};
    assert(memory.read(0x1004, output));
    assert(output == input);

    std::uint32_t value = 0;
    assert(memory.read_u32_le(0x1004, value));
    assert(value == 0x12345678U);
    assert(!memory.read(0x100E, output));
    assert(!memory.write(0x2000, input));
    assert(!memory.read(0x3000, output));
    assert(memory.region_count() == 2);
    return 0;
}
