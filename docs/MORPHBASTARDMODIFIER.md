# MorphBastardModifier.cpp

`src/reconstructed/MorphBastardModifier.h` / `MorphBastardModifier.cpp`.
Names are provisional.

Evidence:
- **`__FILE__`:** the literal at `0x0056df3c`, xrefs
  `0x004a3215..0x004a5447`.
- **RTTI:** `MorphBastardModifier : D3DIMSoultreeModifier : GraphicsTest :
  GameObject`, vtable `0x00555230`. It overrides slot 0 and the pure slot
  27.
- **Caller:** KrustyBike.cpp `0x004910de` allocates 0x58 bytes, constructs
  it, and loads "RiderMorph.mbf" through `0x004a33b0`.

Extent: `0x004a3150..0x004a562b`.
- `0x004a3120` before it is a memcpy helper used only by DebugRealloc, so
  it belongs to the allocator's unit.
- The unit ends with its four kVec3 `$E` pairs.
- Motnctrl.cpp starts at `0x004a5630`.

Exact: 15 calibration cases. These are the constructor, the destructor and
its deleting wrapper, the qsort comparator `0x004a3bb0`, the vector angle
`0x004a3be0`, the per-object channel update `0x004a4bb0`, the mesh copy
`0x004a5290` and the eight `$E`. Source forms needed:
- The dot product in `0x004a3be0` is written `a.z*b.z + a.x*b.x + a.y*b.y`.
- MorphBastardObject has an empty constructor, which gives retail's
  inlined `new[]` countdown.
- The mesh's +0x20/+0x24 pair is one 8-byte struct.

Near misses (in the unit in retail order, each with a `// Not exact` status
comment; they moved in from a sample and two bring-up copies, and
`KbMorphMod::LoadParameterFile`, KrustyBike.cpp's view, links to the unit's
loader through a link alias):
- The 2046-byte parameter-file loader `LoadParameterFile` `0x004a33b0` (1950 of 2046): an
  existing channel records the target index `j`; the stack homes of the loop
  scalars and the operand order of the channel inverse's products differ.
- Slot 27 `0x004a4c60` (185 of 1571).
- The controller-angle solver `ControllerAngles` `0x004a3c80..0x004a4ba5` (3878 bytes, not
  2340; 570 of 3878): structure reconstructed; retail calls the out-of-line Vector3
  constructor `0x00404e60` and has a larger frame.

Slot 27 differs in stack-slot and register assignment.

`MorphBastardNode` is an alias of the existing `SoultreeObject` declaration.
The matrix calls in `UpdateChannels` use the complete node pointer at offset
zero: `GetLocalMatrix` (`0x004fca60`) and `GetMatrixIn` (`0x004fca80`) now
resolve to their existing `soultree.cpp` implementations. This is tier 2
call-site evidence; it does not reconcile every separate Soultree declaration
or establish the original header structure. The 166-byte caller remains strict
exact. The parameter loader uses the same alias.

D3DIMSoultreeModifier.h now gives slot 27 its signature `(object, mesh,
out)`, taken from the call site `0x00440ded`.

### Forms tried

The notes the near-miss sample carried before the bodies moved into the unit
(scores are per the sample at the time; the unit's ControllerAngles adds the
bring-up's two rounding fixes to the sample's per-block sets, 570/3878):

