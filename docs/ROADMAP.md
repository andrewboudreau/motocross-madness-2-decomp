# MCM2 decomp bootstrap roadmap

## Goal

Build a reproducible, agent-friendly path from a user-owned retail MCM2 installer to readable C++ whose emitted historical-MSVC x86 code can be compared mechanically against the retail executable.

## Bootstrap v0.6 — current state

Completed:

- deterministic installer/CAB extraction of 104 shipped files;
- retail EXE hash + PE/Rich-header fingerprinting;
- 111 recovered original source/header filenames;
- 252 RTTI type descriptors / 249 logical class hierarchies;
- **fixed multi-COL parsing: 271 vtables including 22 secondary subobject vtables**;
- primary-vtable override/inheritance maps;
- direct class-layout evidence from tiny virtual bodies;
- 145 canonical VC6 scalar deleting destructors with destructor-core targets;
- 28 vtable `this`-adjustor thunks;
- 492 decoded vtable-write sites for constructor/destructor hunting;
- 57 conservative easy-function classifications;
- automatic C++98 probe generation for high-confidence patterns;
- **39/39 generated exact probes under clang-cl MSVC x86 ABI**;
- **19/19 hand-written exact samples**;
- **40 unique exact functions** total in the bootstrap validation corpus;
- 16 compiler-calibration targets, including BaseObject special members and FollowCamera slots 63/69/70/71/72;
- relocation-aware COFF matcher;
- Linux/Wine VC6 compiler wrapper + private-tree importer/fingerprinter;
- source-xref proximity evidence;
- structured agent queue in `analysis/function_manifest.json`;
- 249 joined class dossiers plus ranked `analysis/work_queue.json`;
- self-test that verifies workspace readiness, retail hash, analysis floors, and both clang gates;
- promoted `BaseObject` reconstruction under `src/reconstructed/`;
- FollowCamera exact preset slice plus state-toggle/cyclic-list/hidden-return-buffer/state-dispatch calibration targets.

## Next hard gate — remaining VC6 source/profile calibration

Native Windows SP3 execution is verified: 19/19 manual samples match without
relocation masking, 39/39 generated probes pass with their relocations resolved,
and calibration passes 8/16. Compiler acquisition is complete for this pass.
See [VC6_MATCHING.md](VC6_MATCHING.md) for the exact scope and remaining failures.

To reproduce on another machine with a privately owned SP3-patched `VC98` tree:

```bash
make import-vc6 VC6_SOURCE=/path/to/vc98-or-archive
make wine-init
make vc6-gate VC6_ROOT=$PWD/toolchains/vc6sp3
```

Calibration checklist (passing entries remain regression checks):

1. preserve 39/39 strict generated probe results (both global addresses are now validated);
2. preserve 19/19 hand-written exact smoke samples;
3. `BaseObject::BaseObject()` reproduces the retail `mov eax,ecx` / vptr / `refCount=1` shape;
4. the BaseObject scalar deleting destructor reproduces the 30-byte canonical VC6 wrapper;
5. validate the vptr destination in the currently masked-exact destructor core;
6. `BaseObject::Release()` becomes exact;
7. UI slots 61/62 reproduce VC6's two-register subtraction;
8. preserve the exact UIStatic return-zero `33 C0 C3` encoding;
9. preserve the four exact UIMultiState indexed-element methods.

Treat failures first as compiler-version/profile evidence, not permission to distort readable source.

## After compiler calibration

1. Lock empirically supported flags (`/O*`, `/G*`, runtime, exceptions, function-level linking).
2. Extend the promoted BaseObject model into GameObject and the next coherent base hierarchy.
3. Build coherent class slices rather than isolated functions:
   - GameObject/BaseObject
   - UIControl family
   - Vehicle/Bike
   - camera hierarchy
   - physics
   - quadtree/terrain
4. Use vtable-write sites + deleting-destructor cores to locate constructors/destructors.
5. Add callgraph and data/string xrefs around promoted functions.
6. Recover real field types only from use-site evidence.
7. Map reconstructed functions back to original translation units when attribution is strong.
8. Attempt whole-object matching before whole-executable matching.

## Bootstrap v1 exit criteria

- VC6 SP3 reproducibly compiling under Linux/Wine;
- compiler component hashes/version evidence recorded;
- BaseObject special members and `Release` exact under VC6;
- 50+ exact functions across several subsystems;
- coherent recovered declarations for at least BaseObject/GameObject/UIControl or Vehicle/Bike;
- machine-readable per-function/class evidence manifests;
- no copied machine code / inline-assembly match cheats;
- one-command agent workflow.
