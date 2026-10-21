# Followed-racer collision notifications

`TrackGameViewOwner::AnnounceFollowedRacer` now calls the existing
`KrustyBike::Fn_00496F90(KrustyBike*)` provider through explicit primary-pointer
casts. Three sites retire one provisional method spelling. There is no new
provider, storage, header declaration, wrapper, or type-wide alias.

## Receiver and call evidence

The complete 113-byte retail caller at `0x004a9f90` obtains each `other` pointer
from `BikeRace::NextBike` and makes the three notification calls at `0x4a9fc5`,
`0x4a9fd5`, and `0x4a9fe1`. ECX is the receiver, one unchanged pointer is on the
stack, and the existing notification provider ends with `ret 4`.

The retained raw checks cover all nine `KrustyBike` allocation/constructor paths
in `CreateRacers`: allocation size `0x1638`, constructor `0x48fa60`, unchanged
primary ECX returned in EAX, and direct stores into the local racer or racer/AI
arrays. Primary, character, and virtual GameObject COL offsets are respectively
0, `0x21c`, and `0x160c`; the conversion never selects either secondary address.

The switch at `0x4a9e80` passes its original two racer parameters unchanged to
Announce. Its two direct retail callers are a primary KrustyBike slot-28 method
and the GameObject-facing Tick implementation. The latter explicitly subtracts
`0x160c` before passing the previous racer. The picker independently stores the
same iterator/local/array primary pointers in the followed-racer field. These
are call-boundary claims; `TrackGameViewOwner` and `UnknownEventRacer` remain
provisional views rather than recovered complete class identities.

The provider uses the two primary CollisionObjects stored at KrustyBike
`+0x1414/+0x1418`, the primary Tire pointers in the front/rear wheel fields,
and Wrecker's primary ConstraintMethodCollisionModel pointer at `+0x38`.
Retained constructor/store and raw RTTI checks distinguish Tire's
CollisionObject at zero from its CollisionPoint at `+0xb8`, and the constraint
model's CollisionObject at zero from its GameObject at `+0xc`.

## Complete source preservation

The canonical FollowRacer object, sample aggregate, and standalone Picker
preserve all **22 nondebug sections and 59 relocation records** under exactly
one external method-name substitution. Section characteristics, payload/addend
bytes, local destinations, and all other symbols remain unchanged. Fresh live
and private-root compiles agree without filename normalization or exclusions.
The new include is the genuine existing `KrustyBikeCore.h` declaration.

The retained regression checks **20 complete strict comparisons**, three
whole-object pins, both unchanged sample partials, and the three actual
relocation destinations. Picker remains **342 candidate bytes / 67 matching
positions** against its complete 340-byte retail extent. SetFollowedRacer
remains **272 bytes / 259 positions**. Their previously pinned complete
resolved hashes are independently retained; this connection does not promote
or improve either partial implementation.

## Native evidence and limits

The fixture executes actual source bodies against independent state and fault
oracles. It does **not** map or execute a retail closure. Retail equivalence is
established separately by complete strict comparisons of both the full source
object and each unchanged function extracted for execution:

| Body | Retail address | Complete bytes |
| --- | --- | --- |
| AnnounceFollowedRacer | `0x4a9f90` | 113 |
| BikeRace::NextBike | `0x4204e0` | 162 |
| KrustyBike::Fn_00496F90 | `0x496f90` | 981 |
| CollisionObject::AddIgnoredOwner | `0x439410` | 118 |
| DebugRealloc | `0x4a2ec0` | 256 |
| DebugMemcpy | `0x4a3120` | 35 |

No game method is scripted or patched. The fixture supplies prepared raw object
storage and isolated tracker/global pointers, then links actual allocator
helpers to native CRT allocation/copy functions. It does not claim byte
identity for the CRT or run game constructors, event/network delivery, or
concurrent gameplay.

**20,432 cases** comprise 14,400 notification cases, 6,000 complete Announce /
NextBike cases, and 32 actual heap-growth/duplicate-owner cases. The independent
oracle models the 50 ordered ignore-list operations, including repeated owner
suppression and first-hole reuse. Self notification, shared colliders, shared
lists, preexisting entries, null receivers, null wreckers, null destination
colliders, and null list reads are covered. Every case compares complete
prepared world storage and guards; all **10,960 AVs** have independently
predicted read/write kind, address, and prior partial stores. Successful cases
must complete without an exception. The ordinary list matrix has enough
prepared holes, so it does not allocate.

The separate allocation matrix calls the real AddIgnoredOwner / DebugRealloc /
DebugMemcpy chain with CRT-owned buffers. It checks count, preserved entries,
appended owner, accounting, and complete receiver/guard bytes. Allocation
failure, arbitrary heap corruption, tag-stack lifetime, general pointer aliasing
into receiver fields, read-only/split-page faults, and allocator exception paths
are outside this finite matrix. Null-pointer behavior is an observed VC6/native
boundary, not a portable C++ guarantee. There is no floating-point arithmetic
in the tested notification chain.

Reproduce with authentic VC6 and the private image configured:

```
python -m unittest test_followed_racer_collisions -v
```

The actual build `work/game-build/build-wuy_ooiv` compiles 271/271 sources
and removes exactly the predicted name: **193 -> 192** unresolved symbols,
none added. All 271 previous objects preserve 10,833 nondebug sections and
46,129 relocations under the one name substitution, with no exclusions.
All previous calibration and physics results retain their full extents,
matching-position counts and strict status. The three affected tests pass.
Source-built startup, integrated menus and racing remain unverified.
