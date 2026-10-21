# Visibility call connections

Five existing method imports now use `VisibilityClipper`, whose implementation
was already present in `src/krusty2/visibility/VisibilityQuadTree.cpp`. This joins
cube-face visibility, follow-camera projection and racer-name projection to the
same source methods. It does not supply the still-missing clipper global storage
or establish that the game can render a frame.

| Previous declaration | Existing method | Complete retail extent | Strict caller and direct site |
| --- | --- | --- | --- |
| `UnknownCubeClipper::TransformPoints` | `VisibilityClipper::TransformVectors` | `0x52f0a0`, 156 bytes | `DrawableCube::MarkVisibleCells`, 398 bytes; `0x43e43b` |
| `UnknownCubeClipper::CanFaceCamera` | `VisibilityClipper::TestDot` | `0x52f140`, 72 bytes | same caller; `0x43e3c1` |
| `UnknownCubeClipper::IsQuadOnScreen` | `VisibilityClipper::CullQuad` | `0x52f4d0`, 155 bytes | same caller; `0x43e46e` |
| `UnknownFollowCameraProjector::ProjectPoint` | `VisibilityClipper::ProjectPoint` | `0x52f340`, 389 bytes | `FollowCamera::UnknownVirtualSlot38`, 780 bytes; `0x463754` |
| `UnknownProjector::ProjectPoint` | same `ProjectPoint` | same 389 bytes | `NameOverlay::UnknownVirtualSlot14`, 654 bytes; `0x519300` |

`CullQuad` calls the existing complete 234-byte `CullPolygon` at `0x52fac0`.
All five provider bodies and the three affected callers remain strict with
relocations resolved. No provider body, argument evaluation order or temporary
lifetime changes in this slice.

## Receiver and argument evidence

The five sites load ECX from the same retail pointer cell, `0x575a98`. The
selected methods do not access receiver fields; `CullQuad` forwards the unchanged
receiver to `CullPolygon`. `VisibilityClipper` remains a provisional source view,
not a newly recovered RTTI identity. Its existing class declaration moves
verbatim into `VisibilityClipper.h`, with forward declarations for argument
records. The original header includes that one declaration at its original
location. The caller globals retain their existing declarations, with casts only
at the verified method boundaries.

The global is not a null default: retail `0x575a98` contains `0x68acd4`. This
slice neither defines that pointer nor infers the target object's complete
storage or initialization from these fieldless methods.

Camera evidence is separate from clipper identity. Raw Camera, PCCamera and
FollowCamera COLs identify their primary tables, and PCCamera/FollowCamera's
Camera base has displacement zero. PCCamera's complete 25-byte constructor
calls the 402-byte Camera constructor without an adjustment, then installs its
primary vptr. `RenderTarget::SetCamera` stores its actual argument at `+8`;
CubeDraw and NameOverlay load that field. FollowCamera passes its own primary
Camera address. The retained test checks the raw constructor, receiver-load,
matrix-address and viewport-store instructions, as well as complete strict
constructor and SetCamera/SetViewport bodies.

The argument records are verified prefixes: three adjacent float words for
vectors, a 64-byte matrix, and the Camera fields read by these methods. The
actual Camera declaration independently verifies screen matrices at `+0xec`,
field of view at `+0x16c`, viewport width/height at `+0x1a8/+0x1ac`, aspect ratio
at `+0x1b8` and near plane at `+0x1bc`. This does not assert whole provisional
camera-record type equivalence or C++ object lifetimes in prepared storage.

The old TransformPoints parameter names suggested destination/source, while
the canonical method takes source/destination. This caller passes the same
`s_viewPoints` pointer in both positions, preserving the actual in-place call.
ProjectPoint's last argument is a writable unsigned four-byte outcode. The
follow-camera switch already tests its bit values; NameOverlay never reads its
former `float depth` local. Both now use unsigned locals directly. No depth
conversion, hidden result argument, or pointer adjustment was introduced.

## Complete-object preservation

The retained baseline fingerprints cover every nondebug section byte, flags,
relocation kind/addend and resolved section/offset destination in seven objects:

| Object | Sections | Relocations |
| --- | ---: | ---: |
| CubeDraw | 47 | 174 |
| FollowCamera | 98 | 250 |
| TrackOverlay | 207 | 863 |
| CubeDrawNearMisses | 17 | 55 |
| FollowCameraNearMisses | 66 | 336 |
| TrackOverlayNearMisses | 37 | 162 |
| VisibilityQuadTree | 87 | 341 |
| Total | 559 | 2,181 |

The same-path private before/after comparison allows exactly the five decorated
method-name substitutions recorded in
`tests/fixtures/visibility_connections_objects.json`. It excludes no data,
initializer, exception metadata or partial function. The portable retained pins
make only recognized complete source filenames relative to the checkout; local
symbols resolve to actual section identities and offsets. The included
FollowCameraTick source filename is explicitly covered.

There is one prerequisite baseline repair: the original TrackOverlayNearMisses
source does not compile because GUIManager is incomplete at its tooltip font
accesses. Adding the genuine `GUIManagerCore.h` include fixes that omission. The
sample's complete-object comparison starts from this repaired baseline, before
any visibility changes; it is not a claim of equality with a compilable original
HEAD sample object. The other six before objects need no baseline repair.

The isolated game link removed exactly these five missing names, with no added
undefined symbols. Parent integration checks cover all existing game objects
because this declaration header also has transitive consumers.

## Native validation

`test_visibility_connections.py` reports **214 strict comparisons**, seven
complete-object pins and **66,845 native scenarios**, including 432 access-fault
outcomes. It extracts the unchanged five complete source method definitions,
compiles them separately with authentic VC6 SP3, and requires both their full
bytes/relocations and their strict retail matches to equal the canonical TU.
The extracted object has no nonempty data, BSS or CRT initializer sections and
only the `__fltused` undefined marker. Its CullQuad executes the actual source
CullPolygon. The reference executes the corresponding complete hash-pinned
retail methods with **no call patches, dependency mocks or substituted math**.

The finite scenario matrix is:

- 38,400 TransformVectors cases: counts `-1/0/1/4/8`, independent, in-place and
  shifted input/output overlap, plus matrix and camera-storage output aliases.
- 7,680 TestDot and 10,560 ProjectPoint cases: viewport unsigned edges, affine
  and perspective matrices, zero `w`, overlapping point/screen/code storage,
  and optional null screen/outcode outputs.
- 4,800 CullQuad and 4,800 CullPolygon cases: repeated indices, negative/zero
  counts where applicable, near-plane, aspect and field-of-view branches.
- 600 null/guard-page cases across all five methods, comparing partial writes,
  exact access kind/address and floating-point flags at the fault boundary.
- Five independent identity projection and inside/outside/cull outcome checks.

The ordinary matrix uses 80 deterministic seeds across four rounding modes and
three x87 precision modes, including signed zero, denormal, infinity and NaN
inputs in a bounded subset. Complete prepared storage, guards, return values,
floating-point status and control are compared. Every ordinary case must finish
without an exception. Only access violations are caught in deliberate invalid
pointer cases; unexpected exception kinds fail. Null and inaccessible receiver
addresses also confirm these particular methods do not read receiver fields.

Floating-point exceptions remain masked. This is finite evidence for these
methods, not exhaustive real arithmetic, unmasked exception timing, transient
x87 register/TOP identity, camera lifetimes, the full three callers, a renderer
or a complete frame. Real caller preservation is established by the complete
strict/object comparisons above.

```powershell
$env:PYTHONPATH='.;tests'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_visibility_connections -v
```