```text
MorphBastardModifier::LoadParameterFile (0x004a33b0, 2046 bytes): the
parameter-file loader, 1950 of 2046 positions. Calls, strings, lines,
field offsets and the frame size (0x350) match retail; the object's rigid
inverse is MorphBastardInvertRigid (an inline building a Vector3; the
plain-locals form leaves the frame 0x18 bytes short). An existing channel
records the target index j (retail reads it from j's slot), and its test
is `channel->controller == controller && channelAxis == channel->axis`
(retail's strength-reduced channel offset in ebx and its `cmp eax, ecx`).
The new channel's inverse goes through MorphBastardInvertRigidInPlace,
which gives retail's term order; the products' operand order inside
t.x/t.y/t.z still differs (six term orders per sum, all 216 combinations,
tried).
Remaining: the stack homes of the spilled scalars (retail, offsets from
esp after the prologue: controller +0, j +4 (shared with the second k),
targetValue +8, axis +0xc, isNew +0x10, k +0x14, i +0x18, targetCount
+0x20 (shared with `this` and the two `new` results before the loop),
channelAxis +0x24, found +0x30, probe +0x34; the two inverse temporaries
+0x38/+0x44 in the other order). The candidate gives the second k and
targetCount slots of their own (+0x18, +0x34). `probe` declared in the axis loop beside
`found` gives 1942 (function scope 1938). Declaration scopes of targetCount, found and i,
constant channel axes, the isNew/controller test order and the
targetValue initialisation do not move them; tools/frame_layout.py finds
no use-count adjustment for the swapped pairs (j/controller, k/isNew,
i/k). In the channel inverse retail loads the rotation element first in
8 of 9 products (m[3][0] * m[0][2] is the exception); the translation
read into locals before or after the swaps, a Vector3* translation
pointer and a Vector3 position copy are worse (1068-1343).
s14 (strict 1938 unchanged): function-scope j, controller, isNew, t, r
and target are inert, a function-scope channel or targetValue gives
1346-1362 and one function-scope k for both channel loops 1341;
frame_layout puts all three swapped pairs (j/controller, k/isNew,
i/second k) at adjacent ranks with no count adjustment. The channel
translation summed before the swaps from the untransposed elements, or
through a `const Vector3&` to row 3, is worse (masked 935-1148 of 1738,
base 1634); writing every product's operands the other way round
changes nothing.
2026-11: `targetValue` is a function-scope local assigned 0 before the
target loop (1942 -> 1950; the same at object-loop scope). As a local of
the channel loop with an initialiser it was 1942, declared in the target
loop 1351, ahead of the channel pointer 1273. (The s14 function-scope
figure above was measured on an older base.) The scopes of the other two
address-taken scalars: targetCount at function or object-loop scope is
the same code, probe/found/channelAxis in the axis or target loop the
same, at object-loop or function scope 4 worse. On the new base the
scopes of k (shared by both channel loops: 1342), j, isNew, i and
controller (108 combinations) change nothing. Left: the remaining home
offsets, the store scheduling at the end of the object's inverse
(retail stores m[3][1] before the last faddp and takes &targetCount
after the m[3][2] store), and the m[3][0] products of the channel
inverse.

s22, leaf-age copies in the channel inverse: m[0][0] and/or m[0][1]
copied into float locals before the Vector3 (retail loads them before
m[3][0] in the x and y sums) cost a 5-byte spill each and move the frame
(masked 1146-1175 of 1743, base 1638 of 1738); m[3][0] in a local 910-930.

MorphBastardModifier::ModifyMesh (0x004a4c60, 1568 bytes): blends
the target deltas of every channel into the copied mesh. The control flow,
calls and the signed /32 vertex base match; register allocation of the
object/channel loops differs (retail keeps the object offset in esi and
reloads the index from +0x28, the channel in ebp).

MorphBastardModifier::ControllerAngles (0x004a3c80..0x004a4ba5,
3878 bytes, ret 0x44): the controller angles. Controller matrix times
the rest inverse (inline, through a copy), then per axis: rotate the
identity's axis onto the controller's (cross product, 0x004a3be0 angle,
normalise or fall back to the unit axis, axis-angle matrix, 0x00436500)
and measure the angle between the next rows in degrees (57.2957764),
signed by the cross product. Stores -x, -z, -y at +0x60/+0x64/+0x68.
The first axis inlines identity, rows, cross product and the axis-angle
matrix; the later ones call 0x004a2350, 0x00515600, 0x0042de90,
0x0040ae30 and the Vector3 constructor 0x00404e60 (likely VC6's inline
budget). Normalised ratio 0.93 over the first half; 872 of 896
instructions, retail frame 0x1fc (ours 0x1b4); the source here uses the
inline Vector3 constructor where retail calls 0x00404e60, and VC6's
term order differs in the products. Retail's only 0x00404e60 call is
the (0, 0, 1) fallback of the z axis block (push 1.0, 0, 0 and call at
0x004a49a8). It stays inline here: VC6 expands Vector3's constructor at
that site, 0x00404e60 is the out-of-line copy of Math3D.h's Vec3
constructor, and Math3D.h cannot be included next to MatrixUtil.h (both
define Matrix4), so the call would need a declared-only stand-in type.
Retail's in/out-of-line split is a budget split of one helper set: the
three axis blocks are the same code (row getter 0x004a2350, cross
product 0x00515600, the normalisation calling the dot product 0x0040ae30,
axis-angle 0x0042de90, the Vector3 constructor), expanded in the first
block and called from the second on. Rewritten that way, with the
helpers as header-style inlines bound to those copies and the product as
MatrixMultiply's text with both operands by value (inlined, VC6 copies
only the operand aliasing the output, retail's rep movsd), VC6 expands
less than retail when the product is a helper (the first axis-angle
matrix, cross product and rows are called; masked ratio 0.67-0.69) and
everything when the product is written in the body (0.63). Retail's
caller lies between the two, so the hand split below (0.885) is kept.
The linked bring-up (src/bringup/Group3_MorphBastard.cpp) keeps the hand
split but declares the called helpers out of line (no inline bodies) and
binds them with link_aliases, the z fallback through a declared-only
MbVector3Call stand-in for 0x00404e60: retail's exact callee multiset,
linked masked 0.932 -> 0.942 (strict 0.333 -> 0.357). Tried there and
no better: the x block's rotation stores in AxisAngleMatrix's order
(0.942), an inline axis-angle helper for the x block (0.936, either
sqrt sum order), `t*y*x` product spellings (0.942). Left: channel in
ebx, frame 0x1b4 vs 0x1fc, retail's t*y/t*z CSE in the x block.
ModifyMesh there now calls the 0x00464e80 stub with i (retail
0x004a4ce9) and leaves group uninitialised like retail: the missing
callee is gone, masked 0.909 -> 0.908; group at function scope is the
same score.
k-scan (s22): compiled behind 0..63 prepended typedefs, 0x004a33b0,
0x004a3c80, 0x004a4c60 have the same code at every count, so their
differences are not a declaration-count effect (docs/VC6_OPERAND_ORDER.md
section 2).
Big-gap pass on 0x004a3c80 (strict 230 -> 564 of 3878, frame 0x1b4 ->
0x1f0 of retail's 0x1fc, `channel` now in ebx like retail). First
divergence was the frame size at offset 2 and the register holding
`channel` at offset 7, one cause: retail has one from/to/cross/axis set
per axis block (tools/frame_layout.py voted two or three retail homes
for each of them; offsets below esp+0x1fc: x block from 0x3c, to 0xe0,
axis 0x14; y block from 0x10c, to 0xf4, cross 0x124, axis 0x30; z
block from 0x118, to 0x100, cross 0x130, axis 0x24; two scalars at
0xec/0xf0). One set reused by all three blocks packs the frame and
takes ebx for something else. Partial splits: y and z sharing a set
521, x and y sharing 275, from/to only 440, from/to/axis 548, cross
only 240, axis only 354, no axis 483.
Left: 0xc of frame. Retail keeps the x block's `to` (controller row 0)
as a copy at 0xe0, loaded with from through ecx/edx/eax, and pushes
AngleBetween's operands from the copies; VC6 propagates our read-only
copy away and pushes from the controller row. Tried on the split base
and flat (564-573, frame 0x1f0): AngleBetween through a reference or
pointer inline, pointer-taking inline dot/cross helpers, a `toPtr`
local, `to = to`, memcpy rows, an inline row getter returning the
cast row, the cross product written on the matrix elements. An inline
row getter building Vector3(m[r][0], m[r][1], m[r][2]) aligns more
instructions (806 of 896 against 770) but copies through x87 where
retail uses integer moves and drops strict to 497; the angle taken
before the cross product 338.
```

## Function names

Names given from each function's behaviour (string literals, D3D/DirectDraw
method slots and arguments, callers); the address-derived names they
replace are in Git history. Tier 3 unless the entry says otherwise.

- `0x004a3bb0` `CompareTargets`
- `0x004a3be0` `AngleBetween`
- `0x004a4bb0` `UpdateChannels`
- `0x004a5290` `CopyMesh`
