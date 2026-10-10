# File stream helpers

Source: `src/reconstructed/FileStream.cpp` (the exact methods) and
`samples/io/FileStream.cpp` (the near misses), each with its bindings. The
class is `UnknownTextureStream`, declared in `src/reconstructed/TextureMap.h`.

## Evidence

- The code runs from `0x00460d10` to `0x00461d57`.
  - Before it: the FastMath helpers.
  - After it: the shadow fill code at `0x00461d60` and `0x00461e60`, which
    is inline assembly. `0x00461d60` (to `0x00461e58`, `ret 0xc`) has an
    ebp frame and two floor-division blocks that reload their operands
    from homed stack slots into fixed esi/edi and clear edx with
    `xor edx, edx` (not `cdq`) before `idiv`.
- There is no `__FILE__` literal and no RTTI. The file name and every
  member name are ours (tier 3).
- Callers include Track, SceneManager, Motnctrl, CarProcedural,
  ResourceManager and TGAFile.
- A stream reads either:
  - a file directly, or
  - a slice of an inner stream at `+0x1c`. The slice starts at `+0x130` and
    its length is at `+0x04`.
- Encoded files start with "FAOE". Their bytes go through a running key at
  `+0x01`, seeded from the file name.

## Exact (11), `src/reconstructed/FileStream.cpp`

| VA | Size (bytes) |
|---|---|
| `0x00460d60` | 15 |
| `0x00460d70` | 57 |
| `0x00460e70` | 209 |
| `0x00461310` | 39 |
| `0x00461600` | 63 |
| `0x00461980` | 224 |
| `0x00461a60` | 56 |
| `0x00461aa0` | 229 |
| `0x00461cb0` | 107 |
| `0x00461d20` | 30 |
| `0x00461d40` | 24 |

Source forms that mattered:
- **`0x00460e70`:** the NULL test guards the rest, so its message is
  emitted last.
- **`0x00461980`:** the decode reads the byte into a `char` temporary and
  the key straight from the field: `c = *p; decoded = key ^ c; key += c;
  *p = decoded;`. That is the one spelling under which VC6 loads the key
  first and copies it, as retail does; naming the key in a temporary makes
  it copy the byte instead.
- **`0x00461aa0`:** two forms mattered.
  - `#pragma inline_depth(0)` keeps the end-of-stream test `0x00430ff0` out
    of line.
  - `count < size - 1` replaces a decremented size.
- **`0x00461cb0`:** returns `_fstat`'s result, not void. `TextureMap.h` and
  `SceneManager.bindings.json` now use the `int` signature.

## Near misses (7), `samples/io/FileStream.cpp`

The notes are in the source header.

| VA | Size (bytes) | Masked match | Function |
|---|---|---|---|
| `0x00460d10` | 77 | 22% | constructor |
| `0x00460db0` | 186 | 16% | header check |
| `0x00460f50` | 957 | 30% | open |
| `0x00461340` | 702 | 29% | seek |
| `0x00461640` | 663 | 32% | read |
| `0x004618e0` | 152 | 88% | write |
| `0x00461b90` | 282 | 90% | write the header |

Each miss comes down to one of:
- register assignment;
- instruction scheduling (the 0xfa seed store and the name pointer in
  `0x00461b90`, the argument loads in `0x004618e0`);
- tail merging.

The control flow of each one is reconstructed.

## Reproduce

```bash
PYTHONPATH=. python tools/run_calibration.py --compiler vc6 --vc6-root "$VC6_ROOT"
```
