# Parameterblocks.cpp INI reader

`src/reconstructed/Parameterblocks.h` / `Parameterblocks.cpp` reconstruct
`D:\aardvark\VC\krusty2\Parameterblocks.cpp`, `0x004b6f30..0x004b8465`. Under
`vc6_o2_mt`, all 27 functions match exactly with every relocation bound
(27 calibration cases).

The TU reads INI-style text: `[section]` headers, then `key=value` lines or
comma-separated rows. Lines starting with `;` are comments. It reads through
the game's file stream, and archives can redirect that stream.

| VA | Size | Role |
|---|---:|---|
| `0x004b6f30` | 118 | classifies a token: 2 has a letter, 1 has a '.', 0 otherwise |
| `0x004b6fb0` | 130 | parses a token into a typed value (string copy at line 44) |
| `0x004b7040` | 64 | converts a value to int |
| `0x004b7080` | 57 | converts a value to float |
| `0x004b70c0` | 108 | converts a value to text |
| `0x004b7130` | 91 | constructor |
| `0x004b7190` | 136 | destructor |
| `0x004b7220` | 157 | opens a file through the archives |
| `0x004b72c0` | 664 | finds a section by scanning and loads its lines |
| `0x004b7560` | 575 | indexes every section's offset, line count and mode |
| `0x004b77a0` | 130 | opens a stream at an offset, optionally indexing it |
| `0x004b7830` | 84 | returns a section's index, or -1 |
| `0x004b7890` | 83 | frees the loaded lines |
| `0x004b78f0` | 573 | selects a section, using the index when there is one |
| `0x004b7b30` | 437 | looks up the raw value of `key` |
| `0x004b7cf0` | 379 | looks up an int value (accepts T/F/ON/OFF/Y/N) |
| `0x004b7e70` | 74 | looks up a float value |
| `0x004b7ec0`, `0x004b7f10`, `0x004b7f40` | 77, 41, 41 | the same lookups with a default |
| `0x004b7f70` | 155 | starts reading a section's rows |
| `0x004b8010` | 357 | reads the next row into up to 16 values |
| `0x004b8180`, `0x004b81c0`, `0x004b8200` | 56 each | get a row value as int, float or text |
| `0x004b8240` | 280 | lists the section names, each followed by '?' |
| `0x004b8360` | 261 | strips whitespace outside brackets and quotes (includes the jump tables) |

## Evidence

- Ownership. Confirmed: functions in this range push the `Parameterblocks.cpp`
  literal (`0x0056f808`). Strong inference: the range has no `$E`
  initializers. It ends where `Parser.cpp`'s code starts (`0x004b8470`, whose
  literal is `0x0056f864`). It starts after the destructor of the previous
  TU's palette, which ends at `0x004b6f26`.
- The class. Strong inference: it has no vtable and no RTTI.
  - `D3DIMSoultreeMotnctrl.cpp` allocates `0x5c4` bytes for it with its own
    `__FILE__` (`0x004458b5`), then calls the constructor `0x004b7130`.
  - It later frees the object by calling `0x004b7190` and then the plain
    operator delete (`0x00445931`), so `0x004b7190` is the destructor.
  - The layout comes from the constructor's stores and the field uses.
    Samples under `samples/physics/motion/` call the object "SltFile". Its
    first section there is `General info`.
- Callees outside the TU, each bound to its own address:
  - The tokenizer `0x00515dc0`/`0x00515df0`, declared in
    `src/reconstructed/UnknownTokenizer.h` and reconstructed in `Track.cpp`.
  - The stream members `0x00461340`, `0x00461600` and `0x00461aa0`.
  - The archive members `0x004e9360`, `0x004e9030` and `0x004e9430`.
  - The out-of-line copies of three recursive inline stream accessors:
    `0x0043e9e0` (get the mode byte), `0x0043e9b0` (set it) and `0x00461d20`
    (reset it).
  - The stream and archive views are declared under their own names
    (`UnknownParameterStream`, `UnknownParameterArchive`). The reason is that
    `TextureMap.h` lacks the line reader and the inline accessors, and
    `UnknownResourceManager.h` declares `0x004e9030` as `void`, while this TU
    tests its result.
- Allocator line numbers are literal operands:
  - 44, 141, 147, 273, 275, 285, 290, 318, 321, 362, 459, 471 and 802.

## Source details retail needs

- `0x004b6fb0` computes the string size into a local before calling
  `DebugMalloc`. Passing `strlen(text) + 1` as the argument moves the
  `__FILE__` and line pushes ahead of `not ecx` (93%).
- The 1023-character clamps are `PB_MIN(PB_LINE_SIZE - 1, length)`, with the
  length in its own `int` local. VC6 loads 0x3ff first and then branches on
  `jg`. Putting the length first, or clamping with `if`, gives the reverse
  branch. Clamping `strlen` directly evaluates it twice.
- In `0x004b72c0`, `lines` must be cleared before the line count and the
  capacity. In any other order VC6 keeps a zero in `ebx` from the function's
  entry (32% match).
- `0x004b7cf0`, `0x004b7e70` and `0x004b7f70` keep their failure `return 0`
  at the end, after the success block. Early returns invert the branch.
- `0x004b8360` declares `count`, `quoted`, `depth` and then `length`. VC6
  then places `length` in the dead `in` slot, as retail does. The case
  bodies follow the source order `[`, `"`, `]`, whitespace.
- The int and float converters copy the raw 32 bits for both numeric types,
  in two identical case bodies. VC6 does not merge them.

## Register weight from a duplicated call

`0x004b7220`: retail keeps `this` in `ebx` and `path` in `ebp`, and saves
`ebp` only around the not-found block. With the found call
`UnknownFunction4b77a0(entry->field_0x14, entry->field_0x18, index)` written
once after the not-found block, VC6 swaps the two registers (144 of 157
bytes); early returns, if/else, a top-declared entry, a local copy of the
path, `== 0` comparisons, inline helpers for the lookups and a `self` copy of
`this` do not change it. Writing the call out a second time at the end of the
not-found block makes it exact: VC6 cross-jumps the two copies into one block,
but the extra source reference raises `this` above `path` in the callee-saved
register order. In a probe, a field store through `this` (one reference
more than the duplicated call) raised it above `name` into `edi` instead.

## Remaining uncertainty

Every name is provisional (tier 3). Field meanings come from how the TU uses
them. These fields are never read here: `+0x04`, `+0x0c`, `+0x10..0x113`,
`+0x114` and `+0x534`. The stream's `+0x01` and `+0x03` mode bytes are named
only by their accessors.

## Reproduce

```bash
python tools/compile.py src/reconstructed/Parameterblocks.cpp -o work/pb.obj \
  --vc6-root "$VC6_ROOT" --profile vc6_o2_mt
python tools/match.py --exe work/game/mcm2.exe --target-va 0x004b7b30 \
  --target-size 437 --obj work/pb.obj \
  --symbol '?UnknownFunction4b7b30@UnknownParameterBlock@@QAEHPBDPADH@Z' \
  --bindings src/reconstructed/Parameterblocks.bindings.json --json
```
