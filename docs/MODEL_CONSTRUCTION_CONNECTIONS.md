# Model construction and scene-node connections

CarProcedural and D3DIMSoultreeCharacter now allocate the existing canonical
D3DIMSoultreeObject implementation. Car setup also calls the existing
SoultreeObject lookup, position and local-bounds providers. The rendering
declaration of GetLocalBounds uses the same Vec3 pointer signature. This removes
six missing call-view names without adding implementation bodies or wrappers.

| Provider | Retail entry | Complete bytes | Connected callers |
| --- | --- | ---: | --- |
| D3DIMSoultreeObject constructor | `0x43f160` | 275 | Car slot 27 once; Character slot 0 twice and slot 9 once |
| SoultreeObject::FindByName | `0x4fdae0` | 84 | Car slot 27, seven named-node lookups |
| SoultreeObject::GetPosition | `0x4fc970` | 33 | Car slot 27, body translation |
| SoultreeObject::GetLocalBounds | `0x4fe0a0` | 74 | Car slot 27 and model ComputeSubtreeBounds |

All four providers are complete strict matches with resolved relocations.
The affected complete callers are Car slot 27 (`0x42f600`, 1,020 bytes),
Character slot 0 (`0x445a70`, 1,021 bytes), Character slot 9 (`0x445ec0`,
248 bytes), and ComputeSubtreeBounds (`0x4434b0`, 243 bytes).

## Receiver and ownership evidence

Each of the four model allocations remains `0x2d8` bytes with the same debug
allocator arguments, constructor argument and failure path. Retail model RTTI
at primary table `0x5513ec` and secondary table `0x55137c` places SoultreeObject
at zero and GameObject at `+0xc`. The constructor writes the corresponding
primary and secondary vptrs. The compiler still uses the primary model address
for construction, lookup and geometry calls. Existing GameObject child-list
insertion retains its separate `+0xc` conversion.

CarProceduralModel and the motion source's local ModelObject remain limited
views for their other accesses. The newly allocated canonical model is cast
back to the appropriate view without changing the pointer. Their allocation
size and base conversion agree with the canonical declarations. The motion
view's texture manager, LOD count and LOD table remain at `+0x240`, `+0x274`
and `+0x28c`. Its existing virtual CopyFrom/Load calls and ownership transfers
are unchanged. This does not assert equivalence of every provisional method
signature or consolidate the complete rendering and physics class views.

FindByName returns the primary scene pointer and compares names without case,
visiting siblings before children. GetPosition returns its explicit output
pointer after copying three DWORDs from local translation `+0xe8`. Car ignores
this return, just as its former void declaration did. GetLocalBounds calls
primary virtual slot 5 when the cache flag at `+0x154` is zero, then copies
the vectors at `+0x158/+0x164` to its two writable output pointers. Vector3 and
Vec3 have identical 12-byte xyz storage; explicit casts preserve these output
addresses and their alias behavior.

## Verification

`tests/test_model_construction_connections.py` compiles the real sources with
VC6 SP3 and checks 158 complete strict cases, all four constructor call sites
and allocations, both RTTI offsets, and both existing rendering near-miss
functions' complete resolved candidate fingerprints. Those two candidates
remain nonexact. Compile-time layout guards include the actual local
ModelObject declaration from the motion source rather than a copied fixture
declaration. Both local views and canonical model declarations emit the same
`+0xc` GameObject conversion.

The native fixture executes the unchanged lookup, position and bounds helper
source extracted into a separate object. Each extracted body and its actual
production object independently match the full retail extent before execution.
The 17,416 cases cover lookup order/case/misses, explicit return-pointer identity,
raw DWORD vector payloads, cached and uncached bounds, the primary virtual cache
callback, identical outputs, and overlapping outputs/source fields. Entire node
and output storage is compared. The cache callback is a fixture dependency;
this is not an end-to-end model loader or rendering test.

The private same-path before/after audit covers CarProcedural,
D3DIMSoultreeMotnctrl, D3DIMSoulTree, soultree and D3DIMSoulTreeNearMisses:
all 362 nondebug sections and 1,512 relocations retain their raw bytes,
attributes and destinations under only the six verified external renames.
Local labels are identified by destination section and offset. No function
extent, instruction bytes or relocation addend is normalized away.
The isolated real-build objects also preserve all 211 pre-existing objects,
10,449 nondebug sections and 45,229 relocations under those same six renames,
covering every compiled header consumer.

The isolated real build compiles 211/211 translation units and reduces missing
symbols from 429 to 423, exactly six removed and none added. A functional game
still requires completing the link and testing startup, menus and racing.

```powershell
$env:PYTHONPATH='.'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python tests/test_model_construction_connections.py
python tools/build_game.py -j6
```
