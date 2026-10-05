# Pixtrans.cpp pixel converters

`src/reconstructed/Pixtrans.h` / `Pixtrans.cpp`: cdecl helpers after
PCVideoCard.cpp's code (literal `0x004d06e9`). PCTextureMap and
ManagedTextureGroup call them. Strides are in pixels; names are
provisional.

Exact (26 calibration cases):

| VA | Size | Role |
|---|---:|---|
| `0x004cde20` | 234 | halves 24-bit pixels (2x2 average) |
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
(24-bit: `0x004cde20`, which indexes the lower row as `top[sourceStride]`) for 1, and otherwise halves through a temporary of
the first level's size (Pixtrans.cpp lines 1329/1350 in the 24-bit one).

Near misses (`samples/render/PixtransNearMisses.cpp`): `0x004cfaf0`, whose
plain-copy path's hoisted strides land in different argument slots;
`0x004d0700` (565 to 8888, keyed; register and slot choice) and
`0x004d07d0` (1555 to 8888; only the pixel pointer's base offset);
`0x004d0aa0`/`0x004d0b90` (24-bit/565 to palette indices; only where the
`palette` argument is loaded).

`0x004d0d40` is hand-written assembly (frame pointer, dead `mov eax, 0`,
`push ebp` inside the loop) and is out of scope. The 24-bit and 16-bit
converters at `0x004d0900`–`0x004d0c40` dither through `0x004cf2a0` when
asked and otherwise map through the 555 tables `0x004de280`/`0x004de290`.

Not reconstructed: the other downsamplers and halvers, the converter
`0x004d1d20` and `0x004d24d0`.
