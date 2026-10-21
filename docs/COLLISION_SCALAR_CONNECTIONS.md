# Collision segment scalar calls

The complete `SegmentTouchesObject` candidate at `0x00439820` now uses the
existing `Vec3DotCall` twice and `CollisionLength` once for its bounding-sphere
pretest. The old `SegmentDot` and `SegmentLength` declarations referred to the
same retail destinations through const-reference signatures. The new calls
pass the same addresses through the existing pointer signatures.

| Provider | Retail VA | Complete strict body |
| --- | --- | ---: |
| Vec3DotCall | `0x0040ae30` | 29 bytes |
| CollisionLength | `0x00435ec0` | 74 bytes |

The numerator and final distance receive vectors returned by the unchanged
by-value addition/subtraction calls. Binding each returned vector to a const
reference and taking its address retains that temporary until the end of the
full expression. No vector copy, arithmetic operation, wrapper or lifetime
extension beyond the call is added. The length call views the same three float
words through `CollisionVec3`; both layouts retain offsets 0, 4 and 8.
`CollisionLength` is already defined in the same production translation unit.

Authentic VC6 comparison preserves every byte of the **1,493-byte caller**, its
stack slots, call argument order, cleanup and return handling. It remains a
nonexact candidate with **1,309 matching positions** against retail. All **135
nondebug sections and 722 relocations** of `CollisionObject.cpp` are unchanged
under only the two named provider substitutions. The old external length
reference resolves to the already-defined canonical section and offset.
Compiler-local label spellings are compared by their actual destinations.

`tests/test_collision_query_connections.py` proves both complete providers
strict, verifies the three actual caller relocations and corresponding retail
call counts, and retains 193 strict cases and all 31 complete partial
fingerprints, including the aggregate that includes `CollisionObject.cpp`.
The existing native query fixture still passes 10,535 boundary cases; these
are query-helper tests, not a new end-to-end segment simulation. The scalar
connections are established by complete caller preservation and strict
provider bytes. Addition/subtraction call views remain unresolved.

The isolated real link `work/game-build/build-n263_9t1` compiles 228 translation
units and removes exactly the two old scalar names: **323 to 321 unresolved
symbols**, with no added missing names. The final composition preserves all
227 old objects, 10,512 nondebug sections and 45,428 relocations under these
two substitutions. Private proof is in `work/game-build/round34-all-objects.json`
and `work/game-build/collision-vector34/independent-review.json`.

```powershell
$env:PYTHONPATH='.'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_collision_query_connections.py -v
```

This preserves an existing incomplete collision candidate. It does not prove
complete collision behavior, source ownership, startup, menus or racing.
