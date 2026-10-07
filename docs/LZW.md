# Lzw.cpp LZW decoder

`src/reconstructed/Lzw.h` / `Lzw.cpp`: all of `D:\aardvark\VC\krusty2\Lzw.cpp`
that retail links, `0x004a01d0..0x004a05da`. Every function is exact under
`vc6_o2_mt` with every relocation bound (11 calibration cases).

| VA | Size | Role | Source shape |
|---|---:|---|---|
| `0x004a01d0` | 5 | `$E2` stub | jump to `$E1` |
| `0x004a01e0` | 60 | `$E1`: per-TU (0,0,0) at `0x0067c4f8` | copy-initialised `static const` |
| `0x004a0220` | 5 | `$E5` stub | jump to `$E4` |
| `0x004a0230` | 60 | `$E4`: per-TU (1,0,0) at `0x0067c508` | copy-initialised `static const` |
| `0x004a0270` | 5 | `$E8` stub | jump to `$E7` |
| `0x004a0280` | 60 | `$E7`: per-TU (0,1,0) at `0x0067c518` | copy-initialised `static const` |
| `0x004a02c0` | 5 | `$E11` stub | jump to `$E10` |
| `0x004a02d0` | 60 | `$E10`: per-TU (0,0,1) at `0x0067c4e8` | copy-initialised `static const` |
| `0x004a0310` | 71 | resets the dictionary | unsigned index, banked macro |
| `0x004a0360` | 106 | allocates missing dictionary banks | `DebugCalloc` lines 96, frees line 104 |
| `0x004a03d0` | 522 | decodes a stream into a buffer | local `GR_BitString` under `/GX` |

## Evidence

- Ownership. Confirmed: `0x004a0360` is the only function that references
  the `Lzw.cpp` literal (`0x0056de34`, at `0x004a0371` and `0x004a03ac`).
  Strong inference: `0x004a0310` and `0x004a03d0` are the only other code
  that touches its `.bss` (`0x0067c524`..`0x00685024`). The four `$E` pairs
  write `.bss` immediately before it (`0x0067c4e8`..`0x0067c520`). They have
  the same shape as the initializers that open about 73 other TUs (see
  `src/krusty2/math/Math3D.h`). `0x004a0190` (a `thiscall`
  `ret 4`) belongs to the previous TU. `0x004a05e0` (an imported call into a
  128-byte buffer, then a string compare) belongs to the next one.
- Behaviour. Confirmed from literal operands: a 35023-entry (`0x88cf`)
  dictionary in 137 banks of 256 twelve-byte entries. Codes 256, 257 and 258
  mean end of stream, one more code bit and dictionary flush. New codes start
  at 259 with 9-bit codes, and the next-bump value is 511. This is the
  variable-width LZW15V layout. The decoder reads `parentCode` (+4) and
  `character` (+8). The reset marks `codeValue` (+0) unused, but no encoder
  is linked: nothing else references these globals.
- The bit reader is `GR_BitString` (RTTI `.?AVGR_BitString@@`, COL
  `0x0055ae08`, vtable `0x00550f50`, base `GR_PixelString`). The decoder
  builds one on the stack with the constructor `0x00423d50(buffer, size * 8,
  0)`. It reads with `0x00423ef0(bits)` and destroys it with `0x00423dd0`,
  which slot 4's deleting destructor `0x00423d30` also calls. The first 32
  bits are read and dropped.
- Callers (8 sites, for example `0x0047dc9f`, `0x004c63d1` and
  `0x004de04a`) pass `(destination, source, size)`. `0x004c63d1` passes
  Tgafile.cpp's bytes per pixel × width × height, so `size` is the
  decoded size.

## Source details retail needs

- `LzwDecodeString` is an `inline` function. Retail expands it at both of
  its call sites, and `/Ob1` would not inline an unmarked function.
- The decoder copies `output` into a local cursor. VC6 then keeps the cursor
  in `ebp` and spills `character` into the dead `output` argument slot, as
  retail does. Writing through `output` itself spills the cursor and keeps
  `oldCode` in the `size` slot (10% match).
- The dictionary allocator tests each bank before allocating, so a second
  call only fills the gaps. The cleanup loop frees banks `0..i-1` with
  `DebugFree(p, __FILE__, 104)` and clears them.

## Remaining uncertainty

Function, variable and field names are provisional (tier 3). The behaviour
is tier 1. The `GR_BitString` declaration only covers what this TU uses;
its other virtuals, its field meanings and its own TU are unexamined. The
original header that supplies the per-TU vectors is not identified; a
TU-local stand-in type reproduces them.

## Reproduce

```bash
python tools/compile.py src/reconstructed/Lzw.cpp -o work/lzw.obj \
  --vc6-root "$VC6_ROOT" --profile vc6_o2_mt
python tools/match.py --exe work/game/mcm2.exe --target-va 0x004a03d0 \
  --target-size 522 --obj work/lzw.obj \
  --symbol '?UnknownFunction4a03d0@@YAXPAEPBXH@Z' \
  --bindings src/reconstructed/Lzw.bindings.json --json
python tools/run_calibration.py --compiler vc6 --profile vc6_o2_mt \
  --vc6-root "$VC6_ROOT" --exe work/game/mcm2.exe --jobs 8
```
