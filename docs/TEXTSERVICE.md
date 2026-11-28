# TextService.cpp

`src/reconstructed/TextService.h` / `TextService.cpp`. Evidence: the
`__FILE__` literal `D:\aardvark\VC\krusty2\TextService.cpp` (`0x00574b30`;
lines 22 and 113), the inline EArray.h constructor (line 53), and the
callers in Overlay (`0x004b5f0a`, `0x004b6046`) and TrackOverlay. The TU
runs from `0x0050ac30` to `0x0050bec9`; ManagedTextureGroup's constructor
follows at `0x0050bed0`. The class has no RTTI; it is named after the
unit, whose constructor pushes the literal and which holds every method
(tier 2, docs/NAMING.md wave 7). Member names are provisional.

The object (0x122c bytes) draws bitmap-font text: the selected
FontTexture (+0), an `EArray<FontTexture*>` (+4), a vertex depth
(+0xc, 0.001f), a clip `Rectangle2D` (+0x10) and 24 × 6 pre-transformed
vertices (+0x2c). `0x0050ae80` creates it from ".cell"/".tga" font files;
`0x0050ba00` lays text out into quads and `0x0050bd30` draws them through
RenderTarget; `0x0050b080`, `0x0050b400` and `0x0050b700` blit text into
32-bit, 4444 and 1555 textures. `Rectangle2D.h` now declares Point2D
(RTTI, vtable `0x00556f78`) and Rectangle2D's two points and
`operator=` (`0x004e8c20`).

Exact: 8 calibration cases (constructor, destructor, select font, set
font, factory, layout, draw, and the out-of-line `CharacterCell::GetRect`,
which the blitters' calls make the unit emit). Select font `0x0050ade0` closed from 117/125 when
`EArray::operator[]` named its offset (`unsigned int offset = index -
m_base;`): retail forms the offset in eax with the array in ecx, which the
direct `m_data[index - m_base]` does not; no other EArray.h includer
changes. Near misses, in TextService.cpp in retail order with `// Not exact`
status comments: the three blitters (register and stack-slot assignment); see
[Near misses: forms tried](#near-misses-forms-tried). The game build linked
the same bodies from `src/bringup/Group2_OverlayText.cpp`, now removed.

## Function names

Names given from each function's behaviour (string literals, D3D/DirectDraw
method slots and arguments, callers); the address-derived names they
replace are in Git history. Tier 3 unless the entry says otherwise.

- `0x0050ade0` `SelectFont`
- `0x0050ae60` `SetFont`
- `0x0050ae80` `Create`
- `0x0050bd30` `DrawVertices`
- `0x0050b080` `DrawTextInto` (dispatches to the 16-bit versions), `0x0050b400`
  `DrawText4444`, `0x0050b700` `DrawText1555` (near-miss sample)
- `0x0050ba00` `TextService::LayoutText`

## Near misses: forms tried

Carried over from the header of the former `samples/render` near-miss file:

DrawText4444 (0x0050b400, 758 bytes; 4444 texels),
DrawText1555 (0x0050b700, 758 bytes; 1555 texels) and the 32-bit
body of DrawTextInto (0x0050b080, 890 bytes; its format dispatch
matches): same instructions, different register/stack assignment.
fontStride declared after `end` gives retail's ebp for it (stride at
[esp+0x34]); differing instructions 54 -> 30 (DrawTextInto) and 55 -> 31
(the 16-bit pair). Left: retail keeps glyphWidth in esi and srcY in the
slot later reused for the row count, VC6 here the reverse (glyphWidth
declared before srcY or srcX is worse: 39, 51), and the row offsets are formed in other registers.
Pointer-walking rows, fully indexed rows, top-of-function declarations,
signed coordinates and inline pitch expressions are all further off.

LayoutText (0x0050ba00, 802 bytes) is exact: the glyph cell's extents are
read through inline helpers (`RectWidth(cell) * 255.0f`,
`RectHeight(cell) * 255.0f`), which moves the two slipped stores to retail's
places. A k-store ruler first showed that retail's block behaves as if it
held one more IL tuple before the vertex stores (one dead store before the
vertex-2 block fixed the v5.sx slip); the inlined accessor supplies it
without code. The helpers declared but unused, or used only for the layout
rectangle's `right`/`bottom`, give 770. As `Rectangle2D::Width()`/`Height()`
members the function is exact too, but the two member declarations in
Rectangle2D.h move BikeRace::CreateRacers (0x00419970, 13400 -> 13394), so
they are file-local in TextService.cpp. The blitters use them as well
(unchanged scores). Forms tried before, at 770:
only the scheduling of two stores (v3.sx and v5.sx) differed. Writing through a
vertex pointer, other vertex orders, field orders and an inline vertex
setter are further off (all 720 orders of the six vertex blocks swept
with tools/sweep_variants.py: 652..718 against 770 for the kept order).
Both are one-instruction scheduling slips: retail loads x for v3.sx
before the v1.tv store and copies right (`fld st(1)`) for v5.sx before
the v3.tu store. Moving v1.tv, v3.sx or v5.sx alone to each position in
its block (512 forms) keeps 4 differing instructions at best; v3.sx
third gives 781 positions but 6 differing instructions.
Session 19: struct copies of vertex 0 into 3 (94), 1 into 5 (71) or both (87)
are far off.

CharacterCell::GetRect (0x0050beb0, the out-of-line copy of
FontTexture.h's inline) is emitted only in a TU that calls it, so it is
compiled (and matches exactly) in this file.
k-scan (s22): compiled behind 0..63 prepended typedefs, 0x0050b080,
0x0050b400, 0x0050b700, 0x0050ba00 have the same code at every count, so
their differences are not a declaration-count effect
(docs/VC6_OPERAND_ORDER.md section 2).
