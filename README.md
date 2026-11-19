# Xbox Compatibility Research Lab

An open-source C++ research project exploring the foundations required to understand Xbox software compatibility on desktop operating systems.

> **Status: early research scaffold — not an Xbox emulator.** This repository does not currently boot or run Xbox, Xbox 360, Xbox One, or Xbox Series X|S games. It provides safe file-signature inspection and an initial guest-memory abstraction for controlled tests.

## Project direction

The reference project AnyPS5 is an executable-porting system: it relinks console executables for desktop platforms and supplies independently implemented system-library replacements. This project studies a similar *research direction* for Xbox, but Xbox generations have different formats and execution environments. We will not copy AnyPS5 code.

The first implementation target has not yet been selected. Original Xbox, Xbox 360, Xbox One, and Xbox Series X|S must be treated as separate platforms. There is no promise of 100% compatibility.

## Current features

- `xboxlab --help` — command-line help.
- `xboxlab --inspect <file>` — reads at most the first 16 bytes and recognises MZ/PE, XEX2, ZIP, and ELF signatures.
- `GuestMemory` — a host-independent, bounds-checked guest address-space model with mapped regions and read/write permissions.
- C++20 / CMake build and automated tests on Windows and Linux through GitHub Actions.

The file probe is not a complete parser. An MZ or ZIP signature does not prove a file is an Xbox game. GuestMemory is only a data model; it does not execute instructions or allocate host virtual memory.

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
4. [ ] Read-only, bounds-checked format metadata parsers
5. [ ] CPU state and execution-interface research
6. [ ] Select one Xbox generation for a focused prototype
7. [ ] Document and implement appropriate public OS compatibility interfaces
8. [ ] Graphics, audio, input, storage and test-harness research
9. [ ] Homebrew test programs and reproducible compatibility reports

Unchecked items are plans, not implemented features.

## Contributing

Read [CONTRIBUTING.md](CONTRIBUTING.md). Contributions should be small, documented, testable, and legally sourced.

## Licence

MIT. See [LICENSE](LICENSE). Third-party components, if introduced later, may have their own licences and notices.
