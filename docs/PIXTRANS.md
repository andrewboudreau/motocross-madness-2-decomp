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
| `0x004d1150` | 109 | palette indices to 565 through `entries565` (+0x510); the dispatch's 8 to 565 case |
| `0x004d11c0` | 109 | palette indices to 555 through `entries555` (+0x310); the dispatch's 8 to 555 case |
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
the row index (`(RgbaPixel*)source + y * sourceStride`); VC6 then
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
palette and 24-bit cases, `default: return 0;`). Near misses, in the unit in retail order with
`// Not exact` status comments (forms tried below): `0x004ce420` (the
1555 halver, 280/458 strict: the lower pair read into frame-slot locals before
the tests and one reused upper `pixel` give retail's x loop; the source row
stays in ebp from entry where retail copies it from eax at each row head); `0x004cdf10`, the 8888 halver, which
averages colour over the 2x2 pixels with alpha set (the fourth pixel adds
the third one's alpha in retail; 182 of 203 instructions aligned with the
block pointer derived from x, only the lower row's induction bias and spill differ);
`0x004cee30` (the palette halver, 1129/1133 strict: written as an if/else with one
`return 1`, ebp is pushed after the palette checks as in retail; only the
post-loop reload of the source stride and width comes in the other order).

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
near miss in the unit (534 of 566
instructions aligned, 276 of 1759 strict: the abs() intrinsic, quantized
levels stored at once and row pointers derived from y give retail's
registers and frame; the ebp push and the y loop's setup order differ). The table getters
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
- `0x004cfe70` `Downsample24To8888`
- `0x004d1d20` `ConvertPixels`
- `0x004d24d0` `AverageColor`
- `0x004d1b40` `ReplaceColor8`
- `0x004ddec0` `Build16BitTables`

Texture formats are written as their decimal bit layouts (8, 555, 565, 888,
1555, 4444, 8888); retail's 0x22b, 0x235, 0x378, 0x613, 0x115c and 0x22b8
are exactly those numbers.

## Near misses: forms tried

Scores are matching bytes under `vc6_o2_mt` with relocations resolved.

Halve8888 (0x004cdf10, 636 bytes with its jump table; candidate 628, 110
strict, 182 of 203 aligned, 0.925 masked): the 8888 halver. Control flow, the
count switch and the retail alpha quirk line up. Retail derives the block
pointer from x (`sourceRow + x * 2` inside the x loop) and reads the lower
pixels as `top[sourceStride]`: separate `top`/`bottom` pointers stepped by 2
in the for increment kept the upper pointer on the stack and most registers
differed (82 of 203 aligned). Retail reads the first alpha twice rather than
through a local; an `alpha0` local cost 0.23 masked. Still differs: VC6
strength-reduces the lower row to `top + sourceStride` (lea edx, [ecx+ebp*4])
where retail biases it to the alpha byte (+3) and spills it; the order of the
green/blue reads moves the upper pointer's bias (+1 green first, +2 blue
first) but not the lower one. Ruled out: accumulator declaration orders,
dropping the alpha1/alpha2 locals (same code), fully indexed `sourceRow[x * 2
+ k]` and `sourceRow[i + k]` forms, `bottom` declared in the loop, a
count-down x loop.

Halve1555 (0x004ce420, 456 bytes; candidate 458, 280 strict): the 1555 halver
behind 0x004d0170. The arithmetic, the >= 2 test and the packing match. Retail
reads the lower pair into locals before the first test and keeps them in frame
slots (0x10 frame), and reuses one `pixel` variable for the upper pair (read
in place, both in dx): declaring the lower pair and the reused pixel after the
accumulators gives that (55 -> 246), and `*to++ =` in both arms instead of
`to++` in the x loop increment gives retail's shared `add eax, 2` tail (280).
Still differs: VC6 here loads `source` into ebp at entry (so ebp is pushed
before the height guard and popped last) and computes the next source row in
ebp, where retail uses eax and copies it to ebp at the row head; the first
opaque pixel's zero-extended value is formed in ecx (retail: in blue's edi).
All 24 accumulator orders, the row/pointer declaration orders, char/int row
pointers, for-init pointers, count-down loops, the first arm's statement
orders (48), pixel types and 1..33 dead locals leave it at 280 or worse;
earlier: every pixel declaration order, a lower-row pointer, a 1555 bitfield
struct and an inline AddPixel1555 helper.

