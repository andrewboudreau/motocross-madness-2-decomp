# Shared TrackGame pointer

Retail `0x0056e26c` contains a pointer initialized to the singleton at
`0x006851a0`. `MSZoneInterface.cpp` supplies that storage as
`TrackGame* g_TrackGame = &g_TrackGameInstance`; its bindings record both
addresses. The initializer at `0x004aa7c0` constructs the singleton through
`0x00520870` and registers destruction through `0x00521ae0`.
These addresses and pointer behavior are confirmed. The original translation
unit ownership of the pointer definition is not established.

Fourteen provisional views previously declared different C++ extern names or
types for this same address. They now read `g_TrackGame` and explicitly cast
its current value to their local view. There is one supplied pointer, with no
additional pointer copies, reference aliases, or cached view pointers.
This consolidates storage identity; it does not establish the complete
TrackGame layout or promote these local types to confirmed class identities.

| Former declaration | Retained provisional view |
| --- | --- |
| `g_Bike_0056e26c` | `BikeGlobal_0056e26c` |
| `g_collisionGameContext` | `CollisionGameContext` |
| `g_gridGameSettings` | `GridGameSettings` |
| `g_kbGame` | `KbGame` |
| `g_gameContext` | `ParticleGameContext` |
| `g_0056e26c` | `SoultreeGlobals` |
| `g_soultreeStats` | `SoultreeStats` |
| `g_terrainGame` | `TerrainGameSettings` |
| `g_profiler` | `VisibilityProfiler` |
| `g_TrackGame` in EcoSystem | `UnknownEcoTrackGame` |
| `g_pGame` | `SurfaceGame` |
| `g_pShadowSettings` | `ShadowSettings` |
| `g_BikeAIGame_0056e26c` | `UnknownBikeAIGameView` |
| `g_UnknownKrustyGame56e26c` | `UnknownKrustyGame` |

The last four declarations occur only in partial sample callers. Their bodies
remain in `samples/` and their acceptance status is unchanged. Canonical and
sample binding files now bind `?g_TrackGame@@3PAVTrackGame@@A` to `0x0056e26c`.
Ten unresolved names in the source game build are removed by this change.

`TrackGameGlobal.h` provides the forward declaration used by the local views.
`TrackGame.h` retains its original, identically typed extern declaration after
the complete class. Declaration order affects VC6 register allocation:
introducing the forward declaration into that header changed
`ManagedTextureGroup::DrawSelection` from its exact 1834-byte body to 1838
bytes. Keeping the original declaration restores the exact body. Explicit
casts also preserve instruction scheduling that a templated accessor changed
in several callers.

## Validation

A comparison of all 21 directly changed translation units covered 507
registered target checks and preserved all 467 that were already strict
matches, including relocation destinations. The existing calibration and
physics suites check their canonical and partial callers against retail;
shared-header dependents also require the full calibration gate.

`tests/test_trackgame_global.py` compiles three independent native VC6
translation units. The two reader/writer units use the existing
`ParticleGameContext` and `UnknownEcoTrackGame` views. Their COFF files import
the same canonical external pointer. The executable checks cross-view writes
at offset `0x2d0`, switches the shared pointer between two backing buffers,
and verifies both readers observe each reassignment. This fixture owns
validation-only storage; it does not construct a TrackGame or demonstrate
game startup.

```powershell
$env:VC6_ROOT = 'C:\path\to\vc6sp3'
python -m unittest discover -s tests -p test_trackgame_global.py -v
```
