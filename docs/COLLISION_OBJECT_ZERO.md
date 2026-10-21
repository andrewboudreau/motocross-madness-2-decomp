# CollisionObject initialized zero-vector connection

The constructor's two reads of retail `0x005797b0` now use the existing TU-local
`kVec3Zero` emitted by `Math3D.h`. `CollisionVec3` remains a provisional 12-byte
field view, expressed explicitly at these two assignments. This creates no new
storage or initializer and makes no claim of complete C++ type identity.

## Storage and constructor evidence

The three DWORDs at `0x005797b0..0x005797bc` occupy writable PE zero-fill storage.
Each component has exactly three encoded references: one initialization write
and two constructor reads. This scan does not exclude indirect accesses.
The CRT entry at `0x005660ec` points to the complete five-byte wrapper at
`0x004356f0`, which jumps to the complete 60-byte initializer at `0x00435700`.
That initializer stores three zero components in the same storage.

The complete 346-byte constructor at `0x00431e70` copies those components to
`hitPoint` at `+0xa0` and `hitNormal` at `+0xac`. Its primary vptr and secondary
GraphicsTest subobject at `+0xc` are unchanged. Authentic VC6 layout checks
establish the two 12-byte vector layouts, component offsets `0/4/8`, the two
destination offsets, and `sizeof(CollisionObject)==0xb8`.

## Preservation and limits

Both the canonical source and its existing near-miss sample retain every
nondebug section, raw byte, flag, relocation addend and destination under exactly
one connection: the old external zero-vector name now resolves to the existing
private `_kVec3Zero` at BSS offset zero. Their complete objects contain respectively
135 sections/722 relocations and 136 sections/732 relocations. The same 60-byte
BSS and four `.CRT$XCU` entries remain; no other vector initializer changes.
The private before/after comparison excludes no filename sections. Retained
portable pins normalize only recognized source-path roots.

`tests/test_collision_object_zero.py` checks 18 complete strict bodies across the
two objects: the constructor and all four vector initializers and their wrappers.
It checks the full objects, raw storage/reference evidence, CRT order and the
nine relocations to the local zero vector. Existing collision query, query-state
and point-storage suites also pass, including all 31 partial-body fingerprints.

This is complete byte, storage and ABI evidence for these reads and initialization;
it does not execute the entire CollisionObject constructor or prove integrated
collision behavior. The unused legacy declaration in `CollisionTypes.h` remains
outside this narrow change. Its obsolete canonical binding is removed.

The real game link compiles all 258 translation units and removes exactly the
zero-vector import, reducing unresolved names from 228 to 227 with none added.
The source-built game still requires further providers before runtime testing.

```powershell
$env:PYTHONPATH='.;tests'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_collision_object_zero test_collision_query_connections test_collision_query_state_connections test_collision_point_storage -v
```
