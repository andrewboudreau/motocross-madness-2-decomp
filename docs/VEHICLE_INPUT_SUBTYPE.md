# Vehicle input subtype connection

Vehicle's steering binding now calls the existing `InputDevice::IsDeviceSubtype(int)` declaration. The obsolete `VehicleAxisSource::Method_004897E0` declaration is removed; the provisional prefix, fields, null guard and steering arithmetic remain unchanged. This connects one existing method and adds no provider, storage or initialization.

The canonical getter at `0x4897e0` is the complete 17-byte method: compare the DWORD at receiver +0x10 with the integer argument, return 0 or 1, and pop four argument bytes. The complete 209-byte Vehicle slot63 at `0x528d70` calls it at `0x528de3` with argument 4. Both caller and provider remain strict matches. The declared prefix at +0x0c is not enlarged to pretend that every VehicleAxisSource is a fully recovered InputDevice.

## Receiver evidence

Actual ControlInterface bindings contain an `InputDevice* owner` at +0 and a current value at +0x24. The complete joystick, keyboard and mouse binding producers store their receiver into that owner at `0x489a34`, `0x489f95` and `0x48a429`. The retained test decodes those stores and DeviceSetup::BindAxis's device loads and actual calls. Raw primary COLs, decorated type names and nonvirtual InputDevice base descriptors establish offset zero for all seven inspected input classes; this is not an inference from a shared method address.

TrackGame embeds steering, lean and throttle bindings at +0x3344, +0x3380 and +0x33bc. Their addresses propagate through KrustyBike::Load, Bike::LoadBike and Vehicle::LoadVehicle. The retained test checks the actual outgoing argument order, incoming stack slots after prologue adjustments, and stores into Vehicle +0x4f8, +0x4fc and +0x500. Slot63 follows the steering binding's owner pointer and passes 4 without adjusting the primary receiver. Real header layout assertions independently check the binding, interface and vehicle offsets.

## Preservation

The reviewed connection preserves all ten affected canonical and aggregate objects: 1,083 nondebug sections and 5,239 relocation records under exactly one external-name substitution:

```
?Method_004897E0@VehicleAxisSource@@QAEHH@Z
?IsDeviceSubtype@InputDevice@@QAEHH@Z
```

Same-path comparison excludes no filename sections. Separate fresh actual-root and mirror compilations validate the retained pins' explicit filename normalization. All 233 retained full-body strict checks pass. The complete existing LoadBike and KrustyBike loader bodies are covered by object equality, without claiming that their pre-existing partial retail matches are strict.

Three existing Vehicle whole-object pins (gameplay defaults, particle emitters and vector add-assign) change only through that external name. Each old pin was reproduced before deriving its replacement; all other records are unchanged. Six existing Vehicle gearbox partial fingerprints were independently rechecked and remain unchanged. The legacy vector add-assign serializer retains a public filename symbol spelling; its proof explicitly uses the independently compiled actual-root filename identity, and the retained test passes in the actual checkout.

## Native evidence and limits

`test_vehicle_input_subtype.py` links the actual complete InputDevice source object and executes its getter against the complete mapped retail getter. The retail image is hash-pinned and receives no patches or redirected helpers. The fixture verifies the four-byte member-pointer representation, return value, completion or exact read-fault address/type, and every byte of the prepared 4 KiB page after each call.

The finite matrix contains 11,264 cases: 8,192 successful calls and 3,072 independently modeled access faults. It varies 256 raw subtype values and four arguments, read-only/writable storage, an ordinary receiver, a final readable DWORD, an inaccessible unused receiver prefix, and a non-straddling unaligned DWORD. Fault cases cover null and subtype addresses immediately before or after the readable page. Every successful result independently equals the raw DWORD comparison; every fault leaves the return sentinel and whole page unchanged.

These are prepared raw-storage VC6 observations, not constructed input-device lifetimes or portable out-of-object pointer arithmetic. The fixture does not execute full Vehicle input polling, setup cleanup or gameplay, and does not claim split-access fault behavior. Those boundaries are instead supported by strict complete methods, raw producer tracing and unchanged full objects.

Reproduce on Windows with authentic VC6 SP3 and the private retail image:

```
python -m unittest tests.test_vehicle_input_subtype -v
```

Set `VC6_ROOT` and `MCM2_EXE` as described in TOOLCHAIN.md. The actual source build compiles 270/270 translation units and removes exactly this missing name (203 to 202), with none added. All 270 prior objects preserve 10,816 nondebug sections and 46,075 relocations under the one name, without filename exclusions. Calibration remains 3,023/3,034 strict and physics 1,086/1,162 strict, with every prior full extent and score unchanged. Six affected regression tests pass. Evidence is in `work/game-build/round59-input-link-proof.json`, `connections-round59-preservation.json` and `round59-input-retained-final.log`. The game still does not link; startup, integrated menus and racing remain unverified.
