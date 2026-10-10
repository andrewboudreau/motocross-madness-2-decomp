# Pixtrans.cpp pixel converters

`src/reconstructed/Pixtrans.h` / `Pixtrans.cpp`: cdecl helpers after
PCVideoCard.cpp's code (literal `0x004d06e9`). PCTextureMap and
ManagedTextureGroup call them. Strides are in pixels; names are
provisional.

Extent: `0x004cde20..0x004d28af` (strong inference; no `$E`). The code
before it is window code called from PCGame, with its own bss.

Exact: 36 calibration cases, plus eight more in `Pixtrans.cpp` (strict
exact, not yet calibration cases): the seven per-format downsamplers
`0x004cfaf0` (24-bit), `0x004cfc40` (8888), `0x004d0020` (4444),
`0x004d0170` (1555), `0x004d02c0` (565), `0x004d0440` (555) and
`0x004d05c0` (palette), and the keyed 24-bit to 8888 converter
`0x004cfda0`. Calibration cases include:
- The three ditherer row readers `0x004cf980`, `0x004cf9c0` and
  `0x004cfa10`.
- 565 to 24-bit `0x004cfa60`.
- The keyed 24-bit downsampler `0x004cfe70`.
- The 24-bit, 565 and 555 to palette-index converters `0x004d0aa0`,
  `0x004d0b90` and `0x004d0c40`. These call the palette's thiscall table
  getters `0x004de280`/`0x004de290`, which TextureMap.h declares.
- The format-pair dispatcher `0x004d1d20`, which returns int. Its new key is
  in the PCTextureMap and ManagedTextureGroup bindings.

Earlier cases:

| VA | Size | Role |
|---|---:|---|
| `0x004cde20` | 234 | halves 24-bit pixels (2x2 average) |
| `0x004ce190` | 641 | halves 4444 pixels (rounded 2x2 average per channel) |
| `0x004d0870` | 134 | 4444 to 8888 |
| `0x004d0900` | 204 | 24-bit to 565, dithered through `0x004cf2a0` when asked |
| `0x004d09d0` | 204 | 24-bit to 555, dithered through `0x004cf2a0` when asked |
| `0x004d0e40` | 200 | 555 to 8888; the key becomes opaque magenta |
| `0x004d0f10` | 156 | 555 to 24-bit; 555 magenta (0x7c1f) stays magenta |
| `0x004d0fb0` | 114 | 555 to 565 |
| `0x004d1030` | 114 | 565 to 555 |
| `0x004d10b0` | 150 | palette indices to 24-bit RGB (+0x10 entries) |
| `0x004d1150` | 109 | palette indices through the 16-bit table at +0x510 |
| `0x004d11c0` | 109 | palette indices through the 16-bit table at +0x310 |
| `0x004d1230` | 152 | 8888 to 4444 |
| `0x004d12d0` | 157 | 8888 to 1555, opaque at or above an alpha threshold |
| `0x004d1370` | 208 | 24-bit to 1555; the 0xRRGGBB key becomes 0 |
| `0x004d1440` | 227 | 4444 to 555; alpha below the threshold becomes a colour |
| `0x004d1530` | 187 | 4444 to 1555; alpha below the threshold becomes 0 |
| `0x004d15f0` | 228 | 4444 to 565; alpha below the threshold becomes a colour |
| `0x004d16e0` | 151 | 555 to 1555, transparent where the pixel is the key |
| `0x004d1780` | 142 | 1555 to 555; transparent pixels become the key |
| `0x004d1810` | 170 | 565 to 1555, transparent where the pixel is the key |
| `0x004d18c0` | 172 | 1555 to 565; transparent pixels become the key |
| `0x004d1b90` | 389 | dispatches downsampling by pixel format |
| `0x004d1970` | 161 | replaces 32-bit `from` pixels with `to` |
| `0x004d1a20` | 147 | replaces 24-bit `from` pixels with `to` |
| `0x004d1ac0` | 115 | replaces 16-bit `from` pixels with `to` |
| `0x004d1b40` | 74 | replaces 8-bit `from` indices with `to` |

