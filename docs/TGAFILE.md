# Tgafile.cpp format helpers

`src/reconstructed/Tgafile.h` / `Tgafile.cpp`: the cdecl helpers before
Tgafile.cpp's literals (`0x00511b54`). PCTextureMap, TextureMap and PCGame
call them. Names are provisional. File formats are the texture loader's
codes 1..34; pixel formats are the engine's 8 (palettised), 0x22b (555),
0x235 (565), 0x378 (24-bit), 0x613 (1555), 0x115c (4444) and 0x22b8 (8888).

Exact (18 calibration cases):

| VA | Size | Role | Source shape |
|---|---:|---|---|
| `0x00511740` | 180 | bytes per stored pixel of a file format | grouped switch |
| `0x00511800` | 79 | whether a file format is compressed | all of 0..34 listed |
| `0x00511850` | 79 | whether a file format stores mip levels | all of 0..34 listed |
| `0x005118a0` | 200 | the pixel format a file format decodes to | grouped switch |
| `0x00511970` | 78 | bytes per pixel of a pixel format | one case per format |
| `0x005119c0` | 266 | fills a DirectDraw pixel format | switch with fallthroughs |
| `0x00511ad0` | 30 | whether a pixel format is 4444 or 8888 | inverted first test |
| `0x00511af0` | 75 | the pixel format of a DirectDraw pixel format | uninitialised local |
| `0x00511b40` | 446 | reads a header into a new or given file | chained reads |
| `0x00511d00` | 207 | opens a path in a new stream and reads the header | `new` under /GX |
| `0x00511dd0` | 174 | reads a header and the pixels for its depth | per-depth `goto failed` |
| `0x00512720` | 113 | writes 24-bit pixels to a TGA file | header on the stack |
| `0x005127a0` | 67 | fills a 24-bit TGA header | field stores |
| `0x005127f0` | 113 | writes 32-bit pixels to a TGA file | header on the stack |
| `0x00512870` | 67 | fills a 32-bit TGA header | field stores |
| `0x005128c0` | 119 | writes 16-bit pixels to a TGA file | header on the stack |
| `0x00512940` | 67 | fills a 16-bit TGA header | field stores |
| `0x00512dd0` | 85 | frees a loaded file | debug deletes |

Retail details the source keeps:

- `0x00511800`/`0x00511850` index a byte table that starts at format 0.
  VC6 drops cases that only reach the default, so every format 0..34 is
  listed; the 0 group then merges with the default return.
- `0x00511970`'s 16-bit formats return separately. VC6 merges the four
  identical blocks into the one retail places first; a grouped label puts it
  last.
- `0x005119c0` reports 8888 with a 24-bit count (it falls into the 24-bit
  case after setting the alpha mask) and 1555 through the 555 masks.
- `0x00511af0` returns its argument's stack slot for 16-bit masks other than
  555 and 565: `format` is left unset there, and VC6 keeps it in the dead
  parameter slot as retail does. Formats other than 16, 24 and 32 bits
  become 8.

The writers build an `UnknownTgaFile` on the stack (the TGA header fields
unpacked, the pixels at +0x14 and the name at +0x1c), copy the path in and
return the result of `0x00512990`. The value PCTextureMap's level dump
passes as 32 is the header's descriptor byte (top-left origin).

Near miss (`samples/render/TgafileNearMisses.cpp`): the writer
`0x00512990` (1074 bytes). It opens the file "wb", writes the header field
by field and the pixels: 24-bit one by one with red and blue swapped,
32-bit swapped in place then in blocks, 16-bit rows as they are for 555
(green mask 0x3e0) or pixel by pixel converted from 565. The 16-bit
failures return without closing the file. The frame and branches line up;
the loop variables land in different dead argument slots.

The stream is TextureMap.h's `UnknownTextureStream` (0x134 bytes; the
constructor `0x00460d10`, destructor `0x00460d60` and `0x00460f50`, which
opens a path, sit with its read helpers). src/krusty2's collision code
calls the same `0x00460f50` as `CollisionFileStream::Open`.

The header reader allocates the file (line 354) when given none, seeks to
the stream's +0x130 start (when it wraps an inner stream at +0x1c) or past
a positive offset, and accepts 16, 24 and 32-bit files of image type 2
(raw) or 10 (run-length). On failure it frees the file (lines 421/422),
even one the caller passed in.

Not reconstructed: the pixel readers `0x00511e80`, `0x00512100` and
`0x00512370`, and `0x005125c0`.
