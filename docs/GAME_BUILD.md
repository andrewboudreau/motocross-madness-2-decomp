# Building the game from C++

`python tools/build_game.py` compiles all C++ files under `src/` and links
their objects with VC6 SP3 and the Windows/DirectX import libraries. The
source list, include paths and libraries are in `config/game_build.json`.
`samples/` is excluded: code needed by the game must be promoted into canonical
source with its evidence and checks.

```powershell
python tools/build_game.py --vc6-root work/vc6sp3-disc -j 4
python tools/build_game.py --vc6-root work/vc6sp3-disc --compile-only
```

The compiler root defaults to `$VC6_ROOT`, then `work/vc6sp3-disc`. The
compiler, linker and CRT hashes must match the private-input pins. On Linux,
use the installed private environment:

```bash
python3 tools/with_private_env.py -- python3 tools/build_game.py
```

Every invocation creates a fresh output directory under `work/game-build/`.
`latest.json` points to that run through its `build_directory` field and
records source/header hashes, the compiler profile and per-source results.
Each object has an adjacent compiler log. Import-library and linker logs
are in the build directory. Failed compilation or linking returns nonzero;
an old executable cannot satisfy a subsequent build.

The default profile is `vc6_o2_mt`. Objects are ordered by source basename,
then path. This is a reproducible working order, not proof that reconstructed
files recover original translation units. Ordinary import libraries supply
external DLL functions; the build never resolves game functions to retail
addresses and never forces unresolved references into an executable.

## Current gaps

The 2026-10-19 build compiles all 159 C++ files and reaches LINK with 855
unresolved external symbols, down from 859 before the FastMath promotion.
This is a measured link failure, not a source-completion percentage. The
current run's `link.log` is authoritative for remaining failures.

Many remaining declarations use provisional class views or different method
signatures from the available implementation. Those need evidence-based
type/caller integration, alongside genuinely missing functions and globals.
Resolving an address in a matching bindings file does not supply a C++ linker
definition. The FastMath provider is now promoted from samples, with its
runtime-initialized lookup-table storage. All seven functions retain strict
matches over 523 bytes with resolved relocations. A separate native VC6
compile/link/run regression exercises initialization, square roots and trig
tables (`tests/test_fastmath_link.py`, enabled by `VC6_ROOT` on Windows).

The source still lacks `Game::Tick` at `0x004685d0`, called by
`TrackGame::Tick`; its retail timing uses `rdtsc` (see [Game](GAME.md)).
Startup also lacks providers such as `GetDXVersion` and `RecordExceptionInfo`.
These are gameplay blockers in addition to incompatible class views and
unresolved globals. Import libraries alone cannot supply them.

An eventual successful link is only the next gate. Startup/static
initialization, external assets, resources, current-Windows display behavior,
menus and actual racing must still be tested on the source-built executable.
The existing `runtime/build.py` route stages the retail demo executable and
does not prove any of those source-build requirements. Source-byte matching
remains a separate check through the existing VC6 calibration and physics
suites.
