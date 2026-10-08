Candidate implementations for this area are preserved in `samples/physics/motion/`.
Shared headers stay here. See [physics validation](../../../docs/PHYSICS_VALIDATION.md).

# motion (Spheres.cpp, SteeringControl.cpp)

Validation: all 75 targets under `src/krusty2/motion/` pass strict VC6 SP3 comparison
(Spheres 14, SteeringControl 21, Motnctrl 40). In `samples/physics/motion/`, SurfaceMap.cpp
is strict exact for 6 of 6 targets and D3DIMSoultreeMotnctrl.cpp for 23 of 26; the rest are partial.

Wave 4. Both files are linked back to back in `0x504940..0x5051ec`; SurfaceMap and the keyboard hooks
that follow live in `samples/physics/motion/`.

## Spheres.cpp
- `__FILE__` string `D:\aardvark\VC\krusty2\Spheres.cpp` at `0x00574568`; one use (debug `new`, line 4) in the
  `$E` initialiser at `0x00504950`.
- Owns `0x00504940..0x00504b5c`: global `g_pSphereManager` initialiser, `SphereManager` ctor (`0x005049c0`),
  deleting dtor (slot 0, `0x005049f0`), dtor body (`0x00504a10`) and the four per-TU `Vec3` constant initialisers.
- RTTI `.?AVSphereManager@@` (COL `0x0055f290`), direct base GameObject, vtable `0x005581d8`, object size `0x418`.
  `char field_0x2c[0x3e8]; int field_0x414` are unnamed (no reads in this file).
- 14 strict exact, 0 partial.

## SteeringControl.cpp
- `__FILE__` string at `0x005745ac`; one use (debug `new`, line 0x17) in the ctor.
- Owns `0x00504b60..0x005051ec`: ctor, Release, SetAxisFromDirection, SetAngle, AddAngle, SetAxisFromPoints and
  eight `$E` pairs. Retail has two sets of the four `Vec3` constants here; `Math3D.h` supplies the first and
  `SteeringControl.cpp` declares the second locally (`kSecondVec3*`).
- SteeringControl has no RTTI (no virtuals). Layout: `node` +0 (SoultreeObject*), `angle` +4, `field_0x08`, `axis` +0xc.
- Promoted to `src/krusty2/motion/SteeringControl.cpp` with `SteeringControl.bindings.json`: 21 strict exact
  (5 methods + 16 `$E`). SetAxisFromPoints `0x00504d30` stays a near miss in
  `samples/physics/motion/SteeringControl.cpp` (y spilled to a stack slot in retail).

## Not attributed
The keyboard-hook functions `0x005053b0..0x00505480` and the `$E` at `0x00505480` have no `__FILE__` reference and
are not reconstructed.

## D3DIMSoultreeMotnctrl.cpp
- `__FILE__` string at `0x00568b70`; xrefs `0x4456e7..0x445feb` (slot 11, dtor core line 0xc4, slot 0, slot 9 lines
  0x116/0x11b, slot 2 line 0x12d). Bracket `0x44544a..0x447024`; the matched methods are the contiguous
  `D3DIMSoultreeCharacter` run `0x445460..0x4466d8`.
- `D3DIMSoultreeCharacter : Character`, Character : virtual GameObject. Primary vtable `0x551530` (12 slots), vbase vtable
  `0x5514c0` at object offset 532 (0x214), vtordisp at 0x210. The dtor core receives the GameObject vbase pointer.
- 15 exact (FillNodeNames, ctor `0x4455b0`, slots 0, 3, 5, 6, 8, 9, 10, dtor core, GameObject slots 4/5 and three
  vtordisp thunks), 3 partial: slot 4 98.63% (retail holds the 0x3c flag in ecx, ours in eax), slot 2 99.68% (one SIB
  base/index order byte; many spellings tried), slot 11 `0x445680` 39.6% (789 vs 788 bytes: retail keeps the registry
  entry in esi, reloads a2 from the stack, frame 0x308; ours caches a2 in esi, spills the entry, frame 0x30c).
