# Architecture

This document describes implemented components and intended research boundaries. It is not a claim that an emulator exists.

## Current implementation

### File signature inspection

The CLI reads at most 16 bytes and checks a small set of file signatures. A signature is only a clue; it does not establish platform identity.

### PE/COFF metadata inspection

The `--inspect-pe` path reads at most 1 MiB and performs bounds checks before reading the DOS header, PE signature, COFF header, and optional header. It reports a small set of generic PE fields. It does not parse section contents, imports, relocations, package metadata, or execute code.

A PE machine value or subsystem value is not enough to identify an Xbox One or Xbox Series X|S executable. Platform-specific identification requires reliable, documented metadata and must not be guessed from generic fields.

### Guest memory model

The `GuestMemory` component is a host-independent byte-addressable model intended for unit tests and future CPU/runtime work. Callers map non-overlapping guest address ranges with explicit read/write permissions. Reads and writes must fit entirely inside one mapped region. Invalid, unmapped, overlapping, or permission-violating operations fail without modifying memory.

This abstraction does not map guest addresses into the host process address space and does not yet model page tables, guard pages, memory-mapped devices, or generation-specific layouts.

## Target generations

- **Xbox One:** active research target.
- **Xbox Series X|S:** active research target and long-term priority.
- **Xbox 360 / Original Xbox:** legacy signatures may be recognised for context, but are not current implementation targets.

Generation-specific parsing and runtime behaviour must be kept behind explicit, separately tested interfaces. A shared generic parser must never label a file as belonging to a console generation without supporting evidence.

## Intended layers

1. **Front end** — command line first; a graphical launcher may be considered later.
2. **File and package inspection** — read-only, bounds-checked metadata parsers.
3. **Guest machine model** — CPU state, memory, interrupts, timers and device interfaces.
4. **Operating-system compatibility** — public API research and independent substitutes where lawful.
5. **Graphics and audio** — abstract host backends with reproducible tests.
6. **Input and storage** — controller mapping and virtual devices.
7. **Compatibility tests** — per-title, per-build, per-feature evidence.

## Design rules

- Parsers validate lengths and offsets before reading.
- Memory operations check bounds and permissions.
- Unknown fields are preserved or safely ignored where possible.
- No untrusted game code is executed by inspection tools.
- No firmware, keys, commercial game data, or proprietary libraries are distributed here.
- A title marked "boots" must not be described as "playable".
