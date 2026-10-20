# Shared graphics storage

`src/reconstructed/GraphicsState.cpp` supplies twelve globals needed by the
camera, display enumeration, texture cache and model detail settings. This grouping provides shared
storage; it does not claim the original translation-unit ownership.

| Global | Retail VA | Size | Initial state and extent evidence |
|---|---|---:|---|
| Default near plane | `0x00550f6c` | 4 | `.rdata` float 1; Camera constructor and ClampLimits |
| Default far plane | `0x00550f70` | 4 | `.rdata` float 100000; same callers |
| Blade device GUID | `0x00556040` | 16 | `.rdata` GUID used by PCGame's renderer selection |
| Gamma ramp | `0x00570570` | 1536 | Initialized `.data`; three channels of 256 unsigned shorts passed to DirectDraw gamma-control slot 4 |
| Texture memory debug flag | `0x00689964` | 4 | PE zero-fill; PCTextureMap's recount-memory condition |
| Shared mipmapped surfaces | `0x0068a36c` | 40 | PE zero-fill; TextureMapManager clears 40 bytes and releases ten entries |
| Shared single-level surfaces | `0x0068a394` | 40 | Same initialization and release loops |
| Display pointers | `0x0068a754` | 16 | PE zero-fill; both enumeration callbacks cap the count at four |
| Display count | `0x0068a764` | 4 | PE zero-fill; enumeration increments it after successful initialization |
| Hardware detail presets | `0x00574428` | 160 | Ten initialized float/int/int/int rows selected by Scene and BikeRace |
| Blade detail presets | `0x005744c8` | 160 | Ten rows for the software renderer; the next address begins string data |
| Active detail presets | `0x00689f18` | 4 | PE zero-fill; Scene/BikeRace select a table before D3DIMSoultreeObject reads it |

The earlier gamma declaration incorrectly described a zero-initialized array.
The retail image contains a nonlinear, channel-specific table, with its first
nonzero byte at `0x005705cf`. The source preserves all 768 values. Its exact
formula is not established; it must not be replaced with a guessed linear
curve or zero array. `UnknownDisplay::CreateFlipChain` passes this table at
`0x004c9ffc` when `useGammaRamp` is enabled.

`GraphicsState.bindings.json` records the existing callers' addresses. The
data regression compiles the provider with authentic VC6, checks each global's
complete extent and initial bytes against the pinned PE (including zero-fill),
checks writable versus readonly storage, and rejects unexpected code or
dynamic initialization in the object. It does not assert linked retail layout
or successful rendering.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:PYTHONPATH = '.'
python -m unittest discover -s tests -p test_graphics_state.py -v
```
