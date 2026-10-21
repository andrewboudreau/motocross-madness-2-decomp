# Immediate joystick state

## Identity and complete extent

`PCJoystickDevice::ReadImmediateState` is called at `0x004c39d9` by the
independently strict 172-byte `PCJoystickDevice::Update` at `0x004c3960`.
The retail reader occupies **454 bytes**, `0x004c3790` through `0x004c3955`.
Its three `ret 4` instructions begin at `0x004c37aa`, `0x004c37c5` and
`0x004c3953`; ten following NOPs precede Update. The former 452-byte sample
extent cut the last return instruction. Complete decoding finds only internal
branch destinations, seven direct calls and one indirect COM call, with no
SEH registration or cleanup funclet. No registered calibration target used the
old extent.

Retail RTTI records PCJoystickDevice, JoystickDevice, PCInputDevice and
InputDevice with zero-offset, nonvirtual PMDs. The source's inherited helper
calls therefore use the same primary pointer. This does not establish original
translation-unit ownership.

The standalone [candidate](../samples/inputdevice/JoystickImmediateState.cpp)
is extracted unchanged from PCJoystickDeviceNearMisses.cpp. Authentic VC6 SP3
`vc6_o2_mt` emits **453 bytes, 373 matching positions** against the complete
454-byte reference. It remains nonexact source in samples.

## Behavior and existing providers

A null device returns zero. COM slot 9 receives the device pointer, size 80
and a local DIJOYSTATE-shaped output buffer. A negative HRESULT returns zero;
any nonnegative HRESULT consumes that complete output. The reader does not
initialize the output before the call.

Except for subtype 4, the reader converts six signed axes to floats, stores
the unflipped values at +0x4e4 and feeds the binding lists. Axis 1 is optionally
flipped around its maximum; axis 2 is always flipped. The button loop stores
each high-bit state before comparing its previous value, clears the matching
press or release timestamp pair, and queues changes in ascending button order.
Each event reloads the current global game/control pointer. Four POV DWORDs
are copied without interpretation. The argument is unused.

| Dependency | Retail VA | Complete strict extent |
| --- | --- | --- |
| JoystickDevice::FeedAxis | 0x00489c00 | 84 bytes |
| UnknownControlBinding::SetFromAxis | 0x0043cd20 | 104 bytes |
| ControlInterface::QueueEvent | 0x0043cea0 | 82 bytes |
| PCJoystickDevice::Update caller | 0x004c3960 | 172 bytes |

The native test independently proves these canonical bodies with relocations
resolved. The separately compiled candidate imports only FeedAxis, QueueEvent,
the existing `g_TrackGame` pointer and CRT `__fltused`; it has no static
initializer or EH data. SetFromAxis is a transitive dependency of FeedAxis.
The test also proves the exact retail Update-to-reader call site. No new
provider or alias is needed for these dependencies.

## Functional build review

[The retained test](../tests/test_joystick_immediate_behavior.py) compiles the
whole standalone candidate, the original aggregate and a private native
fixture with authentic VC6. It pins the retail image SHA-256, decodes the
complete extent and checks all seven direct sites plus the slot-9 COM site.
The candidate and aggregate reader have identical complete bytes and named
relocations. Layout probes use the actual headers: device +0x25c, index +0x260,
32 button records +0x264, axis values +0x4e4, binding lists +0x4fc, maxima +0x58c,
POV +0x5d8, total object size 0x5ec, and game control pointer +0x14.

**5,014 native source/reference scenarios pass.** Both sides execute the real
strict FeedAxis, SetFromAxis and QueueEvent source bodies. Observing wrappers
record entry/exit state and permit callback mutations or exceptions; their
compiled helper bodies are separately checked against their complete retail
extents. Only the private reference's seven verified E8 call operands are
redirected to these wrappers. COM uses a scripted device vtable on both sides.
No retail image or dispatch is used by production source.

Comparisons cover the complete joystick, two game/control objects, binding
objects/lists, input bytes, global pointer identity, ordered event arguments,
return value, FP status and failure state at every scripted boundary and exit.
Cases exercise all subtype bytes, positive and negative HRESULTs, all button
byte values across previous states, signed-axis precision boundaries, four
rounding modes, zero/negative/infinite/NaN maxima, optional direction flips,
aliased bindings, nonpositive list counts, arbitrary POV bit patterns and
bounded queue occupancy. Callbacks change the current game, control pointer,
index, subtype, flip bit, later maxima, later buttons, binding lists or queue
count. Every reached boundary is tested with an exception; invalid pointers
also compare AV read/write kind and address. Independent assertions check
axis/binding results, all 32 ordered events and timestamp pairs, gamepad bypass,
unchanged-button suppression, ignored argument, flipped-axis storage, null
device and failed-COM short circuits even with a null game pointer.

The COM contract is deliberately bounded: on success it writes all 80 output
bytes, and on failure it leaves them untouched and unobserved. Success with an
unwritten or partially initialized output is not validated. Queue occupancy
stays within the declared 160-event storage, and positive binding counts stay
within allocated lists. The test does not assert out-of-bounds queue behavior,
real DirectInput acquisition, physical controller behavior or concurrent
mutation. Callback mutation/fault cases establish observable sequencing at the
scripted boundaries, not actual device scheduling. No exception cleanup is
invented; this function has no owned resources or EH registration.

The constructor and registry lookup default `BufferedJoystick` to 1. The
separate [buffered reader](JOYSTICK_BUFFERED_STATE.md) has its own reviewed
nonexact implementation. This review establishes only the immediate-input
operation; complete device startup remains unverified.

The real build `work/game-build/build-x_vk5ibp/report.json` compiles all 222
translation units. Adding this reader removes exactly its missing definition,
reducing unresolved symbols from 359 to 358 without adding any. Startup,
menus and racing remain unverified while linkage is incomplete.

## Extraction preservation and reproduction

Private `work/joystick30/check_extraction.py` and `extraction-proof.json`
compare the aggregate against commit `ffc9f7e5`: both complete functions
(immediate 453 bytes and buffered 1,641 bytes) and all three remaining nondebug
COFF sections retain their full bytes, characteristics and named relocations.
The one excluded string section on each side is explicitly identified as the
compiler's absolute `__FILE__` path: `_JoystickImmediateBaseline.cpp` versus
`PCJoystickDeviceNearMisses.cpp`. That difference arises solely from compiling
the private baseline under a different filename; production keeps the original
buffered reader's diagnostic path. No EH, initializer or other data section is
dropped from the comparison.

```powershell
$env:PYTHONPATH='.'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_joystick_immediate_behavior.py -v
```
