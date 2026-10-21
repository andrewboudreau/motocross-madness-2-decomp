# Character playback and VUE loading

## Functional build review

`samples/physics/motion/Motnctrl.cpp` is explicitly included as a complete,
reviewed nonexact candidate in `config/game_build.json`. It supplies the
missing `Character::CharacterVirtualSlot7` and `MotionManager::LoadVue`.
The existing canonical motion unit remains in `src/krusty2/motion/`.
The retail source path and loader diagnostic references support Motnctrl.cpp
ownership; the neighboring pose helpers have bracket evidence only. Semantic
names and reconstructed types remain provisional.

| Body | Retail VA | Retail bytes | Candidate bytes | Status |
| --- | --- | ---: | ---: | --- |
| LoadVue | `0x004a5e40` | 1090 | 1088 | Nonexact |
| Character slot 7 | `0x004a70c0` | 3327 | 3333 | Nonexact |
| RotatePose | `0x004a7dc0` | 523 | 523 | Nonexact |
| PoseRotation | `0x004a7fd0` | 1121 | 1121 | Nonexact |
| ClampFloat | `0x004a8440` | 44 | 44 | Strict exact |
| InterpolatePose | `0x004a8470` | 1205 | 1205 | Strict exact |
| CharacterPose constructor | `0x004a8930` | 3 | 3 | Strict exact |

Slot 7 advances the current motion, interpolates or extrapolates its poses,
applies them through virtual slot 4 or 5, and handles blend completion,
loop restart and stopped motion. The blend path preserves retail's repeated
interpolation call and the node translation override. Two native comparisons
exposed precision differences in the earlier candidate:

- The blend extrapolation must round the subtraction, both scales and addition
  separately. Existing `Vec3SubtractCall`, `Vec3ScaleCall` and `Vec3AddCall`
  declarations preserve the decoded call boundaries. Inlining the arithmetic
  produced a one-bit position difference even at 15 fps.
- Playback rotation retains the integer frame rate in x87 while translation
  receives its rounded float copy. An explicit double conversion preserves the
  integer above `2^24`; the earlier float conversion changed the rotation.
  The blend rotation also preserves its different `(angle * time) * fps` order.

`AxisAngleMatrix` now names the existing provider's `TreeMatrix4` signature.
The local matrix and provider both use sixteen consecutive floats; a narrow
pointer cast connects them without importing conflicting complete headers.
The three explicit vector call views also occur in canonical motion,
Soultree, Vehicle and BVH source. Their complete strict implementations now
live in `src/reconstructed/VectorCalls.cpp`; both native playback and pose
tests link that actual production object. See [the provider proof](VECTOR_CALLS.md).

## Native comparison

`tests/test_character_motion_behavior.py` compiles the actual standalone
candidate with VC6 SP3 `/O2 /GR /GX /MT /Z7` and checks its complete COFF extent
and three strict helper matches. It executes both the candidate and the
hash-pinned retail bodies in 835,584 scenarios, comparing complete Character,
motion, frame, input-pose and node-table state, the return value, and ordered
applied poses with Character state at each callback. Only the pose's three
uninitialized padding bytes are normalized.

Coverage includes zero through three poses in four-frame motions, stopped and
looping states, both application slots, translation flags, blend weights below,
inside and above the transition, interpolation and extrapolation, fractional
time steps, fps 1 through 120 at selected rates and 16777215/16777217/16777219.
Another 16,384 of those scenarios vary finite axes, positions and times with a
deterministic generator. Candidate pose helpers execute unchanged. External
math other than the explicit vector calls, frame advancement and restart use retail dependencies; application
slots are observers. Four decoded CRT call sites use native VC6 CRT functions.

`tests/test_character_pose_helpers.py` separately exercises 159,744 complete
state comparisons: 43,008 RotatePose, 9,216 PoseRotation and 107,520
InterpolatePose cases. It checks separate buffers, in-place rotation,
interpolation into either input, identical inputs, and axis/angle output
aliases into input storage. Finite random axes span zero through 1024 scale;
parameters include signed zero, negative angles and values on both sides of
the interpolation clamp boundaries. Input/output storage and guard bytes are
compared without normalization. Other external math dependencies still use retail.

`tests/test_motion_vue_behavior.py` checks the standalone 1088-byte loader and
its one-state C++ unwind metadata, then compares 33,280 scenarios. It observes
complete Motion, Character and manager storage, heap bytes and liveness, and
ordered stream/allocation/node-lookup calls. Cases include mixed-case text,
signed fractional and exponent values, y/z conversion, frame/transform counts,
known-node subsets, truncation, open failure, seek return failure, allocation
throw/null paths, read and node-lookup exceptions, and constructor unwind.
The native CRT parses both sides; streams, allocation and node lookup are
scripted. The test preserves the retail early-open-failure leak and checks
constructor placement-delete behavior. Unrelated dependencies fail immediately
if accidentally reached.

These are bounded differential tests of real compiled bodies, not proof for
every float bit pattern, malformed asset or hierarchy. They do not establish
working source-built animation, asset loading or racing. Private retail mapping
exists only in tests; production contains ordinary C++ and no retail dispatch.

```powershell
$env:PYTHONPATH = '.'
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_character_motion_behavior.py -v
python -m unittest discover -s tests -p test_character_pose_helpers.py -v
python -m unittest discover -s tests -p test_motion_vue_behavior.py -v
python tools/build_game.py -j6
```

The motion unit removes two missing names and introduces none. Its admission
and the later vector-provider integration preserve all prior strict matches;
both near-miss comparisons report zero drops. [Current game-build evidence](GAME_BUILD.md)
records the complete composition and remaining linker failures.
