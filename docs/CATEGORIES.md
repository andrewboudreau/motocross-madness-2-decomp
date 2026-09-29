# Category-driven source triage

## Deliverable

`make categories` builds a many-to-many evidence map from allocation-category
selection sites to candidate code ranges, literal source paths and RTTI virtual
method uses. It is a source-navigation and review tool, not a reconstruction of
the original directory hierarchy.

Generated outputs, ignored by Git and Docker:

```text
analysis/categories/
    category_map.json
    review_queue.json
    REPORT.md
```

Each record preserves the selector call VA, recovered argument-string VA,
intervening instructions, candidate entry/ranges, decoded source-reference VAs
and non-inherited primary vtable uses. Unknown arguments and ambiguous
containing candidates remain explicit. The JSON records input, configuration,
provenance-snapshot and tool hashes.

## Measured first pass

For `mcm2.exe` SHA-256
`31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874`:

| Measure | Result |
|---|---:|
| Observed selector call sites | 55 |
| Calls with recovered literal labels | **52** |
| Remaining unresolved arguments | 3 |
| Distinct literal categories | 15 |
| Containing candidate routines | 25 |
| Candidates selecting multiple categories | 6 |
| Source/category co-occurrence edges | 28 |
| Distinct source paths in those edges | 11 |
| Class/category co-occurrence edges | 8 |
| Distinct RTTI classes in those edges | 5 |
| Original translation-unit assignments made | **0** |

The previous allocation analyzer recovered 45 literal arguments. Seven more are
recovered here by accepting intervening register moves, memory-to-register moves
and DWORD writes to stack locations above the top argument. The earlier
allocation analyzer is unchanged; its more restrictive 45-site result is not
silently rewritten.

The recognizer still rejects arbitrary pointer writes which could alias the
argument, writes to ESP or the top stack argument, nonliteral pushes, intervening
calls, decoding gaps, unsupported instructions, and known incoming branches
which could bypass the argument push. It is bounded to 16 instructions/96 bytes.
This is not a general stack/alias analysis or an indirect-control-flow proof.

## Useful source/class connections

These are observed co-occurrences, not exclusive ownership assignments:

| Evidence | Candidate entry | What it supports |
|---|---|---|
| `Collision` + `CarProcedural.cpp` + a `CarProcedural` primary virtual use | `0x0042f600` | A concrete collision-related method investigation with independent source/class clues |
| `EcoSystem` + `EcoSystem.cpp` + an `EcoSystem` primary virtual use | `0x0045aad0` | A concrete ecosystem-related method investigation |
| `UI` + `GUIManager` primary virtual uses | `0x004857f0`, `0x00485830`, `0x00485870` | Three UI-context methods; no direct source path established by this pass |
| `Terrain` + `Terrain` primary virtual uses | `0x00506220`, `0x00507610` | Two terrain-context methods; this does not label every Terrain method |
| `Terrain` + `Terrain.cpp` source reference | `0x005079f0` | A separate source-path anchor; no virtual ownership asserted |
| `DebugOverlay` + `Game.cpp` source reference | `0x00467b70` | A game-side debug-overlay context, not proof of a separate DebugOverlay.cpp |

Class uses are recomputed from the target's RTTI/vtables. Unchanged inherited
primary slots and ambiguous primary-base relationships do not create duplicate
class attribution. Secondary tables are not flattened into primary methods.

## The most important result: mixed-category orchestration

The candidate at **`0x004de590`**, which contains decoded references to
**`QuarryStuntEvent.cpp`**, selects twelve distinct labels:

```text
3DObjects, Audio, BikeRace, Collision, EcoSystem, Particles,
QuadTree, Scene, Shadow, Sky, Terrain, TextureCache
```

That is a strong lead for a loading/orchestration routine. It would be wrong to
classify the entire candidate, source file, or all its callees as Terrain code
because one part selects `Terrain`.

The other mixed-category candidates are:

| Candidate | Same-candidate source reference | Labels |
|---|---|---|
| `0x00417ed0` | `bikerace.cpp` | Audio, BikeRace |
| `0x004598d0` | Not established | EcoSystem, QuadTree |
| `0x00459ce0` | Not established | EcoSystem, QuadTree |
| `0x004ecd60` | `SceneManager.cpp` | Collision, Scene |
| `0x00521050` | `trkgame.cpp` | Audio, Scene, Startup, UI |

The review queue keeps these in a `mixed_category_orchestrator` lane, separate
from single-category anchors and unresolved-argument candidates. The existing
byte-match manifest is not modified by this triage view.

## Three arguments deliberately left unresolved

- `0x00401771`: argument is pushed through a register, not an immediate literal.
- `0x004dfa95`: unsupported pointer-based writes occur between the literal push and call.
- `0x00500fca`: unsupported pointer-based writes occur between the literal push and call.

Nearby strings or earlier allocations are not substituted for argument proof.
The last two may be recoverable with alias/stack analysis, but this pass does not
silently assume their writes cannot modify the argument.

## Run and query

After extracting the owned installer:

```bash
make categories                 # Regenerates provenance, then builds this map
make categories-test            # Synthetic tests; no game or VC6 required

python3 tools/build_categories.py --query Terrain
python3 tools/build_categories.py --query GUIManager
python3 tools/build_categories.py --query QuarryStuntEvent.cpp
python3 tools/build_categories.py --query 0x004de590
```

An existing provenance snapshot bound to the same input can also be used:

```bash
python3 tools/build_categories.py \
    --exe /path/to/mcm2.exe \
    --provenance /path/to/provenance.json \
    --out /path/to/category-output
```

Python 3.10+ and GNU objdump are sufficient. VC6 and a Windows VM are not needed.
Queries read an existing map and return category-anchor candidates only, not an
exhaustive list of every function implementing a subsystem.

The selector body hash is checked before disassembly. Candidate ranges come from
the provenance snapshot; the new tool re-decodes the executable and reconstructs
source-reference observations and class uses rather than trusting cached
ownership labels. All 55 observed calls had one containing candidate in the
measured snapshot, but those boundaries remain heuristic, not original symbols.

## Validation completed

- **28 new synthetic tests passed**: argument recovery/barriers, branch entrances,
  hash/schema checks, ambiguous/unmapped containers, mixed categories, fresh
  source-reference extraction and non-propagation to callees/inherited methods.
- **33 existing allocation tests passed**, including the 14 native candidate
  behavior scenarios. Total across these two suites: **61 passing tests**.
- The allocation pass was rerun against the extracted input; all 21 reviewed
  hashes validated and its strict candidate result remained 1/7, as before.
- Two independent category runs produced byte-identical JSON and Markdown.
- Terrain, filename and address queries were exercised on the generated map.
- The local allocation, PE, RTTI, MSVC and COFF modules and allocation config
  match the Git blob hashes on `main` at `02b4f73`.

The local category integration used the previously generated, input-hash-bound
provenance snapshot. The old full bootstrap and historical VC6 suite were not
rerun. No original game executable or DLL was executed, no files were moved,
and no additional byte matches or proven original translation units are claimed.
The dedicated GitHub workflow runs both input-free test suites; its observed
run status is separate from these local checks.

## Next use of this map

Start with the `CarProcedural`/Collision and `EcoSystem` anchors, where source
references and virtual-method evidence agree, while investigating `0x004de590`
as orchestration rather than exclusive subsystem code. Recover select/restore
lifetimes and branch-sensitive call contexts before annotating downstream calls.
Keep shared allocators and utility routines outside automatic category ownership.
Authentic VC6 library-object comparison remains a separate, still-unresolved gate.
