# Allocation subsystem: separating accounting from runtime

## Result

The common target of the 145 recognized scalar deleting-destructor wrappers,
`0x004a30c0`, is not just a jump into a generic free implementation. It performs
category-based allocation accounting first. Treating that entire function as
replaceable Microsoft CRT code would lose observable behavior.

This pass reviews 21 functions in the supplied executable, with a SHA-256 check
for the input and for every selected function range. All behavioral labels,
record names and candidate symbol names below are ours; none is claimed to be
an original Rainbow source symbol. No original translation unit has been
assigned to this subsystem.

## The three layers

```text
Game/class code and compiler-generated deleting destructors
    |
    v
Application-side category accounting
  0x004a2e60 / 0x004a3060 / 0x004a30c0
    | size query                 | deallocation
    v                            v
  0x005351f0                   0x00537929
  allocation-size-like         free-like
    | small-block lookup / indexed locking / heap fallback
    v
Windows imports: HeapSize, HeapFree
```

Allocation follows a corresponding path through `0x0053789d`, its helper
`0x005378db`, and the Windows `HeapAlloc` fallback. The lower-level routines
have behavior consistent with a statically linked malloc/free/_msize family,
but **the exact CRT library/object identities remain unverified**. We have not
compared them with authentic VC6 library objects.

The API-boundary evidence is specific, not just a vague DLL attribution:

| Reviewed function | Imported API | Local IAT address | Call-site VA |
|---|---|---|---|
| `0x005351f0` | `KERNEL32!HeapSize` | `0x00550238` | `0x00535229` |
| `0x00537929` | `KERNEL32!HeapFree` | `0x00550198` | `0x00537969` |
| `0x005378db` | `KERNEL32!HeapAlloc` | `0x0055019c` | `0x00537920` |
| `0x0053821f` | `KERNEL32!EnterCriticalSection` | `0x0055025c` | `0x00538277` |
| `0x00538280` | `KERNEL32!LeaveCriticalSection` | `0x00550284` | `0x0053828d` |

These are local import-pointer slots, not addresses of the implementation inside
Kernel32. The graph records possible call paths, not a claim that all branches
execute on every allocation. Several deeper small-block routines are still
outside the reviewed graph.

## Named categories

The category selector at `0x004a2d00` has 55 decoded direct-call sites. A
conservative local argument recognizer finds literal arguments at 45 of them,
covering 15 distinct names:

```text
3DObjects  Audio      BikeRace  Collision  DebugOverlay
EcoSystem  Particles  QuadTree  Scene      Shadow
Sky        Startup    Terrain  TextureCache  UI
```

The initial category label is `Unclaimed`, referenced during tracker
initialization at `0x004a2b00`. Selection calls take a category name, find or
add it, replace the active index, and return the previous index.

This corroborates application-specific subsystem accounting. It does **not**
prove that the implementation came from a particular `.cpp`, or distinguish a
Rainbow game translation unit from a Rainbow/internal library.

The provisional, nonpolymorphic record layout is:

| Offset | Directly supported use |
|---|---|
| `+0x00` | Category count |
| `+0x04` | Buffer of 128-byte category-name records |
| `+0x08` | Per-category allocation counters |
| `+0x0c` | A second counter array; purpose not fully named |
| `+0x10` | Current category index |
| `+0x14`, `+0x18` | Initialized fields; semantic names withheld |

`0x0056df04` holds the active record pointer; its initial pointer value is
`0x006850c0`. `0x006850dc` is the fallback counter used when the record/counter
array is unavailable. This is another reminder that RTTI does not enumerate
all useful application structures.

## Source reconstruction and matching

`samples/allocation/AllocationAccountingProbe.cpp` contains ordinary C++98
candidates for the index setter, tracked deallocation, two allocation wrappers,
and a calloc-like wrapper. It neither overrides the host's real global delete
operator nor executes the original game.

**New exact match:** the category-index setter at `0x004a2d90` is **10/10 bytes**
under clang-cl's i686 MSVC ABI. It has no relocations and no ignored bytes.
The original method/class names and signedness remain unproved.

The other six target comparisons are calibration data:

