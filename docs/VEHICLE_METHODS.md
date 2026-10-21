# Vehicle primary-vtable methods

`src/reconstructed/VehicleMethods.cpp` supplies thirteen exact method bodies;
`src/reconstructed/VehicleReset.cpp` supplies the complete slot 36 reset.
All fourteen now have canonical providers for the production Vehicle, Bike
and KrustyBike tables. Semantic names and original source grouping remain
inferred; matching does not establish a complete working vehicle.

| Primary slot | Retail VA | Bytes | Behavior |
|---:|---:|---:|---|
| 23 | `0x0040caf0` | 38 | Tests airborne state and forward-speed threshold |
| 24 | `0x0040cb20` | 59 | Tests increasing throttle, crash state and speed |
| 25 | `0x0040cb60` | 59 | Tests decreasing throttle, crash state and speed |
| 34 | `0x0040c4c0` | 118 | Reads node axes and computes the side vector |
| 35 | `0x0040c540` | 130 | Writes node axes and computes the side vector |
| 36 | `0x0040c710` | 368 | Resets orientation state and steering node |
| 45 | `0x0040c880` | 7 | Returns `3.0f` |
| 51 | `0x0040c890` | 7 | Returns the spawn-protection field |
| 52 | `0x0040c8a0` | 14 | Tests whether crash state is zero |
| 68 | `0x0040cac0` | 18 | Writes `15` through its argument and returns one |
| 69 | `0x0040cae0` | 13 | Reloads the crash timer |
| 78 | `0x00492220` | 76 | Queries input-map entries and virtual slot 77 |
| 79 | `0x00492270` | 76 | Queries the other input-map entries and slot 77 |
| 92 | `0x004a6ba0` | 3 | Returns, popping two pointer arguments |

Retail vtable `0x00558a84` contains each target in the listed slot. Its COL
at `0x0055f9b0` has object offset zero and type descriptor `0x00566c98`
containing `.?AVVehicle@@`. The separate tables at offsets `0x21c` and
`0x5c0` are not used to assign these methods. Even the three-byte slot 92
body is admitted through its own table entry, not by assuming that shared
tiny addresses identify the same C++ method.

These bodies lie outside the Vehicle.cpp range: most occur among Bike.cpp
methods, slots 78/79 in the KrustyBike.cpp range, and slot 92 in the
Motnctrl.cpp range. Inline member COMDAT selection explains the distribution
as a strong inference. Both files are reconstruction groupings, not recovered
original translation units. Semantic field names remain provisional.

The [slot 36 reset](VEHICLE_RESET.md) uses the existing ordinary header-local
zero and Y-axis vectors, plus the canonical SetPosition, SetAxesPtr and
SteeringControl::SetAngle providers. Its obsolete sample and bindings have
been removed, and the physics target and provider regression use canonical
source. The legacy address `0x005778c8` is Y axis despite its earlier provisional
zero-vector name; both relevant initializer bodies are independently strict.

VehicleMethods has only `__fltused`, SoultreeObject::GetAxesIn and
SoultreeObject::SetAxesIn undefined. GetAxesIn is canonical; SetAxesIn is supplied
by the reviewed nonexact `samples/physics/helpers/SoultreeMatrix.cpp` candidate.
VehicleReset has only `__fltused` and its three existing direct methods
undefined. Each object retains four ordinary header-local vector constants and
initializer entries. No original TU ownership or linked initializer order is
claimed for these reconstruction groups.

Bike's own zero-vector reads use its existing header-local `kVec3Zero`.
Its strict 60-byte initializer at `0x00407890` establishes that storage's
retail destination at `0x005778a8`; this does not consolidate private constants
across production objects.

`tests/test_vehicle_method_providers.py` checks retail RTTI/COL and each target
slot, actual production vtable relocations and all fourteen complete strict
bodies. `tests/test_vehicle_reset.py` additionally checks slot 36 in all three
primary tables, the real direct providers, zero/Y-axis initialization, and
6,144 native reset state/callback/exception cases with explicit helper boundaries.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
$env:PYTHONPATH = '.'
python -m unittest discover -s tests -p test_vehicle_method_providers.py -v
python -m unittest discover -s tests -p test_vehicle_reset.py -v
```
