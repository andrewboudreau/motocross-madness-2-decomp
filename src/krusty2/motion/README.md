Candidate implementations for this area are preserved in `samples/physics/motion/`.
Shared headers stay here. See [physics validation](../../../docs/PHYSICS_VALIDATION.md).

# motion (Spheres.cpp, SteeringControl.cpp)

Validation: counts labeled "exact" below are historical relocation-masked
diagnostics, not strict acceptance. Use `tools/run_physics_samples.py --strict`
with reviewed bindings before accepting these candidates.

Wave 4. Both files are linked back to back in `0x504940..0x5051ec`; SurfaceMap and the keyboard hooks
that follow live in `samples/physics/motion/`.

## Spheres.cpp
- `__FILE__` string `D:\aardvark\VC\krusty2\Spheres.cpp` at `0x00574568`; one use (debug `new`, line 4) in the
  `$E` initialiser at `0x00504950`.
- Owns `0x00504940..0x00504b5c`: global `g_pSphereManager` initialiser, `SphereManager` ctor (`0x005049c0`),
  deleting dtor (slot 0, `0x005049f0`), dtor body (`0x00504a10`) and the four per-TU `Vec3` constant initialisers.
- RTTI `.?AVSphereManager@@` (COL `0x0055f290`), direct base GameObject, vtable `0x005581d8`, object size `0x418`.
  `char field_0x2c[0x3e8]; int field_0x414` are unnamed (no reads in this file).
- 13 exact, 0 partial.

## SteeringControl.cpp
- `__FILE__` string at `0x005745ac`; one use (debug `new`, line 0x17) in the ctor.
- Owns `0x00504b60..0x005051ec`: ctor, Release, SetAxisFromDirection, SetAngle, AddAngle, SetAxisFromPoints and
  eight `$E` pairs. Retail has two sets of the four `Vec3` constants here; `Math3D.h` supplies the first and
  `SteeringControl.cpp` declares the second locally (`kSecondVec3*`).
- SteeringControl has no RTTI (no virtuals). Layout: `node` +0 (SoultreeObject*), `angle` +4, `field_0x08`, `axis` +0xc.
- 21 exact (5 methods + 16 `$E`), 1 partial (SetAxisFromPoints 60.7%, y spilled to a stack slot in retail).

## Not attributed
The keyboard-hook functions `0x005053b0..0x00505480` and the `$E` at `0x00505480` have no `__FILE__` reference and
are not reconstructed.

## D3DIMSoultreeMotnctrl.cpp
- `__FILE__` string at `0x00568b70`; xrefs `0x4456e7..0x445feb` (slot 11, dtor core line 0xc4, slot 0, slot 9 lines
  0x116/0x11b, slot 2 line 0x12d). Bracket `0x44544a..0x447024`; the matched methods are the contiguous
  `D3DIMSoultreeCharacter` run `0x445460..0x4466d8`.
- `D3DIMSoultreeCharacter : Character`, Character : virtual GameObject. Primary vtable `0x551530` (12 slots), vbase vtable
  `0x5514c0` at object offset 532 (0x214), vtordisp at 0x210. The dtor core receives the GameObject vbase pointer.
- 15 exact (FillNodeNames, ctor `0x4455b0`, slots 0, 3, 5, 6, 8, 9, 10, dtor core, GameObject slots 4/5 and three
  vtordisp thunks), 3 partial: slot 4 98.63% (retail holds the 0x3c flag in ecx, ours in eax), slot 2 99.68% (one SIB
  base/index order byte; many spellings tried), slot 11 `0x445680` 39.6% (789 vs 788 bytes: retail keeps the registry
  entry in esi, reloads a2 from the stack, frame 0x308; ours caches a2 in esi, spills the entry, frame 0x30c).
- Local stand-in `ModelObject` is D3DIMSoultreeObject (vtable `0x5513ec`, size 0x2d8, ctor `0x43f160(int)`, slot 7
  `0x444560` takes a source object); helper classes ArchiveFile (0x134), SltFile (0x5c4), registry entry (owner +0x10)
  are local, tier 3.
- Not attempted: D3DIMSoultreeShadow `0x446840..0x446f40` (its `__FILE__` xrefs `0x447024/0x447044` straddle the
  bracket end).

## Verified SphereManager slice

Spheres.cpp now has 14 strictly exact cases, including the shared slot-8 override
at `0x00462e30`. RTTI identifies its primary table `0x005581d8` and that slot;
the body calls GameObject slot 8 and returns the saved object pointer.

The manager initializer allocates `0x418` bytes, passes 1 to the constructor,
and writes the result at `0x00689f1c`. Its source-path literal is at `0x00574568`.
The EH stub `0x0054e524` loads FuncInfo `0x00563b88` (magic `0x19930520`).
The four independent initializer bodies write the zero/X/Y/Z float triples to
`0x00689f30`, `0x00689f40`, `0x00689f50`, `0x00689f20`; their thunks target the
bodies at `0x00504a30`, `0x00504a80`, `0x00504ad0`, `0x00504b20`.
Those decoded writes distinguish TU-local constants from similarly named
constants in other files. `Spheres.bindings.json` records this reviewed mapping.
The other motion-source counts above remain historical masked diagnostics.
