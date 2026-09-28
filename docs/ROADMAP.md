# MCM2 decomp bootstrap roadmap

## Goal

Build a reproducible, agent-friendly path from a user-owned retail MCM2 installer to readable C++ whose emitted historical-MSVC x86 code can be compared mechanically against the retail executable.

## Bootstrap v0.5 — current state

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
- 51 conservative easy-function classifications;
- automatic C++98 probe generation for high-confidence patterns;
- **34/34 generated exact probes under clang-cl MSVC x86 ABI**;
- 14/14 hand-written exact samples;
- **35 unique exact functions** total in the bootstrap validation corpus;
- 11 compiler-calibration targets, including BaseObject ctor/dtor/delete wrapper/Release;
- relocation-aware COFF matcher;
- Linux/Wine VC6 compiler wrapper + private-tree importer/fingerprinter;
- source-xref proximity evidence;
- structured agent queue in `analysis/function_manifest.json`;
- 249 joined class dossiers plus ranked `analysis/work_queue.json`;
- self-test that verifies workspace readiness, retail hash, analysis floors, and both clang gates.

## Next hard gate — authentic VC6 SP3 oracle

With a privately owned SP3-patched `VC98` tree:

```bash
make import-vc6 VC6_SOURCE=/path/to/vc98-or-archive
make wine-init
make vc6-gate VC6_ROOT=$PWD/toolchains/vc6sp3
```

Highest-value outcomes:

1. all 34 generated high-confidence probes stay exact under VC6;
2. all 14 hand-written smoke samples stay exact;
3. `BaseObject::BaseObject()` reproduces the retail `mov eax,ecx` / vptr / `refCount=1` shape;
4. the BaseObject scalar deleting destructor reproduces the 30-byte canonical VC6 wrapper;
5. `BaseObject::~BaseObject()` reproduces the retail vptr reset;
6. `BaseObject::Release()` becomes exact;
7. UI slots 61/62 reproduce VC6's two-register subtraction;
8. return-zero functions settle the `33 C0` encoder/profile behavior;
9. UIMultiState slots 34–37 settle VC6 SIB/LEA codegen for indexed 32-byte elements.

Treat failures first as compiler-version/profile evidence, not permission to distort readable source.

## After compiler calibration

1. Lock empirically supported flags (`/O*`, `/G*`, runtime, exceptions, function-level linking).
2. Promote BaseObject into a real recovered header/source model.
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
