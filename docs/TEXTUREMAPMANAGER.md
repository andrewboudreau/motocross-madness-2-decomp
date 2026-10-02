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
- +0x3c / +0x40: the texture cache selected by the debug keys and its index;
- +0x44: `ContainerList<UnknownTextureCache*>` (ContainerList.h), initialised
  to 4 entries growing by 4;
- +0x58: debug key mode; +0x5c / +0x60 two grow-only scratch buffers with
  sizes at +0x64 / +0x68 (used by PCTextureMap slot 5);
- +0x70: a BlockAllocator.cpp pool (`0x00423f70`, 0x30-byte blocks × 0x1800);
- +0x74: frame count (slot 10).

The constructor also clears the shared texture surfaces (`0x0068a36c`,
`0x0068a394`, 10 each) that PCTextureMap slot 4 reuses; the destructor
releases them.

## Status

Exact (23 calibration cases):
- the list: constructor, destructor (unlinks every texture), First, Last,
  Next, Previous, Append, Remove and AppendList;
- the constructor `0x00510bd0`, scalar deleting destructor `0x00510cd0` and
  destructor `0x00510cf0`;
- slots 10 (counts frames), 12, 13 (skipped on AGP displays) and 15 (with
  the "TestKey" debug bit), which forward to every texture cache before the
  GameObject base; slot 18 (`0x00511290`), which calls slot 12 of every
  texture, then the caches;
- slot 23 (`0x00510ee0`), the "TestKey" debug keys: 0x14 cycles a mode,
  0x21 toggles the selected cache's +0x08, 0x1b / 0x1a step forwards /
  backwards through the cache's +0x1c8 or +0x1c4 entries or through the
  caches, 0x15 runs the cache's `0x0050c7e0`;
- `0x00511180`, which creates a 0x258-byte texture cache (TextureCache.cpp
  `0x0050bed0`) and adds it through ContainerList's inline Add;
- `0x005112f0` / `0x00511300` (register / unregister a texture) and the
  scratch buffers `0x00511310` / `0x00511370`.

Not reconstructed: `0x005113d0` (shares texture memory out among the caches)
and `0x00511580` (formats texture memory statistics into local buffers).
