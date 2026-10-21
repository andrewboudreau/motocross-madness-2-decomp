# Per-device button query

## Identity and complete extent

`PCControlInterface::IsDeviceButtonDown(int,int,int,int)` is the missing
primary slot 3 of the PCControlInterface vtable at `0x00555e2c`. Retail RTTI
identifies the class and its zero-offset, nonvirtual ControlInterface base.
The actual compiled PCControl object references the candidate's named symbol
in that slot. Callers include Game hotkeys, Camera and BikeRace, and the
reviewed FollowCamera tick/control sources.

The retail body at `0x004bf4f0` occupies **104 bytes**: 85 bytes of instructions,
three alignment bytes, then four switch-table DWORDs at `0x004bf548..0x004bf557`.
The table targets are `0x004bf500`, `0x004bf51b`, `0x004bf520`, `0x004bf520`.
All direct branches land within the decoded code; `ret 16` occurs at
`0x004bf518`, `0x004bf53d` and `0x004bf542`. Eight NOPs follow the table.
There is no EH registration, direct helper call, allocation or global access.

Kind 0 reads the keyboard pointer at +0x34; kind 1 reads the mouse at +0x30.
Both invoke device slot 5. Kinds 2 and 3 index the eight joystick pointers at
+0x10 and invoke device slot 2. The call passes the original control and
modifier values plus a null entry-output pointer, and forwards the complete
integer result. Null selected devices return zero. Other kinds return zero
before accessing the receiver. No joystick-index bounds check is present.

These target roles are independently checked against RTTI-bound primary tables:
PCKeyboardDevice `0x00555f58` slot 5, PCMouseDevice `0x00555f78` slot 5,
JoystickDevice `0x005541fc` slot 2 and PCJoystickDevice `0x00555ef8` slot 2.
The canonical source vtables select the same existing query implementations.

## Compilation context and preservation

The [standalone candidate](../samples/control/DeviceButtonQuery.cpp) contains
only the unchanged readable switch body and header dependencies. It emits
**124 bytes, 43 matching positions against the complete 104-byte retail body**
under authentic VC6 SP3 `vc6_o2_mt`. It remains nonexact source in samples.
Original translation-unit ownership is unproven.

The old ControlInterfaceNearMisses.cpp includes the complete canonical
ControlInterface.cpp before this function. That compilation context emits
**104 bytes, 76 matching positions**: keyboard/mouse tails share code, with
different block placement from retail. The standalone keeps those tails
separate. Its lower score is an explicit compilation-context difference;
the C++ function body was not changed or replaced by a wrapper.

The aggregate now includes the standalone source at the former body location.
Private `work/control-query32/check_extraction.py` and `extraction-proof.json`
compare against commit `b6655bd0`: **all 27 full function bodies and 35 nondebug
COFF sections are preserved**, including bytes, characteristics and named
relocation destinations/addends. Local compiler labels are normalized by
section/offset. There is no filename or data exception. The aggregate is a
historical validation context and must not be admitted to the functional build,
since doing so would duplicate canonical definitions.

The standalone contains one function and its switch data, has no unresolved
imports and has no CRT initializer or EH section. Its only runtime dependencies
are the selected device's virtual query implementation.

## Functional build review

[The retained test](../tests/test_device_button_query_behavior.py) separately
compiles the actual standalone, aggregate and private fixture with authentic
VC6. It pins the retail SHA-256, checks the complete extent/table and both
indirect call sites, and verifies the primary vtable identities and actual
source layouts. Production contains no retail mapping or dispatch.

**45,489 native source/reference scenarios pass.** The private reference body
is executed without call-site patches. Both sides use prepared device vtables
for the observed calls. The fixture compares the complete control, keyboard,
mouse, eight joystick and game storage, global-pointer identity, callback
arguments and before/after state, return value, exception code and access-
violation kind/address. Qualified source calls invoke the whole candidate;
the reference executes the complete retail entry.

Scripted cases cover all eight valid joystick indices, all 1,024 device-presence
masks with varied indices, invalid kind extremes, null selected devices,
signed control/modifier/result extremes, aliased devices, callback mutations
and callback exceptions. The callback must receive the selected receiver,
correct slot, unchanged integer arguments and null output pointer. Independent
assertions verify exact dispatch, arbitrary integer return forwarding, zero
returns on missing devices and unchanged whole storage when callbacks do not
mutate it. Invalid receiver/device/vtable pointers compare fault ordering;
invalid kinds also verify the early zero return with a null or invalid receiver.

The integration cases execute unchanged source bodies of the actual providers,
not simulated key or modifier results:

| Provider | Retail VA | Complete strict bytes |
| --- | --- | --- |
| PCKeyboardDevice::IsButtonDown | 0x004c4570 | 110 |
| PCMouseDevice::IsButtonDown | 0x004c4a90 | 125 |
| JoystickDevice::IsButtonDown | 0x00489980 | 145 |
| KeyboardDevice::ModifierHolds | 0x0048a240 | 139 |

Each canonical body and its separately compiled validation body matches its
complete retail extent strictly, with resolved relocations. The three private
query methods have distinct nonvirtual names solely for direct invocation by
the observing callback; the production definitions are unchanged. Integration
cases cover key/button states 0, 1 and 2, all 256 modifier-state bytes, thirteen
requested modifier masks, debug-key conditions, and keyboard-present/absent
rules. Independent expected modifier results supplement full state comparison.
The dispatcher always supplies a null entry, so the providers' optional entry-
copy branches are outside this integration path; their bytes remain covered by
strict comparison.

The normal joystick-index contract is bounded to 0..7 and real helper button
indices stay within their allocated device storage. No index clamping is added,
and this does not validate arbitrary out-of-bounds array access. Prepared raw
storage and scripted vtables do not establish a constructed-device lifecycle,
real DirectInput polling, concurrent callbacks or working gameplay. No cleanup
is invented for thrown callbacks or faults. The overall game still needs its
remaining providers and runtime validation.

The isolated real link in `work/game-build/build-w7963nyq` compiles all 226
translation units and removes this one unresolved name: 342 to 341, with no
new missing symbols. Source/header/configuration hashes bind that report to
the reviewed composition. Linking, startup, menus and racing remain incomplete.

```powershell
$env:PYTHONPATH='.'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_device_button_query_behavior.py -v
```
