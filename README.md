# Xbox Compatibility Research Lab

An open-source C++ research project exploring the foundations required to understand Xbox software compatibility on desktop operating systems.

> **Status: early research scaffold — not an Xbox emulator.** This repository does not currently boot or run Xbox, Xbox 360, Xbox One, or Xbox Series X|S games. It includes a small working file-format inspection utility and a modular C++ foundation for future, lawful compatibility research.

## Why this project exists

Modern console compatibility is much more than translating graphics calls. A serious implementation must understand executable formats, CPU behaviour, memory, system APIs, graphics, audio, input, storage, timing, and security boundaries. Xbox generations also differ significantly; support for one generation does not imply support for another.

There is no promise of 100% compatibility. Every future capability must be demonstrated and tested.

## Current working feature

The `xboxlab --inspect <file>` command identifies a few common file signatures:

- Windows PE / possible Xbox-family executable (MZ header; identification only)
- XEX2 container signature
- ZIP-based package signature (which may include MSIX/AppX packages)
- ELF executable signature
- Unknown file

This is a basic signature probe, **not** a complete parser, loader, decryptor, or game launcher. A file signature alone cannot prove that a file is a valid Xbox title.

## Build

Requirements: CMake 3.20+, and a C++20 compiler (Visual Studio 2022, recent Clang, or GCC).

### Windows

Install Visual Studio 2022 with the **Desktop development with C++** workload and CMake. From a Developer PowerShell in the repository:

```powershell
cmake -S . -B build
cmake --build build --config Release
.`build\Release\xboxlab.exe` --help
```

### Linux

```bash
sudo apt install build-essential cmake
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j2
./build/xboxlab --help
```

### Run tests

```bash
ctest --test-dir build --output-on-failure
```

## Project structure

- `src/` — executable and implementation
- `include/xboxlab/` — public interfaces
- `tests/` — lightweight tests
- `docs/` — architecture and research roadmap

## Research principles

- Work from public documentation, independently developed code, and lawfully obtained test material.
- Do not bundle commercial games, firmware, console keys, proprietary SDKs, or copyrighted system libraries.
- Do not implement circumvention of console security, DRM, or access controls.
- Keep compatibility claims specific, reproducible, and backed by tests.
- Preserve attribution and comply with the licences of any third-party code used.

## Roadmap

1. [x] C++20 build system and command-line shell
2. [x] Basic file signature probe and tests
3. [ ] Document public executable/container formats
4. [ ] Build safe, read-only metadata parsers
5. [ ] Research a guest memory model and CPU execution interface
6. [ ] Investigate documented OS API compatibility boundaries
7. [ ] Add graphics, audio, input and storage abstractions
8. [ ] Publish a per-title test and compatibility reporting format

Items not checked are planned work, not implemented features.

## Contributing

Read [CONTRIBUTING.md](CONTRIBUTING.md). Contributions should be small, documented, testable, and legally sourced.

## Licence

MIT. See [LICENSE](LICENSE). Third-party components, if introduced later, may have their own licences and notices.
