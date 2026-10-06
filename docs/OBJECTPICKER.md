# ObjectPicker.cpp

`src/reconstructed/ObjectPicker.h` / `ObjectPicker.cpp`. Evidence: the
`__FILE__` literal `D:\aardvark\VC\krusty2\ObjectPicker.cpp` (`0x0056edfc`,
referenced from `0x004b0239`, `0x004b0330`, `0x004b0388` and `0x004b041c`),
RTTI `ObjectPicker : GameObject` (vtable `0x0055550c`, COL `0x0055dad0`;
only slot 0 is overridden), and the four file-static axis vectors whose
`$E` initializers end the TU. The code runs from `0x004b01c0` (after
Nulls.cpp) to `0x004b08eb`. Method and field names are provisional.

ObjectPicker casts a ray from the camera through a screen point and
returns what the CollisionObject at +0x30 hits. `0x004b0210` (init) builds
that CollisionObject and, unless a cursor is passed in, a GameCursor driven
by two control bindings (+0x40, +0x44). `0x004b04c0` picks at the cursor
(y measured from the bottom of the render target) through `0x004b0500`,
which reads the camera's position, viewport and camera-to-world rows. The
camera fields are read through `UnknownPickCamera`, because Camera.h keeps
them protected and declares +0x198 as int while this code loads it as a
float. GameCursor gained `UnknownFunction43f100` (`0x0043f100`), which
returns the cursor position as two ints through a hidden pointer.

Exact: 14 calibration cases. These are the constructor, the destructor,
the deleting destructor, init `0x004b0210`, `0x004b04c0`, the ray
`0x004b0500`, and the eight `$E` initializers for kVec3Zero
(`0x00688708`), kVec3XAxis (`0x00688718`), kVec3YAxis (`0x00688728`) and
kVec3ZAxis (`0x006886f8`); the whole TU is matched.

Matching notes for `0x004b0500`. Retail squares y, then x, then z, with z
reloaded from memory; the source is written `z*z + (x*x + y*y)`. The
0x30-byte frame needs the scale applied in place per component, then
`direction += position`; any `direction * 800.0f` form adds a 12-byte
temporary.

`ObjectPicker.h` declares only the CollisionObject members this file uses.
The bases come from RTTI: QuadTreeObject at +0 and GraphicsTest (a
GameObject) at +0xc, which is why retail adds 0xc before calling
`0x00469190`. Init's third argument is stored at CollisionObject+0x88, the
on-hit callback slot that `krusty2/collision/CollisionObject.h` describes.
