# Terrain texture construction

Terrain's loader and pooled-surface allocator now construct the existing
`PCTextureMap` and `ManagedTexture` classes and register managed textures
through `ManagedTextureGroup::AddTexture`. These replace three unresolved
constructor/method names belonging to provisional terrain views.

| Provider | Retail address | Complete strict bytes |
|---|---:|---:|
| `PCTextureMap(TextureMapManager*, int)` | `0x004c5f00` | 46 |
| `ManagedTexture(TextureMapManager*)` | `0x00510500` | 123 |
| `ManagedTextureGroup::AddTexture(ManagedTexture*)` | `0x0050c6c0` | 145 |

All three implementations already exist in canonical source. This change
adds no provider bodies or aliases. Allocation expressions use the complete
canonical classes; explicit casts retain Terrain's partial surface view for
its existing virtual calls. No shared header, vtable, inheritance declaration
or virtual-call signature changes.

## Identity and call boundary

The PCTextureMap constructor writes vtable `0x00555fec`, whose COL at
`0x0055e440` identifies PCTextureMap, TextureMap and BaseObject at offset zero.
The ManagedTexture constructor writes `0x00558430`; COL `0x0055f4c8` adds
ManagedTexture above the same chain, also at offset zero. Native VC6 checks
the canonical and provisional sizes, `0x80` and `0xbc`, and the common
average-color and texture-surface fields at `0x3c` and `0x74`.

The complete retail `Terrain::AcquireOwnedObject` (`0x00505600`, 455 bytes)
and `Terrain::Load` (`0x005059d0`, 2,116 bytes) each allocate both sizes and
call the corresponding constructors. Each calls `0x0050c6c0` once for the
managed branch. They pass the original manager pointer and unchanged group
receiver/texture pointer. Constructor results need no base adjustment.

`TerrainSurfaceDesc` remains a partial view of the 24-byte texture choice:
its manager and registry at offsets zero and four correspond to
`UnknownTextureFormatChoice::manager` and `opaqueGroup`. Terrain copies that
registry pointer to its `+0x38` field. ManagedTextureGroup's own RTTI
(`0x0055f418`, vtable `0x005583c0`) identifies its BaseObject base at offset
zero. Its actual AddTexture implementation manages the texture list, shares
the first 8-bit palette with cache pages, stores the current frame, and sets
the texture's `+0x90` group pointer. The connection preserves that complete
provider and its existing dependencies. Semantic names and the incomplete
terrain views do not become complete original source declarations.

## Validation

`tests/test_terrain_texture_connections.py` decodes the retail COLs, base
descriptors, constructor vptr writes and both callers' allocation/call
sequences. It compiles the native layouts, checks all three actual COFF
providers/imports, and preserves **106 strict caller/provider comparisons**.
The existing terrain regression also passes all 190 comparisons.

Private before/after objects preserve all **116 non-debug sections and 587
relocations** across Terrain.cpp and TerrainSupport.cpp. Section bytes,
relocation sites/types/addends and destinations remain identical under the
three explicit external-name substitutions; compiler-local targets are
compared by section and offset. This includes complete partial functions:
the 2,117-byte Load candidate and 3,321-byte QueryGround candidate remain
unchanged, without being claimed as retail matches. Load's retained named
relocation fingerprint changes only to reflect the canonical provider names.
Private evidence is in `work/game-build/texture-round18-preservation.json`,
`texture-renames.json` and `check_texture_objects.py`.

The isolated real build `build-pcc4h93e` compiles all 202 translation units
and removes exactly these three unresolved names: **503 → 500**, with none
added. The link still fails. These checks establish the narrow construction
and registration connections; actual DirectDraw surfaces, resource lifetime
and rendered terrain await the source-built game's runtime tests.

```powershell
$env:PYTHONPATH = '.'
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_terrain_texture_connections.py -v
```
