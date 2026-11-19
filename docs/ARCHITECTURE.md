# Architecture

This document describes implemented components and intended research boundaries. It is not a claim that an emulator exists.

## Current implementation

### File signature inspection

The CLI reads at most 16 bytes and checks a small set of file signatures. A signature is only a clue; it does not establish platform identity.

### PE/COFF metadata inspection

The `--inspect-pe` path reads at most 1 MiB and performs bounds checks before reading the DOS header, PE signature, COFF header, optional header, and section-header table. It reports generic PE fields plus each section name, virtual size/address, raw size/offset, and characteristics. It does not read section payloads, parse imports or relocations, inspect package metadata, or execute code. The section table is capped at 96 entries as a defensive limit.

A PE machine value or subsystem value is not enough to identify an Xbox One or Xbox Series X|S executable. Platform-specific identification requires reliable, documented metadata and must not be guessed from generic fields.

### XVC package family filename inspection

The `--inspect-package` command examines only the supplied filename. Microsoft's GDK documentation describes `_x.xvc` package names for the Xbox One family and `_xs.xvc` package names for the Xbox Series X|S family. The implementation recognises these suffixes case-insensitively and reports the matching family.

This is a naming-convention check, not a parser for XVC binary contents. It cannot verify package integrity, signatures, encryption, title identity, installability, or compatibility. An arbitrary file can be renamed to match the convention. The project intentionally does not decrypt or bypass protections on console packages.

### Guest memory model

The `GuestMemory` component is a host-independent byte-addressable model intended for unit tests and future CPU/runtime work. Callers map non-overlapping guest address ranges with explicit read/write permissions. Reads and writes must fit entirely inside one mapped region. Invalid, unmapped, overlapping, or permission-violating operations fail without modifying memory.

This abstraction does not map guest addresses into the host process address space and does not yet model page tables, guard pages, memory-mapped devices, or generation-specific layouts.

### x86-64 guest CPU state foundation

The `GuestCpuState` component stores the sixteen x86-64 general-purpose registers, the instruction pointer (RIP), and selected RFLAGS bits. It provides checked register access, flag helpers, and a deterministic reset state. This is a host-independent architectural context model intended for tests and future runtime interfaces.

It does **not** decode or execute instructions, switch host CPU context, load executable code, implement privileged state, or prove Xbox CPU compatibility. Xbox One and Series X|S CPU/runtime requirements still need separate research. The project has not chosen an execution backend; a future design must evaluate native host execution and the required OS/API compatibility rather than assuming a full CPU emulator is necessary.

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

### Execution backend boundary

The ExecutionBackend interface defines a future boundary between the guest CPU/memory models and an execution engine. A bounded run request returns an explicit stop reason (budget exhausted, guest halt, unsupported instruction, memory fault, guest exception, unavailable backend, or invalid request) plus an instruction count and optional exception code.

This is an interface contract only. No production backend is implemented, and the current test backend deliberately reports that execution is unavailable. The interface does not decide whether native host execution, translation, or interpretation is safe or feasible for a particular Xbox title. Those choices require separate research into executable loading, memory permissions, calling conventions, exception handling, OS services, and generation-specific behaviour. No untrusted game code is executed by the current tools.
