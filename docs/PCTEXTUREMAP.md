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

Exact (20 calibration cases):
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
- slot 8, which creates the texture surface. It copies +0x70's
  description, adjusts flags and caps (0x2000 for `b`, AGP for `c`) and
  calls the display's DirectDraw CreateSurface. It shares +0x70 instead
  when that is in video memory and `c` is clear, or when Game+0x2d0 is set.
  It attaches the 8-bit palette and asks slot 9 to upload when `a` is set.
  Retail's shared `return 0` / `return 1` exits come from `goto failed` /
  `goto done`, with `done:` first in the source: VC6 lays labelled return
  blocks out in reverse order;
- `0x004c7420`, which recreates a lost texture through slot 8;
- `0x004c7b00`, which blits +0x70 into another surface unless told to
  skip. The `if (!skip)` form puts the blit first, as retail does;
- `0x004c83a0`, which finds the mip level of a given width through
  GetAttachedSurface (method 12), reporting errors other than `0x887600ff`
  (line 2034). Retail keeps the attached surface in the dead `width`
  parameter's stack slot: a separate `next` local reproduces that, since
  VC6 packs it into the slot;
- slot 15, which builds the mip chain by downsampling each level from its
  parent through Pixtrans.cpp's `0x004d1b90` (error line 1421). One
  `stride` variable carries the parent's stride; the lock and unlock
  failures share one `return 0` through `goto failed`;
- slot 18, which colour-keys the 16-bit (and `0x613`) formats on every
  level (`0x004c7ef0`, error line 1811), re-uploads, sets the surfaces'
  colour key and records render states 0x29 = 1 and 0x1b = 0 through an
  inline find-or-append helper. Retail compares the format with `0x22b8`
  in the re-upload test (sic);
- slot 20, which fills every mip level (error line 2084). Loading +0x70
  into a local before the caps memset reproduces retail's store order;
- `0x004c84e0`, which locks a level (flags 0x811), fills it through
  `0x004c8550` and unlocks it.

Near misses (`samples/render/PCTextureMapNearMisses.cpp`):
- `0x004c7e30`, the colour key: a 24-bit colour packed as 555 or 565, or
  looked up in the palette's 555 table (+0x2c, +0x710). VC6 factors the
  common shift out of every `|` form tried;
- slot 9 (117 of 385 bytes), the upload: BltFast down the mip chain with
  partial texture blits or a positive mode, otherwise the device's Load.
  The frame matches with separate `next` surfaces. Retail keeps `this` in
  ebp and tests the level loop at the top on every pass.

Not reconstructed: slots 4, 5 and 6, the TextureMap base
(`0x0050a4e0`, `0x0050ab40`), `0x004c7b40`, `0x004c7e30`, `0x004c7ef0`,
`0x004c8550` and the error reporter `0x004c86e0`.
