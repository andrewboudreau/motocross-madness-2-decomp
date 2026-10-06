# VCR.cpp and VCRfile.cpp: the replay buffers

Two adjacent translation units, both strictly exact under the default VC6
profile (36 calibration cases). Neither class has a vtable or RTTI, so the
class names `UnknownVcr` and `UnknownVcrFile` and all member names are
provisional.

## Evidence

- `D:\aardvark\VC\krusty2\VCR.cpp` (`0x00575848`) is the `__FILE__` of the
  allocations at lines 29, 44, 83, 150, 237 and 261 and the frees at lines
  54, 62, 82, 149 and 258 (`0x0052412e`..`0x00524684`).
- `D:\aardvark\VC\krusty2\VCRfile.cpp` (`0x00575868`) is the `__FILE__` of
  the allocations at lines 33, 81, 240 and 264 and the frees at lines 50 and
  54 (`0x00524a9a`..`0x005252ed`).
- File extents: `0x005240e0` and `0x00524100` before VCR.cpp are
  TrackGameMode methods already bound by other files; `0x00525520` after
  VCRfile.cpp is slot 0 of the Vehicle vtable (`0x00558a84`), and
  Vehicle.cpp's literal follows at `0x00525e98`. VCR.cpp therefore holds
  the functions `0x00524110`..`0x00524a30` and VCRfile.cpp
  `0x00524a50`..`0x005254a0`; the new expression at `0x00418f85` sizes the
  file object at 0x248 bytes.
  The four small VCR helpers at the end (`0x005249a0`..`0x00524a30`) are
  called only from VCR.cpp code and precede VCRfile's constructor.
- Construction: recorder.cpp code calls `operator new(0x101c)` and
  `0x00524110` (`0x004e7ab5`..`0x004e7ad9`) and destroys with `0x005241b0`
  then `0x004a30c0` (`0x004e79b0`). bikerace.cpp code constructs the file
  object with `(1, 2)` into the view's `+0x1a8` (`0x00418fab`), loads it with
  `0x00524b80` (`0x00418fda`) and destroys it with `0x00524b10` before
  freeing (`0x0041d020`). EventManager saves it with `0x00524d00`.
  RaceView.h now includes VCRfile.h for that `+0x1a8` member instead of its
  previous one-method stub.
- CRT calls are bound by address: `strncpy`, `fopen`, `fread`, `fwrite`,
  `fclose`, `fseek` (`0x00534eac`) and `ftell` (`0x005365d6`: lock,
  `_ftell_lk`, unlock). `strlen`, `strcmp` and `memcpy` are VC6 intrinsics;
  "rb" (`0x0056666c`) and "wb" (`0x005682b4`) are pooled literals.

## UnknownVcr (VCR.cpp, 0x101c bytes)

A ring of 256 16-byte slots (`+0x0000`: data, capacity, a value, an in-use
byte and a state byte 0..3), three ring cursors (`+0x1000`, `+0x1004`,
`+0x1008`) advanced by `0x005249a0`, `0x005249d0` and `0x00524a00`, a chain
of 0x804-byte blocks of 256 `(value, extra)` pairs (`+0x100c`) with a pair
index (`+0x1010`) and block number (`+0x1014`), and one plain value
(`+0x1018`).

| VA | Size | Role | Source shape |
|---|---:|---|---|
| `0x00524110` | 154 | constructor | local for the allocation, size stored first |
| `0x005241b0` | 97 | destructor | `if (field_0x100c)` before loading the chain |
| `0x00524220` | 192 | claims the next write slot with a size header | `needed > size \|\| !size`, unsigned |
| `0x005242e0` | 20 | sets a slot to state 1 | |
| `0x00524300` | 79 | hands out the next read slot | |
| `0x00524350` | 50 | whether the next read slot is in state 1 | `!=` test first |
| `0x00524390` | 173 | claims the next write slot | |
| `0x00524440` | 27 | stores a slot's value and sets state 1 | |
| `0x00524460` | 215 | hands out the next playback slot | tests repeated in source |
| `0x00524540` | 20 | clears a slot | `d = c = 0` chained |
| `0x00524560` | 42 | whether the slot two ahead is not in state 1 | nested successor calls |
| `0x00524590` | 173 | appends a pair | |
| `0x00524640` | 124 | frees and restarts the pair chain | |
| `0x005246c0` | 251 | steps the pair cursor back | null chain in the `else` |
| `0x005247c0` | 169 | reads the pair at the cursor | three written-out tails |
| `0x00524870` | 114 | advances to a pair with a given value | `do`/`while` |
| `0x005248f0` | 13 | sets `+0x1018` | |
| `0x00524900` | 7 | returns `+0x1018` | |
| `0x00524910` | 43 | resets slot states and cursors | |
| `0x00524940` | 35 | marks the next write slot state 2 | |
| `0x00524970` | 35 | marks the next write slot state 3 | |
| `0x005249a0` | 36 | advances `+0x1000` | |
| `0x005249d0` | 36 | advances `+0x1004` | |
| `0x00524a00` | 36 | advances `+0x1008` | |
| `0x00524a30` | 17 | ring successor | |

