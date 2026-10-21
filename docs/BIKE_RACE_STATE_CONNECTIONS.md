# BikeRace received-state and recorder connections

Seven production calls now use three existing KrustyBike implementations. The
methods retain their provisional names; no new implementation, thunk or alias
is introduced.

| Previous BikeRace view method | Existing provider | Retail extent | Sites |
|---|---|---|---:|
| `UnknownFunction495c00()` | `KrustyBike::Fn_00495C00()` | `0x00495c00`, 151 bytes | 1 |
| `UnknownFunction4933e0(message, state)` | `KrustyBike::Fn_004933E0(const KbNetDelta*, KbNetState*)` | `0x004933e0`, 634 bytes | 3 |
| `UnknownFunction496e20(recorder)` | `KrustyBike::Fn_00496E20(KbRecorder*)` | `0x00496e20`, 13 bytes | 3 |

The calls serve AnyRacerRemote, incoming network messages, race/replay restart
and recorded frames. Each provider is present in the actual production COFF
object and matches its complete retail extent with resolved relocations.
The isolated source build removed precisely these three missing names
(314 to 311 after the separate Bike placement connection); it still does not
link a complete game.

## Receiver and argument boundaries

CreateRacers has nine constructor routes through `0x0048fa60`, each using a
`0x1638`-byte allocation. Retail passes the allocator result unchanged as ECX;
the constructor preserves that value in ESI and returns it in EAX. Each branch
stores that primary pointer directly, or through unchanged EDI, into the local
racer pointer or the racer/AI arrays. The regression decodes these allocation,
constructor and store instructions and the seven consumer destinations.

KrustyBike's three COLs identify offsets zero, `0x21c` and `0x160c`. The latter
two are the Character and virtual GameObject subobjects; these calls use the
primary pointer at zero. Native layout checks also retain the receiver fields
at `0x735`, `0x11c8`, `0x1358` and `0x1388`. This proves these specific pointer
routes, not complete equivalence of the provisional racer declaration.

The message decoder consumes a packed `0x16`-byte prefix of the caller's
`0x17`-byte packet. Its signed byte deltas start at offsets 1, 5, 8 and 11;
the unsigned step byte is at 4 and the two unaligned dwords are at `0x0e` and
`0x12`. The caller's racer index at `0x16` is outside that prefix.
The decoder writes selected fields through `0x50` in the output's `0x54`-byte
prefix. The actual received record is `0x60` bytes: its state occupies `0x58`,
followed by flags and freshness at `0x58` and `0x5c`. These records and the
callers' rotation/freshness logic retain their existing types and behavior.

The recorder method only stores the raw pointer at receiver offset `0x13fc`.
The cast does not assert recorder class equivalence, ownership transfer, or
network/replay functionality beyond that store.

## Shared declarations and preservation

Five Core headers contain the unchanged actual class declarations for
SoultreePhysicsBaseObject, SoultreePhysicsCharacter, Vehicle, Bike and
KrustyBike. Vehicle/Bike collaborator views stay in their existing headers.
The embedded loader record and bike value records have shared headers.
Existing implementation headers retain their dependency ordering, while
BikeRace can include the class declarations without the conflicting rendering
Soultree declaration or extra Math3D per-unit constants. These are shared
canonical declarations, not alternative class layouts.

The private canonical caller/provider comparison preserved all 379 nondebug
sections and 2,030 relocations under the three external-name substitutions.
The composed build comparison independently preserved all 231 pre-existing
objects, 10,548 sections and 45,478 relocations under these three names plus
the separate Bike placement name. No new initializer or storage sections were
introduced. Full calibration and physics matrices retained every strict case.

The unadmitted BikeRaceNearMisses aggregate has one explicit exception. In its
complete 13,428-byte CreateRacers body, offsets `[538,545)` exchange independent
`XOR EDI,EDI` and `MOV EBX,1`. Both orders leave EDI zero, EBX one and the XOR
flags, without memory access. Independent full decoding found no interior
branch entry, label or relocation. All other body bytes and all 1,929 aggregate
relocation destinations are preserved; 205 of 206 nondebug sections are
unchanged. Its strict matching-position count changes from 13,396 to 13,390;
it remains a nonexact, unadmitted candidate.

| Complete CreateRacers fingerprint | Before | After |
|---|---|---|
| Raw body SHA-256 | `1a2b16687fa89e9a0ed33dba31a1e46452caaaefa3ad73ce630b317a32bf07c4` | `9c7483a4d5ff8514f8a350f528dbe11d8d82d4d694439f87d069fd6fbf9e748b` |
| Resolved body SHA-256 | `a6159f61caafc2347d11f9ef24dfc6956a1da5b180cd6749c63609a4716f8482` | `dfc0ee81775e2529319f4644466baacd436755506630b8d402b6621e29cea48c` |

Only this explained instruction schedule updates the previous resolved-body
pins. The race-event OnNetworkMessage pin also changes its named-relocation
fingerprint for the verified decoder-name substitution; its complete bytes and
resolved target are unchanged. Five previously missing ChatOverlay keys in the
aggregate bindings now match the existing production bindings, allowing the
expanded included-body checks. The aggregate retains its old recorder binding
for its own unadmitted call.

## Native verification

`test_race_state_connections.py` performs 160 strict body checks and five complete
partial fingerprints, plus both production and extracted execution objects for
all three providers. The native extraction uses unchanged source bodies and
normal VC6 `/O2`; its only undefined dependencies are the real CRT `_ftol` and
floating-point marker. No dependency is scripted.

The fixture runs 50,496 guarded cases: 768 null/non-null recorder stores, 576
remote/cell comparison cases, and 49,152 decoded packet cases. Packet cases
cover signed byte extrema, every unsigned step value, unaligned input,
independent output and both embedded received-record locations. Whole receiver,
packet and output storage is compared, including untouched bytes. Cell cases
cover zero, signed zero, fractional truncation, negative values and equal or
unequal integer coordinates. The arithmetic oracle alone uses `/Op` so its
explicit intermediate float rounding is honored; all three executed provider
bodies remain independently strict under the usual profile.

The fixture prepares raw data storage; it does not construct racers or test
their lifetimes, network transport, replay scheduling, arbitrary overlapping
buffers, or all floating-point exceptional modes. Finite bounded arithmetic
checks supplement complete provider byte proof; they are not a claim that
racing works.

```powershell
$env:PYTHONPATH = '.;tests'
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_race_state_connections test_character_motion_connections test_garage_preview_connections test_model_render_state_connections test_race_event_connections -v
```
