# Shared texture-loader declaration

`src/reconstructed/TextureLoader.h` declares the existing `TerrainLoadTexture`
provider in `TextureMap.cpp`: `0x0050a590`, 1442 bytes. The function name is
provisional. Its decoded implementation and complete relocation-resolved
VC6 match establish the shared call destination; this change adds no loader
body or claim about original translation-unit ownership.

Seven translation units now import this one declaration: SoultreeMaterial,
BikeNumberPainter, FontTexture, TrackOverlay, EcoSystem, Particles and Terrain.
Their previous signatures produced five unresolved decorated names despite
calling the same retail address. Canonical and retained sample binding keys
now use the provider's signature, including `Palette8*` and
`UnknownTextureFormatChoice*`.

The six-word `SoultreeTextureOptions` view remains local to material code.
The compile-time fixture checks its size and every corresponding field offset
against `UnknownTextureFormatChoice`; the call explicitly converts the pointer.
FontTexture retains its integer-valued options parameter and explicitly
converts those bits at the boundary. EcoSystem, Particles and Terrain retain
their provisional texture return views with explicit conversions. These
connections do not establish complete class identity or layout for those views.

The adjacent motion connection replaces the former `SceneItem` declaration
with `SoultreeMaterial`, supplying three more existing provider names. The
[material evidence](SOULTREEMATERIAL.md) records the caller's allocation and
direct-call addresses. No wrappers, new implementations or allocator changes
are involved.

`tests/test_texture_loader_connections.py` compiles the actual providers and
callers using VC6 SP3, inspects their COFF definitions/imports, checks the
retail allocation and three call destinations, and verifies 236 complete
calibration bodies plus 55 exact physics targets with resolved relocations.
The fixture also asserts `sizeof(SoultreeMaterial) == 0xd0`. The two existing
partial Terrain functions remain outside the required exact set. These checks
prove the declarations connect the existing implementation while retaining
the checked complete bodies; they do not exercise graphics hardware or gameplay.

The calibration set includes all 51 SceneManager bodies. Its complete material
class include is placed with the texture-accounting functions that use it;
earlier light parsing has no material dependency. This declaration order
preserves all 2102 bytes of ReadLight (`0x004eb570`) under VC6 after the loader
overloads were consolidated, without changing that function's body.

Reproduce in PowerShell:

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_texture_loader_connections.py -v
```
