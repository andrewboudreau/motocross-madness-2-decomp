# Parser.cpp tag/value reader

`src/reconstructed/Parser.h` / `Parser.cpp` reconstruct
`D:\aardvark\VC\krusty2\Parser.cpp`, `0x004b8470..0x004b89f5`. Under
`vc6_o2_mt`, 13 of the 14 functions match exactly with every relocation bound
(13 calibration cases). `0x004b87f0` is a near miss in
`samples/parser/ParserNearMisses.cpp`.

The TU reads `<Tag>value</Tag>` pairs from a text buffer into a singly linked
list. Lookups by tag are case-insensitive. Blank runs and lines starting
with `;` are skipped.

| VA | Size | Role |
|---|---:|---|
| `0x004b8470` | 71 | node constructor: clears, then copies tag and value |
| `0x004b84c0` | 5 | node destructor (jumps to the clear helper) |
| `0x004b84d0` | 27 | node tag match (`lstrcmpiA`) |
| `0x004b84f0` | 17 | clears a node |
| `0x004b8510` | 209 | list append (`new` at lines 73 and 86, `/GX` frame) |
| `0x004b85f0` | 48 | list find |
| `0x004b8620` | 71 | list decimal value (`strtoul`, base 10) |
| `0x004b8670` | 103 | list text value, bounded by the caller's size |
| `0x004b86e0` | 53 | list destructor |
| `0x004b8720` | 143 | skips blanks and `;` comment lines (recursive) |
| `0x004b87b0` | 61 | finds a character |
| `0x004b87f0` | 408 | parses the buffer (**near miss**; body identical, prologue differs) |
| `0x004b8990`, `0x004b89d0` | 49, 37 | parser text and decimal lookups |

## Evidence

- Ownership.
  - Confirmed: `0x004b8510` pushes the `Parser.cpp` literal (`0x0056f864`)
    with lines 73 and 86.
  - Strong inference: the range starts where `Parameterblocks.cpp` ends. It
    ends before `0x004b8a00`, which belongs to a class with vtable
    `0x00555ab8`.
- Layout.
  - Confirmed: a node is `0x404` bytes (operator new at `0x004b8537`): tag
    `+0x000`, value `+0x200`, next `+0x400`.
  - Strong inference: the parser is a pair count at `+0` and the list head
    at `+4`. Callers (`0x0049c35d..0x0049c72f`) build one on the stack and
    destroy the list with `ecx = parser + 4`. Those callers sit near
    `krustyui.cpp` references, but that is proximity only. One key they look
    up is `EventTypeIndex`.
- Calls go through the `lstrcpyA`, `lstrcmpiA`, `lstrlenA`, `lstrcpynA` and
  `lstrcmpA` imports, plus `strtoul` (`0x00537f43`) and the debug allocator
  (`0x004a3010`/`0x004a30c0`). Each is bound to its own address.

## Source details retail needs

- The blank skipper advances its `text` parameter and keeps a `start` copy for
  the "at line start" test. Advancing a local copy instead swaps the
  registers.
- The append stores `new` into the head or tail link and the result local in
  one expression, then returns `node ? S_OK : E_OUTOFMEMORY`.
- `0x004b87f0` needs `while (1)` with a `break` on every failed search.
  - `for (;;)` hoists differently: `lstrcpynA` loads late and `lstrlenA` is
    cached in `ebx`.
  - `while (text)` adds an exit test.
  - Even with `while (1)`, VC6 pushes `ebx` and `ebp` after the `E_FAIL`
    early return, while retail saves all four registers first. No tried form
    changes this; the sample lists them.

## Remaining uncertainty

Names are provisional (tier 3). The `length` argument of `0x004b87f0` is
unused in retail. Its type is inferred from the `ret 8`.

## Reproduce

```bash
python tools/compile.py src/reconstructed/Parser.cpp -o work/parser.obj \
  --vc6-root "$VC6_ROOT" --profile vc6_o2_mt
python tools/match.py --exe work/game/mcm2.exe --target-va 0x004b8510 \
  --target-size 209 --obj work/parser.obj \
  --symbol '?UnknownFunction4b8510@UnknownParserList@@QAEJPBD0@Z' \
  --bindings src/reconstructed/Parser.bindings.json --json
python tools/run_calibration.py --compiler vc6 --profile vc6_o2_mt \
  --vc6-root "$VC6_ROOT" --exe work/game/mcm2.exe --jobs 8
```
