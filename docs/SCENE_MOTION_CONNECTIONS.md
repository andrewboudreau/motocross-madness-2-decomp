# Scene motion connections

`Scene::RestartEntry` now calls the existing `Character::SetMotion(Motion*)`
and `Character::AdvanceMotion(float,int,int)` implementations through explicit
primary-receiver casts. The provisional Scene entry and object declarations
retain their layouts. No provider body, virtual slot or object allocation changes.

| Boundary | Retail address | Complete extent |
|---|---:|---:|
| Scene::RestartEntry | `0x004eb040` | 280 bytes |
| Character::SetMotion | `0x004a8b40` | 27 bytes |
| Character::AdvanceMotion | `0x004a6bb0` | 1,293 bytes |
| AdvanceLooping | `0x004a56d0` | 121 bytes |

All four complete source bodies match retail strictly. The two retired external
names belong to `UnknownSceneObject`; the actual providers remain in
`src/krusty2/motion/Motnctrl.cpp`. No aliases or additional provider definitions
are introduced.

## Receiver and argument evidence

The retail animation reader at `0x004edfe0` constructs a CollisionCharacter
at call `0x004ee6de` and stores the constructor's primary result in entry `+4`
at `0x004ee704`. Entries have a `0x44` stride. Later, `FindMotion` at
`0x004eeaee` receives that primary pointer and its returned Motion pointer is
stored in the entry's motion array at `0x004eeb00`. This is decoded producer
evidence; the full 5,335-byte source animation reader remains nonexact.

CollisionCharacter's primary table `0x0055110c` has COL `0x0055af38`, offset
zero and raw type name `.?AVCollisionCharacter@@`. CollisionCharacter,
D3DIMSoultreeCharacter and Character have PMD `(0,-1,0)` in this hierarchy.
GameObject and BaseObject are virtual, PMD `(0,4,4)`; the separate GameObject
table `0x0055109c` has COL `0x0055aeb0`, offset `0x268`. The entry holds the
primary Character pointer, rather than this secondary view.

RestartEntry loads entry `+4` into EDI at `0x004eb08e`. The call at
`0x004eb0f1` passes the selected pointer from the motion array and restores
ECX from EDI. The call at `0x004eb108` uses the same receiver with binary32
time and two integer zeros. Its integer result is stored in entry `+0x18`.
SetMotion clears the stopped flag and blend flag, installs the Motion pointer
and reads its frame-list pointer at `+0xa4`. This establishes the particular
pointer and argument boundaries without equating all provisional class fields.

## Complete-object preservation

The canonical Scene object preserves all **272 nondebug sections and 1,273
relocations** under exactly the two external-name substitutions. The separate
SceneManagerNearMisses aggregate preserves all **96 sections and 484
relocations**, with no substitutions. That sample includes the shared header,
not the canonical CPP, and does not contain either retired call. Its source and
bindings need no changes.

Same-path before/after proof retains all bytes, relocation locations, kinds,
addends and resolved section/offset destinations, including all partial bodies.
The retained full-object pins normalize only complete recognized `__FILE__`
checkout prefixes and their compiler-generated string names for portability.
Local relocation labels retain the destination section and exact offset.
Private before/after evidence is `work/connections42/trial/proof.json`.

The focused regression checks **59 strict comparisons**: all 51 registered
Scene methods, plus independent full-TU and extracted-body checks for each of
the four bodies above. Each extracted function has identical complete bytes and
relocation records to its canonical definition before native execution. The
test also retains both direct call sites, constructor/store evidence and raw
RTTI. No partial comparison score or extent changes.

## Native validation

The native fixture runs the unchanged extracted RestartEntry, SetMotion,
AdvanceMotion and AdvanceLooping source bodies under authentic VC6. It links
the actual vector, cross-product and FastMath source dependencies. The retail
reference executes its own unchanged four bodies; its single looping-helper
CRT call is redirected, after checking the original target, to native VC6
`_ftol`. Candidate instructions and imports are not patched.

The test passes **62,215 cases**: 62,208 matrix cases and seven independently
asserted outcomes. The matrix varies eight seeded states, three entry indices,
three sequence lengths, six receiver/motion scenarios, two force values, four
requested sequence positions, six finite times and three callback modes.
Frame counts are 1, 3 and 5; pose counts are 0 through 3. It covers looping and
nonlooping motion, time clamping, stopped/forced restart, procedural entries,
missing animation tables, null primary receivers and null motion pointers.
Selected x87 rounding/precision combinations run with FP exceptions masked.

Only virtual pose application and procedural Tick are scripted. At every
callback the fixture observes the full prepared Scene, primary receiver,
procedural object, entries, Motion records, frame lists, poses and guard bytes.
Callbacks either observe, mutate a bounded field or raise a labeled exception.
Final state, return, callback order/payload, access-fault kind/address and FP
status/control are compared. Independent outcomes check motion installation,
frame result, stopped-state behavior, the procedural path and null-motion
partial writes before the actual fault.

These are prepared raw objects, not constructed Scene assets or a running
animation session. SetMotion clears blending, so this fixture does not exercise
the blend branch or prove its vector calculations. It does not establish
unmasked FP traps, transient x87 register-stack context, arbitrary invalid
indices or all malformed asset behavior. It does not execute the complete
animation reader or replace the remaining scene/runtime dependencies.

The isolated real build `work/game-build/build-eoo9_oaw` compiles all 240
translation units and removes exactly the two obsolete names, **280 to 278**,
with no added unresolved symbols. The game still does not link.

```powershell
$env:PYTHONPATH='.;tests'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_scene_motion_connections -v
```
