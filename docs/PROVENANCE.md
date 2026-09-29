# Code provenance: deliverable, plan, and first implementation

## Deliverable

A reproducible answer to **what must be reconstructed, what can be supplied by an external library, and what is still unknown** for the supplied MCM2 executable. The canonical deliverable is the tooling in this repository plus a hash-bound generated evidence snapshot, not another speculative source tree.

The first implementation produces:

| Output under `analysis/provenance/` | Purpose |
|---|---|
| `provenance.json` | Candidate entry addresses, discovery reasons, independent ownership/role labels, source references, class uses, decoded ranges, external calls, unresolved dispatches and input/tool hashes |
| `imports.json` | Normal imports by DLL and name/ordinal, with lookup-table and local IAT slot addresses |
| `components.json` | Packaged/missing DLL inventory, imports, exports/forwarders, version resources, identity strings and hashes |
| `source_units.json` | Literal source/header paths and referencing candidates; explicitly no invented translation-unit ranges |
| `REPORT.md` | Scope summary, corrections, limitations and next evidence to collect |

An address-query command lets agents inspect one candidate or local IAT slot without rescanning the executable.

## Plan and gates

### 1. Establish binary and dependency boundaries — implemented

Parse imports, IAT storage locations, ordinal imports, packaged DLL exports, forwarders and version resources. Inventory packaged support DLLs separately from modules in the EXE's normal import table. Check the known input hash before applying MCM2-specific assumptions. Never execute the installer, game, or DLLs.

**Gate:** synthetic tests cover named/ordinal imports, bound-IAT ambiguity, export forwarders, resource traversal and malformed input. The supplied executable and six packaged DLLs are inspected without loading them.

### 2. Build a conservative internal map — first pass implemented

Seed candidate entry points from RTTI vtables, direct calls, compiler artifacts, import jump stubs and the entry point. Use GNU objdump's linear disassembly and bounded control-flow traversal to gather reachable instruction ranges and decoded source-string references. Do not traverse into call targets as though they belonged to the caller.

Keep two independent axes:

- **Ownership evidence:** project source reference observed, class/filename hypothesis, linker glue, or unknown.
- **Code role:** ordinary/unknown, deleting destructor, adjustor thunk, import thunk, or delete/allocation-subsystem candidate.

A compiler-generated destructor can belong to a Rainbow class. Conversely, a game function calling DirectX remains game-side code; calling a library never transfers ownership to that library.

**Gate:** unknowns stay unknown, original TU assignments remain null, overlapping instruction ranges are unioned for coverage, and no completion percentage is produced. Repeated builds must produce identical artifacts.

### 3. Establish static-library identities — next

Compare candidates with privately supplied VC6 library objects and independently attributable third-party libraries. Record library/component hashes, symbol identity, relocation targets, and comparison evidence. A masked-byte similarity alone is not proof of library identity. The common delete target might include application allocation bookkeeping, so it is not automatically Microsoft CRT code.

**Gate:** confirmed library labels require a reviewable identity/signature match. Until then, keep the provider unknown. No proprietary compiler/library payloads enter Git.

### 4. Improve original-source attribution — next

Use disassembler function boundaries, switch-table recovery, caller/callee relationships and class construction evidence. Resolve COM interface identities and dynamic loader calls where possible. Corroborate source anchors with independent clues before promoting a TU assignment. Preserve shared/inlined code rather than forcing every range into exactly one `.cpp` file.

**Gate:** explicit uncertainty/conflict handling, a documented evidence chain for promoted ownership, and separate counts for confirmed, inferred and unresolved cases.

## Run

Python 3.10+ and GNU binutils (`objdump`) are required. The historical VC6 compiler is **not** needed for this provenance pass.

```bash
# Starting with an owned installer:
make extract INSTALLER=/path/to/MCM2PCG.exe
make provenance
make provenance-test

# Or use an already extracted executable:
python3 tools/build_provenance.py --exe /path/to/mcm2.exe

# Query the common delete/allocation target:
python3 tools/build_provenance.py --address 0x004a30c0
```

The default output is `analysis/provenance/`, ignored by Git and Docker. `--game-dir` changes the directory scanned for packaged DLLs. With `--exe`, its parent is the default component directory. `--out` changes the snapshot destination.

Unknown executable hashes fail by default. `--allow-unknown-build` explicitly opts into exploratory analysis of a different PE32/i386 input; it does not validate that input as the known MCM2 build. Address queries refer to the preferred addresses of the snapshot's EXE, not runtime ASLR addresses or arbitrary DLL address spaces.

## What this does not establish

The normal import table is not a complete DirectX call graph: COM methods dispatch through interface vtables, and dynamic loader calls can resolve additional APIs. Delay imports are detected but not expanded in this first pass. Ordinal-only imports are preserved rather than guessed from a modern DLL.

The 107 observed `.cpp` filenames are a lower-bound set of embedded names, not proof that the original project contained exactly 107 implementation files. A `__FILE__` value can originate in a header or inlined function. Raw nearby strings alone are never promoted into whole-file address ranges.

GNU objdump boundaries and direct-call discoveries are heuristics. Decoded instruction coverage is **not** decomp progress, original-source recovery, or authorship coverage. Neither all Rainbow code nor all static CRT code is identified by this first pass.

## First-pass corrections

See `PROVENANCE_FIRST_PASS.md` for measured results. In particular, the supplied `blade.dll` identifies itself as **Microsoft Blade Software Rasterizer** and exports DirectDraw entry points. The earlier assumption that this was a Rainbow engine DLL is not supported. Also, the lone `d3drm.dll` import is `D3DRMVectorRotate`; that helper alone does not identify the renderer as Direct3D Retained Mode.

## Validation performed

- 21 synthetic unit tests pass without the game or VC6.
- The supplied installer was extracted as data; the EXE hash matches the known input.
- Two independent provenance runs produced byte-identical JSON and Markdown outputs.
- The PE, RTTI and compiler-artifact modules used for the run have the same Git blob hashes as their current GitHub versions.
- The corrupted GitHub copy of `tools/build_class_dossiers.py` was restored from the verified GitHub-ready seed, syntax-checked and run to regenerate 249 dossiers. This repair is separate from the provenance analyzer, which does not consume cached dossier output.

No Windows VM, VC6 execution, full modern smoke-suite rerun, or GitHub-hosted CI success is implied by these checks. The provenance pass is independent of compiler byte-match progress.

## Primary technical references

- PE/COFF imports, IATs and export tables: https://learn.microsoft.com/en-us/windows/win32/debug/pe-format
- COM interface pointers and vtables: https://learn.microsoft.com/en-us/windows/win32/com/interface-pointers-and-interfaces
- Linker object/library inputs: https://learn.microsoft.com/en-us/cpp/build/reference/link-input-files
