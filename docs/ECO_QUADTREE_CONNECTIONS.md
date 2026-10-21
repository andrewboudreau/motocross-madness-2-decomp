# EcoSystem and the shared spatial tree

EcoSystem now uses the existing QuadTree declarations at its tree call
boundaries. Four methods and the existing persistent tree pointer replace five
obsolete game imports; the nonadmitted aggregate also uses the existing
IsValidCode declaration. The surrounding provisional object views remain.

| Existing provider | Retail extent | Status |
| --- | --- | --- |
| ComputeCode | `0x4dc8c0`, 504 bytes | Strict |
| Insert | `0x4dcac0`, 627 bytes | Complete, **620 matching positions; nonexact** |
| NextObjectSorted | `0x4dd600`, 330 bytes | Strict |
| RestartQuery | `0x4dd750`, 22 bytes | Strict |
| IsValidCode, sample consumer only | `0x4ddd90`, 58 bytes | Strict |
| g_collisionQuadTree | pointer cell `0x68aba4` | Existing source storage and lifecycle |

## Receiver and argument evidence

The strict VisibilityQuadTree constructor at `0x52d340` writes the primary
`0x558dfc` table, writes the secondary GameObject table at `this+0x874`, then
stores its unchanged primary pointer to `0x68aba4` at `0x52d389`. Its destructor
clears that cell. Raw COL records and type names independently establish the
primary QuadTree base and secondary GameObject base. This global is distinct
from `g_pQuadTree` at `0x689b78`, the active traversal/allocation context.

Vegetation has a primary QuadTreeObject base. Its strict 58-byte constructor
sets the primary table and the shared query prefix: a WORD stamp at `+4`, a
WORD sort key at `+6`, and BYTE type ID at `+8`. Its complete element size is
`0x1c`. The retained regression compiles the canonical and provisional layouts
separately; it does not equate their entire classes or virtual interfaces.

PlaceStoredObjects (`0x4598d0`, 455 bytes) uses the same
`vegetation base + byte index` receiver for placement and Insert's object
argument. The ComputeCode return is passed unchanged with the two float height
arguments. Each tree receiver is loaded directly from `0x68aba4`. PrepareGeometry
(`0x45aad0`, 771 bytes) receives primary pointers from NextObjectSorted and checks
the object's type ID before reading Vegetation-specific fields. The explicit
return casts preserve that boundary; not every query result is Vegetation.

The global now has its actual `QuadTree*` type. Vegetation's provisional virtual
slot 1 signature is retained, with an explicit primary-pointer cast only at its
ComputeCode call. Insert similarly casts the evidenced primary object prefix.
No forwarding wrappers, alternative storage, linker aliases or new methods
implement these connections.

## Declaration dependency and complete objects

`QuadTreeDeclarations.h` owns the existing records and class declarations,
transferred verbatim. It needs only forward declarations of QuadTreeObject and
BlockAllocator. `Quadtree.h` retains its previous dependency order and includes
the narrow declaration header afterward. EcoSystem can therefore use the real
tree interface without importing a second, conflicting CollisionObject class
view. Its unrelated legacy classes and public signatures remain unchanged.

Same-path before/after compilation preserves four complete objects under
exactly six names: **368 nondebug sections and 1,631 relocations**, including
all function bytes, section flags, data, initializer records and local
destinations. No filename exceptions are used in that comparison. The retained
portable pins normalize only the four enumerated complete source filenames.
All canonical and sample partial bodies are included. There are **29 affected
import sites**: thirteen in canonical EcoSystem and sixteen in its aggregate.

The existing clipper and FilePrintf EcoSystem pins are updated only after their
old digests are reproduced from the published baseline and every current
section/destination is proved equal under these same six mappings. No byte
region, relocation kind, extent or partial score is ignored. Private evidence is
under `work/connections50/eco/`, including `objects.json` and
`prior-pins-proof.json`.

## Insert's nonexact instruction difference

The full `0x4dcac0..0x4dcd32` extent includes the cold Grow block after RET16.
Seven bytes differ across five instructions. At `0x4dcb20..0x4dcb28`, the source
and retail exchange private x/z spill homes. At `0x4dccc0..0x4dcccc`, three MOV
loads restore code/x/z in a different order. Both arrive with the same EAX,
EBX and EDX values and flags. These are the only users of the two local homes;
their addresses are not passed to a helper. Neither window has an interior
branch target or relocation. All other resolved bytes are identical.

