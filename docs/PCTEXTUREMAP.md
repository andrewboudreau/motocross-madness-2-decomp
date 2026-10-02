# PCTextureMap

RTTI: `PCTextureMap : TextureMap : BaseObject` (vtable `0x00555fec`).
`TextureMap`'s vtable (`0x00558354`) leaves slots 4–20 pure (`_purecall`);
PCTextureMap overrides 18 slots. Its code sits among `PCTexMap.cpp`'s
literals; TextureMap's sits among `Texmap.cpp`'s. Canonical source:
`src/reconstructed/PCTextureMap.h` / `PCTextureMap.cpp`. Names are
provisional.

The texture wraps two DirectDraw 7-style surfaces (`UnknownSurfaceInterface`,
see `RenderInterfaces.h`): a system-memory copy at +0x70 and the texture
at +0x74. An object at +0x7c is destroyed through vfwdeco.cpp's
`0x0052d050`.

## Status

Exact (15 calibration cases):
- the constructor `0x004c5f00` (TextureMap's `0x0050a4e0`, then clears
  +0x70..+0x7c);
- the scalar deleting destructor `0x004c5f30` and the destructor
  `0x004c5f50`. The destructor frees +0x7c and takes the +0x70 surface's
  size off the "in DirectX" memory count (`MemTagStack` `0x004a2e00`). The
  size is Tgafile.cpp's bytes per pixel (`0x00511970`) × height × width,
  times 4/3 with mip levels. It then releases both surfaces and, when
  `0x00689964` is set, notifies the manager (`0x00511580`);
- slots 7 (has a texture), 10 (releases it), 11 (sets it as texture
  stage 0 on the global render target's device), 12 (restores a lost
  texture), 13/14 (lock/unlock +0x70), 16/17 (lock/unlock a mip level from
  `0x004c83a0`, flags 0x801) and 19 (binds, then applies the render-state
  pairs at +0x48 through RenderTarget slot 8);
- `0x004c7420`, which recreates a lost texture through slot 8;
- `0x004c7b00`, which blits +0x70 into another surface unless told to
  skip. The `if (!skip)` form puts the blit first, as retail does;
- `0x004c84e0`, which locks a level (flags 0x811), fills it through
  `0x004c8550` and unlocks it.

Near misses (`samples/render/PCTextureMapNearMisses.cpp`):
- `0x004c83a0`, which finds the mip level of a given width through
  GetAttachedSurface (method 12) and reports errors other than
  `0x887600ff` with `0x004c86e0` (line 2034). Retail keeps the surface in
  the dead parameter's stack slot;
- slot 20, which fills every mip level (error line 2084). Retail clears the
  capabilities in an order that the memset, `= {0}` and field-by-field
  forms all fail to reproduce.

Not reconstructed: slots 4, 5, 6, 8, 9, 15 and 18, the TextureMap base
(`0x0050a4e0`, `0x0050ab40`), `0x004c7b40`, `0x004c7e30`, `0x004c7ef0`,
`0x004c8550` and the error reporter `0x004c86e0`.
