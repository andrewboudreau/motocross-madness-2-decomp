# Shared TrackGame pointer

Retail `0x0056e26c` contains a pointer initialized to the singleton at
`0x006851a0`. `MSZoneInterface.cpp` supplies that storage as
`TrackGame* g_TrackGame = &g_TrackGameInstance`; its bindings record both
addresses. The initializer at `0x004aa7c0` constructs the singleton through
`0x00520870` and registers destruction through `0x00521ae0`.
These addresses and pointer behavior are confirmed. The original translation
unit ownership of the pointer definition is not established.

Fourteen provisional views previously declared different C++ extern names or
types for this same address. Five still read `g_TrackGame` and explicitly
cast its current value to their local view. The other nine callers read the
complete `TrackGame` class (see below), whose Game and PCGame bases already
declare every offset they use. There is one supplied pointer, with no
additional pointer copies, reference aliases, or cached view pointers.
This consolidates storage identity; it does not establish the complete
TrackGame layout or promote these local types to confirmed class identities.

| Former declaration | Retained provisional view |
| --- | --- |
| `g_Bike_0056e26c` | `BikeGlobal_0056e26c` |
| `g_collisionGameContext` | none: reads `Game::childRoot` (+0x34) through `TrackGame` |
| `g_gridGameSettings` | none: Griddraw.cpp reads `Game::bladeRenderer` (+0x2d0) and `PCGame::minFilter`/`mipFilter` (+0x550/+0x554) |
| `g_kbGame` | `KbGame` |
| `g_gameContext` | none: the particle code reads `Game::paused` (+0x1c4); the old `hidden` word is `Game::bladeRenderer` (+0x2d0) |
| `g_0056e26c` | none: SoulTreePhysics.cpp reads `TrackGame::sceneObject` (+0x574) |
| `g_soultreeStats` | none: SoultreeQuadTreeRenderer.cpp reads `Game::debugOverlay` (+0x38) and `Game::testKey` (+0x2d4 bit 2) |
| `g_terrainGame` | none: Terrain.cpp reads `Game::display` (+0x0c), `debugOverlay` (+0x38), `bladeRenderer` (+0x2d0), `testKey` (+0x2d4 bit 2) and `PCGame::magFilter`/`minFilter`/`mipFilter` (+0x54c..+0x554), and calls `GetRegistryInt`/`SetRegistryInt` (slots 20/25) |
| `g_profiler` | none: VisibilityQuadTree.cpp reads `Game::debugOverlay` (+0x38) |
| `g_TrackGame` in EcoSystem | none: reads `TrackGame` (the view was removed) |
| `g_pGame` | `SurfaceGame` |
| `g_pShadowSettings` | `ShadowSettings` |
| `g_BikeAIGame_0056e26c` | none: reads `TrackGame` (the view was removed) |
| `g_UnknownKrustyGame56e26c` | `CueTrackGameView` |

The last four declarations occur only in partial sample callers. Their bodies
remain in `samples/` and their acceptance status is unchanged. Canonical and
sample binding files now bind `?g_TrackGame@@3PAVTrackGame@@A` to `0x0056e26c`.
Ten unresolved names in the source game build are removed by this change.

The terrain, grid, soultree and visibility views named only offsets that
Game, PCGame and TrackGame already declare, so no member was added. Retail
loads `[0x0056e26c]` and then reads, with the same width:

- `+0x2d0` dword: Terrain::Load `0x00505a2a`, Terrain::Render3D `0x00506280`,
  `0x0050661e`, DrawableGridNode `0x00480cc9`, `0x00480fb9`.
- `+0x2d4` byte, mask 4: Terrain::Render3D `0x005069fb` (`test byte ptr`),
  SoultreeQuadTreeRenderer slot 23 `0x005048d9`.
- `+0x0c` and `+0x38` dwords: Render3D `0x00506299`, `0x00506a08`;
  SoultreeQuadTreeRenderer slot 14 `0x00504578`; VisibilityQuadTree
  PrepareGeometry `0x0052ef9d`.
- `+0x54c/+0x550/+0x554` dwords: Render3D `0x0050666f`..`0x005066e9`;
  DrawableGridNode `0x004810cf`, `0x004810ea`.
- `+0x574` dword: the slot 40 loaders `0x00503b68`, `0x00503fdc`.
- vtable `+0x50`/`+0x64` (slots 20 and 25): Render3D `0x005062c3`, `0x00506411`.

