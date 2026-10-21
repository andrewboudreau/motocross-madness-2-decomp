# Buffered joystick state

## Identity and complete extent

`PCJoystickDevice::Update` at `0x004c3960` calls the buffered reader at
`0x004c39d2`. The constructor and the `BufferedJoystick` registry setting both
default to buffered mode. The primary receiver has the zero-offset,
nonvirtual PCJoystickDevice/JoystickDevice/PCInputDevice/InputDevice base
relations independently present in retail RTTI.

The complete retail function at `0x004c3100` occupies **1,665 bytes**, including
its referenced switch data. Executable instructions end with `ret 4` at
`0x004c36f2..0x004c36f4`. The other returns begin at `0x004c311c`, `0x004c317d`
and `0x004c31ad`. All direct branches stay inside the decoded instruction range;
three indirect branches use the following tables:

| Dispatch | Pointer table | Byte remap | Bound |
| --- | --- | --- | --- |
| Gamepad axis | 0x004c36f8, 7 DWORDs | 0x004c3714, 21 bytes | offset <= 20 |
| Other axis | 0x004c372c, 7 DWORDs | 0x004c3748, 21 bytes | offset <= 20 |
| POV | 0x004c3760, 5 DWORDs | 0x004c3774, 13 bytes | offset - 32 <= 12 |

The final remap ends at `0x004c3780`; fifteen NOPs follow before the immediate
reader at `0x004c3790`. The former **1,652-byte** extent stopped at
`0x004c3773`, omitting that entire final remap. The corrected extent is derived
from the decoded bounds and table references, not from candidate length.
There is no SEH registration, owned allocation or cleanup funclet.

The unchanged [standalone candidate](../samples/inputdevice/JoystickBufferedState.cpp)
emits **1,641 bytes with 128 matching positions against 1,665 retail bytes**
under authentic VC6 SP3 `vc6_o2_mt`. Comparing against the former truncated
1,652-byte range also gave 128 positions; that old result is not a complete
function comparison. The source remains a nonexact candidate in samples.
Its original translation-unit ownership is unproven.

## Behavior and dependency closure

The reader requests up to sixteen 16-byte records using COM slot 10,
`GetDeviceData`, with flags zero. On either input-lost/not-acquired HRESULT
(`0x8007001e` or `0x8007000c`), it calls slot 7, `Acquire`, then retries once
on success. It reuses the first call's count without resetting it. A failed
Acquire reports the **original GetDeviceData HRESULT**, diagnostic source file
and line 317, then returns zero. Other negative results also return zero;
nonnegative results process a positive count and return one.

Every button record queues an event, including repeated unchanged states.
The previous press/release timestamp receives the old corresponding timestamp
before the new record timestamp is stored. Gamepad subtype 4 stores unsigned
DWORD axis values as floats. Other subtypes optionally reject spikes greater
than 10,000 when the previous raw value converts to the current filtered value.
X/Y then use the dead-zone helper; all six axes feed bindings with the existing
Y-option/Z-always direction rules. Raw and filtered values remain separate.
POV records copy their DWORD payload, and unrecognized offsets are ignored.

All imported game providers already exist; the retained test proves their
complete canonical and validation-helper bodies independently:

| Provider | Retail VA | Strict bytes |
| --- | --- | --- |
| ButtonIndexFromOffset | 0x004c2ef0 | 412 |
| ApplyAxisDeadZone | 0x004c3090 | 102 |
| UnknownReportError | 0x004bf6a0 | 978 |
| JoystickDevice::FeedAxis | 0x00489c00 | 84 |
| UnknownControlBinding::SetFromAxis | 0x0043cd20 | 104 |
| ControlInterface::QueueEvent | 0x0043cea0 | 82 |
| PCJoystickDevice::Update caller | 0x004c3960 | 172 |

The remaining imports are existing `g_TrackGame` and CRT `__ftol`/`__fltused`.
The fixture compares all **39 bytes** of its linked VC6 `__ftol` with the pinned
retail body at `0x00534a88..0x00534aae` before executing scenarios. Both sides
then execute that actual CRT implementation, including its rounding-control
save/restore and integer conversion. The candidate adds no initializer or EH
section and no extra game-owned dependency.

