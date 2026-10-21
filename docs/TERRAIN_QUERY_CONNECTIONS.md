# Terrain query caller connections

Nine translation units now call the existing `Terrain::QueryGround` through
its canonical declaration instead of five unresolved provisional method names.
The complete caller bodies retain their instructions and argument order; the
change replaces the external COFF destination names at 23 call sites.

| Consumer | Calls |
|---|---:|
| FollowCamera slot 47 | 1 |
| VehicleCamera slots 35 and 68 | 2 |
| KrustyBikeCamera slot 52 | 1 |
| AuralScape tick | 1 |
| EcoSystem authored placement | 1 |
| RunwayLights tick | 5 |
| Placement square clamp | 4 |
| BikeRace track drawing | 6 |
| FollowCamera tick candidate | 2 |

Each retail call targets `0x00507c10`. The receiver is the primary Terrain
pointer; explicit boundary casts preserve its address. The existing vector
views retain their separate declarations and three-float layouts. The first
argument is writable position storage; the second is an optional normal
output, followed by the flat-shading integer and optional material-byte output.
AuralScape now passes its material pointer directly instead of casting it to
an integer. All four arguments occupy the same stack slots as before.

Terrain and EcoSystem previously declared `GroundFogableObject` independently.
Their identical four-byte declaration now lives in one header so the actual
Terrain declaration can be included by EcoSystem. Both primary RTTI hierarchies
identify this nonpolymorphic base at `+0x2c`; its complete nine-byte constructor
at `0x004aae20` stores one zero word. The field's semantic role remains
provisional. Other partial views and class ownership claims are unchanged.

## Evidence and limits

`tests/test_terrain_query_connections.py` checks both RTTI hierarchies, the
constructor bytes, native layouts, all 23 retail call destinations and COFF
imports, and **289 strict caller comparisons**. It also pins the complete
3,255-byte QueryGround candidate and its named relocations. That provider
remains nonexact and still has unresolved helper dependencies; this connection
does not establish correct terrain behavior or a functioning source-built game.
The original connection-only slice preserved a 3,321-byte provider with a
truncating cell conversion. A separate [conversion repair](TERRAIN_CELL_CONVERSION.md)
now passes 278,411 native cases through the first cell request and refreshes
that fingerprint. Full interpolation and unmasked exceptions remain unverified.
The separate FollowCamera tick regression continues to script the terrain
boundary, now through the canonical method signature.

The private `work/game-build/ground19-objects.json` comparison preserves every
non-debug section and relocation across all nine callers under the five
explicit external-name substitutions. Defined local targets are compared by
section and offset. This includes partial candidate bodies, not only registered
strict functions. No provider code, runtime dispatch shim or linker alias is
introduced.
The shared header renumbers Load's compiler-generated EH target from `$L4894`
to `$L4895` at relocation offset 3. Its target section and offset, and every
instruction byte, remain unchanged. The named fingerprint was refreshed only
after the complete object comparison verified that identity.

The isolated real build `build-pyjmj3h2` compiles **204/204 translation units**
and removes exactly those five unresolved names: **492 to 487**, with none
added. Actual terrain loading, normals/materials and camera/audio/placement
integration await a linked executable and runtime tests.

```powershell
$env:PYTHONPATH = '.'
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python tests/test_terrain_query_connections.py -v
```
