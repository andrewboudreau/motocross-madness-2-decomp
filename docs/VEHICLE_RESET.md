# Vehicle orientation reset

`src/reconstructed/VehicleReset.cpp` supplies the complete **368-byte**
`Vehicle::UnknownVirtualSlot36` at `0x0040c710`. The primary Vehicle, Bike and
KrustyBike tables at `0x00558a84`, `0x00550900` and `0x00554548` each select this
body in slot 36. Their RTTI object offset is zero; secondary tables are not
used to infer this method identity. The semantic reset name and new source
grouping remain inferred rather than original TU ownership.

When the facing vector has no horizontal component, the method derives its
horizontal direction from the previous up vector and the sign of facing Y.
It writes Y axis to the up vector, clears facing Y, invokes virtual slots 35
and 34 in that order, resets roll/pitch and copies the current orientation
into saved fields. It then resets the steering node's position and axes and
requests steering angle zero in the current pose frame. Fields and pointers
can be changed by callbacks; the original reloads and ordering are retained.

The old sample used two unsupplied global views. `0x005778a8` is the zero
vector; **`0x005778c8` is Y axis**, despite its provisional zero-vector name.
The promoted body uses `kVec3Zero` and `kVec3YAxis`, already present in the
sample through the ordinary Math3D header. Their complete 60-byte initializer
bodies match retail `0x00407890` and `0x00407930`. Native CRT startup supplies
the values before the reset test runs. Four private constants and four normal
initializer routes remain in this object; their extra copies do not establish
original TU grouping or the retail executable's full initialization order.

The obsolete VehSceneNodeView::SetAxesPtr call now uses the actual primary
SoultreeObject pointer. The full reset bytes and all 15 old sample sections
are preserved under the two storage joins and one method-name join. There
is no added wrapper or copied machine code. Its only external requirements
are `__fltused` and these three existing complete strict methods:

| Provider | Retail VA | Bytes |
| --- | --- | ---: |
| SoultreeObject::SetPosition(const Vec3&) | `0x004fc660` | 38 |
| SoultreeObject::SetAxesPtr | `0x004fbd70` | 733 |
| SteeringControl::SetAngle | `0x00504e20` | 150 |

The retained test recompiles those actual providers, verifies their complete
strict bytes and all three reset call relocations, and checks both virtual
call sites. The physics target and vehicle method regression now use canonical
source; the duplicate sample and obsolete bindings are removed.

The native fixture links the actual production reset object and runs **6,144
cases** over prepared Vehicle, node and steering storage. It observes whole
storage and guard bytes at all five call boundaries and at completion. Inputs
include positive/negative and signed-zero horizontal components, both vertical
signs, subnormal and nonfinite vertical values, and finite previous-up vectors.
Expected horizontal signs use bit operations independently of the production
multiplication. Checks cover reset scalars, saved field copies, argument values,
exact field-output addresses, node/steering/pose replacement between calls,
callback field mutation and exceptions at every boundary. Late callback writes
must remain visible; the method must not repeat saved-field copies afterward.

Virtual slots 35/34 and the three direct helper calls are scripted in this
fixture. The helpers' actual complete bodies are separately strict; the native
fixture does not execute their scene/matrix/steering closure. It establishes
reset state and call behavior for the prepared cases, not object construction,
full helper integration, arbitrary invalid pointers or complete vehicle physics.

The isolated real link `work/game-build/build-pbb3dlst` compiles 230 translation
units and removes exactly this missing method: **317 to 316 unresolved symbols**,
with no new missing names. Startup, menus and racing still require a successful
source build and runtime validation.
