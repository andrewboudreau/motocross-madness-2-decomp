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

The shared [SurfaceMap provider](SURFACEMAP.md) now supplies its constructor,
SetTexture and Select calls. Its declaration retains the retail caller's
`0x70` allocation, including the unknown tail beyond the material fields.

Exact: 11 calibration cases. These are the constructor, the destructor and
its deleting wrapper, attach `0x004ff0b0`, the render-state switch
`0x004ff180`, restore `0x004ff410`, the stream load, the texture load, the
parameter parse, the NONE reset `0x005000b0` and copy `0x005000f0`. In the
render-state switch, mapping types 3 and 6 are separate case statements with
identical bodies that VC6 merges into one block; written as `case 3: case 6:`
the case 8 call gets the other vtable register (650 of 652 bytes).

Parameterblocks.h and PCRenderTarget.h gained declarations for the calls
and the field at +0x1c0; the layout is unchanged.

The motion controller's former `SceneItem` view now names this actual class.
Its complete `CharacterVirtualSlot0` caller at `0x00445a70` (1021 bytes)
allocates `0xd0` bytes at `0x00445cd0`, then directly calls the material
constructor at `0x00445cf4`, Attach at `0x00445d46`, and CopyFrom at
`0x00445d85`. Their destinations are `0x004fefe0`, `0x004ff0b0` and
`0x005000f0`; the existing providers match all 164, 103 and 295 bytes.
The controller retains its provisional multiple-inheritance `ModelObject`
view. Explicit conversions pass its existing context, manager and options
words into the typed material interface; allocation size and ownership
operations are unchanged. See [the shared loader connection](TEXTURE_LOADER_CONNECTIONS.md)
for the caller and layout regression checks.

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
