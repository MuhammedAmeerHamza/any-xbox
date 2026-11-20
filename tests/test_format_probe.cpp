#include "xboxlab/format_probe.hpp"
#include "xboxlab/guest_memory.hpp"
#include "xboxlab/guest_cpu.hpp"
#include "xboxlab/guest_timing.hpp"
#include "xboxlab/guest_exception.hpp"
#include "xboxlab/guest_interrupt.hpp"
#include "xboxlab/guest_device.hpp"
#include "xboxlab/manifest_probe.hpp"
#include "xboxlab/execution_backend.hpp"
#include "xboxlab/package_probe.hpp"
#include "xboxlab/pe_inspector.hpp"

#include <array>
#include <algorithm>
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

class UnavailableTestBackend final : public xboxlab::ExecutionBackend {
public:
    [[nodiscard]] std::string_view name() const noexcept override { return "test-unavailable"; }
    [[nodiscard]] xboxlab::ExecutionResult run(
        xboxlab::GuestCpuState&, xboxlab::GuestMemory&, std::uint64_t) override {
        return {xboxlab::ExecutionStopReason::BackendUnavailable, 0, 0};
    }
};

class TestGuestDevice final : public xboxlab::GuestDevice {
public:
 [[nodiscard]] std::string_view name() const noexcept override { return "synthetic-test-device"; }
 [[nodiscard]] std::uint64_t base_address() const noexcept override { return 0x4000; }
 [[nodiscard]] std::size_t size() const noexcept override { return bytes.size(); }
 [[nodiscard]] bool read(std::uint64_t offset, std::span<std::uint8_t> out) override {
  if (offset > bytes.size() || out.size() > bytes.size() - static_cast<std::size_t>(offset)) return false;
  std::copy_n(bytes.begin() + static_cast<std::ptrdiff_t>(offset), out.size(), out.begin()); return true;
 }
 [[nodiscard]] bool write(std::uint64_t offset, std::span<const std::uint8_t> in) override {
  if (offset > bytes.size() || in.size() > bytes.size() - static_cast<std::size_t>(offset)) return false;
  std::copy(in.begin(), in.end(), bytes.begin() + static_cast<std::ptrdiff_t>(offset)); return true;
 }
 std::array<std::uint8_t, 16> bytes{};
};

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
    const std::size_t section_table = coff + 20 + 112;
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

    // x86-64 guest CPU context: register storage, RIP, and selected RFLAGS bits.
    xboxlab::GuestCpuState cpu;
    CHECK(cpu.rflags() == xboxlab::GuestCpuState::kReservedRflagsBit);
    std::uint64_t register_value = 0;
    CHECK(cpu.read_gpr(xboxlab::X64Gpr::Rax, register_value));
    CHECK(register_value == 0);
    CHECK(cpu.write_gpr(xboxlab::X64Gpr::Rax, 0x1122334455667788ULL));
    CHECK(cpu.read_gpr(xboxlab::X64Gpr::Rax, register_value));
    CHECK(register_value == 0x1122334455667788ULL);
    CHECK(cpu.write_gpr(xboxlab::X64Gpr::R15, 0xF0E1D2C3B4A59687ULL));
    CHECK(cpu.read_gpr(xboxlab::X64Gpr::R15, register_value));
    CHECK(register_value == 0xF0E1D2C3B4A59687ULL);
    CHECK(!cpu.write_gpr(static_cast<xboxlab::X64Gpr>(255), 1));
    CHECK(!cpu.read_gpr(static_cast<xboxlab::X64Gpr>(255), register_value));
    cpu.set_instruction_pointer(0x0000000140001000ULL);
    CHECK(cpu.instruction_pointer() == 0x0000000140001000ULL);
    CHECK(cpu.set_flag(xboxlab::X64Flag::Zero, true));
    bool zero_flag = false;
    CHECK(cpu.flag_is_set(xboxlab::X64Flag::Zero, zero_flag));
    CHECK(zero_flag);
    CHECK(cpu.set_flag(xboxlab::X64Flag::Carry, true));
    CHECK(cpu.flag_is_set(xboxlab::X64Flag::Carry, zero_flag));
    CHECK(zero_flag);
    CHECK(cpu.set_flag(xboxlab::X64Flag::Zero, false));
    CHECK(cpu.flag_is_set(xboxlab::X64Flag::Zero, zero_flag));
    CHECK(!zero_flag);
    CHECK((cpu.rflags() & xboxlab::GuestCpuState::kReservedRflagsBit) != 0);
    CHECK(!cpu.set_flag(static_cast<xboxlab::X64Flag>(255), true));
    cpu.set_rflags(0);
    CHECK(cpu.rflags() == xboxlab::GuestCpuState::kReservedRflagsBit);
    cpu.reset();
    CHECK(cpu.instruction_pointer() == 0);
    CHECK(cpu.rflags() == xboxlab::GuestCpuState::kReservedRflagsBit);
    CHECK(cpu.read_gpr(xboxlab::X64Gpr::Rax, register_value));
    CHECK(register_value == 0);

    // Deterministic virtual clock: no wall-clock dependency or hardware claim.
    xboxlab::GuestTiming timing;
    CHECK(timing.ticks() == 0);
    CHECK(timing.advance(5));
    CHECK(timing.advance(7));
    CHECK(timing.ticks() == 12);
    CHECK(timing.advance(0));
    CHECK(timing.ticks() == 12);
    CHECK(!timing.advance(std::numeric_limits<std::uint64_t>::max()));
    CHECK(timing.ticks() == 12); // overflow must not change the clock
    timing.reset();
    CHECK(timing.ticks() == 0);
    CHECK(timing.advance(std::numeric_limits<std::uint64_t>::max()));
    CHECK(timing.ticks() == std::numeric_limits<std::uint64_t>::max());
    CHECK(!timing.advance(1));
    CHECK(timing.ticks() == std::numeric_limits<std::uint64_t>::max());

    // A single pending normalized exception is retained until explicitly cleared.
    xboxlab::GuestExceptionState exceptions;
    xboxlab::GuestException exception{};
    CHECK(!exceptions.has_pending());
    CHECK(!exceptions.pending(exception));
    CHECK(exceptions.raise(xboxlab::GuestExceptionCode::PageFault, 0x12345000ULL));
    CHECK(exceptions.has_pending());
    CHECK(exceptions.pending(exception));
    CHECK(exception.code == xboxlab::GuestExceptionCode::PageFault);
    CHECK(exception.address == 0x12345000ULL);
    CHECK(!exceptions.raise(xboxlab::GuestExceptionCode::DivideByZero, 0xABC));
    CHECK(exceptions.pending(exception));
    CHECK(exception.code == xboxlab::GuestExceptionCode::PageFault);
    exceptions.clear();
    CHECK(!exceptions.has_pending());
    CHECK(!exceptions.pending(exception));
    CHECK(exceptions.raise(xboxlab::GuestExceptionCode::DivideByZero, 0xABC));
    CHECK(exceptions.pending(exception));
    CHECK(exception.code == xboxlab::GuestExceptionCode::DivideByZero);
    CHECK(exception.address == 0xABC);
    exceptions.clear();

    // This test double verifies dispatch without pretending guest code executes.
    UnavailableTestBackend unavailable_backend;
    xboxlab::GuestCpuState backend_cpu;
    xboxlab::GuestMemory backend_memory;
    CHECK(unavailable_backend.name() == "test-unavailable");
    const auto unavailable_result =
        unavailable_backend.run(backend_cpu, backend_memory, 100);
    CHECK(unavailable_result.reason ==
          xboxlab::ExecutionStopReason::BackendUnavailable);
    CHECK(unavailable_result.instructions_executed == 0);
    CHECK(unavailable_result.exception_code == 0);


    // Synthetic, non-copyrighted MicrosoftGame.config-style metadata fixtures.
    constexpr std::string_view manifest_fixture =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<Game configVersion=\"1\">"
        "<Identity Name=\"Example.Publisher.Game\" Publisher=\"CN=Example-Publisher\" Version=\"1.2.3.4\"/>"
        "<ExecutableList><Executable Name=\"ExampleGame.exe\" Id=\"Game\" "
        "TargetDeviceFamily=\"Scarlett\" IsDevOnly=\"false\"/></ExecutableList>"
        "<ShellVisuals DefaultDisplayName=\"Example &amp; Test\" PublisherDisplayName=\"Example Studio\"/>"
        "</Game>";
    const auto manifest = xboxlab::inspect_game_manifest(manifest_fixture);
    CHECK(manifest.valid);
    CHECK(manifest.config_version == "1");
    CHECK(manifest.identity_name == "Example.Publisher.Game");
    CHECK(manifest.publisher == "CN=Example-Publisher");
    CHECK(manifest.version == "1.2.3.4");
    CHECK(manifest.executables.size() == 1);
    CHECK(manifest.executables[0].name == "ExampleGame.exe");
    CHECK(manifest.executables[0].id == "Game");
    CHECK(manifest.executables[0].target_device_family == "Scarlett");
    CHECK(!manifest.executables[0].development_only);
    CHECK(manifest.display_name == "Example & Test");
    CHECK(manifest.publisher_display_name == "Example Studio");

    constexpr std::string_view identity_text_fixture =
        "<Game configVersion=\"1\"><Identity Name=\"Ids.Demo\" Publisher=\"CN=Ids\"/>"
        "<MSAAppId>1234567890ABCDEF</MSAAppId><TitleId>1234ABCD</TitleId>"
        "<ExecutableList><Executable Name=\"Ids.exe\"/></ExecutableList></Game>";
    const auto identity_text = xboxlab::inspect_game_manifest(identity_text_fixture);
    CHECK(identity_text.valid);
    CHECK(identity_text.msa_app_id == "1234567890ABCDEF");
    CHECK(identity_text.title_id == "1234ABCD");

    // XML edge cases found during the repository audit.
    const std::string bom_manifest = std::string("\xEF\xBB\xBF") + std::string(manifest_fixture);
    CHECK(xboxlab::inspect_game_manifest(bom_manifest).valid);
    CHECK(!xboxlab::inspect_game_manifest(
        "<Game configVersion=\"1\"><Identity Name=\"bad<name\" Publisher=\"p\"/>"
        "<ExecutableList><Executable Name=\"x.exe\"/></ExecutableList></Game>").valid);
    CHECK(!xboxlab::inspect_game_manifest(
        "<Game configVersion=\"1\"><Identity Name=\"x\" Publisher=\"p\"/>"
        "<MSAAppId>bad]]>text</MSAAppId><ExecutableList><Executable Name=\"x.exe\"/></ExecutableList></Game>").valid);
    CHECK(!xboxlab::inspect_game_manifest(
        "<Game configVersion=\"1\"><Identity Name=\"x\" Publisher=\"p\"/>"
        "<MSAAppId/><MSAAppId/><ExecutableList><Executable Name=\"x.exe\"/></ExecutableList></Game>").valid);
    CHECK(!xboxlab::inspect_game_manifest(
        "<Game configVersion=\"1\"><Identity Name=\"x\" Publisher=\"p\"/>"
        "<Extra><!-- invalid -- comment --></Extra>"
        "<ExecutableList><Executable Name=\"x.exe\"/></ExecutableList></Game>").valid);
    const std::string raw_control_manifest =
        std::string("<Game configVersion=\"1\"><Identity Name=\"x") + char(1) +
        "\" Publisher=\"p\"/><ExecutableList><Executable Name=\"x.exe\"/></ExecutableList></Game>";
    CHECK(!xboxlab::inspect_game_manifest(raw_control_manifest).valid);
    const auto nested_ids = xboxlab::inspect_game_manifest(
        "<Game configVersion=\"1\"><Identity Name=\"x\" Publisher=\"p\"/>"
        "<Extra><MSAAppId>not-a-root-field</MSAAppId></Extra>"
        "<ExecutableList><Executable Name=\"x.exe\"/></ExecutableList></Game>");
    CHECK(nested_ids.valid);
    CHECK(nested_ids.msa_app_id.empty());
    CHECK(!xboxlab::inspect_game_manifest(
        "<Game configVersion=\"1\"><Identity Name=\"Ids.Demo\" Publisher=\"CN=Ids\"/>"
        "<TitleId>1234&ampbad;</TitleId><ExecutableList><Executable Name=\"Ids.exe\"/></ExecutableList></Game>").valid);

    constexpr std::string_view legacy_fixture =
        "<Game configVersion=\"0\"><Identity Name=\"Legacy.Test\" Publisher=\"CN=Legacy\"/>"
        "<ExecutableList><Executable Name=\"Legacy.exe\" IsDevOnly=\"true\"/></ExecutableList></Game>";
    const auto legacy_manifest = xboxlab::inspect_game_manifest(legacy_fixture);
    CHECK(legacy_manifest.valid);
    CHECK(legacy_manifest.config_version == "0");
    CHECK(legacy_manifest.executables[0].development_only);

    CHECK(!xboxlab::inspect_game_manifest("").valid);
    CHECK(!xboxlab::inspect_game_manifest("<!DOCTYPE Game [<!ENTITY x SYSTEM \"file:///etc/passwd\">]><Game/>").valid);
    CHECK(!xboxlab::inspect_game_manifest(
        "<Game configVersion=\"1\"><Identity Name=\"x\"Publisher=\"p\"/>"
        "<ExecutableList><Executable Name=\"x.exe\"/></ExecutableList></Game>").valid);
    CHECK(!xboxlab::inspect_game_manifest("<Game configVersion=\"2\"></Game>").valid);
    CHECK(!xboxlab::inspect_game_manifest("<Game configVersion=\"1\"><Identity Name=\"x\" Publisher=\"p\"/></Game>").valid);
    CHECK(!xboxlab::inspect_game_manifest("<Game configVersion=\"1\"><Identity Name=\"x\" Publisher=\"p\"/><Identity Name=\"y\" Publisher=\"q\"/><ExecutableList><Executable Name=\"x.exe\"/></ExecutableList></Game>").valid);
    CHECK(!xboxlab::inspect_game_manifest("<Game configVersion=\"1\"><Identity Name=\"x\" Publisher=\"p\"/><ExecutableList><Executable Name=\"x.exe\" IsDevOnly=\"yes\"/></ExecutableList></Game>").valid);
    CHECK(!xboxlab::inspect_game_manifest("<Game configVersion=\"1\"><Identity Name=\"x\" Publisher=\"p\"/><ExecutableList><Executable Name=\"x.exe\"/></Game>").valid);
    CHECK(!xboxlab::inspect_game_manifest("<Game configVersion=\"1\"><Identity Name=\"x\" Publisher=\"p\"/><ExecutableList><Executable Name=\"x.exe\"/></ExecutableList>").valid);
    CHECK(!xboxlab::inspect_game_manifest("<Game configVersion=\"1\"><Identity Name=\"x\" Publisher=\"p\"/><ExecutableList><Executable Name=\"x.exe\" A=\"1\" A=\"2\"/></ExecutableList></Game>").valid);
    CHECK(!xboxlab::inspect_game_manifest("<Game configVersion=\"1\"><Identity Name=\"x&ampbad;\" Publisher=\"p\"/><ExecutableList><Executable Name=\"x.exe\"/></ExecutableList></Game>").valid);
    CHECK(!xboxlab::inspect_game_manifest("<Game configVersion=\"1\"><Identity Name=\"x\" Publisher=\"p\"/><ExecutableList><Executable Name=\"x.exe\"/></ExecutableList><Extra></Game>").valid);
    const std::string oversized_manifest(64U * 1024U + 1U, ' ');
    CHECK(!xboxlab::inspect_game_manifest(oversized_manifest).valid);

    // Bounded interrupt requests are delivered FIFO; no hardware vector semantics are implied.
    xboxlab::GuestInterruptController interrupts;
    xboxlab::GuestInterrupt interrupt{};
    CHECK(interrupts.empty());
    CHECK(!interrupts.acknowledge(interrupt));
    CHECK(interrupts.request({0x20, 1}));
    CHECK(interrupts.request({0x21, 2}));
    CHECK(interrupts.pending_count() == 2);
    CHECK(interrupts.acknowledge(interrupt));
    CHECK(interrupt.vector == 0x20 && interrupt.source == 1);
    CHECK(interrupts.acknowledge(interrupt));
    CHECK(interrupt.vector == 0x21 && interrupt.source == 2);
    CHECK(interrupts.empty());
    for (std::size_t i = 0; i < xboxlab::GuestInterruptController::kCapacity; ++i)
        CHECK(interrupts.request({static_cast<std::uint8_t>(i), static_cast<std::uint32_t>(i)}));
    CHECK(!interrupts.request({0xFF, 999}));
    CHECK(interrupts.pending_count() == xboxlab::GuestInterruptController::kCapacity);
    interrupts.reset();
    CHECK(interrupts.empty());

    // Device bus dispatches only bounded accesses to explicitly attached synthetic devices.
    TestGuestDevice test_device;
    TestGuestDevice overlapping_device;
    xboxlab::GuestDeviceBus device_bus;
    CHECK(device_bus.device_count() == 0);
    CHECK(!device_bus.attach(nullptr));
    CHECK(device_bus.attach(&test_device));
    CHECK(!device_bus.attach(&test_device));
    CHECK(!device_bus.attach(&overlapping_device));
    CHECK(device_bus.device_count() == 1);
    const std::array<std::uint8_t, 3> device_input{0xA1, 0xB2, 0xC3};
    CHECK(device_bus.write(0x4004, device_input));
    std::array<std::uint8_t, 3> device_output{};
    CHECK(device_bus.read(0x4004, device_output));
    CHECK(device_output == device_input);
    CHECK(!device_bus.read(0x400F, device_output));
    CHECK(!device_bus.write(0x5000, device_input));
    CHECK(!device_bus.read(0x4000, std::span<std::uint8_t>{}));
    device_bus.clear();
    CHECK(device_bus.device_count() == 0);
    CHECK(!device_bus.read(0x4004, device_output));

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
