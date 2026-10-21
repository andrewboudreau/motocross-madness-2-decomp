# Frozen visibility camera lifecycle

The camera snapshot at retail `0x0068a968` now uses the existing `PCCamera`
constructor and implicit destructor. `VisibilityQuadTree.cpp` defines the actual
`PCCamera` object and keeps explicit `VisibilityCamera` record casts at its two
copy/query boundaries. The legacy record and all public query signatures remain
unchanged. This connects two missing lifecycle names; it introduces no new storage
or handwritten constructor/destructor implementation.

## Identity and storage

The raw PCCamera complete-object locator at `0x0055e190` has object offset zero,
type descriptor `0x00566b48` names `.?AVPCCamera@@`, and primary table `0x00555da4`
points back to that locator. PCCamera, Camera, GameObject and BaseObject base
records all have `mdisp=0`, `pdisp=-1`. Authentic VC6 layout checks establish
`sizeof(PCCamera)==sizeof(VisibilityCamera)==0x220`, the context pointer at `+0x18`
and eye position at `+0x170`. This supports the primary-pointer boundary without
asserting that every provisional field or camera-like object has the same C++ type.

The entire `0x0068a968..0x0068ab88` range lies in writable PE zero-fill storage.
Retail references the base explicitly at four sites: constructor setup, destructor
setup, the snapshot destination, and selection of the frozen snapshot. The scan
covers encoded addresses, not arbitrary indirect accesses.

The lifecycle has direct instruction evidence:

| Operation | Retail body | Behavior |
| --- | --- | --- |
| CRT entry | `0x00566558` | Points to initializer thunk `0x0052d2e0` |
| Initializer thunk | `0x0052d2e0`, 10 bytes | Calls construction, then registers cleanup |
| Construction | `0x0052d2f0`, 13 bytes | Passes flags `1`, receiver `0x0068a968`, calls `0x004bed80` |
| Registration | `0x0052d300`, 12 bytes | Registers `0x0052d310` through actual `atexit` |
| Cleanup | `0x0052d310`, 10 bytes | Loads the same receiver and jumps to `0x004624d0` |
| PCCamera constructor | `0x004bed80`, 25 bytes | Calls Camera construction, installs primary table `0x00555da4`, returns the receiver, `ret 4` |
| Implicit destructor | `0x004624d0`, 5 bytes | Jumps to the existing Camera destructor `0x0042f020` |

The complete 135-byte control handler copies `0x88` DWORDs into the snapshot. The
complete 252-byte preparation method selects that same address when freezing is
active. The copy includes vptr, ownership and file-related fields, exactly as
retail does; this connection does not add new ownership or cleanup rules.

## Compiler and preservation evidence

`tests/test_frozen_camera_connections.py` checks 44 complete strict bodies: all 41
registered VisibilityQuadTree targets, its newly emitted implicit destructor, and
the constructor/destructor in the actual PCCamera provider. The compile-only
layout fixture also checks the unchanged primary conversion and selected offsets.
No constructor or destructor is replaced by a test stub.

All 87 original nondebug sections and 341 relocations retain their raw bytes,
flags, addends and destinations under these exact names:

- `??0VisibilityCamera@@QAE@H@Z` → `??0PCCamera@@QAE@H@Z`
- `??1VisibilityCamera@@QAE@XZ` → `??1PCCamera@@UAE@XZ`
- `?g_frozenCamera@@3UVisibilityCamera@@A` → `?g_frozenCamera@@3VPCCamera@@A`

There is one explicitly checked definition-resolution change: cleanup's destructor
reference now targets the canonical implicit destructor emitted in this object.
Its new COMDAT section contains the exact five-byte jump plus eleven alignment
NOPs, with one relocation to Camera's destructor. The resulting object has 88
sections and 342 relocations. This is not a rename-only whole-object claim.

The retained test verifies the complete current object, then reverses only that
proven cleanup resolution and removes only the fully checked new COMDAT to compare
against the complete prior-object pin. BSS remains 620 bytes, with the camera at
`+64`; the same eight `.CRT$XCU` entries remain in the same order. There is no new
initializer or private header storage.

The existing visibility, clipper-global and EcoSystem object pins were independently
reproduced before updating only their VisibilityQuadTree entries. Their focused
regressions pass: 214 strict checks and 66,845 visibility cases; 345 strict checks
and 46 clipper pointer loads; 120 strict checks and 13,347 QuadTree Insert cases.
These native suites cover their documented operations, not this camera lifecycle.

## Runtime boundary

This slice proves source linkage, storage and complete retail lifecycle bytes. It
does not claim native execution of full camera startup or teardown. A private
actual-source link probe reached the broader virtual-method dependency graph:
adding real Game, rendering, input and TGA providers exposed unrelated unresolved
startup/display methods and `WriteTga`. No substitute camera methods were supplied
to force a successful executable. The compile-only layout fixture constructs no
objects. The provisional record casts describe the preserved VC6 memory boundary,
not portable equivalence of the complete classes.

The independent Terrain camera at `0x0068a090` is unchanged. Its proposed header
connection altered strict relocation ordering in Terrain's draw method and was
not included here.

The isolated game build compiled all 258 translation units and reduced unresolved
names from 230 to 228, removing exactly the two visibility camera lifecycle names
with no additions. Startup, menus and racing still require the remaining providers.

```powershell
$env:PYTHONPATH='.;tests'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_frozen_camera_connections test_visibility_connections test_clipper_global_connections test_eco_quadtree_connections -v
```
