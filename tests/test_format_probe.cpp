#include "xboxlab/format_probe.hpp"

#include <cassert>
#include <cstdint>
#include <vector>

int main() {
    using xboxlab::FileKind;
    using xboxlab::probe_bytes;

    assert(probe_bytes({0x4D, 0x5A}).kind == FileKind::PeOrMzExecutable);
    assert(probe_bytes({0x58, 0x45, 0x58, 0x32}).kind == FileKind::Xex2Container);
    assert(probe_bytes({0x50, 0x4B, 0x03, 0x04}).kind == FileKind::ZipPackage);
    assert(probe_bytes({0x7F, 0x45, 0x4C, 0x46}).kind == FileKind::ElfExecutable);
    assert(probe_bytes({0x00, 0x01, 0x02, 0x03}).kind == FileKind::Unknown);
    assert(probe_bytes({}).kind == FileKind::Unknown);
    return 0;
}
