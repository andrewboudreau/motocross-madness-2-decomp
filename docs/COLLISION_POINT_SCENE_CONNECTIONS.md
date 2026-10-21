# Contact scene and dot-provider connections

`CollisionPoint::UpdateRelativeMotion` now calls the existing canonical
`Vec3DotCall` at its three out-of-line dot sites.
`UpdateCollisionPointWorldPositions` calls the existing
`SoultreeObject::LocalToWorldPoint` provider through the contact's primary
scene-node address. These are declaration/call-boundary connections; no
provider body, vector arithmetic, class layout or storage was added.

| Existing source provider | Retail VA | Complete strict extent | Connected production calls |
| --- | --- | --- | --- |
| `Vec3DotCall(const Vec3*, const Vec3*)` | `0x0040ae30` | 29 bytes | 3 |
| `SoultreeObject::LocalToWorldPoint(const Vec3&)` | `0x004fd660` | 168 bytes | 1 |

The dot provider in `VectorCalls.cpp` preserves retail's `(y*y + x*x) + z*z`
product ordering at these self-dot sites. The shared inline `DotProduct` is
not substituted. The pointer casts preserve the same three consecutive
float components and the same input addresses, including the self-alias.
`CollisionPoint.cpp` includes the narrow vector, matrix and scene declarations;
it repeats the canonical dot prototype to avoid importing `Math3D.h`'s
per-unit vector initializers.

## Receiver and result-buffer evidence

Bike's contact producer supplies real primary scene-node addresses. Its loader
calls the constructor at `0x004a23a0` for the handlebars component. That
constructor calls `SoultreeObject::FindByName` at `0x004fdae0` and stores its
returned pointer unchanged at component offset `0x40`. The contact-allocation
calls at `0x004089c2` and `0x004089fb` pass this pointer; the third call at
`0x00408a38` passes Bike's model pointer from `+0x3bc`. The existing
`AddCollisionPoint` body stores the owner argument unchanged at contact `+4`.
The source already declares those node/model inputs as `SoultreeObject*`.

SoultreeObject RTTI reached through primary vtable `0x00557c18` records the
primary offset as zero and the GameObject base at `+12`. The transform helper
uses the primary node's world matrix at `+0xf8`, after calling its real
`UpdateWorldMatrix`. No GameObject adjustment is appropriate here. This
establishes the accessed boundary, not equivalence of every provisional
`CollisionContactOwner` use or every larger owner type.

The old call view explicitly supplied a temporary output pointer and a point
pointer. The canonical method returns a 12-byte `Vec3` through VC6's hidden
first stack argument; the input reference is the second pointer, ECX is the
unchanged node, EAX returns the output pointer, and `ret 8` pops both arguments.
Assigning through the canonical vector view preserves the entire 88-byte
consumer, including its temporary result buffer and subsequent contact copy.
The input is contact `+8`; the final world position is contact `+0x14`.

## Preservation and native validation

A private same-path baseline/current compilation compared both complete
`CollisionPoint.cpp` and the aggregate `CollisionPointNearMisses.cpp`:
62 nondebug sections and 239 relocations remain identical under the two
explicit external-name substitutions. Raw bytes, section properties,
relocation kinds/addends and resolved local section/offset destinations are
preserved. The aggregate's two complete partial bodies remain 845 bytes with
820 matching positions and 615 bytes with 604 matching positions, with their
previous resolved-byte hashes unchanged.

The isolated authentic build compiled all 216 sources and reduced unresolved
symbols from 375 to 373, removing exactly these two names and adding none.
Its complete comparison against the preceding build preserved all 216 old
objects: 10,468 nondebug sections and 45,323 relocation records under the
same two substitutions. Ignored evidence is in `work/game-build/contact28/`.

`tests/test_collision_point_scene_connections.py` verifies 39 complete strict
comparisons, both partial hashes, provider imports, decoded producer/callee
instructions, RTTI and native layouts. Its native executable links the actual
complete `CollisionPoint.obj` and `VectorCalls.obj`. The scene cache and point
transform are unchanged source extractions; both emitted methods must match
retail and the full production object at their complete 858- and 168-byte
extents before execution. Their matrix product and recursive parent-cache
updates are actual source, not scripted helpers.

The 19,541 native cases exercise allocation-to-owner storage, zero/negative
counts, inactive and null-owner contacts, duplicate list entries, cached and
stale matrices, roots and parent chains, transformed positions, vector
self-aliasing and dot results. Whole guarded contact/node storage and the
pointer list are compared against an independent finite integer-valued
matrix/vector oracle. A null contact entry preserves the decoded read fault
at address 4. Only allocation is scripted on the exercised contact path;
unexercised dependencies terminate the fixture if accidentally called.
This is bounded native evidence, not universal floating-point, arbitrary
scene-hierarchy or full collision-system validation.

The existing bike-contact test still passes 68 strict checks, seven complete
partial hashes and 13,056 native allocation/removal cases. Its unreachable
link guards now use the canonical signatures. The vector-constructor suite
also retains 355 strict checks, 28 complete partial hashes and 72,240 native
raw-bit cases.

## Deferred boundaries

The `Vec3Normalize` explicit-output call remains unresolved: a private
canonical return-value trial changed the strict 1,003-byte caller's operand
order; a named-result form grew it to 1,005 bytes. No arithmetic or baseline
was changed to accommodate that trial.

The nonadmitted `SoultreeRefreshContacts` sample retains its own explicit-output
scene call: the tested canonical rewrite changed its 615-byte body to 627
bytes. Its old declaration and binding remain for that sample. Both binding
files retain this key because the physics manifest currently routes the
sample's partial comparisons through the production binding file. No such
import remains in the production object. The sample body remains nonexact
and excluded from the game build.

Reproduce with native private inputs configured:

```powershell
$env:PYTHONPATH='.;tests'
python -m unittest test_collision_point_scene_connections test_bike_contact_connections test_vector_constructors
```
