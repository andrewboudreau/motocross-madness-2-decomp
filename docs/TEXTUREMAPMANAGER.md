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

Near misses (`samples/render/ManagedTextureGroupNearMisses.cpp`):
- `0x0050c960` (3742 bytes), the repack: plans each used texture's level from
  its recorded use (at least 5), drops unused ones, empties the pages the
  planned textures want least until the textures that need a place fit,
  refills them largest level first (CacheTexture `0x0050f9b0`, `0x0050fdb0`,
  slot 9) and writes its figures to the debug overlay's "TextureManager no
  partial" page. Control flow, calls and VC6's inlining choices match: VC6
  inlines `ContainerList::Reserve` only in the last two `Add` calls, as
  retail does, once three small helpers carry part of the work (its inline
  budget scales with the function's size). Stack-slot packing differs;
- `0x0050dad0` (5020 bytes), the repack with partial texture blits (chosen
  by `0x0050c8c0` when the display's +0x5bc is positive): plans and lowers
  the used textures' levels to fit the group, turns the level counts into
  the page plan (+0x18), sets aside pages whose root region (CacheTexture
  +0x84) is one reserved leaf, re-plans the rest (`0x0050f9b0`, returning
  whether everything fitted), places the textures
  still needing room (`0x0050fdb0`, which also lists them in +0x144; when
  every page is tried it calls the shared empty body `0x00464e90`) and
  spreads their blits (`0x005102d0`) over up to two passes within +0x74
  texels, then fills the overlay's "TextureManager partial blts" page.
  Control flow, calls, inlining and the scalar stack slots match (one
  `managed` variable across the loops, `while (pass < 3 && ...)` for the
  blit passes); two of the five 9-entry arrays (0x8c/0xb0) are swapped and
  the overlay rows rotate eax/ecx/edx, about 145 of 1600 instructions;
- `0x0050ef70` (1832 bytes), the debug display for manager slot 15: the
  selected texture copied into the top right of the render target with
  its use and Un/Hi/Lo state, or the selected page with its textures and an
  outline (GDI pen, MoveToEx/LineTo, TextOutA on the surface's DC). Retail
  saves ebx/esi only after the first early return.

## CacheTexture

RTTI `CacheTexture : PCTextureMap` (vtable `0x005583d8`, 0x190 bytes) is
the page a ManagedTextureGroup packs its ManagedTextures onto. Its code
(`0x0050f6a0`-`0x005104fb`) sits between ManagedTextureGroup's and
ManagedTexture's. Canonical source: `src/reconstructed/CacheTexture.h` /
`CacheTexture.cpp`; 14 functions are exact.

The page is a quadtree of 0x30-byte regions taken from the manager's
`BlockAllocator` (TextureMapManager+0x70): four quarters, parent, occupant,
level, the u/v rectangle and a reserved flag. The page keeps free (+0x88)
and reserved (+0xac) leaf counts per level and a list of reserved leaves
per level (+0xd0, nine `ContainerList`s of 32). Exact:

- the constructor `0x0050f6a0` (one free root of the given level), the
  implicit destructor `0x0050f830` (it does not reset the vtable pointer;
  an explicit empty destructor would) and deleting wrapper `0x0050f810`;
- `0x0050f890`/`0x0050f8e0` (list the occupants), `0x0050fc40` (empty
  the page), `0x0050fc60` (first quarter, splitting on demand),
  `0x0050fc90` (evict and unreserve a leaf), `0x0050fd60` (collapse a
  subtree back into the pool), `0x0050fdb0` (place textures on reserved
  leaves, blitting them when no list is given), `0x0050ffa0` (split),
  `0x00510120` (unplace, merging empty parents), `0x00510250` (occupy)
  and `0x005102b0` (take a texture off).

Two recurring VC6 shapes: chained assignments (`a = b = 0`) give retail's
reversed store order, and the region `Init` helper is inlined wherever a
block is taken.

Near misses (`samples/render/CacheTextureNearMisses.cpp`):
- `0x0050f9b0` (646 bytes), reserve the leaves a level plan wants: only
  two register choices differ (11 instructions);
- `0x005102d0` (558 bytes), blit the occupant's mip levels into its
  region (DirectDraw `GetAttachedSurface`/`Blt`, then slot 9 or +0x188):
  retail places the shared `return 0` block before the final branch
  (18 instructions).