The replacers take `from` and `to` by value (4- and 3-byte pixel structs for
32 and 24 bits) and return 1. Except in 8-bit, a pixel already equal to
`to` is first nudged off it so the colour key stays unique: the blue byte
moves up (255 moves down), a 16-bit pixel moves one step (down when its low
five bits are set). The 16-bit nudge is an if/else; a ternary makes VC6 mask
`to ± 1` to 16 bits.

The converters take `(destination, source, width, height,
destinationStride, sourceStride[, extra])`, loop rows then pixels and
return 1. What reproduces retail:

- separate row and pixel pointers, with the source row declared,
  initialised and advanced first (0x004d1230 and the 16-bit-only converters
  vary the order per function; see the source);
- channel packing written as `(c >> 4) << 12 | ...` and `(c >> 3) << 10 |
  ...`, which VC6 turns into retail's masks and factored shifts;
- `*to++` in each branch where retail keeps the two pointers apart;
- the alpha threshold reassigned in place as `(t >> 4) << 12` (it lives in
  its argument slot), and the 24-bit key's red/green bytes as locals
  declared just before the row loop;
- keys from `Pack555`/`Pack565`, now shared through `Pixtrans.h`.

0x004d1780 copies nothing for opaque pixels: it only clears the alpha bit
of what is already in `destination` (retail behaviour).

The palette's +0x310 and +0x510 regions are 256-entry 16-bit tables.

The downsampler `0x004d1b90` hands `levels` (the number of halvings) to a
per-format helper: palette `0x004d05c0` (with the palette), 555
`0x004d0440` and 565 `0x004d02c0` (with a filter flag), 24-bit
`0x004cfaf0`, 1555 `0x004d0170`, 4444 `0x004d0020`, 8888 `0x004cfc40`.
Each copies rows for 0 levels, halves once through a per-format halver
(24-bit: `0x004cde20`, which indexes the lower row as
`top[sourceStride]`) for 1, and otherwise halves through a temporary of the
first level's size (Pixtrans.cpp lines 1329/1350 in the 24-bit one).

The downsamplers' plain-copy path and `0x004cfda0` address each row from
the row index (`(UnknownPixel32*)source + y * sourceStride`); VC6 then
strength-reduces the row offsets and keeps the hoisted steps in the dead
argument slots exactly as retail does. Walking row pointers advanced by the
stride put the steps in other slots. The downsamplers' halvers are declared
in `Pixtrans.h` and bound. 4444 `0x004ce190` is exact. Bit 0 of its last
argument (always 0) selects a packed path. That path loads the four pixels as
`unsigned int` in one multi-declarator declaration; separate declarations
reorder the loads. Like retail, it counts pixels with `height` through an
unsigned `!=` loop. 555 `0x004ce5f0` and 565 `0x004cea10` take a filter flag,
then the magenta key 0x7c1f/0xf81f. They are out of scope because their packed
path is hand-written assembly: an ebp frame, a memory accumulator, and a
balanced add/sub of the source pointer. 1555 `0x004ce420` and palette
`0x004cee30` are near misses. The `DebugMalloc`/`delete` line numbers of the downsamplers
run from 1329 (24-bit) to 1840 (palette).

`0x004d0700` (565 to 8888, keyed) and `0x004d07d0` (1555 to 8888) are
exact. `0x004d0700` takes the row pointers from the arguments before
`Pack565(key)` (VC6 hands the dead argument slots to the locals in the
order of the arguments' first use, so the y counter lands in the key's
slot as in retail) and stores alpha 0xff in both arms; `0x004d07d0` writes
alpha with an if/else (a `?:` changes the base offset of the
strength-reduced pixel pointer).

