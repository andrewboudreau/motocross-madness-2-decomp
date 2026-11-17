# FollowCamera per-frame update

`FollowCamera::Tick(float)` (slot 10) is in [FollowCamera.cpp](../src/reconstructed/FollowCamera.cpp) (retail `FollowCam.cpp`), in retail
order, with a `// Not exact` status comment. The behaviour test links a
validation unit named `FollowCamera.cpp` holding the file's vector statics,
the vector operators and Tick, all sliced from the canonical file.

## Retail extent and source preservation

The primary FollowCamera vtable at `0x0055282c` has `0x00465c20` in slot 10.
Full decoding of that function reaches `ret 4` at `0x00466a75`, ending at
`0x00466a78`: **3,672 bytes**. Eight alignment NOPs follow it. Every branch
destination is an instruction boundary inside the full extent. Sixteen direct
calls reach terrain sampling, Direct3DRM rotation, camera frame/FOV setters,
`FastSqrt`, debug allocation and the base camera tick. Other calls use the
decoded camera slots and ControlInterface slot 3. The function has no local
EH registration or allocation cleanup funclet.

Authentic VC6 SP3 emits 3,672 bytes with **3,659 matching positions**. The
remaining 13 bytes are the existing instruction-scheduling differences near
the second rotation argument setup and the state-5 square-root argument setup.
The candidate's arithmetic and control flow were not rewritten to improve the
score. The test checks the full extent and the exact set of differing offsets.

Forms that leave the 13 bytes in place (3659) or are worse: a pointer local for
either argument of the second `D3DRMVectorRotate` (3659), the first rotation's
result passed as the second one's argument (677), scaling through the returned
pointer (3553), a `yaw` local (497), a `float sum` local for the whole
square-root argument (966), and partial-sum locals, regrouped or reordered
terms in that argument (3659). Retail's two differences are both argument
scheduling: `lea ecx, [esp + N]` ahead of `push eax` in the second rotation,
and the square-root argument's `push ecx` after the two squares.

The regression compares the validation unit's complete Tick bytes and named
relocations, including addends, with the canonical unit's. Only the
`__FILE__` literal, identified by its contents, is named portably: its
mangled name encodes the unit's path.

The validation unit's eight axis initializers (wrappers and bodies
`0x00466f10`..`0x004670c0`, 5/60 bytes each pair) are strict. The regression
checks the four ordered CRT entries and every emitted four-byte floating
constant against its retail value.

## Functional build review

The operation places an unplaced camera, obtains heading and followed target,
handles crash-view restoration, and asks the virtual camera controls for the
desired aim and position. It allocates the six missing easing objects, updates
target and position easing, blends transitions, applies camera modes and FOV
limits, and invokes the base camera tick only when this camera was current at
entry. The state-5 distance expression intentionally adds **unsquared `dy`**
to the two squared horizontal deltas, as retail does. Mode restoration,
one-byte virtual results, global input-pointer reloads and the ordering of
allocation/ownership stores remain unchanged.

`tests/test_follow_camera_tick_behavior.py` compiles the validation unit
and actual `FastMath.cpp` with `/O2`. It separately compiles the
private fixture with `/Od` because VC6's optimizer crashes on that fixture's
large callback-table setup. Both `FastSqrt` and `InitFastSqrtTable` are checked
strict before the source implementations execute with the initialized table
on both sides. They are not numerical approximations supplied by the fixture.

**674,335 native scenarios** compare all camera bytes, all easing and allocated
storage, ordered callback arguments, callback-entry and final storage, return
values, access-violation details and final x87 status. Scripted render-target,
subject, game and control storage is compared in full within the declared
fixture bounds; this does not claim complete runtime objects for those views.
Coverage includes:

- State values -1 through 8, current/inactive cameras, all 512 tested flag
  combinations and all 64 virtual-result masks, with initial placement and
  allocation paths.
- Mutations and C++ exceptions at every callback in representative complete
  branch traces, including all six allocations. Mutations switch the global
  game/control pointer and the render target's current camera, exercising the
  observed reloads and the cached entry-time current-camera decision.
- All four x87 rounding modes; zero, negative, near-boundary and large finite
  time steps, easing rates, yaw, blend clock, distance and FOV inputs; and
  8,192 deterministic additional finite floating-point cases. Zero rates and
  resulting masked floating exceptions are retained and compared.
- Failure of each allocation and invalid render-context, FOV-easing,
  target-easing and global game pointers. Allocation failures preserve the
  retail null dereference and already-stored ownership state; no cleanup or
  error recovery is invented.

Independent assertions verify ordered complete traces for current and inactive
cameras, the six allocation pointers' Z/X/Y then X/Y/Z ownership order,
allocation canaries, final return and base-tick count. The private mapper checks
each original direct-call opcode/displacement before redirecting its dependency
call. It executes the complete Tick body; production contains no mapped-retail
dispatch or instrumentation.

Terrain `QueryGround`, Direct3DRM rotation, other camera methods, allocation and
virtual/input callbacks are scripted boundaries. In particular, this does not
validate or connect the still-separate `FollowCameraTerrainView::QueryGround`
declaration, nor validate the default virtual implementations or rendered
camera behavior. The rotation callback is a deterministic test transformation,
not a replacement Direct3DRM implementation. Finite full-state equivalence under
these boundary scripts supports explicit **reviewed-nonexact** build admission;
it does not establish end-to-end camera operation or whole-game readiness.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_follow_camera_tick_behavior.py -v
```
