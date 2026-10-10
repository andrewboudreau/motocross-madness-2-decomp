# CarProcedural.cpp and Grid1.cpp

`src/reconstructed/CarProcedural.*` and `Grid1.*`. Names are provisional
unless RTTI gives them. Both files declare their own boundary views for the
models, collision and grid terrain they touch. This keeps them away from
the conflicting declarations in Griddraw.h, LightEmitter.h and
ObjectPicker.h.

**CarProcedural.cpp** (`0x0042f390..0x00430feb`). Evidence: the `__FILE__`
literal at `0x005683e4` (xrefs `0x0042f657..0x0042fb52`) and RTTI
`CarProcedural : GraphicsTest : GameObject` (vtable `0x00551000`, 28
slots; overrides 0 and 10 and adds 27). Its four kVec3 `$E` pairs at
`0x00430eb0..0x00430feb` are listed in `.CRT$XCU` right after Camera.cpp's.
Camera.cpp ends at `0x0042f38b` (its last functions are exact; see
[OVERLAY.md](OVERLAY.md)). `0x00430ff0` is the TextureMap.h inline copy,
which is already covered.

Exact (15): the constructor, the destructor and its deleting wrapper,
slot 27 (1020 bytes), the path-file parser `0x0042fa00`, the path distance
search `0x004308e0`, the Hermite weights `0x00430e60` and the eight `$E`.
Source forms needed:
- Slot 27 looks up "Body" through a named local, and its clamp is written
  `n > 0xff ? 0xff : n`.
- The parser keeps its sscanf buffers block-scoped.
- The distance search writes `z*z + (x*x + y*y)`.

Near miss (`samples/race/CarProceduralNearMisses.cpp`): the path evaluation
`0x00430b10`. Retail inlines all 14 vector operators, but VC6 runs out of
inline budget ([VC6_INLINE_BUDGET](VC6_INLINE_BUDGET.md)). Retail's final
sum evaluates `t1 * time` first, then `t0 * h2`, `h1 * b`, `h0 * a`
(VC6's right-to-left argument order through the nested `operator+`). The
budget grows with the caller's tree size, not with empty statements: with
the operator forms, about 30 extra trivial statements (dead code counts,
`;` does not) make every site inline, and the first 0x19b bytes then match
retail (ratio 0.78; the tangent temporaries' slots differ). So the original
caller was about 15% larger in front-end nodes than the decoded code shows,
or its helpers cheaper; no padded source is kept. Slot 10 `0x0042fd80` (2909 bytes) is written
in the same sample (221/2844 strict; with relocations and frame offsets
masked 781 instructions like retail's, 88% aligned): the first update sets
a two-point mesh on the sensor collider and the wheel height; each frame it
wraps `field_0x194` with `fmod`, evaluates the path at the look-ahead
(result named before the subtraction), behind (`-1.0f`) and steering
(`-field_0x198`) distances through `0x004308e0`/`0x00430b10` (the last two
subtract `field_0x1c4` straight from the call result and keep the offset
on the x87 stack, `fld t; fsub st(1)` ... `fstp st(0)`), spins the wheels
(`fmod` by 2 pi), clamps the steering angle to +-30 degrees, queries the
ground (`0x00507c10`), builds the sensor collider's transform with an
inline helper taking the world axes by value (`0x00435830`; the first
cross product expanded, the rest through `0x00515600`/`0x005087b0`),
blends the surface normal (the sensor contact's normal is at +0x08,
retail's `add ecx, 8`), orients the body (`0x004fbd70`) with a target
built per component and the `0x004fd5c0` offset as a named copy, sets the
wheels (`0x004b5d00` or sin/cos of the spin) and pushes the body
collider's hit into its rigid body per component (`* 1.005f`). Left: the
frame (0xd0 here, 0xdc in retail: the wheel loop's axisA shares a slot
with the orientation's `flat` and axisB with nothing, here both share with
the first-update and steering blocks), the steering clamp, which retail
writes through a memory local (`fst`, `fld; fld` reloads) where VC6 keeps
the angle on the x87 stack in every helper and branch form tried, and the
component/scalar load order of the `back` normalisation and blend.

**Grid1.cpp** (`0x0047c880..0x0047db5f`). Evidence: the `__FILE__` literal
at `0x0056c0a0` (xrefs `0x0047c97a`, `0x0047d49b`) and RTTI
`DrawableGridNodeSharedTextures : DrawableGridNode` (vtable `0x00553e54`,
9 slots). It follows GraphicsTest's `$E` pairs and has no `.CRT$XCU` entries
of its own; Gridbase.cpp starts at `0x0047db60`. `0x0047d780` is called only
from Grid1's slot 3, so it belongs here.

Exact (10): the constructor, the destructor and its deleting wrapper, slots
2, 4, 6, 7 and 8, and the two eviction callbacks that slot 3 registers with
AgeManager. Slot 7 reads the block record's index and own bits (one word)
through `x->blocks[block]` each time, which gives retail's single load and
its esi/edi/ebp assignment. Near misses (`samples/render/Grid1NearMisses.cpp`,
notes there): slot 3 `0x0047caa0` (the stage switch with retries), and the
two run-length texture fills `0x0047d470` and `0x0047d780`. All three keep
retail's control flow and calls and differ in register allocation. Slot 3
gets retail's ebp arrangement only when `block` is left uninitialised;
that source is rejected.

TextureMap.h gained `UnknownTextureStream::UnknownFunction461aa0`, which
reads a line.
