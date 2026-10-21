# Terrain cache age manager

Terrain's two cache-manager pointers and the grid caches now use the existing
`AgeManager` and `AgeEntry` declarations. The former `TerrainOwned` and
`GridAgeManager` views named the same existing constructor, destructor and
methods with incompatible C++ symbols. No provider bodies are added or
replaced. `AgeEntry.h` contains the complete entry record; `AgeManager.h`
needs only its forward declaration. Consumers include the complete record
where they store or access it, preserving the existing compiler input order.

The class has no RTTI or virtual functions. Its name follows the literal
`D:\aardvark\VC\krusty2\AgeManager.cpp` at `0x005665c0`, used by the destructor
and registration allocator; class identity is a strong inference, while
method and member names remain provisional. Confirmed retail instructions,
allocation size, object fields, constructor/destructor calls and callback
arguments support the shared declaration, independently of shared addresses.

## Layout and lifetime evidence

The manager occupies `0x14` bytes: current age at +0, count at +4, capacity
at +8, pointer array at +`0x0c`, and accounted bytes at +`0x10`. Terrain's
loader allocates two `0x14`-byte objects and calls `0x00401000`; the resulting
pointers occupy Terrain+`0xc84` and +`0xc88`. Terrain's destructor calls
`0x00401020` on each before ordinary deletion. The manager destructor frees
its pointer array through the existing debug allocator; it does not destroy
the entries or invoke resource callbacks.

An entry is also `0x14` bytes: age, callback pointer, owner pointer, context
integer and byte size at offsets 0, 4, 8, `0x0c`, `0x10`. The grid mesh entry
is at draw-data+`0x144`; texture entries are at node-extra+`0x14` and
block-record+8. Native VC6 layout checks retain these offsets and both terrain
manager fields. TerrainShadow's existing byte view of the entry remains;
its `MarkUsed` call casts only that entry address to `AgeEntry*`.

`Register` stores an `int (__cdecl *)(void*, int)` callback. `EvictStale`
pushes the stored context and owner and calls the stored function pointer.
`EvictNodeBuffers` at `0x0047ecc0` and `EvictNodeTexture` at `0x0047ca00`
therefore gain an unused second integer parameter. Their complete emitted
bodies remain strict, 231 and 56 bytes respectively. The block-texture
callback already uses its second argument as the block index. None requires
a forwarding wrapper or changes stack cleanup.

## Existing complete providers

| Method | Retail VA | Full strict bytes |
|---|---:|---:|
| Constructor | `0x00401000` | 19 |
| Destructor | `0x00401020` | 24 |
| `AdvanceAge` | `0x00401040` | 3 |
| `Register` | `0x00401050` | 122 |
| `Unregister` | `0x004010d0` | 92 |
| `TotalSize` | `0x00401130` | 73 |
| Internal sort comparator | `0x00401180` | 34 |
| `EvictStale` | `0x004011b0` | 150 |
| `MarkUsed` | `0x00401250` | 11 |

These are the actual `src/reconstructed/AgeManager.cpp` definitions. The
native object uses only `DebugRealloc`, `DebugFree` and CRT `qsort`. Complete
compiler-provided extents, bytes and resolved relocations are checked; the
internal comparator's extent comes from CodeView procedure metadata.

The existing behavior is retained, including its accounting details:

- Registration grows the pointer array by 1,000 entries. Allocation failure
  leaves the manager and new entry unchanged.
- `Unregister` swaps the last pointer into the removed slot and clears the
  vacated slot; it does not subtract the entry's size from `totalBytes`.
- `EvictStale` returns early only when total bytes are strictly below the
  limit. Otherwise it sorts newest first and visits stale entries from the
  end. A callback's nonzero result counts as success.
- The byte debit uses the size captured before calling the resource. Real
  grid callbacks clear their entry size. The manager applies total/count
  updates after the callback loop, reducing count by successful callbacks;
  it does not separately compact failed entries.
- Age stamps and stale comparisons are unsigned. These connections do not
  redefine the sort comparator or repair the manager's existing behavior.

## Validation

`tests/test_age_manager_connections.py` compiles the affected canonical and
sample translation units and retains **175 complete strict comparisons**.
It checks actual COFF imports/definitions, the shared layouts, the two callback
signatures and the retail indirect callback sequence. Reconstructed consumers
compile without an added physics include path; TerrainShadow uses its normal
physics include configuration.

The separate native fixture links the actual production manager object with
real CRT `qsort` and scripted debug allocation boundaries. **2,308 cases**
cover registration growth and failure, destruction, unregistration, stale and
current entries, callback success/failure combinations, exact owner/context
values, callback-visible accounting, captured byte sizes, limit boundaries,
`MarkUsed`, and unsigned age wrap. The fixture's resource callbacks are
scripted; it does not claim to run terrain resource creation or rendering.
The actual grid callbacks are independently covered by full strict matching.

Private before/after comparison additionally retains every non-debug section,
raw byte, relocation site/type/addend and destination in nine whole objects:
Terrain, TerrainSupport, Griddraw, Grid1, their two grid samples, TerrainShadow,
AgeManager and EcoSystem. Only the nine explicitly mapped external symbol
spellings change; local targets are identified by section and offset. This
includes the existing partial bodies without treating them as retail matches.
The reports are `work/game-build/age-object-preservation.json` and
`age-renames.json`. Separate checks preserve all 20 complete function bodies
and relocation records across EcoSystemNearMisses, GridNodeNearMisses and
GridNode (`age-extra-preservation.json`). The baseline is `364d4e27`.

The isolated source build `build-ev5772um` compiles all 201 translation units
and reduces unresolved symbols from 515 to 509, with six missing names
removed and none added. The remaining renames affect already supplied or
sample-only references. The real link still fails; cache rendering and the
source-built game's startup, menus and racing remain unverified.

```powershell
$env:PYTHONPATH = '.'
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python tests/test_age_manager_connections.py -v
```
