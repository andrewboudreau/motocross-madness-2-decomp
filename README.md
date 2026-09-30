# Motocross Madness 2 decompilation

Reconstruct readable C++ that reproduces the retail x86 executable under VC6 SP3.
The project is at function/class reconstruction, not a complete game build.

[Current matches and next targets](docs/VC6_MATCHING.md) ·
[Setup](docs/TOOLCHAIN.md) · [Agent workflow](AGENTS.md)

## Project layout

| Directory | Purpose |
|---|---|
| `src/reconstructed/` | Matched class source and retail address bindings |
| `samples/` | Unfinished candidates and compiler/behavior probes |
| `mcm2tool/`, `tools/` | Binary analysis, compiler invocation and matching |
| `config/` | Reviewed ranges, hashes and compiler profiles |
| `tests/` | Parsing, matching and candidate-behavior regressions |
| `docs/` | Current findings and usage |
| `analysis/`, `generated/`, `work/` | Ignored evidence, filename skeletons and build output |

Start with [BaseObject.cpp](src/reconstructed/BaseObject.cpp). All six emitted
bodies match VC6 SP3 `/O2` without `/G6`, with relocations resolved. Unproven
semantic names and original source ownership remain explicit.

## Run

Install private inputs using [TOOLCHAIN.md](docs/TOOLCHAIN.md), then:

```bash
python3 tools/with_private_env.py -- make analyze
python3 tools/with_private_env.py -- make vc6-gate
python3 tools/with_private_env.py -- make status
make static-check test
```

An owned installer is an alternative: `make bootstrap INSTALLER=/path/to/MCM2PCG.exe`.
The private bundle already contains the EXE; it does not need the full installer.
Native Windows commands are in the setup guide. VC6 is the matching authority;
clang checks exercise ABI/tooling only. Public CI runs input-free tests.

## Reconstruction guides

- [Class/layout evidence](docs/CLASS_MODEL.md)
- [FollowCamera](docs/FOLLOW_CAMERA.md)
- [Code and library ownership](docs/PROVENANCE.md)
- [CRT identity and address atlas](docs/VC6_CRT_ATLAS.md)
- [Allocation accounting](docs/ALLOCATION.md)
- [Category/source navigation](docs/CATEGORIES.md)
- [Terrain cleanup and category lifetimes](docs/CATEGORY_PILOTS.md)
- [EcoSystem](docs/ECOSYSTEM.md)
- [External references](docs/REFERENCES.md)

Do not commit game data, Microsoft toolchain files, generated objects, private
bundle URLs or tokens. Keep private inputs in ignored directories or an external
cache. Git history holds superseded reports; current docs describe what is known,
what remains unresolved, and how to reproduce it.
