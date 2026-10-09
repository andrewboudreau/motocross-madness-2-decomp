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

Exact: 11 calibration cases. These are the constructor, the destructor and
its deleting wrapper, attach `0x004ff0b0`, the render-state switch
`0x004ff180`, restore `0x004ff410`, the stream load, the texture load, the
parameter parse, the NONE reset `0x005000b0` and copy `0x005000f0`. In the
render-state switch, mapping types 3 and 6 are separate case statements with
identical bodies that VC6 merges into one block; written as `case 3: case 6:`
the case 8 call gets the other vtable register (650 of 652 bytes).

Parameterblocks.h and PCRenderTarget.h gained declarations for the calls
and the field at +0x1c0; the layout is unchanged.

## Function names

Names given from each function's behaviour (string literals, D3D/DirectDraw
method slots and arguments, callers); the address-derived names they
replace are in Git history. Tier 3 unless the entry says otherwise.

- `0x004ff0b0` `Attach`
- `0x004ff180` `ApplyRenderStates`
- `0x004ff410` `RestoreRenderStates`
- `0x004ff450` `ReadSaved`
- `0x004ff620` `LoadTexture`
- `0x004ff9e0` `ReadKeys`
- `0x005000b0` `MakeUntextured`
- `0x005000f0` `CopyFrom`
