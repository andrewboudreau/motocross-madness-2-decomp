# Terrain caller connections

Three provisional method names now call the existing canonical Terrain
definitions. Each caller includes `src/krusty2/broadphase/Terrain.h` and casts
only at the evidenced call boundary; partial views and all other operations
remain unchanged. No wrapper, replacement provider or alternate Terrain class
was added.

| Caller | Retired method view | Canonical provider | Full extent |
| --- | --- | --- | --- |
| Grid1 texture eviction callbacks | `Grid1Terrain::RetireOwnedObject` | `Terrain::RetireOwnedObject`, 0x005057d0 | 83 bytes |
| QuarryStuntEvent detail controls | `UnknownTerrain::SelectQuality` | `Terrain::SelectQuality`, 0x00507960 | 129 bytes |
| NameOverlay visibility test in TrackOverlay | `UnknownTerrain::CastSegment` | `Terrain::CastSegment`, 0x00506e90 | 1654 bytes |

## Boundary evidence

Confirmed retail RTTI identifies Terrain's primary GameObject base at zero and
its GroundFogableObject base at +0x2c (COL 0x0055f338). That establishes the
canonical inheritance, not the identity of every provisional view by itself.
The connections also use the original receiver-producing instructions, direct
calls and complete caller byte matches:

- Grid1's callbacks at 0x0047ca00 and 0x0047ca40 obtain the node's terrain,
  retire its built texture and clear the texture/age fields. The node constructor
  retains its supplied terrain pointer. The view's focal length at +0x70 and
  texture table at +0x544 match Terrain. The actual provider searches the live
  pointer array at +0xcb8, shrinks its count at +0xcbc, swaps in the last live
  entry, and keeps the retired pointer immediately after the live prefix.
- QuarryStuntEvent's control handler at 0x004e0ce0 changes the detail setting
  and supplies its `eventTerrain` directly. The event loader creates Terrain
  with 0x00505830 and loads it with 0x005059d0. The provider preserves the
  existing quality table, detail override and dirty-distance behavior.
- NameOverlay's 0x00519000 visibility test supplies the event's terrain, the
  camera eye position, target point and a local hit vector. Its integer return
  remains the hit/occlusion result. `Vector3` and `TerrainVec3` are each three
  floats at offsets 0/4/8; the two input pointers remain const and the third
  remains writable. The thiscall receiver and all six stack arguments are
  unchanged. Terrain's segment cast scales and clips the segment, calls its
  height-field interface and writes the world-space hit on success.

These are specific method-boundary connections. They do not merge the whole
`Grid1Terrain` or `UnknownTerrain` declarations with Terrain or infer equivalence
of unrelated virtual slots.

## Verification and remaining dependencies

With native `VC6_ROOT` and the hash-pinned private `MCM2_EXE` configured:

```powershell
$env:PYTHONPATH = '.'
python tests/test_terrain_connections.py -v
```

The regression compiles the three actual caller TUs, Terrain.cpp,
TerrainSupport.cpp and native layout assertions. It preserves **178** strict
cases, proves the three full COFF function extents and retail bytes with
resolved relocations, and checks that actual caller objects import the
canonical providers rather than the retired names. It also retains complete
COFF-body/named-relocation hashes for the two existing partial Terrain functions
(`Load` and `QueryGround`); these hashes are preservation checks, not retail
matches. Retail RTTI and native layout checks protect the selected base,
receiver-field and vector boundaries.

Reconstructed consumers and the layout fixture compile without an added physics
include search path, matching the calibration driver. Terrain.h uses relative
paths for its core/math dependencies, with their original include order intact.

The isolated source build compiled **198/198** translation units and reduced
unresolved names **527 to 524**, exactly these three names, with none added.
This is not full terrain dependency closure: CastSegment still imports the
existing unresolved `TerrainVec3::operator*=`, and its runtime height-field
object must be supplied by the remaining game reconstruction. No terrain
runtime integration or completed game is claimed.

## QueryGround remains blocked

The five production QueryGround view names were examined but left unchanged.
The existing provider at 0x00507c10 is registered as partial. Its current native
COFF extent is **3321 bytes**, against the registered **2963-byte** retail body;
older notes recording a 3225-byte candidate describe an earlier composition.
It also has unresolved helper bindings, so no complete strict comparison is
claimed for it.

There is a semantic conversion mismatch, not just a different instruction
sequence. Retail subtracts 0.5 from each scaled coordinate, stores the float,
then uses `FISTP` at 0x00507c86 and 0x00507ca0 without changing the active rounding
mode. The current `FloatToInt` uses a C++ integer cast, emitted through
`__ftol`, which truncates. With round-to-nearest and a scaled coordinate of
1.25, retail converts 0.75 to cell index 1 while the source selects index 0.
This changes the inputs to `GetCellCorners`, before height/normal interpolation.
This example follows the decoded conversion semantics; it is not a full native
behavioral validation of QueryGround or a claim about every runtime control-word
state.

Connecting more callers to that body would not establish correct ground
sampling. It needs a separate full-body review and native differential proof,
including conversion modes, cell boundaries, surface-byte selection and normal
outputs. `/QIfist` is a possible compiler-calibration experiment for ordinary
C++; it has not been adopted here. No assembly, rounding substitute or weakened
comparison was introduced in this slice.
