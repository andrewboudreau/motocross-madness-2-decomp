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

Exact (23 calibration cases):
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
- slot 5 (2129 bytes), which loads a texture from a stream (`0x00461600`
  position, `0x00461640` read). Tgafile.cpp's file-format helpers give the
  decoded format (`0x005118a0`), whether mip levels are stored
  (`0x00511850`), whether data is compressed (`0x00511800`) and the stored
  bytes per pixel (`0x00511740`). Two manager scratch buffers
  (`0x00511310`, `0x00511370`) hold the read and decoded bits; Lzw.cpp's
  `0x004a03d0` expands file formats 6–12 / 0x15–0x1b. Mipmapped files hold
  an offset table (each entry relative to where it was read) and the levels
  from 1x1 up; levels at least `minimumSize` wide are converted into the
  surfaces slot 4 creates (GetAttachedSurface, then Pixtrans `0x004d1d20`)
  and the levels the format choice's +0x14 drops are read past. Other files
  are halved (Pixtrans `0x004d1b90`) up to +0x14 times while wider than 32
  and handed to slot 4. Indexing the offset table (`offsets[level]`) rather
  than walking a pointer is what reproduces retail's registers: VC6
  strength-reduces the index itself. The pixel-size maximum is evaluated
  max-macro style (the larger size is recomputed), and the raw-level reads
  go through an inline helper, which makes VC6 compute the row size before
  pushing the row count;
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
- slot 20, which dumps every mip level (error line 2084). Loading +0x70
  into a local before the caps memset reproduces retail's store order;
- `0x004c7e30`, the colour key: a 24-bit colour packed as 555 or 565, or
  looked up in the palette's 555 table (+0x2c, +0x710). The file-local
  `Pack555`/`Pack565` helpers cast the whole `|` to `unsigned short` and
  list blue first; without the cast VC6 factors the common `>> 3` out of
  the expression, and red-first order swaps two terms in `0x004c7ef0`;
- `0x004c7ef0`, which locks a level (flags 0x801) and replaces magenta
  with the key colour through Pixtrans.cpp's replacers (`0x004d1970`
  32-bit RGBA, `0x004d1a20` 24-bit RGB, both taking pixel structs by
  value; `0x004d1ac0` 16-bit; `0x004d1b40` 8-bit, from the palette's
  magenta entry +0x710[0x7c1f]), storing the converted key. Magenta itself
  only sets the key (`0x004c7e30`) except in format 0x22b8. Lock and
  unlock failures share one `return 0` through `goto failed`;
- `0x004c84e0`, which locks a level (flags 0x811), writes it to a file
  through `0x004c8550` and unlocks it. Slot 20 calls it on every level with
  no name, so slot 20 dumps the mip chain.

Near misses (`samples/render/PCTextureMapNearMisses.cpp`):
- `0x004c7b40` (about half of 750 bytes), the table blit: BltFast without
  a table; otherwise it locks both surfaces (0x811), clips the rectangle to
  the destination and sets each pixel to `table[source << 8 | destination]`
  (8-bit; 16-bit pixels read the destination byte through the pixel value).
  The loop nest's register assignment differs;
- `0x004c8550` (391 of 393 bytes), the level dump: the first free
  `C:\temp\<name><nnn>.bmp` (8-bit, through bmpfile.cpp's `0x004245f0`
  and `0x00424380`) or `.tga` (converted to 32-bit by Pixtrans `0x004d1d20`
  into a `DebugMalloc` buffer, lines 2149/2154, and written by Tgafile.cpp's
  `0x005127f0`). `name` defaults to "tex". Only the buffer size's
  multiplication operand order differs;
- slot 9 (117 of 385 bytes), the upload: BltFast down the mip chain with
  partial texture blits or a positive mode, otherwise the device's Load.
  The frame matches with separate `next` surfaces. Retail keeps `this` in
  ebp and tests the level loop at the top on every pass;
- slot 6 (61 of 600 bytes), the copy. It makes a `ManagedTexture` (RTTI
  `ManagedTexture : PCTextureMap`, 0xbc bytes, registered with the source's
  +0x90 ManagedTextureGroup) when TextureMap+0x68 bit 0 is set, else a
  PCTextureMap. It fills the copy through slot 4 and copies the colour key
  through slot 18. The structure lines up; retail's register assignment
  (constant 1 in ebx, the copy in ebp) does not.
- slot 4 (2031 bytes), the setup: it picks the format (through the
  optional +0x0c/+0x10 choice when the format has alpha), counts mip
  levels by halving both sides down to `minimumSize`, reuses a shared
  surface (`0x0068a394` plain, `0x0068a36c` mipmapped, indexed by level
  count) or creates one through `0x004c68e0`'s format fallbacks, converts
  the bits (Pixtrans.cpp `0x004d1d20`), measures alpha (`0x004d24d0`),
  builds the mips through slot 15 and appends render states 0x29/0x1b
  (+0x13/0x14 address modes when the format has alpha) through an inline
  append helper. Only the non-mip fallback chain differs: VC6 cross-jumps
  its identical call tails into the first case, retail into the last.

Not reconstructed: the error reporter `0x004c86e0`.

## TextureMap

`src/reconstructed/TextureMap.h` / `TextureMap.cpp` (Texmap.cpp's
literals). Exact (4 calibration cases):
- the constructor `0x0050a4e0` (manager at +0x10, registered through
  `0x005112f0` when the second argument is set; clears the size, format,
  mip count, +0x28, +0x30, +0x40, pair count and +0x68/+0x6c);
- the scalar deleting destructor `0x0050a570` and the destructor
  `0x0050ab40`: releases the +0x28 object, unregisters from the manager
  (`0x00511300`) and removes the resource manager's (`0x00572b44`) entry
  for the texture (`0x004e93f0`, `0x004e9010`);
- `0x0050abd0`, which rewrites the 0x13/0x14 address-mode render states of
  an alpha texture.

Not reconstructed: `0x0050a590` (1442 bytes, cdecl), which looks a texture
up through the resource manager (`0x004e9360`), returns its existing map or
reads a header from the entry's stream and creates a PCTextureMap or
ManagedTexture through slots 4/5.
