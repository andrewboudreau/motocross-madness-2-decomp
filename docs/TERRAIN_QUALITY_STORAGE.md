# Terrain quality table storage

The two existing `TerrainQualityEntry[10]` declarations now have ordinary
writable, statically initialized storage. The full 160 bytes at `0x574920`
and the next 160 bytes at `0x5749c0` are literal file-backed retail `.data`,
not inferred defaults. The existing names `g_terrainQualityHardware` and
`g_terrainQualitySoftware` are retained provisionally. Original translation
unit ownership is not established.

The provider includes the genuine unchanged `Terrain.h`. Its existing
four-DWORD record has a 16-byte stride. VC6 emits just a 40-byte `.drectve`
section and a 320-byte writable `.data` section with flags `0xc0400040`;
the two definitions are at offsets 0 and 160. There are no imports, functions,
runtime relocations, or CRT initializer entries. Retail's containing `.data`
section has flags `0xc0000040`. Every initial payload byte is checked.

## Selector and lifetime evidence

The complete `Terrain::Load` body at `0x5059d0` reads Game `+0x2d0`, writes
`0x574920` to the existing selector pointer at `0x68a32c` when that DWORD is
nonzero, and writes `0x5749c0` when it is zero. It then calls `SelectQuality`.
These decoded conditions are authoritative; the provisional hardware/software
names do not independently establish the meaning of that Game field.

The complete strict 129-byte selector at `0x507960` uses `index * 16` and
reads all four DWORD fields. The override branch reads row 9's second field
at offset `0x94`; the selected row still supplies the other fields. The
complete strict 37-byte setter at `0x507930` clamps the first field to zero
and marks the draw-distance dirty flag only when the value changes. There
is no bounds check in `SelectQuality`; the native contract below covers
indices 0 through 9, not arbitrary caller-provided indices.

A private whole-executable linear operand census finds the two selector
address constants and the known reads of `g_pTerrainQualityTable`, with no
discovered direct writes into the table ranges. The fixed retail image has
stripped base relocations. This is bounded evidence, not proof that no
indirect writer exists or that the arrays are immutable. The reconstructed
definitions remain writable, and deliberate test mutations verify that the
actual consumer reads that shared storage.

## Preservation and native validation

No existing source, header, or binding changes. The complete canonical
Terrain object retains all 85 nondebug sections and 517 relocations, including
all existing partial functions. Fresh private and current-root compilations
have identical complete object views after normalization of the single exact
emitted `Terrain.cpp` filename. There are no method-name mappings or other
exceptions. The provider also compiles without project include options.

The retained test checks 22 existing strict Terrain targets plus two strict
native extractions. The extracted `SelectQuality` and `SetField0xbec` bodies
match their complete canonical bodies byte for byte and retain every named
relocation. The selector pointer and quality-value definitions are taken
verbatim from the same source; the new table provider is linked as an actual
separate object. No game helper is mocked or patched. This fixture executes
source code against independent oracles, not a mapped retail game.

There are 5,120 ordinary cases: two actual tables, ten valid indices, two
override states, four initial draw distances, two dirty states, and sixteen
seeds. Eight seeds use the literal table values; eight deliberately mutate
the real exported table fields, including negative distances and arbitrary
DWORD values. Independent checks cover the complete prepared Terrain storage,
surrounding guards, clamped values, dirty preservation, row-9 selection, and
both shared globals. Every ordinary case must complete without an exception.

Ten additional guard-page cases cover short receivers and table ranges.
Eight independently assert the first access-violation address and read/write
direction, all accessible receiver bytes, the preceding global update and
table-page preservation. Two independently require successful completion.
The fixture uses integer operations and prepared raw storage; it does not
construct a Terrain lifetime, execute the nonexact loader or renderer, prove
arbitrary invalid-index behavior, or establish runtime readiness of the game.

Reproduce with the authentic compiler and pinned retail image:

```powershell
$env:PYTHONPATH='.;tests'
python -m unittest test_terrain_quality_state -v
```

## Actual game integration

Build `work/game-build/build-862og0cf` compiles 274/274 translation units and
removes exactly the two table symbols: **180 -> 178**, none added. All 273
previous objects preserve 10,866 nondebug sections and 46,268 relocations
without any mappings or filename exclusions. The new two-section data object
has no functions, imports or runtime relocations. All source, header,
configuration and compiler inputs are checked. Global calibration and physics
retain every prior strict result, full extent and matching-position score.
The retained native test also passes at the actual source path. The game still
does not link; startup, menus and racing remain unverified.