Halve8 (0x004cee30, 1133 bytes; candidate 1133, 1129 of 1133 strict): the palette-index halver. The search for the magenta
entry, both block loops, the key test and the >= 2 rule match, and nested ifs
reproduce retail's single `return 0` block. The keyed and plain loops are an
if/else with one `return 1` after it: with a `return 1` after the keyed loop
VC6 pushes ebp in the prologue, as an if/else it pushes ebp after the three
checks as retail does (docs/VC6_SHRINK_WRAP.md). Declaring the keyed block's
accumulators first in the x loop body gives retail's frame (0x48) and
registers; declared after the four entry copies, VC6 homed `sourceRow` in
`source`'s slot (frame 0x44) and every slot moved. The plain loop ORs the blue
term before the green one as retail does. The four 3-byte entry copies are
declared once at the top of the entries block, shared by both loops: declared
in each x loop body (in any order) they took their slots in the reverse order.
Declaring the accumulators count, red, blue, green orders the two zero stores
as retail's. Still differs: after the keyed x loop retail reloads the source
stride before the width. Updating `row` before `sourceRow`, the row steps or
`to++` in the for increments, declaring `to` before `from`, a `below` row
pointer, counting x or y down and hoisting the loop counters, pointers or
entry indices are the same or worse. So are (40 forms, all 1129) `width > x`,
the x increments swapped, `1 + sourceStride` and five spellings of the source
row step. Session 14 parentheses (all 1129): `(sourceStride * 2)` in the keyed
row step, `(count * 4)` in the three averages, a fully bracketed index
expression, `from[(sourceStride + 1)]`, `(from + sourceStride)[1]` and `*(from
+ sourceStride)`. The two differing loads are the reloads of sourceStride
(eax) and width (edx) at the keyed x loop's exit, both carried into the y
loop's head. Session 15: a `stride` copy of sourceStride for the keyed branch
is propagated back to the argument (1129); an end-pointer x loop (`to < end`)
loses retail's width down-counter (225). Session 26 (all strict now that
`ColorMapper::GetColors` 0x004de270 is bound in Pixtrans.bindings.json, so
the score is 1129 of 1133 strict, not 1121 of 1125 masked): the other
declaration orders of count/red/green/blue (1127-1129), `from += 2` in the
for step or at the end of the body, swapping `from`/`to` or the latch
statements, `count > 1` for `count >= 2` (1129), the four entry copies
declared in the y loop (1089), the four entry indices declared together
(224), the `key == -1` arm written first (152).

DitherConvert (0x004cf2a0, 1759 bytes; candidate 1745, 276 strict, 534 of 566
aligned, 0.954 masked): the Floyd-Steinberg ditherer behind the dithered
converters. Two 16.16 error rows (grown on demand,
0x00689a6c/0x00689a70/0x00689a74), each pixel rounded to the nearest 5-bit
level per channel and, with a palette, to the palette entry when it is exactly
that colour; errors spread 7/16 right, 5/16 below, 3/16 below left and the
remainder below right (retail forms that as e - 7e/16 - 5e/16 - 3e/16 with the
last term negated first, which is the same value). Checked against retail:
free/malloc lines 0x429/0x42c, width * 24 bytes, formats 0x22b/0x235 select
the 555/565 readers, the 555 palette lookup and the output packings. Shape:
retail uses the CRT abs() intrinsic (cdq/xor/sub; a ternary gave jns/neg),
stores each quantized level to its slot at once (the out-parameter
QuantizeChannel), loads the palette entry as colors[index * 3 + k] and derives
the four output row pointers and the next source row from y (`base + y *
stride`), which VC6 strength-reduces into retail's shared induction variables;
stepping separate row pointers at the end of the y loop gave 385 of 566
aligned. Still differs: where ebp is pushed, the order of the y loop's pointer
setup and its rotated loop entry jump.

k-scan (s22): compiled behind 0..63 prepended typedefs, 0x004cdf10,
0x004ce420, 0x004cee30, 0x004cf2a0 have the same code at every count, so their
differences are not a declaration-count effect (docs/VC6_OPERAND_ORDER.md
section 2).
