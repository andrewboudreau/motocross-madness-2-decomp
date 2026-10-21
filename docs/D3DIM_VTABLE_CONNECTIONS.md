# D3DIM primary virtual declarations in the physics view

Seven existing virtual dependencies of `SoultreePhysicsObject` now use the
implemented `D3DIMSoultreeObject` primary-slot declarations. No implementation,
object layout, initializer, or vtable order changes. The physics header still
contains its existing folded SoultreeObject view; this change does not unify
that entire class definition with the rendering view. Slot 9 remains excluded.
The reconstructed file names and higher-level semantic names do not establish
original translation-unit ownership.

| Slot | Existing provider | Retail entry | Complete bytes | Stack cleanup |
| --- | --- | --- | --- | --- |
| 2 | LoadHierarchy(stream) | 0x43f950 | 1257 | 4 |
| 3 | LoadHierarchyFromParameters() | 0x43fe40 | 543 | 0 |
| 4 | SoultreeVirtualSlot4(out) | 0x4440a0 | 146 | 4 |
| 5 | SoultreeVirtualSlot5() | 0x444140 | 763 | 0 |
| 6 | ResetLocalMatrices() | 0x4450c0 | 280 | 0 |
| 7 | CloneFrom(source) | 0x444560 | 1234 | 4 |
| 8 | CopySubtree(source, target) | 0x444b60 | 263 | 8 |

The stream, output-pointer and source/target pointer declarations are the
existing rendering interface, corroborated by the complete provider bodies and
retail return instructions. In particular, slot 4 writes a newly allocated model
through its output pointer; slot 7 reads the source primary model; slot 8 passes
both primary node pointers to the base subtree operation. There is no new
forwarding method, copied provider, symbol alias, or adjusted target extent.

Raw primary table 0x5513ec and the physics owner's secondary table 0x557f28
contain these same seven addresses at slots 2 through 8. Their COLs place them
at offsets 0 and 0x21c respectively. The full 275-byte D3DIM constructor writes
its primary and GameObject vptrs at 0 and 0xc. The full 194-byte physics
constructor calls that constructor at `this + 0x21c`, writes the corresponding
secondary table, and retains this same primary model pointer in `sceneNode` at
+8. Its separate nonvirtual GameObject is +0x228; its virtual GameObject is
+0x4f8. Those pointers are not interchangeable. Raw RTTI base records and all
six relevant COL/vtable pairs are asserted in the retained regression.

Nine complete source/aggregate objects preserve 1,384 nondebug sections and
6,689 relocation records under exactly seven external method-name mappings.
Only the seven entries in SoulTreePhysics's emitted D3DIM primary table use
those mappings. All instruction bytes, section flags, storage, initializer
bodies, addends and defined relocation destinations remain identical. Compiler
local label spellings change with declaration visibility; their complete
section identities and exact offsets are preserved. Both registered physics
partial bodies keep their complete resolved fingerprints and existing scores.
No partial extent or score is relaxed.

The regression pins all nine objects, 50 complete strict physics bodies and
nine complete rendering bodies (the seven providers, constructor and existing
ComputeAllBounds caller). Its 896 native cases compile dispatch functions
against each genuine header independently. Fixture-only nonvirtual observers
check the selected slot, +0x21c conversion, raw argument bits/order, return
stack balance and unchanged prepared storage. These are ABI observations, not
native execution of the seven hierarchy/material providers. Their retail
fidelity is established by the full strict comparisons; allocation, file,
material and hierarchy behavior is not simulated or newly validated here.
No constructed game-object lifetime or complete class equivalence is claimed.

The prior attachment-emitter whole-object pin is updated only after reproducing
its published value and applying these exact seven names. Its partial pins are
unchanged. Twenty-five affected whole-object records and 75 applicable
Python-literal/partial records were independently checked. Other records need
no update. Known source filename literals are enumerated explicitly for
cross-checkout portability; the same-path proof excludes no sections or paths.
The older VectorAddAssign serializer retains a checkout-dependent filename
COMDAT name: its actual-path published pin and private same-path equality were
checked separately, without changing that test or pin.

Run with authentic VC6 and the private retail image configured:

```powershell
python -m unittest test_d3dim_vtable_connections -v
python -m unittest test_soultree_attachment_emitters test_rider_motion_start_connections -v
```

The focused result is 59 strict checks, nine complete objects, two unchanged
partials and 896 dispatch cases. The existing attachment suite also retains
its 66 strict checks and 130,816 native cases; the rider suite retains its three
complete objects and two strict callers. The isolated actual build
`work/game-build/build-63c535mu` compiles 271/271 sources and removes exactly
seven unresolved names: **192 -> 185**, none added. All 271 previous objects
preserve 10,833 nondebug sections and 46,129 relocations under those names,
with no exclusions. Every prior global calibration and physics result retains
its complete extent, score and strict status. Startup and gameplay are unverified.
