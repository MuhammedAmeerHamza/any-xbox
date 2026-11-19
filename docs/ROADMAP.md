# Development roadmap

This roadmap is intentionally staged. The project is not yet an emulator.

## Stage 0 — repository foundation
- [x] C++20 / CMake project
- [x] CLI help and file signature inspection
- [x] Basic automated tests

## Stage 1 — safe file research
- [ ] Add format-specific parsers based on public documentation
- [ ] Validate offsets, sizes, endianness and malformed input
- [ ] Add fuzz tests and test fixtures that contain no copyrighted content

## Stage 2 — machine foundations
- [ ] Define guest memory map and permission model
- [ ] Define CPU state and instruction execution interfaces
- [ ] Add deterministic timing and exception model
- [ ] Select one console generation as the first target

## Stage 3 — compatibility research
- [ ] Document publicly observable OS interfaces
- [ ] Implement independent API shims where technically and legally appropriate
- [ ] Design graphics, audio, input and storage abstractions
- [ ] Add per-title test harness

## Stage 4 — proof of capability
- [ ] Run purpose-built homebrew test programs
- [ ] Publish reproducible logs and compatibility evidence
- [ ] Only then evaluate commercial-title compatibility using lawful user-provided material

No stage guarantees that modern commercial Xbox games will become compatible. Performance and compatibility depend on the target hardware, title, and completeness of the implementation.
