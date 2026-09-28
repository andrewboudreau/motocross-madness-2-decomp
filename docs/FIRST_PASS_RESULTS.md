# Current decomp results — bootstrap v0.5

## Recovered project surface

The retail binary exposes `D:\aardvark\VC\krusty2\` paths for **107 C++ translation units and 4 headers**. `src/krusty2/` preserves those original filenames as skeletons.

MSVC RTTI recovery now distinguishes logical class hierarchies from concrete vtables:

```text
252 RTTI type descriptors
249 logical class hierarchy records
271 vtables
22 secondary base-subobject vtables
17 multi-vtable / multiple-inheritance classes
```

This corrects the older one-vtable-per-type assumption.

## Automated small-function corpus

The conservative recognizer currently finds **51** mechanically obvious vtable bodies across patterns including:

- integer/float getters;
- member setters;
- member copies;
- conditional setters;
- constant/no-op returns;
- address-of-member;
- nested float access;
- output-pointer writes;
- compiler-sensitive zero/boolean/subtraction shapes.

For every `compiler_stability=high` pattern, `tools/generate_easy_probes.py` emits ordinary C++98 and `tools/run_easy_probes.py` compiles/matches it. Current result:

```text
34 / 34 exact after legitimate COFF relocation masking
```

The hand-written sample corpus remains:

```text
14 / 14 exact
```

The union is **35 unique exact retail function addresses**. Clang is only validating source semantics/MSVC ABI plumbing; historical compiler claims wait for VC6.

## BaseObject seed

RTTI identifies the BaseObject vtable at `0x005507c0` and the following retail cluster:

```text
0x00405120 constructor
0x00405130 scalar deleting destructor
0x00405150 destructor core
0x00405160 slot 1: increments [this+4]
0x00405170 slot 2: decrement/release/delete path
0x00401940 slot 3: returns [this+4]
```

The constructor bytes prove the field starts at 1:

```asm
mov eax,ecx
mov dword ptr [eax],005507c0h
mov dword ptr [eax+4],1
ret
```

This strongly supports:

```cpp
BaseObject::BaseObject() : refCount(1) {}
```

The canonical deleting-destructor wrapper calls core `0x00405150` and common operator-delete target `0x004a30c0`. The core restores the BaseObject vptr. These special members are now explicit VC6 calibration targets.

## Multiple inheritance / vtable mechanics

The corrected RTTI pass exposes real secondary-subobject vtables. Examples:

```text
CollisionObject : QuadTreeObject, GraphicsTest
  vtables at +0, +12

Tire : CollisionObject, MovingPart, CollisionPoint
  vtables at +0, +12, +184

Bike
  vtables at +0, +540, +1848
```

The analyzer also finds **28** short `this`-adjustor thunks and **145** canonical scalar deleting-destructor wrappers. See `docs/CLASS_MODEL.md`.

## Constructor/destructor evidence

`analysis/vtable_write_xrefs.json` contains **492** decoded immediate vptr writes across all 249 classes. These provide a direct constructor/destructor hunting queue instead of relying only on proximity or guesswork.

Examples:

```text
BaseObject vtable writes: 0x405122, 0x405150
UIControl vtable writes:  0x4701a4, 0x47046d
Vehicle: six vtable writes covering +0, +540, +1472 layouts
Bike:    six vtable writes covering +0, +540, +1848 layouts
Tire:    six vtable writes covering +0, +12, +184 layouts
```

## Compiler calibration lane

Eleven semantically strong targets are intentionally allowed to differ under modern clang:

- BaseObject constructor
- BaseObject scalar deleting destructor
- BaseObject destructor core
- BaseObject::Release
- UIControl slots 61/62
- UIStatic return-zero slot
- UIMultiState slots 34–37 (32-byte indexed element getters/address)

The first three are especially useful because they exercise old MSVC special-member rules rather than merely instruction selection.

## Agent manifests

Use these together:

- `analysis/function_manifest.json` — function evidence manifest
- `analysis/work_queue.json` — ranked validated/unresolved agent queue
- `analysis/class_dossiers.json` — joined evidence record for each RTTI class
- `analysis/vtable_overrides.json` — primary virtual ownership
- `analysis/class_layout_hints.json` — direct field-offset evidence
- `analysis/deleting_destructors.json` — destructor wrappers/cores
- `analysis/vtable_thunks.json` — MI/virtual-base adjustment
- `analysis/vtable_write_xrefs.json` — ctor/dtor vptr-write leads
