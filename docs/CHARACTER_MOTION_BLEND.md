# Character two-motion pose blending

`Character::Method_0x004a8c50(int, int, float, float)` is a reviewed-nonexact source implementation. The complete retail body is 1,011 bytes, ending with RET 16 at 0x004a9040..0x004a9042. The following 13 NOPs precede the next function at 0x004a9050. Every decoded branch stays inside that extent. Method naming and original translation-unit ownership remain provisional.

Authentic VC6 emits a 441-byte method, a 652-byte private first-interpolation helper, and the existing 368-byte conversion helper. Ordinary helper calls change code layout and stack allocation. No production source contains copied retail instructions or retail dispatch.

## Receiver and call boundary

The genuine Character declaration retains its virtual GameObject base. Raw D3DIMSoultreeCharacter primary COL/type descriptors place Character at nonvirtual offset zero; GameObject is a separate virtual-base view. Bike's rider producer calls the actual D3DIM constructor at 0x00407c6c and stores its returned pointer at Bike +0x5c4 at 0x00407c84. Calls at 0x004075b7 and 0x004075da pass that retained rider pointer and the embedded Character/D3DIM view at Bike +0x21c, respectively.

The narrow rider call cast is applied in Bike and its existing KrustyBike sample caller. This does not establish complete equivalence of BikeA5C4 and Character. The complete Bike caller remains nonexact.

VC6 probes pin Character size 0x1cc, Motion size 0xcc, frame-record size 12 and pose size 44, with pose fields at offsets 0/4/8/20/32. Character's member-pointer representation is 12 bytes. The fixture uses a separate four-byte zero-adjustment thiscall entry for the retail address, rather than copying a raw address into a genuine Character member pointer. The candidate itself uses the genuine class declaration.

## Operation and conversion

The method selects frame pairs using `(frameCount-1)*phase`, a binary32 spill, subtraction of 0.5, another binary32 spill and current-rounding signed FISTP32. It stores the first motion/current-frame fields, caches both frame pairs, interpolates each pose and blends their results with `1-weight`. The loop reloads currentFrame/count after every primary slot-4 callback. Cached frame pointers and subsequently reloaded pose pointers remain distinct. There are no argument clamps or handle checks before frame selection; the individual pose interpolation weights are clamped.

Both conversions reuse one integer destination. The second does not initialize its old destination from the second frame count. `CharacterMotionFloatToInt.h` extracts the already validated CharacterMotionStart helper unchanged: actual FISTP64 supplies FP exceptions, and ordinary integer logic supplies the current-rounding signed-32 result. `/QIfist` is required only for these two candidate translation units. Without it, the retained control catches an inexact exception before the partially updated Character state observed in retail.

The start candidate's complete 14-section/24-relocation object is unchanged after extraction; its existing 320,999-case regression still passes. FP exception status excludes x87 TOP bits. Full saved register-stack contents, instruction pointer identity and exception resumption are not claimed equivalent.

## Actual helper closure and separate dot repair

Retail's first interpolation expands through out-of-line vector operations. Replacing it with a third InterpolatePose call produces an observable PC24 mismatch. `BlendFirst` therefore retains the actual subtract, scale, add, normalize and cross providers. Its 22 direct calls are three subtractions, nine scales, six additions, two normalizations and two cross products. The final three add results are unused, but their six scales and three additions remain emitted; possible FP/read side effects are not deleted.

The second interpolation and final blend call the existing full 1,205-byte InterpolatePose. Its old header-inline DotProduct emitted 29 bytes with a different addition association. The retained PC24 control demonstrates the resulting mismatch despite the strict calling body. Interpolation-specific inline length/unit helpers now call the actual strict Vec3DotCall. Exactly two relocation destinations change in InterpolatePose; all instruction bytes and its full extent remain identical. Every other Motion aggregate section is unchanged, including the old inline DotProduct used elsewhere. This is not a general repair/acceptance claim for those other methods.

| Actual provider | Retail VA | Full bytes |
|---|---:|---:|
| Vec3 constructor | 0x00404e60 | 25 |
| Add / Subtract / Scale | 0x00421cb0 / 0x00421d00 / 0x005015b0 | 69 each |
| Dot call view | 0x0040ae30 | 29 |
| Cross call/value views | 0x00515600 | 91 |
| Vec3Normalize | 0x005087b0 | 150 |
| FastInvSqrt | 0x00460c00 | 107 |
| InitFastInvSqrtTable | 0x00460bb0 | 74 |
| InterpolatePose | 0x004a8470 | 1,205 |

Fourteen strict comparisons cover ten distinct target addresses, including repeated call/value and extracted/full-aggregate checks. The extracted pose object's four non-initializer COMDAT helpers are Vec3 construction, add, scale and cross; each complete body is checked. Vec3Normalize calls actual FastInvSqrt. No nonexact length/dot helper is substituted in the tested blend graph.