- Local stand-in `ModelObject` is D3DIMSoultreeObject (vtable `0x5513ec`, size 0x2d8, ctor `0x43f160(int)`, slot 7
  `0x444560` takes a source object); helper classes ArchiveFile (0x134), SltFile (0x5c4), registry entry (owner +0x10)
  are local, tier 3.
- D3DIMSoultreeShadow `0x446840..` (its `__FILE__` xrefs `0x447024/0x447044` straddle the bracket end) is
  reconstructed in `src/krusty2/shadow/D3DIMSoultreeShadow.cpp`.

## Verified SphereManager slice

Spheres.cpp now has 14 strictly exact cases, including the shared slot-8 override
at `0x00462e30`. RTTI identifies its primary table `0x005581d8` and that slot;
the body calls GameObject slot 8 and returns the saved object pointer.

The manager initializer allocates `0x418` bytes, passes 1 to the constructor,
and writes the result at `0x00689f1c`. Its source-path literal is at `0x00574568`.
The EH stub `0x0054e524` loads FuncInfo `0x00563b88` (magic `0x19930520`).
The four independent initializer bodies write the zero/X/Y/Z float triples to
`0x00689f30`, `0x00689f40`, `0x00689f50`, `0x00689f20`; their thunks target the
bodies at `0x00504a30`, `0x00504a80`, `0x00504ad0`, `0x00504b20`.
Those decoded writes distinguish TU-local constants from similarly named
constants in other files. `Spheres.bindings.json` records this reviewed mapping.

## Motnctrl.cpp
Promoted to `src/krusty2/motion/Motnctrl.cpp` (types in `Motnctrl.h`, `Motnctrl.bindings.json`); the near misses
stay in `samples/physics/motion/Motnctrl.cpp`.
- `__FILE__` string `D:\aardvark\VC\krusty2\Motnctrl.cpp` at `0x0056e034`. Its xrefs run from `0x4a5659` (the
  MotionManager `$E` initialiser, line 9) to `0x4a9a87` (FreeMotion, line 0x5a7). The line numbers rise with the
  address. The link-order bracket is `0x4a5447..0x4aa36c`.
- Functions that reference the string carry `// owner: Motnctrl.cpp (__FILE__ 0x56e034)` (12 of them). The others carry
  `// owner: bracket only` (24): they are inside the bracket, between own-xref functions, with no string use.
- **Classes and layouts** (offsets are tier 1 from reads and writes; names are tier 3):
  - **MotionManager**: 0x18 bytes, global pointer at `0x0068512c`. It is a cache of `Motion` records (0xcc bytes)
    loaded from `.MOT` (binary) or `.VUE` (text) files and reference-counted at `+0xc8`.
  - **Motion frames**: `MotionPoseList {int; CharacterPose* poses; int count}` (0xc bytes).
  - **CharacterPose**: 0x2c bytes, laid out as `{u8 nodeIndex; int hasPose; Vec3 axisZ +8; Vec3 axisY +0x14;
    Vec3 position +0x20}`. It is declared in `MotionPose.h`.
  - **Character**: the playback state is at `+0x0c..+0x38`:
    - `+0x0c` finished
    - `+0x10` time
    - `+0x14` motionCount
    - `+0x18` currentMotion
    - `+0x1c` currentFrame
    - `+0x20` blendFromMotion
    - `+0x2c` blendFromTime
    - `+0x30` blendDuration
    - `+0x34` blendActive

    The paths are at `+0x3c/+0x8c/+0xdc/+0x12c`. `motions` is at `+0x180`, the `MotionPoseList` is at `+0x190` and
    `vutLoaded` is at `+0x19c`. These fields are named in `src/krusty2/motion/D3DIMSoultreeCharacter.h`.
