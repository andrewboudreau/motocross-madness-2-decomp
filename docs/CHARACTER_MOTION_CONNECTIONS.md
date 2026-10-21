# Character motion caller connections

Bike's rider-motion lookup and BikeRace/KrustyUI's named-motion selection now
call the existing Character implementations. Explicit casts occur at those
boundaries; the provisional caller views, fields and virtual declarations
remain unchanged. No new provider, wrapper, linker alias or shared class
declaration is introduced.

| Retired call view | Existing provider | Complete strict extent |
|---|---|---:|
| `BikeA5C4::FindMotion` | `Character::FindMotion`, `0x004a6b30` | 24 bytes |
| `UnknownBikeRaceCharacter::SetMotionByName` | `Character::SetMotionByName`, `0x004a8b10` | 33 bytes |
| `UnknownKrustyUICharacter::SetMotionByName` | Same Character provider | 33 bytes |

The Bike calls retain their motion names and lookup arguments. The canonical
lookup returns `Motion*`; an explicit pointer-to-int cast preserves the pointer
bits in Bike's existing 32-bit pose-handle fields. This is not a numeric motion
index or an ownership transfer. The flag character retains its `Stand`,
`FlagStart` and `FlagLoop` selections, and the garage previews retain `Spin`,
`WaitR` and `WaitB`. The BikeRace sample's setup call uses the same boundary.

## Receiver and provider evidence

The retail D3DIMSoultreeCharacter primary vtable is `0x00551530`. Its RTTI
hierarchy identifies Character as a nonvirtual base at offset zero; GameObject
is a distinct virtual base. The canonical C++ conversion from
D3DIMSoultreeCharacter to Character emits an unchanged-pointer return. None of
these calls passes the virtual GameObject pointer.

Bike's loader constructs D3DIMSoultreeCharacter at `0x004455b0` and retains the
primary result in its rider field at `+0x5c4`. The garage builder contains three
calls to that same constructor, for its garage, rider and bike previews. Race
setup constructs the flag character there too. The preserved complete callers
establish the actual receiver addresses and argument sequences; the provisional
view names alone are not evidence of complete class equivalence. Constructor
calls and the primary hierarchy are independently decoded from the pinned
retail image in the regression.

All providers are in `src/krusty2/motion/Motnctrl.cpp`:

- FindMotion forwards the name and integer argument to the global MotionManager.
- MotionManager::Find (`0x004a6290`, 63 strict bytes) searches its stored entries
  case-insensitively and returns the first match or null. Its second argument
  is unused.
- SetMotionByName looks up the name with argument one, then calls SetMotion.
- SetMotion (`0x004a8b40`, 27 strict bytes) clears `motionStopped`, stores the
  motion pointer, takes its first frame and clears `blendActive`.

The last two methods do not guard missing names. A null result is stored before
the frame-pointer read faults. The connection preserves that behavior; it does
not silently supply a fallback animation or reset unrelated timing fields.

## Preservation and native checks

`tests/test_character_motion_connections.py` passes **181 strict comparisons**
and preserves **12 complete resolved partial-function fingerprints**: seven
Bike methods and five BikeRace sample methods. These fingerprints include full
candidate lengths, matching-position counts and resolved byte hashes. They
guard unchanged partial reconstructions, not retail fidelity.

Terrain's canonical input-header dependency changes VC6 scheduling in the nonexact
`CreateRacers` sample: its 13,428-byte body now has 13,390 matching positions,
previously 13,396. At offset 538, `xor edi, edi` and `mov ebx, 1` exchange order;
all other bytes and all relocation destinations remain unchanged. Both orders
leave the same registers and flags, access no memory, and change no unwind
state. No decoded direct branch, symbol or relocated pointer enters the changed
window. The two caller regressions retain the complete updated fingerprint;
this sample remains nonexact, and no production body was changed to influence
its scheduling.

The regression compiles actual caller and provider translation units with VC6
SP3, checks canonical imports and complete provider extents, and rejects the
three retired imports. The native fixture executes **3,840 cases** covering
case-insensitive lookup, missing names, varying cache counts and unused lookup
arguments, null and non-null first-frame pointers, and all Character storage.
It verifies that unrelated bytes remain unchanged and that missing-name faults
leave exactly the writes that precede the failing frame read.

For this bounded execution test, the four unchanged function definitions are
extracted from the actual Motnctrl source into a temporary unit. Both the
production object and extracted execution object must independently match each
complete retail body and relocation. The native test uses the real extracted
MotionManager lookup and CRT string comparison, with a privately populated
cache; there are no scripted lookup/playback replacements. It does not exercise
resource loading, scene construction, frame advancement or rendered animation.

Private before/after object comparison in
`work/game-build/character-round19/object-preservation.json` preserves all
**674 non-debug sections and 3,787 relocations** across the three canonical
callers. KrustyUI's added Character header changes RTTI record order, so that
comparison associates sections and local targets by defined symbol identity;
their contents and referenced identities are unchanged. BikeRace also contains
the separately validated Terrain query rename in this composed comparison.
No instruction changes are hidden by those external-name substitutions.

The isolated real build `build-pclfp8jr` compiles **204/204 translation units**
and removes exactly these three unresolved names: **487 to 484**, with none
added. The game still does not link. Character's other missing pose methods,
the remaining rider render-view connections and actual animated gameplay are
outside this slice.

```powershell
$env:PYTHONPATH = '.'
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python tests/test_character_motion_connections.py -v
```
