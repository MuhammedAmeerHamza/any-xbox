# Development roadmap

Any Xbox is an early-stage research foundation. It is not yet an emulator, executable porting tool, or game launcher.

<p align="center">
  <img src="../assets/progress-dashboard.svg" alt="Current roadmap milestone progress" width="100%">
</p>

Progress counts checklist items inside each stage only. It is not a percentage of the complete goal of running Xbox games.

## Target scope

- Xbox One: active research target.
- Xbox Series X|S: active research target and long-term priority.
- Keep generation-specific formats, metadata, and runtime assumptions separate.

## Stage 0 — repository foundation
- [x] C++20 / CMake project
- [x] CLI and file signature inspection
- [x] Automated tests and Windows/Linux CI

**Stage status: 3/3 checks complete.**

## Stage 1 — safe inspection foundations
- [x] Initial bounds-checked guest memory regions
- [x] Basic PE/COFF header and section-table inspection with malformed-input tests
- [x] Documented XVC filename generation classification
- [x] Bounded PE section-header metadata parsing
- [ ] Safe parsing of additional documented, unencrypted manifest metadata
- [ ] Add fuzz testing and non-copyrighted fixtures

**Stage status: 4/6 checks complete.**

## Stage 2 — machine model
- [x] Define a tested x86-64 guest register / RIP / RFLAGS state model
- [ ] Define instruction execution backend interfaces
- [ ] Define deterministic timing and exception model
- [ ] Research interrupt and device abstractions

**Stage status: 1/4 checks complete.**

Generation-specific CPU/runtime differences between Xbox One and Series X|S must be documented before generation-specific implementation.

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
