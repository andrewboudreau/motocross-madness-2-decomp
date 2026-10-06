# SoultreeMaterial.cpp

`src/reconstructed/SoultreeMaterial.h` / `SoultreeMaterial.cpp`. Evidence:
the `__FILE__` literal at `0x005740d8`, referenced by `0x004ff620` at lines
0x118, 0x165 and 0x17a, and RTTI `SoultreeMaterial : GameObject` (vtable
`0x00557cb0`; only slot 0 is its own). Extent: `0x004fefe0..0x00500214`. It
starts after soultree.cpp's `$E` code and its out-of-line 3x3 transpose at
`0x004fefb0`, and ends before SoulTreePhysics.cpp at `0x00500220`. Method
names are provisional.

The material holds render states, a SurfaceMap (+0x70) and textures. It
loads them from a stream (`0x004ff450`) or from material keys
(`0x004ff9e0`); both paths use the texture loader `0x004ff620`.

Exact: 10 calibration cases. These are the constructor, the destructor and
its deleting wrapper, attach `0x004ff0b0`, restore `0x004ff410`, the stream
load, the texture load, the parameter parse, the NONE reset `0x005000b0`
and copy `0x005000f0`.

Near miss (`samples/render/SoultreeMaterialNearMisses.cpp`): the
render-state switch `0x004ff180`, 650 of 652 bytes. Only the vtable
register in case 8 differs.

Parameterblocks.h and PCRenderTarget.h gained declarations for the calls
and the field at +0x1c0; the layout is unchanged.
