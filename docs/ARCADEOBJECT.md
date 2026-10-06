# AgeManager.cpp, ArcadeObject.cpp and Arrow.cpp

The first three translation units of the image, `0x00401000..0x00401a2a`.
The sources are `src/reconstructed/AgeManager.*`, `ArcadeObject.*` and
`Arrow.*`. Names are provisional unless RTTI gives them.

**AgeManager.cpp** (`0x00401000..0x0040125a`). Evidence: the `__FILE__`
literal at `0x005665c0`, used at line 17 (the destructor's free) and line 38
(DebugRealloc). It has no RTTI, so the class name comes from the file name
(tier 2). It is an LRU table: entries register, unregister, are touched with
the manager's current age, and are evicted oldest first after a qsort.
Griddraw.h's GridAgeManager is the terrain's view of the same object.

**ArcadeObject.cpp** (`0x00401260..0x00401944`). Evidence: RTTI
`ArcadeObject : GameObject` with vtable `0x00550414`, overriding slots 0, 10
and 14. Its `__FILE__` literal at `0x00566644` is used at lines 78, 104, 154
and 160. The loader `0x00401310` creates the D3DIMSoultreeObject model and
a TransparencyMod; `0x00401540` builds its CollisionObject from a `.col`
file. `0x00401940` is BaseObject's shared GetRefCount and is not claimed.

**Arrow.cpp** (`0x00401950..0x00401a2a`). Evidence: the `$E` pair whose
initializer creates the global ArrowManager at `0x005776d0` (`__FILE__`
line 4), and RTTI `ArrowManager : GameObject` (vtable `0x00550490`). The
code from `0x00401a30` to `0x0040225f` (SoundGroup and two more vtables) has
no Arrow.cpp reference. It is sound code, most likely the start of
AuralScape.cpp; this is unconfirmed and that code is not claimed.

Exact: all 24 functions (9, 10 and 5). Two source forms were needed:
- `0x00401310` stores `field_0x30`, `field_0x44` and `field_0x40` in that
  order.
- `0x004017a0` writes the degrees-to-radians constant as `0.01745329f`.

LightEmitter.h, MemTag.h and ObjectPicker.h gained the declarations these
files call.
