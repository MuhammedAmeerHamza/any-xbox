# Development roadmap

The project is a research foundation, not yet an emulator or executable porting tool.

## Target scope

- Xbox One: active research target.
- Xbox Series X|S: active research target and long-term priority.
- Keep generation-specific formats, metadata, and runtime assumptions separate.

## Stage 0 — repository foundation
- [x] C++20 / CMake project
- [x] CLI and file signature inspection
- [x] Automated tests and Windows/Linux CI

## Stage 1 — safe inspection foundations
- [x] Initial bounds-checked guest memory regions
- [x] Basic PE/COFF header and section-table inspection with malformed-input tests
- [x] Documented XVC filename generation classification
- [x] Bounded PE section-header metadata parsing\n- [ ] Safe parsing of additional documented, unencrypted manifest metadata
- [ ] Add fuzz testing and non-copyrighted fixtures

## Stage 2 — machine model
- [ ] Define CPU state and instruction execution interfaces
- [ ] Define deterministic timing and exception model
- [ ] Research interrupt and device abstractions
- [ ] Document differences between Xbox One and Series X|S before generation-specific implementation

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
