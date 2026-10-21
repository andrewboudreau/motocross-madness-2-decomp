# Bike collision placement connection

Bike primary slot 33 (`0x00406a50`, 135 bytes) now calls the existing
`CollisionObject::SnapPlacement` instead of the provisional
`BikeA38::Method_0x00435fe0` declaration. The receiving expression uses the
already shared collision base of `Wrecker::collisionModel`; no wrapper,
class layout change, or new provider is introduced. Other BikeA38 fields and
the old declaration remain to avoid unrelated header changes.

The complete caller loads Bike's Wrecker pointer at `+0x604`, loads its
collision model at `+0x38`, and calls `0x00435fe0` at `0x00406ac8`. Independent
producer evidence comes from `Wrecker::CreateBodies` (`0x005301c0`, 995 bytes):
the allocation is `0x10c`, the constructor call at `0x00530304` targets
`ConstraintMethodCollisionModel` at `0x0043b8d0`, and the unadjusted returned
pointer is stored at `Wrecker+0x38` by `0x00530313`. A separate `+0xc` address
is formed for GameObject dispatch; it is not the stored primary pointer.

The full 121-byte constructor calls CollisionObject's constructor with its
original primary receiver and writes the primary/secondary vtables at offsets
zero and twelve. RTTI independently records CollisionObject at zero and
GameObject at twelve. Native pointer conversions check the same offsets.
The semantic placement names are reconstructed; the target identities and
offsets come from these instructions, complete byte matches and RTTI.

`SnapPlacement` is the existing strict 17-byte body at `0x00435fe0`. It calls
the strict 36-byte `UpdatePlacement` at `0x00435fb0` twice. The latter first
synchronizes the transform, then reloads the global quadtree and this object's
broadphase flag before deciding whether to update its cell.

A same-path VC6 comparison preserves all **242 nondebug sections and 1,089
relocations** of Bike.cpp under only the documented external-name substitution.
There is no Bike source aggregate. The seven complete nonexact Bike functions
retain their resolved fingerprints and scores; none is promoted by this change.
Private comparison evidence is in `work/game-build/bike-snap36/proof.json`.

`tests/test_bike_snap_placement.py` checks all 52 strict Bike target rows, the
seven complete partial fingerprints, the full producer/constructor bodies,
the actual caller relocation, and both placement functions in their complete
canonical object and in the isolated native execution object. There is no
retail dispatch in the production build or this native fixture.

The native fixture runs **10,240 cases** through the actual two placement
functions. Transform synchronization and quadtree-cell updates are explicit
scripted boundaries. An independently planned byte-state oracle checks the
primary receiver, both pass orders, post-callback global/flag reloads, every
byte of the prepared collision object and guards, and exceptions after each
reached boundary. Positive and negative nonzero flags, null/non-null trees,
and mutations between passes are covered. This does not execute transform
math or quadtree insertion and does not establish integrated vehicle physics.

The isolated real link `work/game-build/build-ca3o0osm` compiles 231 translation
units and removes exactly the obsolete BikeA38 method name: **315 to 314
unresolved symbols**, with no newly missing names. A successfully linked game
and startup, menu and racing tests remain required.

```powershell
$env:PYTHONPATH='.'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_bike_snap_placement.py -v
```
