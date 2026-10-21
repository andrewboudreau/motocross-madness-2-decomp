# SoultreePhysicsBaseObject primary methods

`src/reconstructed/SoultreePhysicsMethods.cpp` supplies eleven methods previously
present only in `samples/physics/soultree_base/SoultreePhysicsInlines.cpp`.
All eleven complete function extents match retail with relocations resolved
under VC6 SP3 `/O2 /GR /GX /MT /Z7`. No class layout, signature or method body
was changed during promotion.

| Primary slot | Retail VA | Bytes | Decoded behavior |
|---:|---:|---:|---|
| 5 | `0x004aa150` | 14 | Returns whether its argument is zero |
| 10 | `0x0040b5c0` | 28 | Compares floats at `+0x154` and `+0x150` |
| 12 | `0x00507920` | 5 | Returns zero and pops one argument |
| 19 | `0x00464e80` | 3 | Returns, popping one argument |
| 22 | `0x0040b5e0` | 24 | Compares integers at `+0x208` and `+0x204` |
| 23 | `0x004aa160` | 38 | Tests airborne state and speed threshold |
| 24 | `0x004aa190` | 3 | Returns zero |
| 26 | `0x00464e90` | 1 | Returns |
| 32 | `0x004aa1a0` | 28 | Calls virtual slot 10; returns zero or load weight |
| 34 | `0x004aa1c0` | 25 | Reads node axes into the object's vectors |
| 35 | `0x004aa1e0` | 37 | Writes those axes to the node |

The retail primary vtable at `0x00557d90` supplies each listed address in its
specific slot. Its preceding pointer names COL `0x0055f0d0`, which has object
offset zero and type descriptor `0x00566c48` containing
`.?AVSoultreePhysicsBaseObject@@`. This distinguishes these methods from the
separate GameObject virtual-base table at object offset `0x220`. The tiny return
bodies are genuine behavior for these verified slots; shared addresses alone
are not used to infer method identity or to add other slots.

The bodies are scattered outside the SoulTreePhysics.cpp range: the Bike.cpp
range contains `0x0040b5c0/0x0040b5e0`, FollowCam.cpp contains
`0x00464e80/0x00464e90`, the Motnctrl.cpp-to-MSZoneInterface.cpp gap contains
`0x004aa150..0x004aa205`, and Terrain.cpp contains `0x00507920`. Inline member
COMDAT selection is a strong explanation for this distribution, not confirmed
original source ownership. The reconstruction filename is organizational;
semantic member names remain provisional.

Slot 6 stays a partial 432-byte sample (68.0693% comparable bytes). Slot 36 stays
an exact 305-byte sample because it introduces the still-unsupplied
`g_SoultreeVec3_685190` dependency. Both retain their previous matching results.
Slot 35 calls `SoultreeObject::SetAxesIn`, which is already unresolved from
other canonical callers and remains sample-only in SoultreeMatrix.cpp. Slot 34
calls the canonical `SoultreeObject::GetAxesIn`. The only other undefined symbol
in the promoted object is the VC6 runtime marker `__fltused`.

The existing includes also emit four local vector constants and their
initializers. No retail addresses or original TU ownership are assigned to
this reconstruction group's initializer set. Exact method bodies do not claim
whole-unit byte identity or original linked initializer order.

`tests/test_soultree_physics_methods.py` independently reads the retail COL,
type descriptor and every target slot, compiles the real class source and
checks that its primary-vtable relocations name the eleven supplied methods,
then checks complete function bytes and dependencies. It also recompiles both
remaining samples to protect the promotion boundary.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_soultree_physics_methods.py -v
```
