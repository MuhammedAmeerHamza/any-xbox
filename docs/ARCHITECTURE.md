# Architecture notes

This document describes intended boundaries, not completed emulator features.

## Current implementation

The current executable is a command-line shell. The format probe reads at most the first 16 bytes and recognises a handful of file signatures. It does not parse executable metadata, load code, emulate instructions, or launch games.

## Intended layers

1. **Front end** — command line first; a graphical launcher may be considered later.
2. **File and package inspection** — read-only, bounds-checked metadata parsers.
3. **Guest machine model** — explicit CPU, memory, interrupt, timer and device interfaces.
4. **Operating-system compatibility** — documented API surface and independently implemented substitutes where lawful.
5. **Graphics and audio** — abstract host backends with reproducible tests.
6. **Input and storage** — controller mapping and virtual devices.
7. **Compatibility tests** — per-title, per-build, per-feature evidence.

## Generation boundary

Original Xbox, Xbox 360, Xbox One, and Xbox Series X|S are distinct targets. Their executable formats, CPU environments, operating-system services, graphics interfaces, and security models are not interchangeable. The project must declare a target generation before implementing generation-specific behaviour.

## Design rules

- Parsers must validate lengths and offsets before reading.
- Unknown fields should be preserved or safely ignored where possible.
- No untrusted game code is executed by the inspection tool.
- No firmware, keys, commercial game data, or proprietary libraries are distributed here.
- A title marked "boots" must not be described as "playable".