## Functional build review

[The retained native test](../tests/test_joystick_buffered_behavior.py) passes
**4,520 scenarios**. It compiles the complete standalone candidate separately
with authentic VC6 and proves its full bytes and relocation destinations equal
to the aggregate version. It pins the private image SHA-256, checks all nineteen
direct call sites, three COM sites, three indirect jumps and complete switch
maps, and independently verifies the actual Update caller and source layouts.

Both sides execute the real six game-helper bodies listed above. Observing
wrappers record arguments and whole state, inject callback changes or exceptions,
then call separately compiled unchanged helper source; those helper bodies are
strictly checked, rather than replaced by arithmetic or queue simulations.
Only the private retail reference's audited E8 operands are redirected to these
wrappers or the proven CRT conversion. Production source contains no retail
mapping or dispatch.

The fixture compares complete joystick, game/control, binding/list and scripted
record storage, current global identity, ordered boundary arguments, return
value, FP status and exception/fault state. Initialized local records and count
are compared while their stack lifetime is active, never after return or unwind.
Cases cover counts -2 through 16, success/positive/error HRESULTs, both retry
errors and failed acquisition, all low-byte offsets plus extreme unknown
offsets, repeated button records, timestamps near the DWORD limit, unsigned axis precision edges,
spike thresholds 9,999/10,000/10,001, current/previous equality, four rounding
modes, zero/negative/infinite/NaN scales and maxima, aliased bindings and bounded
queue occupancy. Callback changes include the device, global game/control,
index, subtype, later records and timestamps, count, axis settings and bindings.
Every reached boundary in normal, gamepad, successful-retry and failed-Acquire
paths is tested with a thrown exception. Invalid receiver dependencies also
compare access-violation kind and address. There is no invented cleanup.

Independent assertions check normal axis/binding/POV values, ordered queued
records and both timestamp pairs, unsigned gamepad conversion, sixteen repeated
presses, spike acceptance versus rejection, ignored offsets, and exact
retry/report order and original HRESULT. These supplement source/reference
comparison so identical fixture behavior cannot alone establish the result.

The COM contract is finite: successful calls initialize every returned record;
returned positive counts never exceed sixteen or the supplied capacity. The
script clamps a configured success count to the caller's reduced capacity.
Negative counts exercise the finite zero-iteration branch, with no record reads.
The fixture does not validate oversized counts, success with unwritten records,
queue overflow, physical DirectInput devices, real acquisition or concurrent
scheduling. Callback changes establish sequencing at explicit boundaries only.

The diagnostic filename differs intentionally. Retail passes
`D:\aardvark\VC\krusty2\PCInputDeviceType.cpp`; standalone source passes its
actual build path ending in `JoystickBufferedState.cpp`. The fixture validates
each complete expected path before normalizing that one argument in the
transcript, then executes the real reporter, which formats a local diagnostic
buffer. It does not claim that those diagnostic strings are byte-identical.

## Aggregate preservation and reproduction

Private `work/joystick31/check_extraction.py` and `extraction-proof.json`
compare against commit `6503e588`: both complete aggregate functions (immediate
453 bytes, buffered 1,641 bytes) and all three other nondebug COFF sections retain
their bytes, characteristics and named relocation destinations/addends. Local
compiler label spellings are normalized by section and offset. Only explicitly
recognized `__FILE__` string storage is excluded: the temporary baseline path
ends in `_JoystickBufferedBaseline.cpp`, while the extracted body now names
`JoystickBufferedState.cpp`. Unlike the prior immediate-reader extraction,
this extraction changes the buffered body's diagnostic filename as described
above. No other data, switch table or code section is omitted.

```powershell
$env:PYTHONPATH='.'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_joystick_buffered_behavior.py -v
```

This is a bounded review of the buffered input operation. It does not establish
complete controller startup, a linked game or a tested race.

The real build `work/game-build/build-1vwzypi6/report.json` compiles all 224
translation units. Adding the reader removes exactly its missing definition,
**347 → 346** unresolved names, without adding any new ones. The full-source
link still fails, so startup, menus and racing remain unverified.
