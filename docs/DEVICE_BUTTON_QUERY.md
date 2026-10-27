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

## Source and strict match

The body lives in canonical [PCControl.cpp](../src/reconstructed/PCControl.cpp)
between `SetDevicesAcquired` (`0x004bf490`) and `IsControlDown` (`0x004bf560`),
the gap its address occupies among that unit's other PCControlInterface
methods; the placement is by address. Compiled there under authentic VC6 SP3
`vc6_o2_mt`, it emits the complete **104-byte extent strictly exact** with
resolved relocations, including the switch table. Every other function in
PCControl.cpp keeps its bytes.

Each case returns its own zero. A `break` to a shared `return 0` places the
mouse load after the keyboard load (76 positions), where retail places it after
the keyboard tail's `ret` and jumps back into the shared call. The keyboard
and mouse calls share one tail only when the body is compiled inside
PCControl.cpp; alone, with only the headers, the same source keeps both tails
(108 bytes). The cause of that context difference is not isolated.

## Functional build review

[The retained test](../tests/test_device_button_query_behavior.py) compiles
PCControl.cpp and requires the strict match, then links the canonical body,
extracted alone, with the private fixture under authentic VC6. It pins the
retail SHA-256, checks the complete extent/table and both indirect call sites, and verifies the primary vtable identities and actual
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

The functional build compiles the body as part of PCControl.cpp; the former
`samples/control` candidate and its aggregate were removed.

```powershell
$env:PYTHONPATH='.'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_device_button_query_behavior.py -v
```
