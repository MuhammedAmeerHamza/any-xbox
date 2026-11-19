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

It does **not** decode or execute instructions, switch host CPU context, load executable code, implement privileged state, or prove Xbox CPU compatibility. Xbox One and Series X|S CPU/runtime requirements still need separate research.

### Deterministic guest timing model

`GuestTiming` provides a monotonically increasing virtual tick counter. Tests can advance it by an explicit number of abstract ticks without reading the host clock, making timing-related logic reproducible. Addition is checked: an operation that would overflow returns failure and leaves the counter unchanged.

These ticks are deliberately abstract. They are not seconds, CPU cycles, frame times, or measurements of Xbox One / Series X|S hardware. No scheduler, timer interrupt, wait API, or hardware clock calibration is implemented.

### Guest exception state

`GuestExceptionState` stores at most one pending normalized exception with a category and guest address. A second raise is rejected until the pending exception is inspected and cleared. Current categories include invalid opcode, general protection, page fault, access violation, divide-by-zero, breakpoint, and unknown.

This is a small bookkeeping model, not an x86 exception-delivery implementation. It does not define vector numbers, error codes, privilege transitions, stack frames, interrupt dispatch, recovery, or Xbox-specific exception semantics. Those require further documented research.

## Target generations

- **Xbox One:** active research target.
- **Xbox Series X|S:** active research target and long-term priority.
- **Xbox 360 / Original Xbox:** legacy signatures may be recognised for context, but are not current implementation targets.

Generation-specific parsing and runtime behaviour must be kept behind explicit, separately tested interfaces. A shared generic parser must never label a file as belonging to a console generation without supporting evidence.

## Intended layers

1. **Front end** — command line first; a graphical launcher may be considered later.
2. **File and package inspection** — read-only, bounds-checked metadata parsers.
3. **Guest machine model** — CPU state, memory, timing, exceptions, interrupts and device interfaces.
4. **Operating-system compatibility** — public API research and independent substitutes where lawful.
5. **Graphics and audio** — abstract host backends with reproducible tests.
6. **Input and storage** — controller mapping and virtual devices.
7. **Compatibility tests** — per-title, per-build, per-feature evidence.

## Design rules

- Parsers validate lengths and offsets before reading.
- Memory operations check bounds and permissions.
- Time-dependent tests use explicit virtual time, not sleeps or host-clock assumptions.
- Exception state is explicit and must not silently overwrite an unhandled pending exception.
- Interrupt requests are bounded and deterministic; device accesses must remain within registered guest ranges.
- Unknown fields are preserved or safely ignored where possible.
- No untrusted game code is executed by inspection tools.
- No firmware, keys, commercial game data, or proprietary libraries are distributed here.
- A title marked "boots" must not be described as "playable".

### Execution backend boundary

The ExecutionBackend interface defines a future boundary between the guest CPU/memory models and an execution engine. A bounded run request returns an explicit stop reason (budget exhausted, guest halt, unsupported instruction, memory fault, guest exception, unavailable backend, or invalid request) plus an instruction count and optional exception code.

This is an interface contract only. No production backend is implemented, and the current test backend deliberately reports that execution is unavailable. The interface does not decide whether native host execution, translation, or interpretation is safe or feasible for a particular Xbox title. Those choices require separate research into executable loading, memory permissions, calling conventions, exception handling, OS services, and generation-specific behaviour. No untrusted game code is executed by the current tools.


### Guest interrupt request queue

`GuestInterruptController` is a bounded FIFO for normalized interrupt requests. It accepts a vector byte and source identifier, rejects requests when its fixed-capacity queue is full, and removes requests in arrival order. This gives later runtime work a deterministic hand-off point.

The queue does not assign Xbox interrupt-vector meanings, hardware priorities, masking rules, interrupt-controller registers, CPU delivery semantics, or timer/device interrupt wiring. It is not an emulation of an Xbox interrupt controller.

### Guest device bus

`GuestDevice` defines a small interface for a synthetic memory-mapped device: a name, guest base address, range size, and bounded read/write operations. `GuestDeviceBus` attaches non-owning device pointers, rejects invalid or overlapping address ranges, and routes accesses only when the entire request fits inside one registered range.

Devices must outlive the bus registrations that reference them. No real Xbox chipset, controller, storage device, GPU, audio device, DMA engine, or hardware register map is implemented. These interfaces are scaffolding for isolated synthetic tests and future public-documentation research.


### MicrosoftGame.config metadata inspection\n\nThe supported manifest concepts are based on [Microsoft's public MicrosoftGame.config reference](https://learn.microsoft.com/en-us/gaming/gdk/docs/reference/system/microsoftgameconfig/microsoftgameconfig-schema?view=gdk-2604) and [overview](https://learn.microsoft.com/gaming/gdk/docs/features/common/game-config/microsoftgameconfig-overview).

`inspect_game_manifest` parses a conservative, bounded subset of the publicly documented MicrosoftGame.config XML manifest. It extracts the config version, identity attributes, executable entries, and selected shell display metadata. Input is limited to 64 KiB, nesting and token counts are bounded, duplicate attributes and structural errors are rejected, and custom/external entities and document type declarations are not supported.

This is metadata inspection only, not full XSD validation. It does not read XVC/MSIXVC package contents, decrypt packages, verify signatures, establish title authenticity, or prove that an executable is an Xbox binary or compatible. Unknown manifest elements are ignored after their XML structure is checked. The parser intentionally supports only the documented fields needed by this research scaffold.