The retained test preserves the full 627/620 result and exact seven mismatch
positions. This explanation does not make the provider strict. Transient stack
contents and register states differ; stack inspection, deliberate aliasing of
private callee locals, asynchronous contexts and stack faults are outside the
equivalence claim.

## Actual source execution and independent outcomes

The native executable links the complete production **Quadtree.cpp and
BlockAllocator.cpp objects**, not copied implementation fragments. Actual
Insert, Grow (50 bytes), AddObject (117), QuadTreeNode constructor (48), the
base node factory (94), and pool constructor/Alloc/AllocateBlock execute. The
regression compares every registered strict function in the affected sources,
including these helpers: **120 strict checks**. Insert remains separately
qualified as nonexact.

The reference's three direct calls are redirected to the same strict Grow and
AddObject helpers after checking each original E8 destination. Its one current-
tree storage operand is checked and redirected to the linked source cell.
The virtual factory boundary uses a validation-only nonvirtual wrapper with an
explicit qualified call to the actual base factory. This avoids using a virtual
member-pointer dispatch thunk as its own vtable target. The real derived
VisibilityQuadTree factory and full visibility traversal are not executed by
this fixture; their source callers and complete registered bodies are checked
separately.

Only debug allocation boundaries use a deterministic fixture heap. Node growth
returns either the same address or a moved copy and poisons the old node prefix.
The real pool code obtains and fills 104-byte blocks with 20-byte elements.
This exercises fresh blocks and their unused-element fast path, not every
free-list/spare-block/reset path or the game debug allocator's accounting.
Unexpected delete/free calls fail the fixture; no artificial cleanup is added.

The **13,347** scenarios comprise:

* 10,752 ordinary cases: twelve valid depths, all sixteen cell-flag masks,
  seven asymmetric coordinate-boundary pairs, both growth modes and one,
  two, five or nine repeated insertions of the same object.
* Nine exact invalid-code `0xf` cases, including invalid receiver/object/tree
  views that must remain untouched.
* Two successful allocation baselines and 224 failures/throws at every reached
  allocation boundary across both growth modes.
* Eight invalid/guarded object, receiver, root, child-array or pool accesses.
* Four initial object-count values including signed boundaries.
* 300 moderate finite Y pairs across all three x87 precision and four rounding
  modes, with independent per-node interval expectations.
* 2,048 deterministic **arbitrary raw float-bit pairs**, including nonfinite
  values when generated, under masked FP controls. These compare complete
  differential state and require normal completion; they do not use a portable
  numerical oracle or establish every FP input.

Independent ordinary-case oracles derive child paths from code bits, check
total object placements, require the live-node count to equal the traversed
tree, and verify each node's interval for the moderate matrix. They check
objectCount once per call, the cleared object stamp, and installation of the
current-tree pointer from a distinct initial sentinel. Repeated calls exercise
preexisting paths and item lists; they use the same Y pair within a case, not a
sequence of independently changing ranges or arbitrary preexisting intervals.

Failure oracles check the exact reached allocation event count, prior successful
node constructions, count/stamp/current-tree stores, exception kind and address.
Null factory, pool and realloc results retain retail's later faults. The guard
page covers selected object-stamp, root-range and child-array access boundaries;
not every field is independently page-guarded. No failure cleanup is invented.

Each run resets and compares the complete prepared tree/object/pool records,
all 64 KiB of allocation arena and allocation metadata, return completion,
exception, final FP flags/control and current-tree pointer. Allocation events
retain ordered arguments and selected count/root/stamp/pool fields; they are
not complete arena snapshots at every callback. These are finite raw-storage
tests, not full constructed game lifetimes or a native vegetation/query frame.
Only ordinary repeated-object sharing is exercised; arbitrary node/object
overlap is not claimed. FP exceptions are masked, and x87 stack/context or
unmasked exception resumption is not compared.

```powershell
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
$env:PYTHONPATH='.;tests'
python -m unittest test_eco_quadtree_connections -v
```

The isolated real build `build-yq4yuhn6` compiles 258/258 translation units
and removes exactly the five obsolete game names: unresolved symbols fall
from 235 to 230, with none added. The full composition preserves all 256 prior
game objects (10,675 nondebug sections and 45,728 relocations), allowing only
these six declaration names and the separately reviewed name-tag flag
visibility/name change. No filename exclusions apply. All 231 header hashes
and 258 source hashes match that report. Every prior strict calibration and
physics result remains, with no partial-score decreases. The game still does
not link; startup, integrated rendering and racing remain unverified.
