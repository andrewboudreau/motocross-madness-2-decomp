# TextureMapManager

RTTI: `TextureMapManager : GameObject` (vtable `0x0055848c`, 0x7c bytes).
Its code cites `D:\aardvark\VC\krusty2\TextureMapManager.cpp` through
`__FILE__` (`0x00574f00`). Canonical source:
`src/reconstructed/TextureMapManager.h` / `TextureMapManager.cpp`. Names are
provisional.

Layout (from the constructor and the methods below):
- +0x2c: an intrusive list of every registered TextureMap, linked through
  TextureMap+0x08 (next) and +0x0c (previous), with head, tail, cursor and
  count. Its methods are out of line (`0x00510a50`–`0x00510b70`); the class
  has no RTTI and its name is unknown;
- +0x3c / +0x40: the ManagedTextureGroup selected by the debug keys and its index;
- +0x44: `ContainerList<ManagedTextureGroup*>` (ContainerList.h), initialised
  to 4 entries growing by 4;
- +0x58: debug key mode; +0x5c / +0x60 two grow-only scratch buffers with
  sizes at +0x64 / +0x68 (used by PCTextureMap slot 5);
- +0x70: a BlockAllocator.cpp pool (`0x00423f70`, 0x30-byte blocks × 0x1800);
- +0x74: frame count (slot 10).

The constructor also clears the shared texture surfaces (`0x0068a36c`,
`0x0068a394`, 10 each) that PCTextureMap slot 4 reuses; the destructor
releases them.

## Status

Exact (25 calibration cases, every function in the file's range):
- the list: constructor, destructor (unlinks every texture), First, Last,
  Next, Previous, Append, Remove and AppendList;
- the constructor `0x00510bd0`, scalar deleting destructor `0x00510cd0` and
  destructor `0x00510cf0`;
- slots 10 (counts frames), 12, 13 (skipped on AGP displays) and 15 (with
  the "TestKey" debug bit), which forward to every ManagedTextureGroup before the
  GameObject base; slot 18 (`0x00511290`), which calls slot 12 of every
  texture, then the caches;
- slot 23 (`0x00510ee0`), the "TestKey" debug keys: 0x14 cycles a mode,
  0x21 toggles the selected cache's +0x08, 0x1b / 0x1a step forwards /
  backwards through the cache's +0x1c8 or +0x1c4 entries or through the
  caches, 0x15 runs the cache's `0x0050c7e0`;
- `0x00511180`, which creates a ManagedTextureGroup (RTTI
  `ManagedTextureGroup : BaseObject`, 0x258 bytes, constructor `0x0050bed0`
  among TextureCache.cpp's literals) and adds it through ContainerList's
  inline Add;
- `0x005113d0`, which on AGP displays restores every grouped
  ManagedTexture (`0x00510760`, a direct PCTextureMap slot 8 call) and
  otherwise hands the display's "TextureCacheLimit" (+0x60) out in
  0x2aaaa-byte steps, each to the group with the fewest steps per texture;
- `0x00511580`, which counts the 32–256 pixel textures, their bytes with
  mip levels and the display's per-kind figure (+0x14 table, accumulated
  but unused), and formats them and GetAvailableVidMem's totals into a
  local buffer that is never output;
- `0x005112f0` / `0x00511300` (register / unregister a texture) and the
  scratch buffers `0x00511310` / `0x00511370`.

## ManagedTexture

RTTI: `ManagedTexture : PCTextureMap` (vtable `0x00558430`, 0xbc bytes; it
overrides slots 0, 7, 8, 11 and 19). Its code sits between
TextureMapManager.cpp's functions (`0x00510500`–`0x005109bc`). Canonical
source: `src/reconstructed/ManagedTexture.h` / `ManagedTexture.cpp`.

A ManagedTextureGroup (+0x90) owns each ManagedTexture. While one is placed
on a CacheTexture page (+0x80, RTTI `CacheTexture : PCTextureMap`, vtable
`0x005583d8`), it records its region (+0x94), its scale and u/v offset
within the page (+0x84..+0x8c), binds the page instead of itself (slots 11
and 19) and maps texture coordinates into the page (`0x00510780`,
`0x00510910`). Slot 8 (create the surface) does nothing; `0x00510760` calls
PCTextureMap's slot 8 directly.

Exact (16 calibration cases, every function in that range): the constructor,
scalar deleting destructor and destructor, slots 7, 8, 11 and 19, and
`0x00510610`, `0x00510670`, `0x00510700`, `0x00510760`, `0x00510780`,
`0x00510820` (records a use: count, largest value up to 9, manager frame),
`0x00510910`, `0x00510990` (log2 of the width) and `0x005109b0`.

## ManagedTextureGroup

RTTI: `ManagedTextureGroup : BaseObject` (vtable `0x005583c0`, 0x258 bytes).
Its code cites `D:\aardvark\VC\krusty2\TextureCache.cpp` (`0x00574b84`).
Canonical source: `src/reconstructed/ManagedTextureGroup.cpp` (declared in
`TextureMapManager.h`).

A group holds ManagedTextures of one pixel format (+0x0c; address modes at
+0x10/+0x14) in +0x44 and packs them onto 256x256 CacheTexture pages
(+0x54, 0x190 bytes each, constructor `0x0050f6a0`). The pages share the
system surface of one PCTextureMap at +0x1c0. Sixteen
`ContainerList<ManagedTexture*>` members (+0x7c, nine at +0x90, +0x144 and
five more) sort the textures by size during repacking; three PCVideoCard.cpp
timers (`0x004cb670`, 5000 ms) and a GDI object (+0x1f8) are also owned.

Exact (14 calibration cases): the constructor, scalar deleting destructor and
destructor; `0x0050c4a0` (sets the page count); `0x0050c6c0` (adds a texture,
taking the palette of an 8-bit group from it); `0x0050c760` (clears the
textures' use records); `0x0050c790` (restores lost pages); `0x0050c7e0`
(appends a report to `C:\temp\TM_debug.txt` through `0x00461d40`); and
`0x0050c8c0` (empties the size lists and repacks); the qsort comparators
`0x0050ee70`, `0x0050eeb0` and `0x0050ef00`; and `0x0050d800`, which lowers
the planned levels in one size list until its texels fit a budget, keeping
the list sorted (ContainerList's inline ordered remove and insert). Its
position search is an inline helper: with it VC6 runs out of inline budget
and calls `ContainerList::Reserve` (`0x005109e0`, also exact) out of line,
as retail does.

Not reconstructed: the repacking itself, `0x0050c960` (about 3.2 KB) and
`0x0050dad0` (with partial texture blits), `0x0050e730` and the debug
display `0x0050ef70` (manager slot 15).
