# Rider motion-start connections

Bike's initial rider pose and KrustyBike's head-turn pose now call the existing
`Character::Method_0x004a8bf0(int, float)` implementation. Three related calls
in the KrustyBike near-miss sample use the same boundary. The provisional
`BikeA5C4` and `KbA5C4` views retain their declarations and layouts; only these
five calls cross explicitly to the primary Character receiver.

The provider remains the reviewed **nonexact** implementation described in
[CHARACTER_MOTION_START.md](CHARACTER_MOTION_START.md): 93 emitted bytes against
the complete 81-byte retail method, with its required per-source `/QIfist`
conversion behavior. This connection changes no provider, compiler flag,
initialization or target extent.

## Receiver and arguments

The complete retail Bike loader at `0x004079c0`, extent 6,745 bytes, constructs a
D3DIMSoultreeCharacter at `0x00407c6c`. It stores the returned primary pointer to
Bike `+0x5c4` at `0x00407c84`. The call at `0x0040930a` reloads that pointer into
ECX, pushes the rider motion handle from `+0x670`, and passes binary32 `0.5`.
The source loader remains nonexact; this decoded receiver evidence does not
promote the complete loader to strict status.

KrustyBike's complete 339-byte head-turn method at `0x0048e280` is strict. Its
call at `0x0048e3b5` uses the same rider pointer field, the handle at `+0x6a4`
and binary32 `0.5`. The following bike-character call separately adjusts the
containing Bike pointer by `+0x21c`; that adjustment is not applied to the
already-primary rider pointer.

The D3DIMSoultreeCharacter primary table is `0x00551530`, COL `0x0055b368`,
object offset zero. Its raw type descriptor is `.?AVD3DIMSoultreeCharacter@@`.
Character's PMD is `(0,-1,0)`. GameObject is virtual, PMD `(0,4,4)`, with its
separate table at `0x005514c0`, COL `0x0055b328`, object offset 532. These
identities establish the particular receiver conversion without equating all
provisional view fields or claiming original header ownership.

## Preservation and validation

Before and after compilation preserves all complete nondebug sections and
relocation locations, types, addends and destinations under only the two
retired motion-start names:

| Source object | Sections | Relocations |
|---|---:|---:|
| Bike | 242 | 1,089 |
| KrustyBike | 243 | 1,054 |
| KrustyBike near-miss sample | 271 | 1,443 |

The total is **756 sections and 3,586 relocations**. Actual same-checkout
before/after comparison excludes no filename bytes. The retained regression
normalizes only complete recognized checkout-relative `__FILE__` literals for
portability, and pins the full current objects. It also checks raw RTTI,
constructor/call instructions, all motion-start imports and the complete strict
head-turn body in both canonical and sample objects.

Two gearbox regression fingerprints include the retired external names. Their
updated fingerprints follow independent verification of all 23 affected
published/current partial bodies plus the complete-object comparison. Function
extents and retail comparison scores are unchanged. The existing gearbox suite
passes all 292 strict checks, 28 complete partial fingerprints and 48 native
boundary cases. The Character connection suite retains 181 strict checks,
12 partial fingerprints and 3,840 native lookup/state cases.

The actual unchanged motion-start provider separately passes **320,999** native
state, callback, rounding and exception cases with its actual strict pose-list
helper. The finite domains, scripted pose callback and exception-context limits
in its own document still apply. These tests do not execute a whole bike loader,
rider animation session or race.

The isolated real build `work/game-build/build-adso_zrp` compiles 240 translation
units and removes exactly two unresolved names, **282 to 280**, with none added.
The game still does not link. Private comparison evidence is in
`work/connections42/rider/proof.json` and `gearbox-pin-proof.json`.

```powershell
$env:PYTHONPATH='.;tests'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_rider_motion_start_connections test_character_motion_start test_character_motion_connections test_vehicle_gearbox_connections -v
```
