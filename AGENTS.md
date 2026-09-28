# Agent workflow

## Objective

Reconstruct readable C++ that reproduces retail MCM2 x86 under the historically correct VC6 SP3-generation toolchain.

## Evidence tiers

1. **Confirmed:** literal RTTI/source-path string, COL/vtable address and object offset, target bytes, PE/import metadata, decoded direct instruction behavior.
2. **Strong inference:** repeated member offsets, parsed base relation, canonical MSVC destructor/thunk shape, source attribution supported by several xrefs/context clues.
3. **Provisional:** semantic method/member names, exact C++ types/signatures not proven by ABI/body, translation-unit ownership from proximity alone.

Never silently promote tier 2/3 claims to tier 1.

## Start here

```bash
make status
```

Primary evidence/work files:

```text
analysis/function_manifest.json
analysis/work_queue.json
analysis/class_dossiers.json
analysis/rtti_classes.json
analysis/vtables.json
analysis/vtable_overrides.json
analysis/class_layout_hints.json
analysis/deleting_destructors.json
analysis/vtable_thunks.json
analysis/vtable_write_xrefs.json
analysis/source_xrefs.json
```

Class dossiers are a join layer for convenience; always trace a claim back to the underlying evidence file before promoting a semantic name/type.

Important: a C++ type can have multiple vtables. Always use `object_offset`; do not assume the first vtable address seen for a class is the complete-object/primary vtable.

## Function iteration loop

1. Pick a target with defensible VA/extent.
2. Inspect class/slot and source hints:
   ```bash
   PYTHONPATH=. python3 tools/inspect_target.py --class UIControl --slot 34
   PYTHONPATH=. python3 tools/nearest_source.py 0x4703c0
   ```
3. Check whether the slot is inherited/overridden/introduced in `vtable_overrides.json`.
4. Check destructor/thunk/vptr-write evidence before reconstructing special members.
5. Keep uncertain code under `samples/`; promote into `src/krusty2/` only with strong translation-unit evidence.
6. Compile with VC6 SP3 when available; clang is only a bootstrap/code-shape oracle.
7. Match using relocation-aware function comparison.
8. If semantics are strong but shape differs, calibrate compiler/profile before contorting C++.
9. Regenerate `analysis/function_manifest.json` after adding candidates.

## Generated easy-probe gate

Do not hand-maintain trivial getter/setter probes. The analyzer classifies conservative patterns and generates C++98 automatically:

```bash
make easy-smoke
```

Current gate: 34/34 high-confidence probes exact under clang-cl's 32-bit MSVC ABI.

When VC6 is available:

```bash
make easy-smoke-vc6 VC6_ROOT="$VC6_ROOT"
```

A generated-probe failure under VC6 is toolchain/profile evidence first.

## Historical compiler gate

```bash
make probe-vc6 VC6_ROOT="$VC6_ROOT"
make vc6-gate VC6_ROOT="$VC6_ROOT"
```

High-value calibration targets:

- BaseObject constructor `0x00405120`
- BaseObject scalar deleting destructor `0x00405130`
- BaseObject destructor core `0x00405150`
- BaseObject::Release `0x00405170`
- UIControl slots 61/62
- UIStatic return-zero slot
- UIMultiState slots 34–37 (indexed 32-byte element access / SIB encoding)

## Constructors/destructors

Use `analysis/vtable_write_xrefs.json` and `analysis/deleting_destructors.json` together. A canonical scalar deleting-destructor wrapper gives the core destructor address directly. Vptr writes near that core help reconstruct base/derived destructor order.

For constructors, clusters of writes to all class subobject vtables are especially strong leads. Do not infer function start solely from a raw vtable immediate occurrence.

## Multiple inheritance

Use `analysis/rtti_classes.json` `vtable_records` and `analysis/vtable_thunks.json`. Preserve:

- secondary base offsets;
- virtual-base indicators (`pdisp`/`vdisp`);
- adjustor thunks.

Do not flatten a complex class to single inheritance merely because it makes source easier.

## Naming

Prefer `UnknownVirtualSlotN`, `field_0xNN`, etc. until behavior/callers/strings/assets support a semantic name. An identical tiny function address may be shared through inheritance or linker identical-COMDAT folding; address equality alone does not prove semantic method identity.

## Do not cheat matching

No inline assembly, naked functions, copied machine-code arrays, `.byte`, or linker tricks whose purpose is just to reproduce target bytes. We want readable C/C++ reconstruction.

## Useful commands

```bash
make status
make selftest
make easy
make smoke
make easy-smoke
make calibration
make class-evidence
make msvc-artifacts
make manifest
make work-queue

PYTHONPATH=. python3 tools/discover_easy_targets.py --class UIControl
PYTHONPATH=. python3 tools/inspect_target.py --class UIControl --slot 34
PYTHONPATH=. python3 tools/nearest_source.py 0x4703c0
```

## External references

`docs/REFERENCES.md` contains community resources such as the archived Motocross Madness file-format repository. Treat them as context, not ground truth for code/class reconstruction unless independently verified against the retail binary.