The average colour `0x004d24d0` is exact (identical explicit returns in the
palette and 24-bit cases, `default: return 0;`). Near misses
(`samples/render/PixtransNearMisses.cpp`): `0x004ce420` (the
1555 halver; retail keeps the two lower-row pixels in frame slots); `0x004cdf10`, the 8888 halver, which
averages colour over the 2x2 pixels with alpha set (the fourth pixel adds
the third one's alpha in retail) and differs in register assignment;
`0x004cee30` (the palette halver, 45/1133: VC6 pushes ebp in the prologue
and homes the source row in `source`'s slot, which shifts every slot).

`0x004d0d40` is inline assembly (frame pointer, dead `mov eax, 0`,
`push ebp` inside the loop, `ebp` used as the loop counter) and is out of scope. The palette-index
converters `0x004d0aa0`/`0x004d0b90`/`0x004d0c40` (24-bit, 565, 555) dither
through `0x004cf2a0` when asked and otherwise map through the 555-to-index
and 565-to-index tables `0x004de280`/`0x004de290`.

Not reconstructed: the halvers `0x004ce5f0` and `0x004cea10`, inline assembly
by strong inference (the only `push ebp; mov ebp, esp` frames among the unit's
halvers; their pixel loops accumulate into a zeroed argument slot in memory,
`mov [ebp+0x1c], 0; add [ebp+0x1c], ebx; ...`, and count down `dec [ebp+0x24]`,
which VC6 `/O2` does not emit for C locals). The ditherer `0x004cf2a0`
(Floyd-Steinberg over two 16.16 error rows, 0x00689a6c..0x00689a74) is a
draft near miss in `samples/render/PixtransNearMisses.cpp` (242 of 1735
compared bytes: flow and arithmetic match, the format, pixel size and
palette registers are permuted and the frame is 4 bytes smaller). The table getters
`0x004de280`/`0x004de290` are exact in `src/reconstructed/Quantize.cpp`.
`0x004cf162` and `0x004d0000` are not function
starts: the first is mid-instruction inside the palette halver
`0x004cee30` (which runs to `0x004cf29c`), the second inside `0x004cfe70`.

## Function names

Names given from each function's behaviour (string literals, D3D/DirectDraw
method slots and arguments, callers); the address-derived names they
replace are in Git history. Tier 3 unless the entry says otherwise.

- `0x004cde20` `Halve24`
- `0x004cdf10` `Halve8888`
- `0x004ce190` `Halve4444`
- `0x004ce420` `Halve1555`
- `0x004ce5f0` `Halve555` (inline assembly)
- `0x004cea10` `Halve565` (inline assembly)
- `0x004cee30` `Halve8`
- `0x004cf2a0` `DitherConvert`
- `0x004cf980` `ReadRow24`
- `0x004cf9c0` `ReadRow565`
- `0x004cfa10` `ReadRow555`
- `0x004cfa60` `Convert565To24`
- `0x004cfaf0` `Downsample24`
- `0x004cfc40` `Downsample8888`
- `0x004cfda0` `Convert24To8888`
- `0x004d0020` `Downsample4444`
- `0x004d0170` `Downsample1555`
- `0x004d02c0` `Downsample565`
- `0x004d0440` `Downsample555`
- `0x004d05c0` `Downsample8`
- `0x004d0700` `Convert565To8888`
- `0x004d07d0` `Convert1555To8888`
- `0x004d0870` `Convert4444To8888`
- `0x004d0900` `Convert24To565`
- `0x004d09d0` `Convert24To555`
- `0x004d0aa0` `Convert24To8`
- `0x004d0b90` `Convert565To8`
- `0x004d0c40` `Convert555To8`
- `0x004d0d40` `Convert555To8Fast` (inline assembly)
- `0x004d0e40` `Convert555To8888`
- `0x004d0f10` `Convert555To24`
- `0x004d0fb0` `Convert555To565`
- `0x004d1030` `Convert565To555`
- `0x004d10b0` `Convert8To24`
- `0x004d1230` `Convert8888To4444`
- `0x004d12d0` `Convert8888To1555`
- `0x004d1370` `Convert24To1555`
- `0x004d1440` `Convert4444To555`
- `0x004d1530` `Convert4444To1555`
- `0x004d15f0` `Convert4444To565`
- `0x004d16e0` `Convert555To1555`
- `0x004d1780` `Convert1555To555`
- `0x004d1810` `Convert565To1555`
- `0x004d18c0` `Convert1555To565`
- `0x004d1970` `ReplaceColor32`
- `0x004d1a20` `ReplaceColor24`
- `0x004d1ac0` `ReplaceColor16`
- `0x004d1b40` `ReplaceColor8`
- `0x004ddec0` `Build16BitTables`

Texture formats are written as their decimal bit layouts (8, 555, 565, 888,
1555, 4444, 8888); retail's 0x22b, 0x235, 0x378, 0x613, 0x115c and 0x22b8
are exactly those numbers.
