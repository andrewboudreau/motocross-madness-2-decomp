# Track

The literal `__FILE__` `D:\aardvark\VC\krusty2\Track.cpp` (`0x0057508c`) is
referenced by `0x00515e70`, `0x00515ed0`, `0x00516800`, `0x00516870`,
`0x00516980`, `0x00517340`, `0x00517930` and `0x005179f0`. No RTTI names a
class, so `Track` is a name inferred from the file only. Canonical source is
`src/reconstructed/Track.h` / `Track.cpp`; every name is provisional.

The Track.cpp range runs from `0x00515dc0` to `0x00518716`. The tyre code
before it ends at `0x00515c90`. TrackOverlay's classes (InstrumentOverlay
and others) start at `0x00518720`, with their own `__FILE__` at `0x00575290`.

## Ownership evidence

| Evidence | Functions |
|---|---|
| `__FILE__` reference (confirmed) | `0x00515e70`, `0x00515ed0`, `0x00516800`, `0x00516870`, `0x00516980`, `0x00517340`, `0x00517930`, `0x005179f0` |
| String pool between the two `__FILE__`s (strong inference) | `0x00518640` (`"%2.2d:%2.2d"`), `0x00518690` (`"--:--.--"`, `"%2d:%05.2f"`) |
| Position only (provisional) | the tokenizer `0x00515dc0`/`0x00515df0`, `0x00516ca0`, `0x00516ef0`, `0x00517310`, `0x005179a0`, `0x00517da0`, `0x00517ea0`, `0x00518080`, `0x00518130`, `0x00518230` |

The tokenizer is called from gameui.cpp, Parameterblocks.cpp and others. It
sits directly before `0x00515e70`, which is the only reason to place it here.

## Data

The structure sizes come from the `DebugCalloc` sizes at the allocation
sites:
- **Node**, 0x18 bytes (line 108). It holds the flags (bit 0: read from the
  file; bit 2: visited by a walk), a length (+0x04), the first and last
  segment (+0x08, +0x0c) and its links (count +0x10, array +0x14).
- **Segment**, 0x30 bytes (line 141). It holds a point (+0x00..+0x08), the
  horizontal distance to the next point (+0x24), and the previous and next
  segment (+0x28, +0x2c).
- **Work-list entry**, 0x10 bytes. Only +0x04 (node) and +0x0c (next) are used.

A track position (node, segment, t) is passed by value. The loader
`0x00515ed0` stores the start node at Track+0x00 and the length from start
to finish at Track+0x04.

## Status

The following 12 functions are exact under `vc6_o2_mt`, with every
relocation bound:

| VA | Bytes | Body |
|---|---:|---|
| `0x00515dc0` | 46 | tokenizer constructor |
| `0x00515df0` | 118 | tokenizer: next token |
| `0x00515e70` | 89 | frees a node array (the loader's failure path) |
| `0x00516800` | 108 | frees a node, its segments and its links |
| `0x00516870` | 262 | frees every node reachable from the start |
| `0x00517310` | 39 | whether a segment belongs to a node |
| `0x00517930` | 99 | frees a work list |
| `0x005179a0` | 67 | whether b is at or after a on the same node |
| `0x00517da0` | 256 | distance along the track from a to b |
| `0x00518080` | 174 | the point at a position |
| `0x00518640` | 67 | formats whole seconds as `mm:ss` |
| `0x00518690` | 134 | formats a lap time, or `--:--.--` when unset |

Near miss: the segment direction `0x00518130` (246/250 bytes,
`samples/track/TrackNearMisses.cpp`). Retail keeps the inverse length on the
x87 stack until a final `fstp st(0)`; VC6 here consumes it in the last
multiply.

Not yet attempted:
- the loader `0x00515ed0` (2342 bytes);
- the closest-position search `0x00516980`;
- the path search `0x005179f0`;
- `0x00516ca0`, `0x00516ef0`, `0x00517340`, `0x00517ea0` and `0x00518230`.

## Source shapes that mattered

- `0x00515df0`: `*p++ = 0; if (*p == 0) p = 0; else while (strchr(...)) p++;`
  gives retail's late `push edi`.
- `0x00516870`: a `while (list)` loop, not do-while.
- `0x00518080`: writing the clamp into the by-value `pos.t`, plus a local
  difference vector.
- `0x00517da0`: a `goto` to the shared path-search call when the segment
  walk runs out, and a second, separate call for different nodes.
- `0x00518130`: the squared length must be `(y*y + x*x) + z*z`.

## Reproduce

```bash
python tools/run_calibration.py --compiler vc6 --profile vc6_o2_mt \
  --vc6-root "$VC6_ROOT" --exe work/game/mcm2.exe --jobs 8
```

The `# Track.cpp` block at the end of `CASES` in `tools/run_calibration.py`
lists the Track cases.
