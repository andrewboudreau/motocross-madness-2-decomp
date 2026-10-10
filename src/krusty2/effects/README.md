# effects: Particles.cpp, NormalDistribution.cpp, Nulls.cpp

Validation: the 19 cases in this directory pass strict VC6 SP3 comparison,
including the added NullManager slot-8 override. Adjacent bindings resolve every
relocation. The emitter samples are strict too (bindings next to each sample source).

Retail files (all under `D:\aardvark\VC\krusty2\`):

| file | `__FILE__` string VA | bracket | own `__FILE__` xrefs | exact / partial |
|---|---|---|---|---|
| NormalDistribution.cpp | 0x0056ece4 | 0x4afdea..0x4b0109 | 0x4affcb..0x4b0059 (debug delete, line 0x1e) | 6 / 0 |
| Nulls.cpp | 0x0056ed14 | 0x4b0059..0x4b0239 | 0x4b0109 (the `new` at line 5) | 6 / 0 |
| Particles.cpp | 0x0056fa00 | 0x4b8584..0x4bb8d8 | 0x004ba4f1 and 0x004ba5c8 (slot 27, line 0x48a and 0x4a8) | 7 / 0 (+ slot 14 unattempted) |

19 strict cases, 0 partial in this directory. `samples/physics/effects/` adds 33 strict exact and 8 partial (41 entries) for the
emitter classes and the vector-constant initialisers, whose ownership is not proven; 0 required failures
(DirtChunk, DirtSpray and Spark slot 10 are 98.3% partials).

## Ownership evidence

* NormalDistribution (0x4aff90..0x4b00d4): the debug `delete` in the dtor (0x4b0050) pushes this file's string
  (0x0056ece4, line 0x1e). The static initialiser, atexit wrapper, ctor and `Lookup` are contiguous with it.
  `Lookup` (0x4b0070) lies inside the Nulls bracket but follows the dtor directly and is a method of the same class.
* Nulls (0x4b00e0..0x4b01bb): the static initialiser 0x4b00f0 does `new(__FILE__, 5)` with string 0x0056ed14.
  `NullManager` methods 0x4b0160..0x4b01bb follow contiguously. ObjectPicker.cpp starts at 0x4b01c0 and is not part of this file.
* Particles (0x4ba390..0x4bac59): slot 27 (0x4ba4d0) references 0x0056fa00 twice; the ctor, dtors, `AddParticle`, slot 10
  and the qsort comparator are contiguous with it. Parser code at 0x4b84c0..0x4b89f5 belongs to Parser.cpp.

## Class layout facts

* `NormalDistribution` (no RTTI): `float* table` +0x00, `int count` +0x04; `static NormalDistribution(100)` at 0x006886e8.
* `NullManager`: RTTI COL 0x0055da80, vtable 0x0055549c (27 slots), base GameObject, size 0x418; slot 8 is the shared stub 0x00462e30.
* `ParticleManager`: RTTI COL 0x0055e0b0, vtable 0x00555d10 (28 slots), base GameObject at offset 0. Overrides slot 0, slot 10
  (0x004baaf0), slot 14 (0x004bac60) and adds slot 27 (0x004ba4d0). Size at least 0x2058; each `Particle` is 0x48 bytes
  (`new(0x48, __FILE__, 0x48a)` x 1000).
* Slot 27 (0x004ba4d0, exact) fills the rest of the object: `field_0x18`(+4)(+0x194) is an IDirect3D7-shaped object whose slot 5
  (CreateVertexBuffer) gets a {0x10, caps 0x800, FVF, count} block three times: +0x1f84 (FVF 0x1c4, 1500), +0x1f88 (0x1e2, 1500),
  +0x1f8c (0x1c4, 4000; locked with flags 1, 0x1f400 bytes zeroed, unlocked). +0x1f94 is `new unsigned short[6000]` (line 0x4a8) of quad
  indices (4i, 4i+1, 4i+2, 4i, 4i+2, 4i+3). +0x1f80 is the 0x0050a590 texture (format 0x115c, key 0xff00ff), slot 8 called with (1, 0, 0).
  +0x1fa4 holds four 0x20-byte vertex templates (x, y = +-1, z = 0), +0x2024 four Vec3 (+-1, +-1, 0.25) * FastInvSqrt(2.0625), +0x2054 the
  last argument. Returns this, or 0 after BaseObject slot 2 when either vertex buffer at +0x1f84/+0x1f88 or the index array is null.
* Global 0x00689158: 61 sprite cells of four (u, v) corners. Slot 27 fills cells 0..13 (4x4 grid of 0.25), 14..17 (0.125 cells from
  (0.25, 0.75)), 29..44 (0.0625 cells from (0.5, 0.75)) and 45..60 (0.0625 cells from (0.75, 0.75)); cells 18..28 are not written there.
  Shapes that mattered: the index loop written over the quad number (`indices[i*6 + k] = i*4 + c`); a quad-counter loop over the
  index position, or a named `v = i*4`, changes the strength reduction.

## Proposed slot names (tier 3)

* ParticleManager slot 10 `GameObjectVirtualSlot10(float dt)`: the per-frame update; returns 1 at once when the game context
  (global 0x0056e26c) `+0x1c4` is nonzero. Ages particles by dt*1000, recycles dead ones, integrates size (flag 8), position (flag 4) and
  velocity with gravity (flag 0x10).
* ParticleManager slot 14: the render pass (sorts the visible particles with `CompareParticleDepth`), not reconstructed.
* Slot 27 (ParticleManager and all five emitters, ICF-folded for the emitters, body 0x004ba040): stays `UnknownVirtualSlot27`.
  The emitter body calls `GameObject::slot 8(arg0)` and stores arg1 at +0x30 (the manager), or releases `this` when it is null.

## Emitter slot 10 bodies (samples/physics/effects)

* Steam 590 (exact) and Dust 732 (exact) call `ParticleManager::AddParticle` along the segment moved this tick. Shapes that mattered:
  component-wise `d`, `lenSq` written `z*z + (y*y + x*x)` (the only term order VC6 compiles to retail's y, x, z sums),
  `(1.0f/count) * d` for Steam but `d * inv` with a named `inv` for Dust, `rate * (intensity * k)` for Dust's speed, and the count
  living in the dead `dt` parameter slot (Dust assigns `dt = len / spacing`).
* DirtChunk, DirtSpray (970 each) and Spark (1055) write particles straight into `manager->particles[manager->activeCount]` and are
  98.3% matches with the same size: the only difference is the first component of each `(n - i)`-scaled vector, where retail emits
  `fld mem; fmul st(1)` and VC6 emits `fld st(0); fmul mem` (three places per function). Tried operand order, a named `r` (which swaps the
  `i`/`r` stack slots) and inline `1.0f/n` (worse). Cause not found.
  DirtChunk and DirtSpray share one shape (Spray: initial age 375, frame wraps at 13); Spark replaces the velocity multiply with a copy of
  `randomVectors[]` and clears its own state when done.
* `field_0x64` is a float (carry or timer) in DirtChunk, DirtSpray and Spark, and `field_0x74..0x7c` (Spark `0x78..0x80`) is a Vec3 `launchVelocity`.

## `$E` numbering

The static-initialiser numbers (`_$E1`, `_$E2`, ...) in the targets were picked against the compiled object, not derived from source
order. The observed numbering is consistent with declaration order (kVec3 pairs E1/E2, E4/E5, E7/E8, E10/E11; NormalDistribution
E1/E3/E2 with the body, atexit registration and dtor wrapper), but no source order was available to prove that.

## Open problems

* ParticleManager slot 14 (0x004bac60..0x004bb61e) is not attempted: it uses an ebp frame and the `fstp; fld; mov eax,[p]; fistp [eax]`
  float-to-int sequence of an inline-asm rounding helper (0x004bb26b, 0x004bb2a5), so it is excluded. It also owns a function-local
  static 16-dword matrix at 0x006898f8 (guard bit 0 of 0x00689154, initialised from 0x004a1410) whose atexit destructor thunk is the
  one-byte `ret` at 0x004bb620 (pushed at 0x004bacbb; the type therefore has an empty user-declared destructor). That thunk can only
  be produced together with slot 14.
* The five emitter constructors are partial: the retail store order interleaves the position/previousPosition zeroing with the
  scalar stores, and Steam's table fill stores one float through a temporary and four ints (a Vec4-like fill); the source shape
  that causes either is not found.

## Reviewed relocation evidence

NormalDistribution's initialization and shutdown wrappers independently use the
same object at `0x006886e8`; the initializer passes resolution 100 to the constructor
at `0x004affc0`, and atexit registers `0x004affb0`, which jumps to `0x004b0050`.
The constructor calls `0x004a2e20` at `0x004affda`: this is DebugMalloc, not the
placement operator-new entry at `0x004a3010`. The source now uses the shared
DebugMalloc declaration. The polynomial constants and source-path literal were
checked against their actual data bytes. `atexit` and `__ftol` are CRT routines.

NullManager's initializer allocates `0x418` bytes, calls its constructor, and
writes the result at `0x006886f0`. RTTI identifies table `0x0055549c` and its
non-inherited slot 8 at `0x00462e30`. That 19-byte body explicitly calls
GameObject slot 8 at `0x004692f0`, then returns the saved `this`; the source now
models the override. The initializer EH stub at `0x0054c674` loads FuncInfo
`0x00562350` with magic `0x19930520` and dispatches to the VC6 handler.

ParticleManager's RTTI table is `0x00555d10`; its destructor's EH stub at
`0x0054c9a8` loads FuncInfo `0x00562628`. The update body reads the context
pointer at `0x0056e26c` and tests its +`0x1c4` flag before multiplying the time
step by the checked `1000.0f` literal at `0x00550f44`. Calls to GameObject
lifetime functions and the allocator agree with their canonical reconstructions.
Semantic names remain provisional; the bindings record observed destinations.
