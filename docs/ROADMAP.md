# Development roadmap

The project is a research foundation, not yet an emulator or executable porting tool.

## Stage 0 — repository foundation
- [x] C++20 / CMake project
- [x] CLI and file signature inspection
- [x] Automated tests and Windows/Linux CI

## Stage 1 — safe runtime foundations
- [x] Initial bounds-checked guest memory regions
- [ ] Add guest memory permissions, boundary and malformed-input tests (initial basic coverage exists)
- [ ] Add format-specific read-only parsers using public documentation
- [ ] Add fuzz testing and non-copyrighted fixtures

## Stage 2 — machine model
- [ ] Define CPU state and instruction execution interfaces
- [ ] Define deterministic timing and exception model
- [ ] Research interrupt and device abstractions
- [ ] Select one console generation as the first target

## Stage 3 — compatibility research
- [ ] Document publicly observable OS interfaces
- [ ] Implement independent API shims where technically and legally appropriate
- [ ] Design graphics, audio, input and storage abstractions
- [ ] Add per-title test harness

## Stage 4 — proof of capability
- [ ] Run purpose-built homebrew test programs
- [ ] Publish reproducible logs and compatibility evidence
- [ ] Evaluate title compatibility only with lawful user-provided material

No stage guarantees that modern commercial Xbox games will become compatible. Performance and compatibility depend on the target hardware, title, and completeness of the implementation.
