# Changelog

## v0.5 — class dossiers, ranked queue, and stronger reproducibility

- Added `analysis/class_dossiers.json` for all 249 logical RTTI classes, joining bases/vtables, primary-slot relations, layout hints, deleting destructors, adjustor thunks, vptr writes, direct function targets, and conservative source-unit hints.
- Added `docs/CLASS_DOSSIERS.md` focused summaries for BaseObject, Game, UIControl/UIMultiState, Vehicle/Bike, PhysicsBody, and QuadTreeObject.
- Added `analysis/work_queue.json` + `docs/WORK_QUEUE.md`; current split is 35 validated targets and 21 unresolved/calibration targets.
- Expanded easy-pattern classification from 45 to 51 functions.
- Added two high-confidence UI setter patterns; generated clang/MSVC-ABI gate grows from 32/32 to **34/34 exact**.
- Unique exact retail functions across manual/generated gates grow from 33 to **35**.
- Added four `UIMultiState` indexed-element compiler-calibration targets, taking calibration coverage from 7 to **11** cases. Three match clang on 19/20 bytes (95%) and differ only in equivalent SIB base/index encoding; the address getter differs by LEA-vs-add code shape.
- Direct class-layout evidence grows from 10 classes / 25 fields to **13 classes / 33 fields**.
- Added `make selftest`, validating retail SHA-256, generated artifact counts, and both clang gates.
- `make status` now explicitly distinguishes a bootstrapped runtime workspace from cached analysis, and all executable-dependent Make targets fail with a useful bootstrap instruction when `work/game/mcm2.exe` is absent.
- Tightened source-unit heuristics so inherited/shared virtual stubs are not treated as translation-unit anchors for derived classes.


## v0.4 — Multiple-inheritance and generated-match gate

- Fixed MSVC RTTI parsing so Complete Object Locator records are preserved instead of collapsing secondary subobject vtables.
- Recovered 271 total vtables, including 22 secondary subobject vtables across 17 classes with multiple-vtable / multiple-inheritance evidence.
- Added generated class evidence:
  - `analysis/multiple_inheritance.json`
  - `analysis/vtable_overrides.json`
  - `analysis/class_layout_hints.json`
- Added VC6-specific artifact analysis:
  - 145 canonical scalar deleting-destructor wrappers
  - 28 `this`-adjustor thunks
  - 492 code sites that write recovered vtable pointers
- Expanded mechanically classified easy targets from 29 to 45.
- Added generated C++ probe generation and a relocation-aware regression gate.
- The generated high-confidence gate is 32/32 exact under clang-cl/i686 MSVC ABI.
- Existing hand-written smoke gate remains 14/14 exact; the combined corpus contains 33 unique exact retail functions.
- Added BaseObject constructor/destructor reconstruction as compiler-calibration targets.
- Calibration set now contains seven intentionally compiler-sensitive functions.
- Added direct object-layout evidence for 10 classes / 25 fields.
- Added `find_vtable_writes.py`, `analyze_msvc_artifacts.py`, `build_class_evidence.py`, `generate_easy_probes.py`, and `run_easy_probes.py`.
- Updated VC6 gate to require both hand-written and generated high-confidence probes.
- Clean `tools/bootstrap.py` run from the original installer verified the full v0.4 pipeline.

No Motocross Madness 2 binaries/assets or Microsoft Visual C++ binaries are distributed with the bootstrap.
