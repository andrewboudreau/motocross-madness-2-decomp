# Bike rider-motion and local zero-vector connections

KrustyBike now calls the existing Character motion methods through the rider's
primary Character pointer. Three obsolete external names are removed across
41 production sites. The isolated source build compiles 222 translation units
and changes **358 to 355 unresolved names** without new unresolved names.

| Retired view | Existing provider | Retail VA / complete bytes |
|---|---|---|
| `KbA5C4::FindMotion` | `Character::FindMotion(const char*, int)` | `0x004a6b30` / 24 |
| `KbA5C4::SetMotion(int)` | `Character::SetMotion(Motion*)` | `0x004a8b40` / 27 |
| `KbA5C4::ApplyRestPose` | `Character::ApplyRestPose()` | `0x004a8b00` / 15 |

All three supplied bodies match strictly with relocations resolved. The
converted production sites are 37 FindMotion calls, three SetMotion calls and
one ApplyRestPose call. Their complete callers also match strictly:

| Caller | Retail VA / bytes | Converted sites |
|---|---|---:|
| Slot 97 | `0x004912d0` / 624 | 15 |
| Load | `0x0048fc80` / 5,384 | 23 |
| Slot 49 | `0x00496790` / 1,400 | 1 |
| Fn_0048D910 | `0x0048d910` / 123 | 1 |
| Fn_0048D990 | `0x0048d990` / 98 | 1 |

There were already 24 calls to these canonical methods in the same object;
the current total is 65. The included sample aggregate has one additional
SetMotion call in its partial slot 102, for a total of 66. No partial method
was promoted. The genuinely missing `KbA5C4::Fn_004A8BF0` and the provisional
field view remain unchanged.

## Receiver and handle boundaries

Bike::Load constructs D3DIMSoultreeCharacter and stores its primary pointer
at Bike `+0x5c4`. Existing Bike methods already pass this pointer to Character
methods. Retail RTTI places Character at nonvirtual offset zero within
D3DIMSoultreeCharacter; its virtual GameObject base is a separate subobject.
These calls use the primary pointer, with no GameObject conversion.

FindMotion returns a Motion pointer through the existing global manager. The
callers retain that pointer's 32 bits in their current integer handle arrays.
SetMotion now explicitly converts the handle back to Motion*. Its complete
body clears stopped, stores the pointer, loads the first frame through
Motion `+0xa4`, and clears blending. Missing motions retain the observed fault
after the first two stores; no speculative null guard was added.

ApplyRestPose passes the receiver's pose list at `+0x190` and mask zero to the
existing 59-byte ApplyPoseListSlot6 method. That method visits the 0x2c-byte
records and invokes primary virtual slot 6. It does not call through the
virtual GameObject base or substitute a guessed pose operation.

The three-provider trial preserves all **611 nondebug sections and 2,815
relocations** across KrustyBike, its sample aggregate and Motnctrl under only
the three verified external-name substitutions. Raw section bytes, flags,
relocation kinds/addends and defined section/offset targets remain unchanged.

## Reusing the existing local zero vector

Six production reads formerly imported `g_kbZeroVec`. They now use this
translation unit's already emitted `kVec3Zero`, declared by Math3D.h. No new
storage or initializer was introduced. The original local COFF symbol is
`_kVec3Zero`, storage class 3, in writable `.bss`. Its complete 60-byte
initializer at `0x004911a0` writes zero to `0x0067c348`, `0x0067c34c` and
`0x0067c350`. The five-byte thunk at `0x00491190` is the corresponding
`.CRT$XCU` entry. Both bodies match strictly; the exact relocation chain is
retained in the test.

This data connection preserves all **514 nondebug sections and 2,497
relocations** in the two caller objects. The one changed identity is the
former external reference becoming a reference to the existing local
section/offset. Initializer, CRT-registration and storage bytes are unchanged.
The isolated link after the separately reviewed UI-default additions changes
**348 to 347 unresolved names**, compiling 223 translation units.

The unadmitted sample slot 11 still reads its old provisional `g_kbZeroVec`
name. Its binding is deliberately retained, preserving that complete partial
candidate and comparison score. The production object no longer imports it.
No whole-class or translation-unit ownership claim follows from this local
data connection.

## Retained regression

`tests/test_bike_motion_connections.py` checks 189 strict bodies and ten complete
partial fingerprints, all 65 current production and 66 aggregate motion calls,
the primary Character relationship, and the zero-vector initializer/storage
chain. Four historical aggregate fingerprints were refreshed only after their
old digests were checked against the private baseline and complete byte/target
preservation was proved. Function extents and scores did not change.

The native fixture extracts unchanged current source and independently requires
strict matches for both the full production-TU and extracted bodies. It executes
the actual MotionManager::Find, FindMotion, SetMotionByName, SetMotion,
ApplyRestPose and ApplyPoseListSlot6 bodies. Only virtual pose application is
observed through a synthetic primary vtable; no direct motion helper is mocked.

The **4,928 native cases** comprise 3,840 lookup/state/missing-name cases,
576 rest-pose iterations and ordered callback checks, 256 direct pointer-handle
round trips, and 256 zero-vector branch cases. The last group executes the
complete strict 103-byte KrustyBike slot 76 body against its own normally
initialized TU-local constant, checking input/receiver/return guards. Its
opposite Bike branch is a fail-fast dependency outside that bounded test.
The test-only address accessor exposes the actual local constant, not another
zero-vector object.

The earlier character, text, network and gearbox regressions also pass on the
composed headers. These results do not establish a full animation lifecycle,
working rendering or a linked game.

```powershell
$env:PYTHONPATH = '.;tests'
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_bike_motion_connections test_character_motion_connections test_bike_text_connections test_bike_network_connections test_vehicle_gearbox_connections -v
```
