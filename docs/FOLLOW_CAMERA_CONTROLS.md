# FollowCamera control handler

`samples/camera/FollowCameraControls.cpp` contains the unchanged complete
`FollowCamera::UnknownVirtualSlot45(bool, float)` candidate. The aggregate
`FollowCameraNearMisses.cpp` includes it, with the original input macros and
angle helpers shared through `FollowCameraControlSupport.h`. The slot name and
original translation-unit ownership remain provisional. No shared production
class declaration is changed.

## Complete extent and preserved source

FollowCamera's primary vtable at `0x0055282c` names `0x00463a30` in slot 45.
Complete retail decoding ends at `0x00464a0a`: **4,058 bytes**, including the
final `ret 8` at `0x00464a07`. Six NOPs separate it from the next function at
`0x00464a10`. Every branch reaches an instruction boundary inside the full
body. The older 4,055-byte estimate stopped immediately before that final
return and is not used as the behavioral extent.

The function makes **33 indirect calls**: 30 call sites for ControlInterface
slot 3 and three for the active joystick's slot 4. It has no direct calls,
local EH registration or cleanup funclet. All camera/vector arithmetic is
inside the compiled body and its inlined helpers.

VC6 SP3 emits **4,046 bytes**, with **927 matching positions** against the
full 4,058-byte retail body. The historical truncated comparison also reported
927 positions; the regression records that comparison separately. No function
extent is shortened to match the candidate.

The persistent test compares the complete standalone and aggregate body,
including named relocation sites, types and addends, and checks every emitted
floating-point constant against retail. The standalone object defines only
the handler, has no CRT initializer, and imports only `g_TrackGame` and the
normal CRT floating-point marker. The operation adds no new helper dependency.

The private extraction proof against `10b2cc52` preserves all five emitted
aggregate function bodies and all **64 nondebug sections**, including EH,
CRT, constants and writable storage. It resolves compiler-generated local
labels to section/offset destinations and explicitly normalizes the checked
diagnostic filenames used by the temporary baseline and aggregate. The new
control handler contains no diagnostic filename reference. Reproduction and
report remain in ignored files:
`work/game-build/check_follow_controls_extraction.py` and
`work/game-build/follow-controls-extraction-proof.json`.

## Functional build review

Inactive calls clear the settled flag and return false without accessing the
game's controls. Active non-override calls in camera states 3, 6 and 7 set
settled and return false. With override and target movement enabled, keyboard
and joystick input translate the cached target. Otherwise state 5 moves the
camera position, while the remaining states adjust pitch, yaw and follow
distance. Input follows the observed short-circuit order for alternate keys.

The handler retains the 0 and 6.5 target-distance guards, the change in turn
rate below distance 50, the minimum follow distance of 7, pitch clamps at
approximately ±1.55, and the single-step yaw wrap by approximately 2π.
Successful nonzero joystick axes return before the two joystick-button
queries. Override target movement deliberately still returns false; this is
the retail behavior, not a newly inferred success convention.

`tests/test_follow_camera_controls_behavior.py` runs **195,339 native
scenarios** against the hash-pinned complete retail body. Both standalone and
aggregate candidates use authentic VC6 SP3 `/O2`. The separate fixture uses
`/Od` because VC6's optimizer crashes on its callback setup. The production
candidate and mapped retail body are not patched or wrapped with replacement
implementations; virtual input tables are the only scripted dependencies.

The test compares all 0x384 camera bytes, all scripted game/control/joystick
storage, ordered callback arguments and receiver identity, callback-entry and
final state, return values, exception/fault details and final x87 status. Its
bounded storage views do not claim complete runtime objects for the external
input interfaces. Coverage includes:

- Every 10-bit input mask, all four override/moving-target combinations,
  camera states -1 through 8, and absent, failed, zero-axis and nonzero-axis
  joystick cases.
- Exceptions and mutations at every callback in representative complete
  branch traces. Mutations change camera parameters and branch fields,
  switch the global game/control pointer, replace or remove the active
  joystick, and clear the global game pointer.
- Distance and angular thresholds, signed zero, varied finite time steps,
  minimum/maximum subnormals, minimum normals, maximum finite values and
  masked arithmetic overflow, under all four x87 rounding modes. An additional
  16,384 deterministic finite cases vary the input masks and float bit patterns.
- Invalid game/control pointers and invalid control/joystick vtables, comparing
  read-fault addresses `0x14`, zero, `0x0c` and `0x10` respectively.

Independent assertions establish inactive and excluded-state behavior even
with an invalid global game pointer. Separate cases require an upward
state-5 movement to add exactly 300 and return true, while upward override
target movement adds exactly 300 and returns false. These assertions protect
against an inert or identically miswired comparison.

This finite evidence supports explicit **reviewed-nonexact** functional build
admission. No actual DirectInput devices, concurrent input updates, rendering
or complete camera lifecycle are exercised. NaN-payload equivalence for
arbitrary input states is not established. The remaining constructor, camera
file loader and unrelated terrain-query declaration require their own evidence;
this operation does not supply or substitute them.

The isolated real build `build-3r0sggzf` compiles **204/204 translation units**.
Against `build-3bhn75px`, unresolved names decrease from **493 to 492**:
only `FollowCamera::UnknownVirtualSlot45(bool, float)` disappears, and no new
missing name appears. The executable still does not link; startup and live
camera operation remain unverified.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_follow_camera_controls_behavior.py -v
```