- **Coverage**: 41 strict exact in `src/krusty2/motion/Motnctrl.cpp`, plus ClampFloat and 5 partials in the sample.
  - The exact targets are:
    - two `$E` initialisers
    - the pose comparator and the two frame-advance helpers
    - the MotionManager ctor, Release, Load, LoadMot and Find
    - the Character ctor and dtor core
    - Character slots 1, 4 and 8
    - LoadMotions, CaptureMotion, SortMotion, FindMotion and FindNode
    - ApplyPoseList (slots 4 and 6), SetMotion and SetMotionByName
    - both BlendToMotion overloads
    - FreeMotion
    - AdvanceMotion `0x4a6bb0` (the per-frame advance and blend; `lastFrame` / `lastFrameTime` at `+0x24/+0x28`).
      Retail keeps the smoothstep input on the FPU stack (`fld st0` / `fmul st1` twice) after a memory-homed clamp.
      Every plain float spelling reloads it from memory for each multiply, and every double-typed use (double
      parameter, `double d = x`, `(double)` cast, `const float&` into a double, pointer or reference helper, long
      double) keeps it on the stack through the clamp too. The one spelling that reproduces retail is a float copy
      through a double expression VC6 folds away, `float t = (float)(x * 1.0);` (also `(float)(x + 0.0)`), in
      `SmoothStep`.
    - the `.CRT$XCU` 188-191 `Vec3` set (`0x4a8940..0x4a8a7b`, 8 functions)
    - ClampFloat `0x4a8440` (sample; an out-of-line copy of an inline)
  - The partial targets are:
    - **LoadVue `0x4a5e40`, 48.08%**: the structure and calls are identical, but the registers are permuted (ebx/ebp and
      esi/edi).
    - **RotatePose `0x4a7dc0`, 90.66%**: the Vec3Normalize temporaries use a different stack slot.
    - **PoseRotation `0x4a7fd0`, 55.70%**: the inline budget differs.
    - **InterpolatePose `0x4a8470`, 19.40%**: VC6 inlines the first CrossProduct, where retail calls `0x515600`.
    - **Slot 7 `0x4a70c0`, 4.79%** (3330 vs 3327 bytes): the frame-accurate advance with pose extrapolation and
      blending. The call sequence is retail's (Vec3Normalize, the `operator+`/`operator-`/`operator*`/CrossProduct
      COMDATs inside the two pose-blend expansions, `_ftol`, FindNode, ClampFloat inlined) and the clamp/smoothstep
      region matches byte for byte, but VC6 distributes the inline budget differently: it calls
      `CharacterPose::CharacterPose` (`0x4a8930`) 3 then 2 times for the unused pose locals where retail calls it 2
      then 4 times (the count follows the number of unused `CharacterPose` locals in the blend helper: 4 gives 3/2,
      6 gives 5/4, 2 gives 1/0, 0 makes `Vec3::Vec3` be called at the first operator sites instead; declaring them
      before ClampFloat makes ClampFloat be called), and it inlines the `operator*`/`operator+` of the first
      extrapolation loop that retail calls (retail inlines only the `operator-` there, calling `Vec3::Vec3` for its
      result). The frame is 0x284 against retail's 0x2dc and every byte after the first expansion shifts. A
      call-view variant (helpers taking pointers) came closer in size (3182-3190 bytes) but lost the call sequence.
- **Inline budget**: VC6 spends its per-function inline budget breadth-first over call sites in source order. The
  calls inside inlined bodies are considered after all direct call sites. In these FPU helpers retail calls the
  out-of-line COMDAT copies of `Vec3::Vec3` (`0x404e60`), DotProduct (`0x40ae30`), `operator*` (`0x5015b0`),
  `operator+`/`operator-` (`0x421cb0`/`0x421d00`) and CrossProduct (`0x515600`) at specific sites. Moving the identity
  branch of PoseRotation last gave the biggest gain. Helper spellings and dummy preceding functions had no effect.
- **Not reconstructed**:
  - The functions after `0x4a9aa0`.
  - `0x4a8bf0`, `0x4a8c50` and `0x4a9050` (2138 bytes), which round with direct `fistp` instructions (no
    `_ftol` call): `__asm` in the original, which the readable-C++ rule excludes.
