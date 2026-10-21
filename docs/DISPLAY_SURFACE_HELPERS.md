# Display surface setup and pixel comparison

Two complete, nonexact helpers are extracted from PCVideoCardNearMisses.cpp.
The aggregate sample includes their standalone sources; declarations and bodies
are unchanged. Semantic names remain provisional. The new files organize the
reconstruction and do not assert original translation-unit ownership.

| Standalone candidate | Retail address | Full retail / VC6 bytes | Matching positions |
|---|---|---:|---:|
| `ModeSurfaces.cpp`: `UnknownDisplay::CreateModeSurfaces` | `0x004ca520` | 116 / 124 | 26 |
| `TextureChannelDifference.cpp`: `MeanTextureChannelDifference` | `0x004cb330` | 629 / 620 | 35 |

The channel comparison's old 628-byte range ended inside a jump. Its complete
reachable CFG includes `fstp st(0)` at `0x004cb5a1` and the two-byte jump at
`0x004cb5a3` back to the unlock sequence. Eleven alignment bytes separate it
from the next function at `0x004cb5b0`. The native test proves coverage of every
byte in the corrected extent. No candidate extent is trimmed to match retail.

## Functional build review

Mode-surface creation uses the selected display mode's dimensions, depth and
buffer count. Full-screen setup compares the signed frame-chain cost with the
video-memory budget minus two 16-bit frames, then calls `CreateFlipChain`.
Windowed setup calls `CreateSystemRenderSurface`. Both receive buffer count
minus one, and any nonzero success becomes return value 1. The existing
canonical providers supply both dependencies.

`tests/test_mode_surfaces_behavior.py` compiles the actual standalone source
with VC6 SP3 and compares it against hash-pinned retail in 200,448 scenarios.
It checks full display/mode storage, callback arguments and mutations, signed
arithmetic edges and overflow behavior emitted by this compiler, ordinary
display sizes, exact budget boundaries and their neighbors, both branches and
zero/positive/negative callback results. These tests use valid mode storage;
they do not establish equivalent fault timing for malformed pointers or
partially readable objects. The candidate differs in load scheduling and
register allocation.

The channel helper locks two textures through slots 13 and 14, expands 16-bit
555 pixels when the first format is `0x22b` and 565 otherwise, and computes
the mean absolute RGB difference in units of 1/256. Dimensions and format are
read after both locks. A failed first lock returns -1 without unlocking;
a failed second lock unlocks the second texture, as retail does. Successful
calls unlock first then second. Zero dimensions retain the retail NaN result.
The candidate differs in register allocation and x87 scheduling.

`tests/test_texture_channel_difference_behavior.py` compares 144,944 native
scenarios. These cover every 16-bit first-pixel value with a distinct partner,
both expansions, nonstandard formats, zero dimensions, padded and odd byte
pitches, identical objects, both lock failures, and callbacks that change
dimensions/format or return negative unlock results. Larger 256-, 512- and
1024-square images exercise accumulation and the actual probe's image size.
The test compares result bits and ordered callbacks, complete texture object
storage, and independently verifies that pixels and surrounding guards remain
unchanged. Negative dimensions and invalid image pointers are outside this
finite validation. There are no external call dependencies; float constants
and complete function bytes are checked with resolved relocations.

Both fixtures script dependencies and map retail only inside private test
executables. They do not test actual DirectDraw surfaces, rendered images or
the source-built game's startup. Functional admission does not claim strict
byte matching.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_mode_surfaces_behavior.py -v
python -m unittest discover -s tests -p test_texture_channel_difference_behavior.py -v
```