| Candidate behavior | Target VAs | Retail size | clang size |
|---|---|---:|---:|
| Tracked deallocation | `0x004a2e60`, `0x004a3060`, `0x004a30c0` | 96 each | 73 |
| Allocate, then charge on success | `0x004a2e20` | 57 | 52 |
| Charge, then allocate | `0x004a3010` | 70 | 41 |
| Calloc, then charge on success | `0x004a2fc0` | 67 | 60 |

These are not counted as exact matches. The existing global smoke-suite
counters are not increased by claims of semantic similarity.

Two pairwise comparisons also establish that the three retail tracked-free
bodies have identical instruction layout/non-relocation bytes and identical
resolved call targets. Their raw bytes differ because the same callees need
different relative displacements at different addresses. This supports shared
behavior, not recovered names such as free/delete/delete[].

## Important behavior preserved

Null deallocation skips the size query but still invokes the lower-level free
entry. The current category is global; the examined wrapper does not look up
the category recorded when the block was allocated. In its category branch it
reloads the active record/index after the allocation-size call.

The two allocation wrappers are intentionally not merged: `0x004a2e20` charges
requested bytes only after success, whereas `0x004a3010` charges **before** the
allocation attempt, even when that attempt fails. Deallocation subtracts the
backend-reported size, not necessarily the original request size. The
reconstruction preserves these details rather than correcting them.

## Stronger comparison contract

`mcm2tool/resolved_match.py` applies i386 DIR32 and REL32 relocations using an
explicit address-binding map. It includes addends and the relocation-site VA,
then compares every resulting byte. Unknown symbols, unsupported relocations,
overlaps, and out-of-range relocations fail closed. A length mismatch cannot
produce a 100% score merely because the common prefix matches.

The binding map is itself reviewed evidence, not a symbol-identity proof. In
particular, binding a candidate called `probe_size_005351F0` to an address does
not establish that the original symbol was `_msize`.

This is an additive, stricter matcher for the new pass. The older mask-based
smoke tools are unchanged and their historical results are not silently
reclassified as fully resolved comparisons.

## Reproduce

After extracting the owned installer:

```bash
make allocation
make allocation-probes     # Requires clang-cl; no VC6 claim
make allocation-test       # No game/SDK inputs required
```

Equivalent direct invocation, usable from another working directory:

```bash
python3 tools/analyze_allocation.py --exe /path/to/mcm2.exe --compile-probe
```

Output is `analysis/allocation/allocation.json` plus `REPORT.md`, both generated
locally and ignored by Git/Docker. The JSON retains reviewed hashes, instruction
addresses, direct callers, literal category arguments, API edges, same-body
comparisons, exact probe results and code/config hashes. `--probe-object` accepts
a separately built i386 COFF candidate; it does not authenticate its compiler.

## Validation completed locally

- **33 tests passed**, including **14 native candidate-behavior scenarios** in
  one C++ test executable.
- All **21 reviewed target ranges** passed their individual SHA-256 checks
  against the known executable.
- Two independent complete runs, including candidate compilation, produced
  **byte-identical JSON and Markdown**.
- The PE, RTTI, MSVC-artifact and COFF modules used locally have the same Git
  blob hashes as their versions on `main` at `5d9f135`.
- No original game/installer/DLL was executed. The native test runs only our
  reconstructed C++ with stubbed allocation backends.

The native scenarios test candidate intent, not equivalence to an executed
original. VC6 has not been run, the old whole-project smoke suite was not rerun,
and no static-library signature identification is claimed here. The dedicated
GitHub workflow runs the input-free tests; its actual run status is separate
from these local results.

## Next gate

Compare the lower-level allocation family against privately supplied VC6
library objects using symbol, relocation-target and surrounding-call evidence.
Then promote only corroborated runtime identities into the provenance map.
The application-side accounting wrapper must remain in scope regardless.

## Primary API references

- HeapSize: https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapsize
- HeapFree: https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapfree
- HeapAlloc: https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapalloc
- _msize: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/msize

These contracts support interpreting named imports; they do not authenticate
the historical runtime library version or original source spelling.
