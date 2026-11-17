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

The reader sits in [PCJoystickDevice.cpp](../src/reconstructed/PCJoystickDevice.cpp)
after the buffered reader, in retail order (retail `PCInputDeviceType.cpp`).
Authentic VC6 SP3 `vc6_o2_mt` emits **454 bytes, 453 matching positions**
against the complete 454-byte reference; it is not exact. The forms tried
are listed under [Forms tried](#forms-tried).

The one differing instruction is the release block's `pressed` argument:
retail pushes the zero register (`push ebp`), the candidate pushes eax. The
candidate assigns and copies the new state in one statement
(`int pressed = buttonStates[i].state = state.buttons[i] >> 7;`) and passes
that copy in the release block. The copy stays in eax while the arguments are
loaded, so the release block loads `joystickIndex` and `g_TrackGame` into
ecx/edx as retail does, and the press block and the POV copy follow in
retail's eax, ecx, edx cycle. With `pressed` stored by its own statement VC6
replaces it by 0 after the `== 0` test, whether the argument is written
`pressed` or `0`; eax is then free, every later scratch register is one step
behind and the function is one byte short (453 bytes, 373 matching). Retail
shows both effects at once, the propagated zero and the busy eax; no form
tried gives both.

Before the chained copy, none of these forms moved the register cycle:
- **Pressed value:** `/128`; an unsigned type; casts to uchar, int and uint;
  `(x & 0x80) >> 7`; and a `& 0x80` test (worse, 321).
- **Branches:** `if (pressed)` with the arms swapped (356); `!pressed`; one
  QueueEvent call taking `pressed ? 1 : 0` (341).
- **Previous state:** reading it before `pressed`; dropping the `previous`
  local (368); copying it from the member (369).
- **Stores:** chained timestamp zero stores in either order (369).
- **Locals:** `joystickIndex` and ControlInterface* locals.
- **Loop counters:** one shared `i` for both loops; `i`/`j` declared at the
  top.
- **Probes:** dead int locals at the loop head, in the release block or before
  the loop.
- **POV copy:** a struct copy (371) or unrolled assignments (460 bytes).
- **Axes:** chained axis assignments, for each axis and for all of them;
  unchaining axis 1 (109) or writing it in member form (116) is much worse.
- **With the chained copy:** the release or changed test on
  `buttonStates[i].state`; `!pressed`, `pressed < 1`, `pressed != 1`, a
  `switch`, `if (previous)`, `continue` on no change, a `changed` local; the
  argument as `pressed != 0`, `pressed ? 1 : 0`, `!!pressed`, `pressed & 1`
  (457 bytes), `pressed == 1` (461 bytes) or `buttonStates[i].state`; the press
  block passing `pressed` (433 bytes, one merged call); zero stores from an
  `int time = 0` local, chained, or storing `pressed` (452 matching); a
  ControlInterface* local at loop, changed-block or arm scope.

## Forms tried

453 of 454 (454 bytes): the new state is assigned and copied in one
statement (`int pressed = buttonStates[i].state = ...`) and the release
arm passes that copy, which keeps eax busy while the arguments are
loaded and gives retail's ecx/edx, eax/ecx rotation through the loop and
the POV copy. Left: retail pushes the zero register (`push ebp`) for that
argument where this pushes eax. A plain `pressed` local stored by its own
statement is replaced by 0 after the `== 0` test, so eax is free again
(373, whether the argument is `pressed` or 0); the chained copy is not.
Unchanged or worse (2026-11-13): a result local, a subtype local, the
previous/pressed order and either inlined, an unsigned char pressed
(309), joystick index and ControlInterface locals (function, loop or
arm scope), explicit POV stores (460 bytes), a shared loop index,
chained zero stores (369), swapped branches (356), `!pressed` and a
button pointer; and with the chained copy: the release test or the
changed test on `buttonStates[i].state`, `!pressed`, `pressed < 1`,
`pressed != 1`, a `switch`, `if (previous)`, `continue` on no change, a
`changed` local (`!=` or `^`), the argument as `pressed != 0`,
`pressed ? 1 : 0`, `!!pressed`, `pressed & 1` (457), `pressed == 1`
(461) or `buttonStates[i].state`, the press arm passing `pressed` (433,
one merged call), zero stores from an `int time = 0` local, chained, or
storing `pressed` (452).
Round 9 (2026-11-14): the scratch rotation runs on from the axis block
(its last temporary is edx) into the loop. Retail's release arm loads its
arguments into ecx, edx and the press arm into eax, ecx; with a literal 0
VC6 gives eax, ecx and edx, eax, one turn early (373). So retail spends
one more turn before the release arm, and the chained copy only imitates
that. All 373 or worse: the chained copy with a literal 0, the assignment
inside the condition, store then test the member, an empty else, the
subtype test inverted (`== 4`) or a goto around the axis block, `i` or
`axis` at function scope, static inline QueueButton helpers (three
shapes), inline ButtonDown helpers, a `continue` loop body, re-testing
`(state.buttons[i] >> 7) == 0` or `& 0x80` (321-326), a `changed` local
first (328), `if (previous)` (370), pressed before previous (372) and
the member as the argument (348).
Round 15, all 453 or worse (retail's rotation is ecx, edx in the release
arm, eax, ecx in the press arm and edx, eax, ecx, edx for the POV copies):
dead statements take no turn (dead int, member, global or parameter reads
before the loop, in it or in the release arm: 373); the axis block and/or
the button loop as static inline helpers 373; the tail duplicated with a
return in the subtype arm 16 (602 bytes); the two release-time stores
chained, the stored zero as the argument (`releaseTime = 0` or an `int
time` chain) and the times stored from `pressed` 371-372 (451 with
`pressed` as the argument); `pressed` spelled `(b & 0x80) >> 7`, `/ 128`,
through casts or a byte/unsigned/int temporary, reassigned, declared
before `previous` or with the declaration split 373; `previous != pressed`
372, xor/sub compares 332, a `changed` local 328; argument expressions on
the chained copy: `-pressed` 354, `<< 1`/`* 2` 348, `pressed | 0`/`+ 0`
453, `pressed - pressed`, `^` or a comparison 373; release tests `<= 0`
372/452, `< 1` 400/337, `0 == pressed`, `!(pressed != 0)`, `== false`
373/453, `(pressed & 1) == 0` 371/451.

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
| ControlBinding::SetFromAxis | 0x0043cd20 | 104 bytes |
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
(immediate 453 bytes at that commit and buffered 1,641 bytes) and all three remaining nondebug
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
