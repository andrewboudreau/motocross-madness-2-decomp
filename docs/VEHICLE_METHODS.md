# Vehicle primary-vtable methods

`src/reconstructed/VehicleMethods.cpp` supplies thirteen exact method bodies
previously present only in `samples/physics/vehicle/VehicleInlines.cpp`.
Their signatures and bodies are unchanged. All thirteen were unresolved in
the production build, referenced by Vehicle and its derived classes.

| Primary slot | Retail VA | Bytes | Behavior |
|---:|---:|---:|---|
| 23 | `0x0040caf0` | 38 | Tests airborne state and forward-speed threshold |
| 24 | `0x0040cb20` | 59 | Tests increasing throttle, crash state and speed |
| 25 | `0x0040cb60` | 59 | Tests decreasing throttle, crash state and speed |
| 34 | `0x0040c4c0` | 118 | Reads node axes and computes the side vector |
| 35 | `0x0040c540` | 130 | Writes node axes and computes the side vector |
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
as a strong inference. The new file is a reconstruction grouping, not a
recovered original translation unit. Semantic field names remain provisional.

Slot 36 remains an exact 368-byte sample. It needs the unsupplied
`g_VehZeroVec3_005778a8` and `g_VehZeroVec3_005778c8` globals and the separate
`VehSceneNodeView::SetAxesPtr` declaration. This promotion does not introduce
those dependencies or claim that this reset path is connected.

The promoted object has only `__fltused`, `SoultreeObject::GetAxesIn` and
`SoultreeObject::SetAxesIn` undefined. GetAxesIn is supplied; SetAxesIn is an
existing unresolved dependency whose body is still partial in
`samples/physics/helpers/SoultreeMatrix.cpp`. No new unresolved name is added.
The existing headers emit four private vector constants and initializer
entries. No retail ownership or original linked order is assigned to this
group's additional initializer copies; strict method proof is not whole-unit
identity.

Bike's zero-vector reads now reference its existing header-local `kVec3Zero`
instead of the undefined `g_BikeVec3_005778a8`. Its full 60-byte initializer
at `0x00407890` establishes that storage's retail destination at `0x005778a8`;
the existing binding and all 51 strict Bike checks remain unchanged in value
and extent. No vector storage or initializer is added. The provider regression
checks the initializer, local COFF storage and actual read relocations. This
does not consolidate the separate globals used by Vehicle's retained slot 36.

`tests/test_vehicle_method_providers.py` reads the verified retail RTTI/COL
and every target slot, compiles the actual Vehicle source and verifies the
primary-vtable relocations resolve to the new definitions, and checks all
fourteen complete function extents with resolved relocations under VC6 SP3
`/O2 /GR /GX /MT /Z7`. It also protects the remaining sample boundary and
checks undefined symbols and local initializer scope.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_vehicle_method_providers.py -v
```
