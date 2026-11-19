# Architecture

This document describes implemented components and intended research boundaries. It is not a claim that an emulator exists.

## Current implementation

### File inspection

The CLI reads at most 16 bytes and checks a small set of file signatures. It does not parse executable metadata, load code, decrypt content, or execute untrusted files.

### Guest memory model

`GuestMemory` is a host-independent byte-addressable model intended for unit tests and future CPU/runtime work. Callers map non-overlapping guest address ranges with explicit read/write permissions. Reads and writes must fit entirely inside one mapped region. Invalid, unmapped, overlapping, or permission-violating operations fail without modifying memory.

This abstraction does not map guest addresses into the host process address space and does not yet model page tables, guard pages, memory-mapped devices, or generation-specific layouts.

## Intended layers

1. **Front end** — command line first; a graphical launcher may be considered later.
2. **File and package inspection** — read-only, bounds-checked metadata parsers.
3. **Guest machine model** — CPU state, memory, interrupts, timers and device interfaces.
4. **Operating-system compatibility** — public API research and independent substitutes where lawful.
5. **Graphics and audio** — abstract host backends with reproducible tests.
6. **Input and storage** — controller mapping and virtual devices.
7. **Compatibility tests** — per-title, per-build, per-feature evidence.

## Generation boundary

Original Xbox, Xbox 360, Xbox One, and Xbox Series X|S are distinct targets. Their executable formats, CPU environments, operating-system services, graphics interfaces, and security models are not interchangeable. The project must declare a target generation before implementing generation-specific behaviour.

## Design rules

- Parsers validate lengths and offsets before reading.
- Memory operations check bounds and permissions.
- Unknown fields are preserved or safely ignored where possible.
- No untrusted game code is executed by inspection tools.
- No firmware, keys, commercial game data, or proprietary libraries are distributed here.
- A title marked "boots" must not be described as "playable".
