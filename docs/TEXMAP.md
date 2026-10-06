# Texmap.cpp and TextureCache.cpp

**Texmap.cpp** (`0x0050a3a0..0x0050ac2f`, `src/reconstructed/TextureMap.cpp`)
is now fully matched: 13 functions. The unit opens with its four kVec3
`$E` pairs; Terrain.cpp's last function ends at `0x0050a392`. Its
`__FILE__` literal at `0x00574b0c` has xrefs `0x0050a6bc..0x0050ab10`, and
TextService.cpp starts at `0x0050ac30`. This change adds 9 calibration
cases: the eight `$E` and the 1442-byte texture loader `0x0050a590`. The
loader needs one `goto failed`, so that the header-read and realloc
failures share retail's single `return 0`.

TextureMap.h changes:
- `UnknownTextureFormatChoice` holds a TextureMapManager and two
  ManagedTextureGroups.
- `UnknownTextureStream` gains its text-mode byte and the inline accessors
  `0x0043e9b0`/`0x0043e9e0`.

**TextureCache.cpp** (`0x0050bed0..`, at least `0x0050f69f`; `__FILE__`
`0x00574b84`): every function in the range is already a calibration case
except five near misses. They are the two repacks `0x0050c960` and
`0x0050dad0`, the debug display `0x0050ef70`, and CacheTexture
`0x0050f9b0` and `0x005102d0`. CacheTexture (`0x0050f6a0..0x005104fb`)
belongs here by proximity only.
