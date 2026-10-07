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
Camera.cpp ends at `0x0042f38b`. Still unmatched there: `0x0042f0e0`,
`0x0042f190`, `0x0042f210` and its eight `$E`. `0x00430ff0` is the
TextureMap.h inline copy, which is already covered.

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
or its helpers cheaper; no padded source is kept. Slot 10 `0x0042fd80` (2907 bytes) is decoded
in outline (path step, wheel spin, steering through `0x004308e0` /
`0x00430b10`, the collision objects' frames through out-of-line cross
product `0x00515600` and normalisation `0x005087b0`) but not written.

**Grid1.cpp** (`0x0047c880..0x0047db5f`). Evidence: the `__FILE__` literal
at `0x0056c0a0` (xrefs `0x0047c97a`, `0x0047d49b`) and RTTI
`DrawableGridNodeSharedTextures : DrawableGridNode` (vtable `0x00553e54`,
9 slots). It follows GraphicsTest's `$E` pairs and has no `.CRT$XCU` entries
of its own; Gridbase.cpp starts at `0x0047db60`. `0x0047d780` is called only
from Grid1's slot 3, so it belongs here.

Exact (9): the constructor, the destructor and its deleting wrapper, slots
2, 4, 6 and 8, and the two eviction callbacks that slot 3 registers with
AgeManager. Near misses (`samples/render/Grid1NearMisses.cpp`, notes there): slot 7
`0x0047d370`, slot 3 `0x0047caa0` (the stage switch with retries), and the
two run-length texture fills `0x0047d470` and `0x0047d780`. All four keep
retail's control flow and calls and differ in register allocation. Slot 3
gets retail's ebp arrangement only when `block` is left uninitialised;
that source is rejected.

TextureMap.h gained `UnknownTextureStream::UnknownFunction461aa0`, which
reads a line.