Retail details the source keeps:

- The destructor and `0x00524640` free only `field_0x1014` blocks of a chain
  holding `field_0x1014 + 1`.
- `0x005246c0`: with the null-chain store in an `else` before the final
  `return 0`, that store shares the return used by the empty case; an early
  `if (!block)` return gives a separate exit.
- `0x005247c0`: retail has three copies of the read-out tail, one with the
  index folded to 0 after the block change. Writing the tail once makes VC6
  reload the index at the join.

## UnknownVcrFile (VCRfile.cpp, 0x248 bytes)

Up to two handles (`+0x000` count), each either an in-memory file or a
stdio `FILE*` (`+0x240`), chosen by the constructor's first argument
(`+0x23c`). An in-memory handle is a 0x118-byte entry at `+0x004` (a flag
tested and cleared by close but set nowhere in this file, a 0x104-byte
name, an 8-byte mode, the position and the size) plus a chain of 0x19004-byte
blocks (`+0x234`: 0x19000 data bytes and a next pointer).

| VA | Size | Role | Source shape |
|---|---:|---|---|
| `0x00524a50` | 178 | constructor `(memory, count)` | count clamped to 2 |
| `0x00524b10` | 103 | destructor | next read before the first free |
| `0x00524b80` | 371 | loads a file into a free handle | `while (1)`, return inside |
| `0x00524d00` | 199 | saves the first "r" handle | size declared before block |
| `0x00524dd0` | 546 | open | `n > 7 ? 7 : n` clamps |
| `0x00525000` | 111 | close | |
| `0x00525070` | 372 | read | `while (1)` with the end test first |
| `0x005251f0` | 468 | write | memory branch returns; `in` before `total` |
| `0x005253d0` | 107 | seek | memory mode returns the offset |
| `0x00525440` | 93 | tell | |
| `0x005254a0` | 117 | end of file | `size <= position`; `feof` macro |

Retail details the source keeps:

- String copies are `n = strlen(src); length = n > max ? max : n;` then
  `strncpy` and a terminator. `if (length > max)` and `n < max ? n : max`
  allocate or order the clamp differently; strlen inside the ternary is
  evaluated twice.
- The block-skipping loops `while (position >= 0x19000)` compile to the
  retail division by 0x19000 (multiply by `0x51eb851f`) and a counted walk.
- The copy loops in read and write, and the `fread` loop in load, are
  `while (1)` with the exit test inside. As `while (cond)` VC6 rotates the
  loop and also gives the enclosing search loop its own epilogue, unlike
  retail's single shared return.
- Write puts the stdio branch after the memory branch's `return 0`; with
  `else if` that return follows the stdio call. Declaring `in` before
  `total` puts `total` in the `size` argument slot, as retail.

## Not reconstructed

Nothing in either file's range; there are no near misses.

## Reproduce

```bash
python3 tools/compile.py --compiler vc6 --vc6-root "$VC6_ROOT" src/reconstructed/VCR.cpp -o work/vcr.obj
python3 tools/compile.py --compiler vc6 --vc6-root "$VC6_ROOT" src/reconstructed/VCRfile.cpp -o work/vcrfile.obj
```

and compare each symbol with `mcm2tool.resolved_match.match_object` using
the matching `.bindings.json`.
