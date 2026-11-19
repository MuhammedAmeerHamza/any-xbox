# Xbox Compatibility Research Lab

An open-source C++ research project exploring the foundations required to understand Xbox software compatibility on desktop operating systems.

> **Status: early research scaffold — not an Xbox emulator.** This repository does not currently boot or run Xbox, Xbox 360, Xbox One, or Xbox Series X|S games. It provides safe file-signature inspection, basic read-only PE/COFF metadata inspection, and an initial guest-memory abstraction for controlled tests.

## Project direction

The project studies an independent Xbox compatibility research direction inspired by executable-porting systems such as AnyPS5. We do not copy its code. Xbox generations have different executable formats and execution environments and must not be treated as interchangeable.

**Target scope:** Xbox One and Xbox Series X|S are both in scope. Series X|S is the long-term priority, while Xbox One remains an important parallel compatibility target. Legacy XEX2 signature recognition is retained as format identification only; it is not a commitment to implement Xbox 360 support.

There is no promise of 100% compatibility. A generic PE header is not proof that a file is an Xbox executable.

## Current features

- `xboxlab --help` — command-line help.
- `xboxlab --inspect <file>` — reads at most the first 16 bytes and recognises MZ/PE, XEX2, ZIP, and ELF signatures.
- `xboxlab --inspect-pe <file>` — reads up to 1 MiB and validates basic DOS, PE signature, COFF, and optional-header bounds; reports machine type, section count, timestamp, optional-header kind, characteristics, and subsystem value when present.
- `GuestMemory` — a host-independent, bounds-checked guest address-space model with mapped regions and read/write permissions.
- C++20 / CMake build and automated tests on Windows and Linux through GitHub Actions.

The PE inspector does not parse section tables, imports, relocations, Xbox package formats, or platform-specific metadata. It does not load or execute code. PE metadata alone cannot distinguish Xbox One / Series X|S software from ordinary Windows software. GuestMemory is only a data model; it does not execute instructions or allocate host virtual memory.

## Build

Requirements: CMake 3.20+ and a C++20 compiler (Visual Studio 2022, recent Clang, or GCC).

### Windows

Install Visual Studio 2022 with the **Desktop development with C++** workload and CMake. In Developer PowerShell:

```powershell
cmake -S . -B build
cmake --build build --config Release
.\build\Release\xboxlab.exe --help
ctest --test-dir build -C Release --output-on-failure
```

### Linux

```bash
sudo apt install build-essential cmake
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j2
./build/xboxlab --help
ctest --test-dir build --output-on-failure
```

Example PE inspection:

```bash
xboxlab --inspect-pe path/to/sample.exe
```

## Project layout

- `include/xboxlab/` — public interfaces
- `src/` — implementations and command-line application
- `tests/` — lightweight automated tests
- `docs/` — architecture, roadmap, and compatibility reporting
- `compatibility/` — format for future per-title test reports

## Research principles

- Use public documentation, independently developed code, and lawfully obtained test material.
- Do not bundle commercial games, firmware, console keys, proprietary SDKs, or copyrighted system libraries.
- Do not implement circumvention of console security, DRM, or access controls.
- Keep compatibility claims specific, reproducible, and backed by tests.
- Preserve attribution and comply with third-party licences.

## Roadmap

1. [x] C++20 build system and CLI
2. [x] Basic file signature probe
3. [x] Initial guest memory abstraction and bounds tests
4. [x] Basic bounds-checked PE/COFF header inspection
5. [ ] Research documented Xbox One and Series X|S package / executable metadata without bypassing protections
6. [ ] CPU state and execution-interface research
7. [ ] Document and implement appropriate public OS compatibility interfaces
8. [ ] Graphics, audio, input, storage and test-harness research
9. [ ] Homebrew test programs and reproducible compatibility reports

Unchecked items are plans, not implemented features.

## Contributing

Read [CONTRIBUTING.md](CONTRIBUTING.md). Contributions should be small, documented, testable, and legally sourced.

## Licence

MIT. See [LICENSE](LICENSE). Third-party components, if introduced later, may have their own licences and notices.
