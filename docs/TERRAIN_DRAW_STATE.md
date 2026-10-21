# Terrain and grid drawing state

`src/reconstructed/TerrainDrawState.cpp` supplies twelve writable scalars already
referenced by canonical `Griddraw.cpp` and `broadphase/Terrain.cpp`. The file is a
storage provider, not an original translation-unit ownership claim. Existing
caller declarations, types and function bodies remain unchanged. These are
actual linker imports, not additional speculative state.

## Storage and writer evidence

The executable is pinned by SHA-256
`31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874`.
Every word resides in retail `.data`, characteristics `0xc0000040` (read/write).
The debug-page sentinel is a file-backed `ff ff ff ff`. All other words are
inside the section's virtual size but beyond its raw bytes, and begin as loader
zero-fill. The decoded stores/increment below independently establish writable
four-byte runtime state. Names describe existing reconstructed uses; they do not
claim original identifier spelling.

| Symbol | VA | Initial value | Representative decoded write |
| --- | --- | --- | --- |
| `g_terrainDebugPage` | `0x574714` | -1 | `0x506a2a` |
| `g_terrainCacheBytes` | `0x689ff4` | 0 | `0x507672` |
| `g_gridDrawMemoryPeak` | `0x68a2fc` | 0 | `0x47f51f` |
| `g_gridDrawMemory` | `0x68a300` | 0 | `0x47ecec` |
| `g_gridDrawFrameCount` | `0x68a304` | 0 | `0x47ef97` (`inc`) |
| `g_terrainAgeTime0` | `0x68a308` | 0 | `0x50770f` |
| `g_terrainAgeTime1` | `0x68a30c` | 0 | `0x507644` |
| `g_terrainCacheCount` | `0x68a310` | 0 | `0x5078ee` |
| `g_terrainPrepareGeometryTicks` | `0x68a318` | 0 | `0x5078f9` |
| `g_terrainRender3DTicks` | `0x68a31c` | 0 | `0x5069f1` |
| `g_terrainTotalTicks` | `0x68a320` | 0 | `0x506ac9` |
| `g_terrainDualTextureProbed` | `0x68a328` | 0 | `0x5064e6` |

The canonical grid routines already add/subtract buffer allocation sizes,
maintain the memory high-water mark and count node visits. Terrain reads the
same memory counters for its debug display. Its render method allocates a debug
page only when the sentinel is negative, records rendering/total times, and
marks the hardware dual-texture probe as complete. The software-rendering branch
does not set that latch; this provider does not change the existing behavior.

Retail preparation code also writes the age/cache/preparation counters. Its
complete source implementation is still a separate gap; supplying initial
storage does not implement those missing updates. No reset or initialization
function is invented. Nearby quality tables, grid texture arrays, peak-hold
objects and other globals are outside this slice.

## Verification

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_terrain_draw_state.py -v
```

The authentic VC6 SP3 object must define only these twelve symbols, as four
initialized bytes and 44 BSS bytes with four-byte individual extents. The test
checks full initial values, file-backed versus loader-zeroed provenance,
read/write section attributes and decoded writer operands. It rejects executable
sections, CRT initializer entries, external imports and non-debug relocations.
Actual caller COFF relocations must consume every definition; both caller units
must reference the same two memory counters. The focused gate also checks all
79 existing complete strict functions (57 Griddraw and 22 Terrain). The two
existing partial Terrain functions remain outside strict acceptance.

The isolated real build compiled 198/198 translation units and reduced missing
symbols from 539 to 527: exactly these twelve definitions resolved existing
imports, with no new unresolved names. The source-built game still does not link.