Every object of the game build is identical before and after, apart from
renumbered compiler `$T`/`$L` labels. Including `TrackGame.h` changes the
order in which SoulTreePhysics.obj emits its vtordisp and deleting-destructor
thunk COMDATs, so the linked build places SoultreePhysicsObject's two
`$4` thunks before SoultreePhysicsCharacter's (retail order: `0x00504260`,
`0x00504270`, then `0x005042e0`..`0x00504350`); each body is unchanged.

`TrackGameGlobal.h` provides the forward declaration used by the local views.
`TrackGame.h` retains its original, identically typed extern declaration after
the complete class. Declaration order affects VC6 register allocation:
introducing the forward declaration into that header changed
`ManagedTextureGroup::DrawSelection` from its exact 1834-byte body to 1838
bytes. Keeping the original declaration restores the exact body. Explicit
casts also preserve instruction scheduling that a templated accessor changed
in several callers.

ManagedTextureGroup now reads only the canonical Game base through
`Game.h` and `TrackGameGlobal.h`. Retail TrackGame COL `0x0055f8e0` and
hierarchy `0x0055f8d0` record PCGame and Game with `mdisp=0`, `pdisp=-1`;
there is no base adjustment. RepackPages reads TestKey at `+0x2d4`, mask 4
(`0x0050c8c8`), and display at `+0x0c` (`0x0050c936`). The native shared-pointer
regression checks the full declaration's base conversion and these field
locations. All 15 ManagedTextureGroup calibration cases remain strict with
its prior helper order; see [declaration-order evidence](VC6_OPERAND_ORDER.md).

## Shared header definitions

The reconstructed headers (`src/reconstructed`) and the krusty2 physics
headers (`src/krusty2`) used to define four classes twice, so a file could not
include `TrackGame.h` together with the krusty2 tree. Each now has one
definition:

| Class | Definition | Included by |
| --- | --- | --- |
| `Matrix4` | `src/reconstructed/Matrix4.h` (the `_11.._44` union with `m[4][4]` and `operator()`) | `MatrixUtil.h`, krusty2 `math/Math3D.h` |
| `MemTagStack` | `src/reconstructed/MemTag.h` | krusty2 `core/MemTag.h` |
| `Character`, `D3DIMSoultreeCharacter` | `src/krusty2/motion/D3DIMSoultreeCharacter.h` | `EventManager.h` |

`TrackGame.h` forward-declares `EventManager`. The files that use its members
include `EventManager.h` themselves. `D3DIMSoultreeCharacter.h` no longer
includes `math/Math3D.h`, because that header's per-TU `Vec3` constants collide
with the reconstructed tree's.

BikeAI.cpp (`src/krusty2/vehicle/BikeAI.cpp`, formerly the BikeAI sample) includes
`TrackGame.h`. It reads `mode.raceSettings.eventType`,
`mode.raceSettings.treeCollision`, `mode.raceSettings.raceMode`,
`mode.difficulty` and `childRoot` without casts. 0x0040eca0 rose from 1218 to
1432 strict bytes and from 3402 to 3731 aligned instructions. Its other rows
are unchanged.

Two exact functions are declaration-count tie-breaks and moved with these header
changes (docs/VC6_OPERAND_ORDER.md section 2.3). Defining a neighbouring
function at its retail position restored both:

- Terrain slot 14 (`0x00506220`): `~Terrain` now follows SelectQuality.
- `ManagedTextureGroup::DrawSelection` (`0x0050ef70`): `FitTexturesToBudget`
  now precedes the comparators.

## Validation

A comparison of all 21 directly changed translation units covered 507
registered target checks and preserved all 467 that were already strict
matches, including relocation destinations. The existing calibration and
physics suites check their canonical and partial callers against retail;
shared-header dependents also require the full calibration gate.

`tests/test_trackgame_global.py` compiles three independent native VC6
translation units. The two reader/writer units both use the complete `TrackGame`
class (`Game::bladeRenderer`), as EcoSystem.cpp and the particle code read it. Their COFF files import
the same canonical external pointer. The executable checks cross-view writes
at offset `0x2d0`, switches the shared pointer between two backing buffers,
and verifies both readers observe each reassignment. This fixture owns
validation-only storage; it does not construct a TrackGame or demonstrate
game startup.

```powershell
$env:VC6_ROOT = 'C:\path\to\vc6sp3'
python -m unittest discover -s tests -p test_trackgame_global.py -v
```