The fixture separately executes actual source and untouched retail table initializers and requires equality of all 128 table bytes. Both blend/helper graphs then execute without reference branch/call/table patches. Only the primary virtual application boundary is scripted.

Math3D header inclusion adds ordinary reconstructed-TU startup overhead: 60 bytes of aligned private vector BSS, 16 bytes of CRT entries and eight initializer bodies (5/60 bytes per pair). Their full bodies/relocations are compared with canonical Math3D's emission. This is not evidence of original TU ownership. The new candidate has 17 nondebug sections and 54 relocations. Every game import already has an actual source provider; the conversion also uses the existing native CRT control-word routine.

## Native evidence and limits

The retained native fixture compares 576,692 scenarios:

- 570,240 pose cases: eight prepared pose sets, both frame counts 1..6, 33 finite phases, five weights and twelve masked PC/RC combinations.
- 48 callback/alias cases: four alias arrangements and twelve modes covering current-frame/count changes, cached versus reloaded frame/pose pointers, current-motion replacement, deliberate throws and invalid pointer mutations. Completion/count or specific exception/access oracles accompany differential comparison.
- 180 progressive pose-boundary cases, four nonpositive pose-count cases, 72 finite weight-clamp cases and four progressive Character-output cases.
- 6,144 exception cases: all combinations of six exception masks, twelve PC/RC combinations and eight specified phase bit patterns. This does not cover arbitrary phases, NaN payloads, frame counts or FP inputs.

Independent oracles check 51,240 frame-boundary AVs, 176 progressive pose-read AVs and three Character write AVs, plus selected-frame state, no-extra-write behavior, callback counts and partial Character state for unmasked inexact/invalid conversions. Callback mutations and the exception matrix contain additional exceptions. Progressive read oracles use the decoded first failing load, including DWORD accesses starting beyond the first guard-page byte.

The full prepared World, both frame pages, callback records and Character guard page are compared. Captured temporary CharacterPose padding bytes +1..3 are normalized because neither body defines them; semantic fields remain compared. Independent pose values use seed-zero identity axes and binary-fraction positions. Varied finite sets receive differential checks, not a claimed independent normalization oracle. `/Od /Op` applies only to the fixture's independent float-store oracle; game sources/helpers retain their ordinary profiles.

Prepared storage is not a constructed object lifetime. The callback models application, not scene-tree updates or rider rendering. Aliases cover shared motion arguments, frame arrays and pose arrays, not arbitrary overlapping Character/Motion objects. Bounded callback counts exclude unbounded corrupted loops. Full exception register context, asynchronous mutation, allocation/cleanup and whole-game startup/racing are outside this experiment.

Three current-harness negative controls must fail at known comparisons: replace the first path with InterpolatePose; restore inline DotProduct; remove `/QIfist`. The first two fail specific PC24 pose comparisons, and the third fails the first unmasked conversion-state comparison.

## Whole objects and portable pins

Bike's full 242-section/1,089-relocation object and the KrustyBike sample's 271/1,443 object are preserved under the rider-call name alone. The Motion sample's 37/249 object changes only two InterpolatePose callee relocations. The start object's 14/24 contents remain identical. Data, CRT, EH and other methods remain intact; no extent is shortened.

Filename normalization enumerates only Bike.cpp, the actually emitted KrustyBike.cpp spelling and the Motion sample. Fresh actual-checkout baseline objects equal their private baseline views under those strings. All four changed objects plus the new candidate also compare completely across two independently compiled private roots. No arbitrary path collapsing, header wildcard, payload exclusion or discarded relocation is used.

Seven existing pin files receive narrow transitions after reproducing every old affected value from the accepted emitter composition, including its repaired Rider pin. Five whole-object fixtures, the gearbox named partials and the Rider test change only for the proven blend-call name. Resolved model fingerprints and 35 additional resolved Bike partial pins remain unchanged. The private composed run passes ten focused tests; it is not a whole-game validation run.

## Functional build review

The actual build compiles 271/271 translation units and removes exactly the
Character method and provisional rider name (195 to 193), with none added.
All 270 old objects preserve their complete sections and relocations under the
rider name except the two independently verified InterpolatePose dot-call
destinations. The new 17-section/54-relocation object equals the reviewed
private candidate. All source/header hashes, configuration and compiler flags
are checked in `work/game-build/round60-blend-link-proof.json`.

Calibration remains 3,023/3,034 strict and physics 1,086/1,162 strict, with all
prior scores and full extents unchanged. Existing Vehicle records in the two
shared fixture files are preserved; only reviewed Bike records are combined.
Three additional pins in the newer Vehicle/Soultree tests were reproduced from
actual link and test objects before their sole name transition. Six combined
affected tests pass at the actual checkout path. This is separate from the
private ten-suite proof above. The game still does not link; bounded native
proof does not establish complete startup, integrated animation or racing.

```powershell
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
$env:PYTHONPATH='.;tests'
python -m unittest test_character_motion_blend -v
```
