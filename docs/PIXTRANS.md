# Pixtrans.cpp pixel converters

`src/reconstructed/Pixtrans.h` / `Pixtrans.cpp`: cdecl helpers after
PCVideoCard.cpp's code (literal `0x004d06e9`). PCTextureMap and
ManagedTextureGroup call them. Strides are in pixels; names are
provisional.

Exact (4 calibration cases):

| VA | Size | Role |
|---|---:|---|
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

Not reconstructed: the converters `0x004d1030`–`0x004d18c0`, the
downsampler `0x004d1b90`, the converter `0x004d1d20` and `0x004d24d0`.
