# TextService.cpp

`src/reconstructed/TextService.h` / `TextService.cpp`. Evidence: the
`__FILE__` literal `D:\aardvark\VC\krusty2\TextService.cpp` (`0x00574b30`;
lines 22 and 113), the inline EArray.h constructor (line 53), and the
callers in Overlay (`0x004b5f0a`, `0x004b6046`) and TrackOverlay. The TU
runs from `0x0050ac30` to `0x0050bec9`; ManagedTextureGroup's constructor
follows at `0x0050bed0`. The class has no RTTI, so it keeps the
`UnknownOverlayText` name TrackOverlay's mangled names use; all names are
provisional.

The object (0x122c bytes) draws bitmap-font text: the selected
FontTexture (+0), an `EArray<FontTexture*>` (+4), a vertex depth
(+0xc, 0.001f), a clip `Rectangle2D` (+0x10) and 24 × 6 pre-transformed
vertices (+0x2c). `0x0050ae80` creates it from ".cell"/".tga" font files;
`0x0050ba00` lays text out into quads and `0x0050bd30` draws them through
RenderTarget; `0x0050b080`, `0x0050b400` and `0x0050b700` blit text into
32-bit, 4444 and 1555 textures. `Rectangle2D.h` now declares Point2D
(RTTI, vtable `0x00556f78`) and Rectangle2D's two points and
`operator=` (`0x004e8c20`).

Exact: 6 calibration cases (constructor, destructor, set font, factory,
draw, and the out-of-line `CharacterCell::UnknownFunction50beb0` checked
from the sample). Near misses
(`samples/render/TextServiceNearMisses.cpp`, notes there): select font
`0x0050ade0` (retail keeps the loop test at the top), the layout
`0x0050ba00` (store scheduling) and the three blitters (register and
stack-slot assignment).
