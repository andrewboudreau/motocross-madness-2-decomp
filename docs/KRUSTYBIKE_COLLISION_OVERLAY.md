# KrustyBike collision and debug-overlay connections

KrustyBike's frame and collision consumers now call three existing canonical
providers. This supplies no replacement algorithms, storage or wrappers. The
real game link removes the three obsolete names (267 to 264 unresolved in the
isolated integration build); linkage and a running game remain incomplete.

| Retired call view | Existing provider | Retail extent | Actual sites |
| --- | --- | --- | --- |
| `KbBody::TestMeshBounds(KbBody*)` | `CollisionObject::TestMeshBounds(CollisionObject*)` | `0x4392c0`, 311 bytes | `0x48dab6`, `0x48daf3`, `0x497a0f` |
| `KbOverlay::Title(int,const char*,...)` | `DebugOverlay::Title(int,const char*,...)` | `0x447fa0`, 94 bytes | `0x497ada` |
| `KbOverlay::Print(int,const char*,...)` | `DebugOverlay::Print(int,const char*,...)` | `0x447f40`, 94 bytes | `0x497af5`, `0x497b47`, `0x497be3`, `0x497c0b` |

The collision operands already have `CollisionObject*` type. Removing the
provisional casts preserves ECX, the single stack argument, signed EAX result
and `RET 4`. The overlay calls cast only the loaded receiver at the call
boundary. These variadic members use cdecl with **this on the stack**; their
original arguments, eight-byte floating-point promotions and evaluation order
remain unchanged. The other provisional game/overlay declarations remain;
this does not establish whole-view class equivalence.

## Receiver and storage evidence

The complete strict KrustyBike loader at `0x48fc80` (5,384 bytes) calls the
CollisionObject constructor at `0x490717` and `0x4907de`. Returned primary
pointers are stored at `+0x1414` and `+0x1418`; the former is loaded at
`0x49087b` and stored to the inherited collision pointer at `+0x128`.
The two strict consumers, slot 28 (`0x48da50`, 416 bytes) and Tick
(`0x4977a0`, 1,153 bytes), load those pointers without a secondary adjustment.
Raw CollisionObject COL `0x55aff0` identifies its primary table `0x5511b8`;
RTTI places its GameObject base separately at `+12`.

Game::CreateObjects (`0x467b70`, 775 bytes) calls the DebugOverlay constructor
at `0x467dbc`, calls Setup at `0x467ddd`, and stores its return at Game `+0x38`
at `0x467de4`. Tick loads this field and pushes it at `0x497ad9` before Title.
Raw DebugOverlay table `0x55166c`/COL `0x55b460` and TrackGame table
`0x558938`/COL `0x55f8e0` confirm primary offsets and the Game base at offset zero.
The regression retains raw type-name/COL/PMD and decoded producer instructions,
not just matching field sizes. The 346-byte collision constructor, 77-byte
overlay constructor, 174-byte Setup and complete Game producer compare strict.

## Complete-object preservation

The canonical KrustyBike object retains all **243 nondebug sections and 1,054
relocations**; its aggregate sample retains **271 sections and 1,443
relocations**. Same-path old/current compilation preserves every payload byte,
section flag, addend and defined section/offset destination under exactly the
three external-name substitutions. Initializers, data, exception metadata and
all partial functions are included. The retained portable pins normalize only
recognized complete source filenames; no function or scheduling window is
excluded. No prior partial score is relaxed.

`tests/test_krustybike_collision_overlay.py` checks both whole-object pins, the
eight actual call relocations, all registered strict KrustyBike bodies, producer
bodies, and the actual provider/helper functions: **96 strict comparisons**.
The unchanged source text for seven native functions is extracted into temporary
translation units. Every complete extracted body is strict and byte/relocation
equal to its full canonical object. BoxOverlap's local switch labels are compared
by the same function and exact destination offset, rather than compiler-generated
label spelling. The emitted CrossProduct helper is also strict and identical.

## Native behavioral evidence and limits

The native VC6 fixture executes actual TestMeshBounds, TreeBoxOverlap,
BoxOverlap, MatrixMultiply and their emitted math helpers. It also executes
actual DebugOverlay Print, Title and SetRowText with the authentic compiler CRT.
No source body is rewritten and no game dependency is replaced by a scripted
algorithm. Prepared raw storage avoids unrelated object construction.

The pinned private retail image is mapped for the reference. Collision reference
code is unchanged. Exactly three verified `E8` reference call operands redirect
CRT calls to the native linked CRT:

* `0x447f0e`: retail `strncpy` at `0x534b10` to linked `strncpy`;
* `0x447f79` and `0x447fd9`: retail `vsprintf` at `0x535e71` to linked `vsprintf`.

Each patch asserts the original opcode and decoded destination. Thus the overlay
comparison shares CRT behavior; it is not a proof of retail/native CRT byte
identity. The candidate calls remain untouched. PE header/section bounds and the
retail hash are checked before mapping.

The **22,225 cases** comprise 10,368 collision shape/matrix/FP combinations,
288 receiver/payload aliases, 45 null-pointer boundary cases, two collision
outcome assertions, 11,520 overlay combinations and two overlay outcome
assertions. The three supported shape kinds use real hull/model/mesh layouts.
Matrix families include identity, rotation, reflection, shear, singular and
separated translations. Comparisons retain complete guarded objects/payloads,
query frame and axis scratch storage, return value, access-fault kind/address,
and FP status/control. Every ordinary case must finish without an exception;
only deliberately installed invalid pointers may produce an access violation.
Unexpected exception kinds are not swallowed. Overlay cases vary enabled/page selection, row counts
and limits, both methods and all five production formats plus unchanged text.
Independent assertions check separation, self overlap, highlighted/new text,
and preservation of color/highlight when text does not change.

The collision matrix uses bounded finite binary fractions, four rounding modes
and three precision modes, with FP exceptions masked. The existing Math3D
`DotProduct` COMDAT is **not strict**: it sums z+y+x, while retail sums y+x+z.
The fixture intentionally links that unchanged actual helper and checks its full
body against the canonical object. Passing these finite cases does not establish
general floating-point equivalence or repair that separate helper. CrossProduct
and all seven extracted main/helper bodies are strict. No malformed shape tags,
unbounded formatting, font rendering, constructors/lifetimes, full Tick execution,
unmasked traps or complete x87 register-stack equivalence are claimed.

Reproduce with authentic private inputs:

```powershell
$env:PYTHONPATH='.;tests'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_krustybike_collision_overlay -v
```
