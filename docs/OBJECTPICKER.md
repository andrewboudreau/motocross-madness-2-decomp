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

Exact: 12 calibration cases. These are the constructor, the destructor,
the deleting destructor, `0x004b04c0`, and the eight `$E` initializers
for kVec3Zero (`0x00688708`), kVec3XAxis (`0x00688718`), kVec3YAxis
(`0x00688728`) and kVec3ZAxis (`0x006886f8`). Init `0x004b0210` and the
ray `0x004b0500` are not reconstructed yet. The best attempt at
`0x004b0500` differs only in the dot-product order and the stack-slot
reuse.
