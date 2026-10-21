# SurfaceMap provider

`src/reconstructed/SurfaceMap.cpp` supplies the six complete functions formerly
in `samples/physics/motion/SurfaceMap.cpp`. The class name is confirmed by RTTI;
the file is a reconstruction grouping, not an established original translation
unit. No source literal identifies its retail owner. Proximity to SteeringControl
and Terrain does not assign either file's ownership to these methods.

| Function | Retail VA | Complete bytes |
|---|---:|---:|
| Constructor | `0x005051f0` | 169 |
| Scalar deleting destructor | `0x005052a0` | 30 |
| SetTexture | `0x005052c0` | 42 |
| Apply | `0x005052f0` | 27 |
| Select | `0x00505310` | 61 |
| Destructor | `0x00505350` | 86 |

All six match under authentic VC6 SP3 `vc6_o2_mt` with full extents and
resolved relocations. `samples/physics/motion/targets.json` routes checks to
the canonical source; no duplicate sample body remains. The eleven existing
SoultreeMaterial calibration cases also remain strict.

## Identity, allocation and interfaces

Retail vtable `0x00558248` has COL `0x0055f2e0`, object offset zero, and type
descriptor `0x00574668` naming `.?AVSurfaceMap@@`. Its hierarchy lists BaseObject
as a nonvirtual base at offset zero. Only the destructor slot is overridden;
AddRef, Release and GetRefCount retain BaseObject's entries. Constructor and
destructor vptr writes independently identify this table. The provider uses
the actual shared BaseObject, replacing the sample's separate SurfaceBaseObject.

The texture pointer is at `+0x08`; the constructor copies 17 floats into
`+0x0c..+0x4f` or writes white diffuse/ambient and zero specular/emissive/power.
The size, ordering and device slot are strong evidence for the D3DMATERIAL7
layout. `SurfaceMaterial` is a reconstructed type name, not recovered source text.

SoultreeMaterial::LoadTexture explicitly pushes `0x70` at `0x004ff650` before
allocation, then calls the constructor at `0x004ff66d`. The shared SurfaceMap
declaration preserves this allocation size. Its unknown `+0x50..+0x6f` tail
is neither assigned meaning nor initialized by these bodies. The former sample's
`0x50` size was insufficient for use by that actual caller.

Texture ownership and selection now use TextureMap's real BaseObject slots
and slot 19, Bind. Apply reads the shared `g_TrackGame` pointer, Game's render
target at `+0x10`, and PCRenderTarget's device at `+0x50`. Device slot 16 is
the COM-style, stdcall material setter; only that interface's declaration is
named SetMaterial. Select's untextured path calls RenderTarget slot 7,
SetTextureStageState, twice with `(0, 1, 1)` and `(0, 4, 1)`. The second call
reloads the global pointer as retail does. No separate game pointer or layout
view is introduced. The complete texture constructor signature is shared with
SoultreeMaterial; these providers remove its three missing SurfaceMap imports.

## Validation boundary

`tests/test_surface_map_provider.py` hashes the private retail input, checks
RTTI/table and caller allocation evidence, compiles the real provider and
consumer, compares all six SurfaceMap and eleven SoultreeMaterial bodies, and
checks actual caller relocations and the five supplied BaseObject dependencies.

Its native executable links production SurfaceMap and BaseObject with controlled
external texture, render-target and device virtual-call fixtures. It checks default
and supplied material data, reference counts, same-texture replacement order,
texture destruction, both selection paths, HRESULT success/failure conversion,
the untouched allocation tail, and a global-pointer change between the two
texture-stage calls. These fixtures verify the observed x86 call interfaces;
they do not initialize DirectX, load real textures or establish working racing.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_surface_map_provider.py -v
```
