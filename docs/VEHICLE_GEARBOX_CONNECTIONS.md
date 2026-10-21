# Vehicle gearbox implementation connections

Vehicle, KrustyBike and the BikeAI candidate now call the existing
`UnknownEngineGearbox` implementations directly. Five obsolete external names
resolve to four supplied methods; the two wheel views shared DriveForce.
The authentic source link changes from **383 to 378 unresolved names**, with
**215/215 translation units** compiled and no new missing symbols.

| Supplied method | Retail VA | Complete bytes | Stack cleanup |
|---|---|---|---|
| Constructor | `0x004d2940` | 1,542 | `RET 0x24` |
| UpdateThrottle | `0x004d2f50` | 215 | `RET 0x0c` |
| AutoShift | `0x004d3030` | 373 | `RET 8` |
| DriveForce | `0x004d31b0` | 388 | `RET 0x1c` |

All four bodies are complete strict matches under VC6 SP3, with relocations
resolved. Five production call instructions have independently decoded retail
destinations: Vehicle's loader, its throttle/shift step, and the two wheel-force
callers. The loader's two source branches share one emitted constructor call.
The BikeAI and KrustyBike sample aggregates also use the canonical declarations.

## Identity and ABI boundaries

The loader allocates `0x1e4` bytes. Its constructor destination, field accesses
and object flow identify the engine state as the canonical gearbox. Bike's
loader passes that same pointer to AddWheel for driven wheels; the wheel stores
it at `+0x2a8`. No secondary-base adjustment or virtual dispatch is introduced.
The gearbox has no vptr or reconstructed base classes. Its descriptive class,
field and method names remain provisional, and `GearRatios.cpp` is not an
attested original translation-unit name.

The older `VehicleSpeedState` and wheel structures remain provisional field
views. Both complete engine layouts have size `0x1e4`; the regression checks
every named overlap, including the integer views of float RPM storage. Their
constructor/throttle/shift/force declarations are removed so they cannot create
competing providers. The shared empty destructor remains unresolved: a tiny
shared address alone does not identify that method.

Three boundaries require explicit treatment:

- The loader labels `gearArg` as a float, while the canonical constructor takes
  an integer at that stack position and stores its unchanged DWORD at `+0x1d8`.
  A `const int&` bit view preserves the original stack bits. It is deliberately
  not a numeric float-to-integer conversion or a claim that the original type
  has been recovered.
- UpdateThrottle consumes an unsigned byte. Connecting the former bool
  declaration preserves the complete callers, including their existing byte
  load/push behavior. The provider retains noncanonical nonzero byte values.
- DriveForce's last output is one float. Vehicle's provisional view calls the
  storage at `+0x280` a vector; the boundary passes its first float and leaves
  the following eight bytes alone. KrustyBike already uses a scalar view there.
  Vec3/Vector3 size and all component offsets are compiler-checked.

## Preservation and validation

Before applying these connections, private same-path baseline/current builds
covered Vehicle, KrustyBike, Bike, both affected sample aggregates and
GearRatios. All **1,033 nondebug sections and 5,189 relocation destinations**
remain identical under exactly five named substitutions, without byte, data,
extent or filename-storage exceptions. Evidence is in
`work/game-build/gearbox27/objects.json` and `renames.json`.

The retained regression checks **292 strict cases** and **28 complete partial
COFF fingerprints**. The latter include full function bytes and relocation
target identities; they do not promote incomplete matching bindings to strict
matches. Portable fingerprints normalize only recognized physical source-path
string suffixes. The independent whole-production-object comparison uses the
same physical checkout paths and has no such exception.

The native executable links the actual complete GearRatios object. It checks
six raw constructor DWORDs, including signed zero and nonfinite float encodings,
plus **48 throttle/force boundary cases**. Those cover all eight drive-cut bits,
cut-drive zero/reverse output, shared receiver addresses and canaries around the
scalar magnitude. This is local ABI evidence, not a racing test.

```powershell
$env:PYTHONPATH = '.'
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_vehicle_gearbox_connections.py -v
python tools/build_game.py -j6
```

The isolated link report is `work/game-build/build-jujsjh2x/report.json`.
The game still does not link; startup, menus and racing remain unverified.
