# Bike and Wrecker connections

Bike's member at +0x604 now has the canonical `Wrecker*` type. The former
`BikeA604` storage view and `KbA604` method view are removed. `Bike.h` only
forward-declares Wrecker; the two production callers include its full header.
No adapter methods or additional Wrecker instances are introduced.

## Retail and ABI evidence

Bike slot 97 (`0x00409420`, 1516 bytes) allocates 0x53c bytes, calls the
Wrecker constructor `0x0052ff90` with 1, and calls `SetupWrecker`
`0x00530190` with its render context, rider character and `"rider.col"`.
Setup returns the Wrecker pointer, which the caller tests for nonnull before
appending its GameObject base. Retail RTTI places Wrecker's GraphicsTest,
GameObject and BaseObject bases at offset zero, all nonvirtual.

The caller's accessed Wrecker fields agree with the canonical declaration:

| Offset | Field |
|---|---|
| +0x34 | character |
| +0x38 | collisionModel |
| +0x44 | wreckState |
| +0xac | hadContact |
| +0xd8 / +0xdc | anyProbeHit / hitCenter |
| +0x11c | particles |
| +0x52c | skipProbeEffects |

KrustyBike slot 97 (`0x004912d0`, 624 bytes) registers four random, five
contact and six indexed animations, both wheels, and the rider's push box.
The canonical append APIs retain their existing integer storage for motion
handles. The explicit pointer-to-int casts preserve the x86 pointer bits;
they are not numeric conversions of animation indices. `AddIndexedAnimation`
returns an index that this caller ignores. Both old ignore-list overloads
join the same `AddIgnoredObject(CollisionObject*)` provider. The wheels are
the complete Tire objects selected from Vehicle's wheel list; retail Tire
RTTI places CollisionObject at offset zero.

Bike still uses explicit subordinate views for the rider's model at +0x1a0
and collision-model fields +0x64/+0xe4. This change does not establish their
complete declarations. The Vec3/Vector3 boundary retains the verified
12-byte, three-float layout: AddProbe and SetPushBox construct the canonical
argument type directly; the crash test reads hitCenter through its existing
Vec3 reference interface without copying or changing its bits.

## Validation and remaining scope

The connection removes 12 provisional missing names by joining 11 existing
providers: constructor, SetupWrecker, AddProbe, SetPushBox, three animation
appends, AddIgnoredObject, EndWreck, StartAnimatedWreck and StartTumble.
`tests/test_bike_wrecker_connections.py` compiles the real Bike, KrustyBike
and Wrecker translation units with VC6 SP3. It checks imports against actual
provider definitions, all 11 complete provider bodies with resolved
relocations, the complete Bike/KrustyBike setup bodies and Bike's 1374-byte
crash test. A native fixture checks the accessed offsets, size, return types
and zero-offset GameObject conversion against the retail evidence.

The focused production and affected near-miss run retains all 127 previously
strict production cases; its combined result is 129/147 strict, with no
required failures. Existing partial samples remain partial. This establishes
these source connections, not complete wreck simulation or working racing.
