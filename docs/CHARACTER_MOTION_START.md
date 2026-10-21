# Character motion start and current-rounding conversion

`samples/physics/motion/CharacterMotionStart.cpp` reconstructs the complete
`Character::Method_0x004a8bf0(int, float)` body at `0x004a8bf0`. The retail
extent is **81 bytes**, ending with `ret 8` at `0x4a8c3e`; fifteen NOPs separate
it from the next method at `0x4a8c50`. The method name remains provisional.
The source is a reviewed **nonexact** candidate: authentic VC6 SP3 emits a
93-byte method and a 368-byte local conversion helper with `/QIfist`.
No original translation-unit ownership or retail address is assigned to the
new local helper.

## Behavior and receiver

The method treats the integer handle as a Motion pointer. It reads
`frameCount` at `+0xa8`, subtracts one, multiplies by the phase argument,
subtracts `0.5f`, and stores/reloads the intermediate as binary32. Retail then
converts that value with signed-32 `FISTP` under the current rounding mode.
There is no clamping or null validation.

It stores the Motion pointer to `Character::currentMotion` (`+0x18`) before
reading the Motion frame-array pointer (`+0xa4`). The selected frame is a
12-byte MotionPoseList. It stores that pointer in `currentFrame` (`+0x1c`)
and invokes the existing `ApplyPoseListSlot4` at `0x4a8a80` with mask zero.
The complete 59-byte helper walks the list and calls virtual slot 4 for each
pose. Callback changes to the list can affect subsequent iterations.

The retained test reads the primary D3DIMSoultreeCharacter COL/type descriptor
and its Character base PMD directly from the pinned retail image. Character is
at offset zero; the virtual GameObject base is distinct. The native fixture
uses the actual source declarations on prepared raw storage. It does not run
constructors or establish complete object lifetime. The separate BikeA5C4 and
KbA5C4 call views are not renamed or equated with Character in this slice;
their receiver-production connections require their own caller proof.

## Why the conversion is explicit

VC6's ordinary cast through `__ftol` temporarily selects truncation, contrary
to the retail instruction's current-rounding behavior. It can also leave that
temporary mode active when a floating-point exception interrupts the helper.
Authentic `/QIfist` instead emits a direct **64-bit** integer store. That alone
is insufficient: signed-32 overflow must raise invalid even when a value fits
in signed 64 bits.

The local helper therefore computes the signed-32 result from the binary32
sign, exponent and significand, honoring nearest-even, down, up and chop.
The `/QIfist` conversion supplies floating-point exceptions only. Inputs
outside signed-32 range use quiet NaN for that conversion, preserving invalid
behavior rather than accepting the wider integer range. Exactly `-2^31`
remains valid. The result bits are copied to the destination without a second
floating-point conversion.

An additional retail detail matters when invalid is unmasked. The FISTP
**does not store its destination** in that case. Retail uses the former
`frameCount-1` temporary as that destination, and the pending exception may
permit later integer stores and helper calls before delivery. The candidate
initializes the destination to that old value and leaves it untouched on an
unmasked invalid conversion. It still executes the exception-producing cast.
This preserves the observed partial object state; it is not a stack-layout
adjustment.

Private failed controls are retained under `work/motion-start37/`. The initial
`__ftol` route changed the control word on unmasked inexact faults. A first
`/QIfist` candidate preserved control but wrote integer-indefinite after an
unmasked invalid conversion: with count zero and phase `+Inf`, its frame
pointer differed from retail's previous-index pointer. The explicit no-store
rule fixes that observed state difference. No failed control is admitted.

`/QIfist` applies only to this candidate source. It is a semantic compiler
requirement, not a change to the project's ordinary compiler profile. The
retained test rejects a `__ftol` import and decodes the emitted FISTP64. This
is compiler-specific x86 behavior, not a claim that C++ floating-to-integer
conversion of NaN is portable.

## Native and object evidence

`tests/test_character_motion_start.py` executes the actual standalone candidate
object. It extracts the unchanged ApplyPoseListSlot4 source solely to link a
small native executable, then requires both that extracted body and the full
canonical Motnctrl TU's body to match all 59 retail bytes and relocations.
The only scripted game dependency is virtual slot 4; no source math or frame
selection helper is mocked.

The fixture maps the hash-pinned private retail image only for comparison and
redirects its sole verified direct call to that same strict pose-list helper.
No copied code or retail dispatch is used in production. A second compilation
through the actual game builder's source-compilation path, include inventory
and `/QIfist` command must preserve **every nondebug section byte and relocation
identity** of the tested object, including the local conversion and ordinary
header-emitted vector initializers. Those private initializers do not establish
retail TU ownership. The candidate imports only the existing pose-list helper,
`_control87`, and the ordinary CRT floating-point marker.

**320,999 native scenarios** compare the guarded Character storage, Motion,
frame lists and their complete committed page, pose storage, ordered callback
payloads and callback-visible state, final floating-point status/control,
exception code, and access-violation kind/address:

- 34,560 combinations cover 18 frame counts, 20 phase bit patterns, all four
  rounding modes and three precision modes, and eight callback behaviors.
- 276,480 cases cover every combination of the six floating-point exception
  masks, the same rounding/precision modes, counts and phase patterns.
- 1,440 cases add negative counts, INT_MIN, INT_MAX and binary32 integer-precision
  boundaries. The emitted subtraction wraps as retail's DEC does.
- 8,192 deterministic finite raw phase cases broaden exponent/significand input.
- 320 cases exercise null and invalid Motion, frame-array and receiver pointers.
- Seven independent outcomes check nearest-even versus upward selection,
  actual pose order, callback list mutation and thrown exceptions, unmasked
  invalid destination retention, and before/after-write null faults.

The frame array resides in a committed page within a 64MB reserved guard
region. An arbitrary invalid index can nevertheless land on unrelated readable
memory elsewhere in the process. If the actual pose walker reaches the callback
with such a frame, the callback records only its arguments, receiver and frame
pointer, then raises a labeled scripted exception **before** dereferencing or
mutating that foreign frame. A separate callback cap raises after eight events.
These are bounded callback comparisons, not claims about real invalid-memory
gameplay behavior. The range check uses unsigned subtraction, with direct tests
for null, a wrapping high pointer, and both valid page boundaries.

The fixture resets the x87 state between scenarios because an unmasked fault
may leave stack entries unpopped. At a floating-point fault it compares the
control word and all status bits **except TOP**, the transient x87 register-stack
index. Floating-register contents, tags, instruction/data pointers, arbitrary
incoming x87 stacks and exception-resume behavior are not compared. Faults are
caught and execution is not resumed inside either body. These finite checks do
not establish universal exception-context or whole-game equivalence.

## Functional build review

The complete candidate is suitable for individual reviewed-nonexact admission
with the required `/QIfist` source flag and the limits above. Existing source
bodies, call sites, headers and matching targets are unchanged; only obsolete
comments claiming original inline-assembly provenance are corrected. The blend
method at `0x4a8c50` and later method at `0x4a9050` remain missing. Linking,
startup, full animation and racing remain separate validation gates.

```powershell
$env:PYTHONPATH='.;tests'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_character_motion_start.py -v
```
