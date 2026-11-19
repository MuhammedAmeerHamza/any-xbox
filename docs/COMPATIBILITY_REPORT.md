# Compatibility report standard

Use this template for future test reports. A report is evidence for one exact build and environment, not a promise about every release of a game.

```yaml
title_id: "unknown"
title_name: "Homebrew test title"
console_generation: "not-selected"
project_commit: "replace-with-commit"
test_date: "YYYY-MM-DD"
host_os: "Windows or Linux version"
host_cpu: "CPU model"
host_gpu: "GPU model and driver"
ram_gb: 0
status: "not-tested"
test_case: "Describe the exact test"
result: "No test has been run"
known_issues: []
evidence:
  - "Link to reproducible logs or test output"
```

Allowed status values:

- `not-tested`: no test evidence
- `does-not-boot`: tested and failed before guest entry
- `boots`: reaches guest entry or an early startup state
- `in-game`: reaches interactive gameplay but may have major issues
- `playable`: sustained testing supports a clearly stated playability claim

Never infer compatibility from a file extension or executable signature.
