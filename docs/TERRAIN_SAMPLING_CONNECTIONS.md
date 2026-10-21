# Terrain and collision-tree sampling providers

Terrain's cell query now calls the existing GridNode corner sampler, vector
dot product and triangle-normal providers. Both collision-tree builders use
the same triangle-normal declaration. These connections replace four missing
call-view names while preserving every instruction and relocation addend in
the complete caller objects.

| Existing provider | Retail VA | Complete strict extent |
| --- | --- | --- |
| `GridNode::GetCellCorners` | `0x00483910` | 1067 bytes |
| `Vec3DotCall` | `0x0040ae30` | 29 bytes |
| `TriangleNormal` | `0x004a11e0` | 281 bytes |

Terrain's loader constructs DrawableGridNodeSharedTextures, whose retail RTTI
places GridNode at primary offset zero. The existing height-field pointer is
passed unchanged to GetCellCorners. Its five stack arguments are the two cell
indices and three output arrays. The provider returns an integer; QueryGround
continues to ignore it. The provisional class view remains for other boundaries.

TerrainVec3, TreeVec3, GridVec3, Vector3 and Vec3 retain their separate layouts
and inline arithmetic. All are three consecutive floats. Explicit boundary
casts preserve the original pointers, including null normal outputs.
MatrixUtil and Griddraw declarations introduce no additional dynamic
initializers in either caller. Original type names remain unproven.

The private `terrain-math-round21-preservation.json` report preserves **125
complete nondebug sections and 785 relocations** across Terrain.cpp and
BoundingBoxTreeBuild.cpp under the four external-name substitutions. Local
targets retain their section identity and offset. This includes the entire
**3255-byte nonexact QueryGround** and **2117-byte nonexact Load**. Load's EH
label changes from `$L4924` to `$L5863`, both section 29 at offset 182; named
fingerprints are refreshed only after verifying that identity.

Real build `build-sn4odjja` compiles **207/207 units** and removes exactly those
four names: **475 to 471**, with none added.

## Missing in-place scale operators

`src/reconstructed/VectorScaleAssign.cpp` supplies the two existing out-of-line
call signatures used by Bike steering and Terrain's segment-hit conversion.
Both complete **33-byte** bodies match `0x0040ae00`. They multiply each float
component in place and return the receiver reference, preserving the thiscall
receiver and four-byte scalar argument. There are no wrappers, linker aliases
or dynamic initializers.

The source grouping and Bike/Terrain call-view names remain provisional.
Identity rests on the three decoded Bike steering calls and Terrain's decoded
segment-hit call, their input/output behavior and complete provider matches;
the shared retail address alone does not establish an original class identity.
Bike's unchanged declaration now lives in a shared header. All **242 nondebug
sections and 1089 relocations** in its object remain identical.

After the garage shadow connections, real build `build-l99te5t5` compiles
**208/208 units** and removes only these two names: **464 to 462**.

## Native evidence and limits

`tests/test_terrain_sampling_connections.py` passes **51 strict provider/BVH
comparisons**, decoded caller checks, retail RTTI and native layout assertions.
It links the actual complete GridNode and VectorScaleAssign production objects
into its native fixture. Reference methods execute unpatched. The sampler has
no scripted callees; its unexercised segment virtual fails immediately.

The **69,699 sampler cases** cover leaves, two levels of descent, missing child
tables/entries, positive upper-bound clamping, optional output arrays, signed
normal bytes, relief/height scales and full storage preservation. Four x87
rounding modes and three precision modes include zero, subnormal, infinity
and NaN scales. Return values, access-fault details, control words and sticky
flags are compared. Independent expectations check corner order, heights,
surface values and missing-cell defaults. Retail's missing-cell branch writes
through a null surface output; that existing fault is retained and checked.
Arbitrary negative child indices and unmasked FP traps are outside this proof.

The **48,792 scale comparisons** exercise both actual providers with edge and
deterministic raw float patterns under all rounding/precision modes. They
compare surrounding storage, returned receiver pointers and FP state; an
independent ordinary scaling example also checks both definitions.

The existing **278,411 first-cell conversion cases** still pass through the
canonical sampler signature, with production/extracted body identity checked.
Neither regression executes the complete QueryGround interpolation pipeline.
Full interpolation, normals/material selection, actual terrain loading and
rendering still require validation. The executable continues to fail its link.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
$env:PYTHONPATH = '.'
python -m unittest discover -s tests -p test_terrain_sampling_connections.py -v
```
