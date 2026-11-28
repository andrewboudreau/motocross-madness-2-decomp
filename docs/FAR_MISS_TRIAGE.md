# Far near-miss triage

Read-only triage of the rows of [NEAR_MISS_INDEX.md](NEAR_MISS_INDEX.md) whose
score is more than 256 bytes short of the retail extent (92 functions, taken
on 2026-11-15). No source was changed. The purpose is a worklist: which of
these are worth another matching attempt, and whether any of them computes
something different from retail.

## Result

- **No behavioural difference was found.** Class counts: B 0, S 31, R 61, U 0.
  Every difference that looked behavioural in the feature comparison (call
  targets, stored immediates, compare constants, x87 operation counts) was
  read in the disassembly and turned out to be an equivalent encoding. This is
  confirmed for the instructions read and a strong inference for the rest of
  each function: the larger R rows were skimmed by hunk, not read end to end.
- All 92 are linked by the current game build. `tools/linked_accuracy.py`
  reports no unexpected or missing call for the 88 it scores; the four it does
  not score are in `Bike.obj` (0x00405190, 0x00406ae0, 0x00407440, 0x0040c000).
- 18 of the 92 have no differential emulation case (19 at triage time) (listed at the end). The
  cases that exist were not run in this pass.
- The index is stale for several rows. Three are now within 256 bytes and so
  no longer far misses: `0x0050dad0` (4913/5020), `0x004e6f80` (2056/2148), `0x0052a940` (3229/3254). Row 40, `0x0052d610` (VisibilityQuadTreeNode::Traverse), is now strict exact in
  `src/krusty2/visibility/VisibilityQuadTree.cpp`; its sample is gone. Rows marked "(masked)" in the index were scored on
  a short extent; the table gives the full-extent strict score.

## Method

1. Rows extracted from the index; each function compiled once with authentic
   VC6 SP3, one compile at a time.
2. `tools/rank_near_misses.py` gave the aligned instruction diff (raw, frame-
   and register-masked, mnemonic-only).
3. A register-independent feature comparison was computed per function:
   size, instruction count, frame size, conditional jump count, and multiset
   differences of call targets, stored immediates, compare constants and x87
   mnemonics.
4. Any function with a feature difference was opened at the differing hunks.
5. `tools/linked_accuracy.py` on the latest build gave the linked source and
   the linked masked score.
6. Scores in the table come from `tools/match_one.py -q` run on 2026-11-15.

Reproduce one row (source, VA and extent from the table):

```bash
export VC6_ROOT="$(pwd -W)/work/vc6sp3-disc" MCM2_EXE="$(pwd -W)/work/game/mcm2.exe" PYTHONPATH=.
python tools/match_one.py src/krusty2/vehicle/KrustyBike.cpp 0x492ad0 2315 -q
python tools/rank_near_misses.py --help
```

Classes:

- **B** behavioural: a different call target, constant, branch condition,
  missing or extra store, or operand order that changes the value.
- **S** structural: same behaviour, different shape (inline boundary, block
  layout, tail merging, frame size, an inline-asm helper in retail).
- **R** register, scheduling or frame-slot noise: same instructions in the
  same order up to register choice, slot offsets and local reordering.
- **U** not determined.

EV (expected value) is the byte gap times my estimate that another attempt
recovers most of it without breaking the compiler-gaming rule; it orders the
table. The estimate is provisional. Diagnoses that begin "Header:" repeat what
the source comment records and were not re-derived. Risk is gameplay risk if
the function were wrong, not a finding that it is.

## Ranked table

| # | VA | Function | Source | Score (strict) | Class | Risk | Linked by game build | EV | Diagnosis |
|---|---|---|---|---|---|---|---|---|---|
| 1 | `0x00493660` | KrustyBike::InterpolateNetworkState | src/krusty2/vehicle/KrustyBike.cpp:2056 | 2305/8558; index 2305/8545 | R | low | yes, same file (0.989) | 625 | 2260/2260 insns, frame and slots exact, same 53 calls and 235 stores; 44 masked diffs (`c` widened through its stack home, `(len+50)^2` store order, fld/fmul order). |
| 2 | `0x00492ad0` | KrustyBike::SendStateDelta | src/krusty2/vehicle/KrustyBike.cpp:1798 | 192/2315 | R | low | yes, same file (0.992) | 531 | 640/642 insns, frame 0x34 exact, 5 hunks: the record-interval test is `je end` where retail has `jne deltas ; push 0 ; jmp end+1`, and the push order of the final call. |
| 3 | `0x004e14a0` | BaseQuarryEvent::PrepareGeometry | src/reconstructed/QuarryStuntEvent.cpp | 139/1756; index 166/1760 | R | low, no emu case | yes, same file (0.994) | 485 | 511/512 insns, 7 masked diffs: which register holds the g_TrackGame load and one `mov edi, eax` placed three instructions later; everything after is positional fallout. |
| 4 | `0x0040eca0` | SimulateLookAhead | src/krusty2/vehicle/BikeAI.cpp | 1479/17585 (sample form 1542, emu-divergent) | S | med | yes, same file (0.957) | 481 | 3931/3966 insns, frame 0x968 vs 0x93c, 133 calls vs 132, retail has 6 more fmul and 9 more fsub. At 0x40f5fb retail computes the accelerations with real subtractions (`fld 0.0 ; fld force.x ; fsub`, `fld -32.0 ; fsub [zero temp]`) where the candidate folds them to two `mov [..], 0xc2000000`. |
| 5 | `0x00417ed0` | BikeRace::Create | src/reconstructed/BikeRace.cpp | 2800/6756 strict | R | low, emu 0/600 differ | yes, the unit (0.998) | 473 | 1813/1813 insns, frame 0x4e8 equal, 4 hunks: at two vector-length sites retail has `fstp [t] ; faddp ; fadd [t]` where the candidate keeps z*z on the x87 stack; one store scheduled two instructions later. |
| 6 | `0x004db0d0` | ProjectedShadow::ComputeBounds | samples/physics/shadow/ProjectedShadowNearMisses.cpp:238 | 211/2020 | R | low | yes, same file (0.996) | 452 | 560/560 insns, identical mnemonics, 3 masked diffs (one load schedule). The whole gap is frame-slot order: disp8 against disp32 makes the candidate 1975 B against 2020. |
| 7 | `0x004de590` | BaseQuarryEvent::LoadEventObjects | src/reconstructed/QuarryStuntEvent.cpp | 2044/7952 strict | R | low, emu 0/600 differ | yes, the unit (0.986) | 394 | 2206 insns against 2208, frame 0x650 equal; slots below 0x1c, register choices in the memory-budget tree and the fog/sky block, the collision loop head and the racer loop's esi/edi swap. |
| 8 | `0x00463a30` | FollowCamera::ProcessCameraInput | src/reconstructed/FollowCamera.cpp:352 | 927/4058; index 903/4042 | R | low | yes, same file (0.982) | 313 | 1088/1080 insns, same call, store and branch counts; 34 masked diffs spread over the keyboard/joystick arms. |
| 9 | `0x00413200` | KrustyBike::SimulateAIFrame | src/krusty2/vehicle/BikeAI.cpp | 2153/4112 (was 2044) | R | low | yes, same file (0.991) | 207 | 1122/1120 insns, frame 0x74 equal, 24 masked diffs: a/b/c slot offsets, operand order of the second lever cross, lean clamp kept on the x87 stack. |
| 10 | `0x00425900` | BoxCapsuleOverlap | src/krusty2/bvh/BoundingBoxTreeQuery.cpp:264 | 2450/4321 | R | low | yes, same file (0.995) | 187 | 1301/1300 insns, 8 masked diffs: retail calls DotProduct (0x40ae30) in the seventh normalisation where the candidate expands it (26 vs 27 calls), two fxch, and a permuted frame that accounts for most of the byte gap. |
| 11 | `0x00459ce0` | EcoSystem::GenerateObjects | src/reconstructed/EcoSystem.cpp:1315 | 366/3257 | S | low | yes, src/reconstructed/EcoSystem.cpp (0.914) | 179 | 3300 vs 3257 B, 68 vs 72 conditional jumps: the candidate materialises `word & 0x3e0` as 0/1 (`neg/sbb/neg`) and tests it later, retail tests in place and branches; plus slot order. |
| 12 | `0x00514550` | UpdateSuspensionProbe | src/krusty2/tire/Tire.cpp:71 | 2566/4258 | R | med | yes, same file (Vehicle.cpp calls it directly) | 169 | 1215/1215 insns, frame 0xc0 equal, same calls and stores; 16 masked diffs: four call/ctor temporaries in `rolled`'s slot, the owner branch's late pointer load, lateral cross load order. |
| 13 | `0x00414370` | KrustyBike::UpdateAIRacingLine | src/krusty2/vehicle/BikeAI.cpp | 782/4802 (was 645) | S | low | yes, same file (0.951) | 166 | Unit body (the former bring-up's) 1294/1302 insns, 53 masked hunks: retail reloads `statusNode->nextGate` at 0x4149e2 where VC6 keeps it in edi, and schedules the vector copies and setup stores in another order. Stores and calls agree. |
| 14 | `0x0046a920` | UIDialog::ReadDialogResource | src/reconstructed/GameUi.cpp | 1615/16130 (the sample form, without the Set_Default FontName fix, was 1627); index 684/2323 (masked) | S | low | yes, same file (0.967) | 145 | True extent is 16120 B (the index row says 2323). 4415/4418 insns, same 417 calls; frame 0x9bf4 vs 0x9c74. Header: retail has an unreferenced 0x80-byte local no source form reproduces, and keeps the control in ebx. |
| 15 | `0x00440f30` | D3DIMSoultreeObject::DrawCurrentLod | src/reconstructed/D3DIMSoulTree.cpp | 751/7851 (was 722; index 714) | S | low | yes, src/reconstructed/D3DIMSoulTree.cpp | 143 | 1884/1895 insns, frame 0x9c4 vs 0x9e8, 294 masked diffs: retail holds vector temporaries on the x87 stack (header: 21 pops against 11), `this` in ebx vs esi. |
| 16 | `0x004a3c80` | MorphBastardModifier::ControllerAngles | src/reconstructed/MorphBastardModifier.cpp:301 | 570/3878 | S | low | yes, src/reconstructed/MorphBastardModifier.cpp (0.914) | 133 | 866/896 insns, frame 0x1f0 vs 0x1fc, 37 calls vs 38: per-axis block locals and one more call in retail. |
| 17 | `0x0044b200` | MainDlg::HandleEvent | src/reconstructed/DlgProcs.cpp:155 | 962/5292; index 925/5232 | S | low, no emu case | merged into the unit (0.992) | 130 | 1381/1382 insns and only 14 masked diffs, but frame 0xc64 vs 0x6c4: retail shares one slot between the two ConnectionInfoType[5] arrays. Header: about 30 source shapes and 16 flags tried; only /GX- shares the slot. |
| 18 | `0x004210f0` | BikeRace::WriteRelativePositions | src/reconstructed/BikeRace.cpp | 466/2960 strict | S | low, emu 0/600 differ | yes, the unit (0.981) | 126 | Call sequence equal to retail's (the out-of-line Vector3 constructor, dot, scale and operator+/- helpers); register choice and scheduling differ. |
| 19 | `0x004e4d10` | RaceSound::RefillStream | src/reconstructed/racesnd.cpp | 806/2038 | R | low | yes, in its unit (0.978) | 123 | 605/603 insns: one `field_0x18 = rest` store cross-jumped in retail, and the looping path's half kept in ebp. |
| 20 | `0x0048b100` | VisualCue::Tick | src/reconstructed/Krusty3DObjects.cpp:286 | 390/2816; index 401/2820 | S | low | yes, same file (0.932) | 121 | Retail gives the two early hide returns their own epilogues and +0x30 stores (6 vs 4); the candidate shares one return and caches 0 in ebx (retail edi). |
| 21 | `0x004c69c0` | PCTextureMap::CreateSurface | src/reconstructed/PCTextureMap.cpp:269 | 518/2033; index 429/894 (masked) | R | low, no emu case | yes, samples/render/TextureSurfaceCreate.cpp (0.957) | 121 | 577/579 insns, same 32 calls and 34 stores; 49 masked diffs. |
| 22 | `0x004a70c0` | Character::AdvanceMotionInterpolated | samples/physics/motion/Motnctrl.cpp:382 | 307/3333; index 234/3412 | S | low | yes, src/krusty2/motion/Motnctrl.cpp (0.905) | 121 | Frame 0x2d0 vs 0x2dc, 67 direct calls vs 68: retail keeps the blend loop's unused sums as operator calls; `this` in edi vs esi. |
| 23 | `0x0048eea0` | KrustyBike::UpdateRiderPose | src/krusty2/vehicle/KrustyBike.cpp:521 | 608/2952 | S | low | yes, same file (0.971) | 117 | Frame 0x50 exact. Tail merging of the head-turn arms differs (the candidate duplicates the +0x1554/+0x1558 stores retail shares), switch compare polarity, crash clamp spill. |
| 24 | `0x0042fd80` | CarProcedural::Tick | src/reconstructed/CarProcedural.cpp:331 | 604/2909 | R | med | yes, src/reconstructed/CarProcedural.cpp (0.941) | 116 | 780/782 insns, frame 0xdc equal, same 31 calls and 35 stores; 81 masked diffs: steering clamp memory local, load order in the `back` normalisation and blend, argument push scheduling. |
| 25 | `0x004e7d60` | VCRInterface::Tick | src/reconstructed/Recorder.cpp:444 | 270/2448; index 272/2448 | R | low | yes, same file (0.960) | 109 | 799/792 insns, same 62 calls and 24 stores; 54 masked diffs, slot order. |
| 26 | `0x004e9030` | ResourceManager::AddArchive | src/reconstructed/ResourceManager.cpp:123 | 120/804; index 100/804 | R | low, no emu case | yes, same file (0.991) | 103 | 265/266 insns, 5 diffs, all from retail homing `archive` in the `new` temporary's slot. |
| 27 | `0x004da7b0` | ProjectedShadow::Init | samples/physics/shadow/ProjectedShadowNearMisses.cpp:18 | 153/778 masked (link error in match_one); index 153/777 (masked) | R | low | yes, same file (0.992) | 94 | 239/239 insns, 4 masked diffs: register choice in the sprintf/QueryFlag set-up. |
| 28 | `0x00405190` | ApplyCollisionEvent @Bike | src/krusty2/vehicle/Bike.cpp | 1758/2874 | R | low: `bike_apply_collision_event` agrees (1200 runs, 0 differ) | yes, Bike.obj (not scored) | 89 | Frame 0xac equal; the 68-insn count gap is the trailing jump and byte tables in the retail extent. Left: slot offsets of velA, blend and the 0x3ea temporary. Collision response with locals left uninitialised on purpose (provisional) and no differential case when triaged; `tools/emu_cases/bike_apply_collision_event.py` now covers every kind (0 of 1200 differ). |
| 29 | `0x004cf2a0` | DitherConvert | src/reconstructed/Pixtrans.cpp:362 | 276/1759 | R | low | yes, same file (0.973) | 89 | 562/566 insns; ebp push placement and y-loop pointer set-up order. |
| 30 | `0x00426be0` | BoxTriangleQuery | src/krusty2/bvh/BoundingBoxTreeQuery.cpp:415 | 1542/2576; index 1673/2572 | R | low | yes, same file (1.000) | 83 | Linked masked score 1.000 (626/626). Min/max of the projections stay on the x87 stack in retail; frame slot offsets. |
| 31 | `0x004c3100` | PCJoystickDevice::ReadBufferedState | src/reconstructed/PCJoystickDevice.cpp:291 | 120/1641; index 102/1449 (masked) | S | low | yes, same file (0.915) | 76 | Inline boundary: 23 calls against retail's 22 (retail expands FilterAxisSpike with arm-local temporaries, which leaves no zero register); 48 masked diffs. |
| 32 | `0x004f2340` | MultiPlayerDlg::SendStartMessage | src/reconstructed/SelectGamePicProcs.cpp:406 | 933/2148 | R | low | yes, src/reconstructed/SelectGamePicProcs.cpp (0.964) | 73 | 584/580 insns, same calls and stores; retail keeps the racer total in memory and the name/headerSize slots swap. |
| 33 | `0x00517340` | Track::PlaceNear | src/reconstructed/Track.cpp:616 | 102/1533; index 57/655 (masked) | S | med | yes, BikeAI.cpp (0.842) | 72 | 501/489 insns, frame 0x24 vs 0x20: VC6 gives the candidate list a frame slot and other registers; the out-of-memory exit sits inside the walk loop. Header: a differential run already found and fixed one argument-order bug here. |
| 34 | `0x004280e0` | TreeTreeQueryNodes | src/krusty2/bvh/BoundingBoxTreeQuery.cpp:686 | 450/2228 | S | low | yes, same file (0.929) | 71 | One constructor still expanded (43 calls vs 42) and the recursion tails are cross-jumped differently. |
| 35 | `0x004513a0` | MPRaceInfoDlg::HandleEvent | src/reconstructed/DlgProcs.cpp:1918 | 583/2004; index 595/2004 | R | low, no emu case | merged into the unit (0.997) | 71 | 522/523 insns, 4 masked diffs: retail reloads EventManager for one sprintf argument. Header: every form tried gives 595. |
| 36 | `0x0042a640` | SegmentSegmentDistance | src/krusty2/bvh/BoundingBoxTreeQuery.cpp:1139 | 73/1471; index 41/1428 | R | low | yes, same file (0.948) | 70 | 462/468 insns, same 31 calls; retail homes floats in the dead d0/d1 argument slots (frame 0x60 vs 0x80). |
| 37 | `0x004a4c60` | MorphBastardModifier::ModifyMesh | src/reconstructed/MorphBastardModifier.cpp:459 | 185/1571; index 185/1570 | R | low | yes, src/reconstructed/MorphBastardModifier.cpp (0.908) | 69 | 532/532 insns: registers of the object/channel loop (retail keeps the object offset in esi and the channel in ebp). |
| 38 | `0x00448560` | GetDXVersion | samples/game/DeviceSetupNearMisses.cpp:40 | 170/1011; index 148/1006 | R | low, no emu case | yes, same file (0.914) | 67 | 344/345 insns; zero register ebp vs edi, frame 0x11c vs 0x118, and the build-number test as `cmp word ptr` where retail loads, masks with 0xffff and compares. |
| 39 | `0x00457480` | EcoSystem::ReadEst | src/reconstructed/EcoSystem.cpp:599 | 561/2629 | R | low | yes, src/reconstructed/EcoSystem.cpp (0.989) | 62 | 13 diffs, all the constant 1 held in edi (VC6 zero/one-register over-cache, docs/VC6_ZERO_REGISTER.md). A fix would be compiler-only. |
| 40 | `0x0052d610` | VisibilityQuadTreeNode::Traverse | src/krusty2/visibility/VisibilityQuadTree.cpp (now exact) | 2265/3027 | R | low | yes, same file (0.988) | 61 | 1056/1056 insns: VC6 cross-jumps a different pair of call tails (direct 32 vs 33, slot-2 indirect 15 vs 14). |
| 41 | `0x004a5e40` | MotionManager::LoadVue | samples/physics/motion/Motnctrl.cpp:19 | 505/1090 | R | low | yes, src/krusty2/motion/Motnctrl.cpp (0.990) | 58 | 301/301 insns, 5 masked diffs: a register permutation (ebx/ebp, esi/edi). |
| 42 | `0x00447a00` | DebugOverlay::LoadFont | src/reconstructed/DebugOverlay.cpp:71 | 463/980 | S | low | yes, same file (0.977) | 54 | Body matches; VC6 copies the epilogue into the failure and success returns where retail has one shared epilogue (303 vs 299 insns). |
| 43 | `0x004507a0` | SPRaceInfoDlg::HandleEvent | src/reconstructed/DlgProcs.cpp:1671 | 583/1652; index 589/1668 | R | low | merged into the unit (0.994) | 53 | 438/442 insns; retail recomputes `lea [TrackGame+0x570]` in each loop test. Header: a dozen accessor and loop forms compile the same. |
| 44 | `0x004cdf10` | Halve8888 | src/reconstructed/Pixtrans.cpp:32 | 101/628; index 110/636 (candidate 628) | R | low | yes, same file (0.954) | 53 | 194/196 insns, 12 masked diffs: the lower-row pointer is `top + stride` where retail biases it to the alpha byte. |
| 45 | `0x004fb8c0` | SetMatrixIn | samples/physics/helpers/SoultreeMatrix.cpp:160 | 262/1134; index 262/1096 | S | low | yes, same file (0.971) | 52 | Inline boundary: 7 calls against retail's 8, 1134 vs 1096 B; retail passes by address what the candidate copies. |
| 46 | `0x00508bc0` | TerrainShadow::IsShadowVisible | samples/physics/shadow/TerrainShadow.cpp:407 | 2749/3092; index 2749/3087 | R | low | yes, src/reconstructed/TerrainShadow.cpp (0.984) | 51 | 823/824 insns, frame and slots match; 14 masked diffs: mode 1 right/up copy scheduling and push order around ProjectVertices. |
| 47 | `0x00481de0` | DrawableGridNode::WalkVisibilityQuadrants | src/reconstructed/Griddraw.cpp:1568 | 73/1750; index 81/1750 (masked) | S | low | yes, same file (0.733) | 50 | 499/500 insns but 248 masked diffs and the lowest linked score of the set (0.733): the depth switch and the size-16 path are laid out in another order; `coarse` in ebx in retail. |
| 48 | `0x004a2560` | MediaControl::Open | src/reconstructed/MediaControl.cpp | 53/656 | R | low, no emu case | yes, same file (0.944) | 48 | 214/218 insns, same calls and stores; retail saves ebp and orders slots differently. |
| 49 | `0x004dbd30` | ProjectedShadow::Present | samples/physics/shadow/ProjectedShadowNearMisses.cpp:449 | 282/1432; index 164/1244 (masked) | S | low | yes, same file (0.837) | 46 | Per-format row loops match in shape; register assignment of the clip fields differs and retail uses byte operations on field_0x94 for the format-9 constants. |
| 50 | `0x004d8c40` | PCLastRaceDlg::PayOutRace | src/reconstructed/ProCircuitProcs.cpp | 427/886 | R | low | yes, src/reconstructed/ProCircuitProcs.cpp (0.967) | 46 | 261/260 insns, 14 masked diffs: two field loads retail does not make. |
| 51 | `0x0051c720` | RadarOverlay::DrawTrackOutline | src/reconstructed/TrackOverlay.cpp:1659 | 254/1016; index 249/962 (masked) | R | low | yes, same file (0.972) | 46 | 299/300 insns, frame 0x74 vs 0x8c. |
| 52 | `0x0047caa0` | DrawableGridNodeSharedTextures::SelectTexture | src/reconstructed/Grid1.cpp:78 | 213/1712; index 213/1728 | R | low | yes, same file (0.981) | 45 | Retail keeps a fourth ladder compare (`g * 4.0f` against detail at 0x47cb50) whose result nothing reads; the source's version folds. Reproducing it needs a no-op form. |
| 53 | `0x004f8220` | MPBikeRiderDlg::FillBikeRiderLists | src/reconstructed/SelectGamePicProcs.cpp:2131 | 110/840 | R | low | yes, src/reconstructed/SelectGamePicProcs.cpp (0.932) | 44 | 258/257 insns; zero register ebp vs edi, and `flags & 4` kept in a slot in retail. |
| 54 | `0x00427c10` | SegmentTriangleQuery | src/krusty2/bvh/BoundingBoxTreeQuery.cpp:590 | 424/1112; index 374/1110 | R | low | yes, same file (0.959) | 41 | 353/352 insns; retail keeps the constant 1 in ebp and copies segment[1] at entry, so registers and slots differ. |
| 55 | `0x004815e0` | DrawableGridNode::ComputeViewerDetail | src/reconstructed/Griddraw.cpp:1362 | 64/1083 | R | low | yes, same file (0.887) | 41 | 308/314 insns; retail keeps both axis deltas on the x87 stack across the near/far branch. |
| 56 | `0x00435830` | CollisionObject::SetTransform | src/krusty2/collision/CollisionObject.cpp:2699 | 890/1684; index 902/1684 | S | med | yes, same file (0.833) | 40 | Hull case's inline 4x4 product: per-element term order differs from retail (131 masked diffs, 513/523 insns). The source parenthesises each sum; `collision_set_transform` agrees bit for bit at 0x27f, 0x7f, 0x37f and 0xe7f (1200 runs, no ulp tolerance), so the difference is load order, not association. |
| 57 | `0x004bdc00` | SoundMemoryManager::LoaderThreadProc | src/reconstructed/PCAudio.cpp:1117 | 91/483 | R | low | yes, same file (0.871) | 39 | Same calls, stores and branches (155/155 insns). Retail keeps audio in ebp and `&cs` in ebx; the candidate caches an import pointer in a register (2 `call reg` against 0). |
| 58 | `0x00483d40` | GridNode::IntersectSegment | src/reconstructed/GridNode.cpp | 178/3984 masked (link error in match_one); index 68/4396 | S | low | yes, same file (0.916) | 38 | Retail has an ebp frame and four bare `fistp [eax]` from an inline-asm rounding helper (8 direct calls vs 12). |
| 59 | `0x005059d0` | Terrain::Load | src/krusty2/broadphase/Terrain.cpp:138 | 894/2117; index 894/2116 | R | low | yes, same file (0.985) | 37 | 627/628 insns; VC6 caches 3 in ebx where retail caches -1 after the palette branch. Header: about 80 forms tried. |
| 60 | `0x0050b080` | TextService::DrawTextInto | src/reconstructed/TextService.cpp:105 | 530/890 | R | low | yes, same file (0.975) | 36 | 255/256 insns, 9 masked diffs: w and v trade esi and a stack slot. |
| 61 | `0x0050b400` | TextService::DrawText4444 | src/reconstructed/TextService.cpp:163 | 398/758 | R | low | yes, same file (0.970) | 36 | 217/218 insns, 9 masked diffs: w and v trade esi and a stack slot. |
| 62 | `0x0050b700` | TextService::DrawText1555 | src/reconstructed/TextService.cpp:216 | 398/758 | R | low | yes, same file (0.970) | 36 | Same shape and same difference as 0x0050b400. |
| 63 | `0x004738a0` | UIEditBox::Draw | samples/ui/GameUiNearMisses.cpp:91 | 76/973; index 76/968 | R | low, no emu case | yes, samples/ui/EditBoxDraw.cpp (0.877) | 36 | 306/310 insns, frame 0x38 vs 0x40; retail keeps the width and the redraw count in memory. |
| 64 | `0x00512990` | WriteTga | src/reconstructed/Tgafile.cpp:721 | 418/1083; index 354/982 (masked) | R | low | yes, same file (0.951) | 33 | 380/377 insns, same 22 calls; 32 masked diffs. |
| 65 | `0x00430b10` | CarProcedural::PositionAtTime | src/reconstructed/CarProcedural.cpp:486 | 299/843 | R | low | yes, src/reconstructed/CarProcedural.cpp (0.934) | 33 | 265/266 insns; `this` in edi vs esi and slots. |
| 66 | `0x0050dad0` | ManagedTextureGroup::RepackWithPartialBlits | src/reconstructed/ManagedTextureGroup.cpp:487 | 4913/5020; index 1887/5022 | R | low, no emu case | yes, src/reconstructed/ManagedTextureGroup.cpp | 32 | 1597/1597 insns, equal size, 10 hunks: one `[esp+0x28] = ebx` store 17 instructions later than retail, two `mov ecx` / `fmul` pairs swapped, one push/load order. |
| 67 | `0x00519a20` | StatsOverlay::DrawStandings | src/reconstructed/TrackOverlay.cpp:948 | 605/1004; index 535/894 (masked) | R | low | yes, same file (0.965) | 32 | 342/342 insns, same calls; 16 masked diffs. |
| 68 | `0x004079c0` | LoadBike @Bike | src/krusty2/vehicle/Bike.cpp:506 | 6428/6745 | R | low | yes, Bike.cpp's own body (the Group1_KbBikeLoader forwarder linked at 0.987 until it was retired) | 32 | 1633/1633 insns, equal size, same 108 calls and 168 stores; spill homes and store scheduling of the string tables and Vec3 constants. |
| 69 | `0x0047d780` | DrawableGridNodeSharedTextures::BuildBlockTexture | src/reconstructed/Grid1.cpp:478 | 227/1010 | R | low | yes, same file (0.871) | 31 | 322/319 insns, same calls; 71 masked diffs, slot order. |
| 70 | `0x00491d10` | FindPlacement @KrustyBike | src/krusty2/vehicle/KrustyBike.cpp:1496 | 548/1296 | S | low | yes, same file (0.947) | 31 | 1264 vs 1296 B: the mode 2/3 normalise join and the case 0/4 call sit in different arms; 101 masked diffs. |
| 71 | `0x00486170` | GUIManager::CopyScreenToTexture | src/reconstructed/GUIManager.cpp:535 | 284/896; index 280/896 | R | low, no emu case | yes, samples/ui/GuiScreenTexture.cpp (0.867) | 31 | 284/286 insns, frame 0xa4 vs 0xb0, zero-register placement. |
| 72 | `0x004cb330` | MeanTextureChannelDifference | src/reconstructed/PCVideoCard.cpp:837 | 35/629 | R | low | yes (0.923) | 30 | 200/205 insns, same calls; registers and loop entry. |
| 73 | `0x004650e0` | FollowCamera::LoadCameraFile | src/reconstructed/FollowCamera.cpp:866 | 429/1014; index 519/1014 | S | low | yes, same file (0.842) | 29 | Inline boundary: retail expands both side cross products (12 fmul, components in memory temporaries), the candidate expands one and calls for the other (19 calls against 18, 6 fmul); frame 0x198 vs 0x18c. docs/FOLLOW_CAMERA_LOAD_FILE.md: every inline spelling tried. |
| 74 | `0x0047d470` | DrawableGridNodeSharedTextures::BuildNodeTexture | src/reconstructed/Grid1.cpp:394 | 313/786 | R | low | yes, same file (0.931) | 28 | 262/262 insns; line pointer spilled (frame 0x838 vs 0x834), first/row in other registers. |
| 75 | `0x004cab00` | PCVideoCard::ProbePartialTextureUploads | src/reconstructed/PCVideoCard.cpp:637 | 423/1129 | S | low, no emu case | yes, same file (0.826) | 28 | 353/352 insns but 117 masked diffs: `this`/texture slots swapped, first texture cached in ebx, CreateTextureSurface success arm after the `ret`. |
| 76 | `0x004e5d00` | BuildStatusList | src/reconstructed/RaceStatus.cpp | 168/519 | S | low | yes, src/reconstructed/RaceStatus.cpp (0.907) | 28 | 167/168 insns, same calls and stores; retail reloads a count from the stack (`mov ebp,[esp+N] ; test`) and steps a 0x50-byte stride three times where the candidate indexes. |
| 77 | `0x00478570` | UIMultiState::Draw | samples/ui/GameUiNearMisses.cpp:128 | 139/682 | R | low, no emu case | yes, samples/ui/MultiStateDraw.cpp (0.921) | 27 | 203/201 insns, same calls and stores. |
| 78 | `0x004ca5a0` | PCVideoCard::ProbeNonLocalTextureMemory | src/reconstructed/PCVideoCard.cpp:460 | 66/490; index 66/486 (candidate 490) | R | low, no emu case | yes (0.932) | 25 | 146/148 insns; retail holds 480 in ebp. |
| 79 | `0x00406ae0` | SelectRiderPose @Bike | src/krusty2/vehicle/Bike.cpp | 107/2436; index 115/2396 | S | med, no emu case | yes, Bike.obj (not scored) | 23 | Retail has an ebp frame and three inline-asm `fistp`, and repeats each clamp per branch (59 vs 68 jcc, 2 vs 5 calls, +0x65c stores 16 vs 20). The candidate rounds through a software helper. Out of reach under the no-asm rule. |
| 80 | `0x0041f1d0` | BikeRace::NextCameraTarget | src/reconstructed/BikeRace.cpp | 313/887 strict | S | low, emu 0/600 differ | yes, the unit (0.829) | 23 | Retail has three expanded copies of the 0x0041eb20 call path (4 calls vs 3) and keeps 0 in edx from the first instruction. |
| 81 | `0x004570a0` | Vegetation::GetCollisionObject | samples/ecosystem/VegetationCollisionObject.cpp:8 | 103/386 | R | low | yes, same file (0.896) | 23 | 120/121 insns, same calls and stores; retail keeps the definition-table entry in edi across the position conversions. |
| 82 | `0x004049d0` | BackgroundImage::GetDc | src/reconstructed/BackgroundImage.cpp:381 | 115/678; index 108/606 (masked) | R | low, no emu case | yes, same file (0.943) | 23 | 222/219 insns; VC6 caches 0 in edi where retail stores immediates (docs/VC6_ZERO_REGISTER.md). |
| 83 | `0x004c7b40` | PCTextureMap::CopyRectTo | src/reconstructed/PCTextureMap.cpp:656 | 364/775; index 257/767 | R | low, no emu case | yes, samples/render/TextureRectCopy.cpp (0.887) | 21 | 235/232 insns; row pointers swap esi/ecx, frame 0x110 vs 0x10c, two 16-bit loads zero-extended with an explicit mask. |
| 84 | `0x004e6f80` | RecorderWorkerThread | src/reconstructed/Recorder.cpp:25 | 2056/2148; index 652/2180 | R | low | yes, same file (0.996) | 18 | 619/619 insns, 4 masked diffs around 0x4e719b (one load and one `lea` scheduled differently); case 2's suffix cross-jump. |
| 85 | `0x00446f40` | D3DIMSoultreeShadow::BuildShadowVertices | samples/physics/shadow/D3DIMSoultreeShadowNearMisses.cpp:207 | 122/1533 | S | low | yes, same file (0.891) | 14 | Retail rounds with an inline-asm `fistp` (6 of them, 10 calls, ebp frame); the candidate calls a conversion helper (16 calls). Out of reach under the no-asm rule. |
| 86 | `0x004b6710` | Overlay::RestoreRect | src/reconstructed/Overlay.cpp:265 | 105/365; index 63/361 | R | low | yes, same file (0.838) | 13 | 132/133 insns; retail keeps `whole` and the byte count on the stack, the destination in ebp. |
| 87 | `0x00407440` | UpdateRiderPose @Bike | src/krusty2/vehicle/Bike.cpp | 188/485 | R | low, no emu case | yes, Bike.obj (not scored) | 12 | 133/135 insns; retail keeps a dead `cnt += 2` store that VC6 drops from the source, and swaps the idx/next/step/cnt registers. Reproducing the dead store would be compiler-only. |
| 88 | `0x00509aa0` | TerrainShadow::BuildShadowVertices | samples/physics/shadow/TerrainShadow.cpp:115 | 660/1788; index 78/1788 | S | low | yes, src/reconstructed/TerrainShadow.cpp (0.875) | 11 | Retail converts clipped points with an inline-asm `fistp` helper (6 fistp, 7 direct calls vs 13). |
| 89 | `0x004db8c0` | ProjectedShadow::RenderShadow | samples/physics/shadow/ProjectedShadowNearMisses.cpp:360 | 31/1096 masked (link error in match_one); index 21/1136 | S | low | yes, same file (0.738) | 11 | Inline-asm `fistp` and ebp frame in retail (2 fistp, 6 direct calls vs 8). |
| 90 | `0x0040c000` | ComputeSlideAngle @Bike | src/krusty2/vehicle/Bike.cpp | 154/1069; index 162/1015 | S | med, no emu case | yes, Bike.obj (not scored) | 9 | Retail has an ebp frame and one bare `fistp` (inline asm); 31 vs 34 conditional jumps, 2 vs 3 calls. |
| 91 | `0x004dc2b0` | ProjectedShadow::TintCasterVertices | samples/physics/shadow/ProjectedShadowNearMisses.cpp:127 | 88/375 masked (link error in match_one); index 29/352 | S | low | yes, same file (0.774) | 3 | Inline-asm `fistp` and ebp frame in retail; 143 vs 121 insns. |
| 92 | `0x0052a940` | SimulateFrame @Vehicle | src/krusty2/vehicle/Vehicle.cpp | 3229/3254; index 2339/3254 | R | low | yes, same file (0.998) | 2 | 948/948 insns, equal size, 8 diffs: the lift tilt-axis arm sits after the Rotate(k) arm in retail, and prevSpeed loads before the wheel-count store. Header: 30 control-flow forms tried. |

The number after the linked source is the linked masked score from `linked_accuracy.py`.

## Top 15: next steps

Selection: the twelve highest expected values after leaving out the documented
dead ends (0x0046a920, 0x00440f30, 0x0044b200) and the block-layout case
0x00414370, plus three picked for other reasons: 0x00405190 for gameplay risk,
0x004e9030 and 0x004da7b0 because the remaining difference is one decision
each. None of these steps was tried in this pass; every one is a proposal to
test, and each must stay within the compiler-gaming rule (a form a person
would plausibly write, singled out by the bytes).

1. **0x00493660 KrustyBike::InterpolateNetworkState** (2305/8558).
   Confirmed: 2260/2260 instructions, frame and slots exact, same 53 calls and
   235 stores. The 44 masked diffs cluster in three places: `c` widened through
   its stack home, the `(len + 50)^2` store order, and fld/fmul operand order.
   Next: take the three clusters one at a time, starting with the operand
   order (swap the factors of the affected products to retail's load order).
   Each early fix re-aligns everything behind it, so the strict score should
   move in large steps. Strong inference that the statement order is already
   right, since stores agree one for one.

2. **0x00492ad0 KrustyBike::SendStateDelta** (192/2315).
   Confirmed: frame 0x34 exact, 5 hunks. Retail's record-interval test is
   `jne deltas ; push 0 ; jmp end+1`; the candidate has `je end`. That is the
   shape of a full-packet send whose first argument push is shared with the
   common exit, so retail's source most likely reaches the full send by
   falling through from the interval test, not by the `goto sendFull` at
   src/krusty2/vehicle/KrustyBike.cpp:1807 (provisional).
   Next: restructure the record arm so the delta block is the `else` of the
   interval test, and check the final call's argument order against retail's
   pushes. The header already records some forms; read it first.

3. **0x004e14a0 BaseQuarryEvent::PrepareGeometry** (139/1756, no differential case).
   Confirmed: 511/512 instructions, 7 masked diffs; the first is which register
   receives the `g_TrackGame` load at the top and one `mov edi, eax` placed
   three instructions later. Everything after is positional fallout, which is
   why the strict score is so low for so small a difference. Next: try a local
   for the game pointer (or for `debugOverlay`) across the first two tests at
   the top of PrepareGeometry in src/reconstructed/QuarryStuntEvent.cpp, which is what retail's
   register lifetime suggests (strong inference). Also add an emu case.

4. **0x0040eca0 SimulateLookAhead** (1479/17585 in the unit, medium risk).
   Confirmed: at 0x0040f5fb retail loads 0.0 and -32.0 from memory and performs
   the subtractions (`fld force.x ; fsub`, `fld -32.0 ; fsub [zero temp]`);
   the candidate folds them into two immediate stores. Retail has 6 more fmul,
   9 more fsub, one fewer call and a 0x2c smaller frame. The values are the
   same (subtracting stored zeros), so this is not a behaviour difference.
   Next: at src/krusty2/vehicle/BikeAI.cpp:801-812 write the
   acceleration as one expression of non-constant operands, for example a
   `gravity` Vec3 local holding (0, -32, 0) and
   `accR = (rearForce + gravity) - rearDrag` through the inline operators, so
   VC6 cannot fold it (provisional; the exact expression has to be read off
   the retail operand order). The byte gap is the largest of the set, but most
   of it is frame fallout, so recovery is all-or-nothing.

5. **0x00417ed0 BikeRace::Create** (2800/6756).
   Confirmed: 1813/1813 instructions, frame 0x4e8 equal, 4 hunks. At two
   vector-length sites retail has `fstp [t] ; faddp ; fadd [t]`, that is, z*z
   stored to a temporary and added last, where the candidate keeps it on the
   x87 stack. BIKERACE.md records many forms, including a volatile probe that
   is not acceptable. Next: check whether those two sites call a different
   length helper from the other length sites in the unit (one whose body
   computes `zz` into a named float before the sum); if every other site in
   retail uses the stacked form, the two odd ones are a distinct inline
   function, not a spelling of the same one (strong inference).

6. **0x004db0d0 ProjectedShadow::ComputeBounds** (211/2020).
   Confirmed: 560/560 instructions with identical mnemonics and 3 masked diffs.
   The whole gap is frame-slot order: retail's hot locals sit at disp32
   offsets and the candidate's at disp8 (1975 B against 2020). Next: run
   `tools/frame_layout.py` on both, then reorder or re-scope the local
   declarations per docs/VC6_FRAME_LAYOUT.md. Declaration order is something a
   person chooses, so this is within the rule.

7. **0x004de590 BaseQuarryEvent::LoadEventObjects** (2044/7952 strict, every
   relocation resolved). Confirmed: 2206 instructions against 2208, frame 0x650
   equal, the call sequence and EH states retail's. The rest is scheduling:
   slots below 0x1c, register choices in the memory-budget tree and the
   fog/sky block, the collision loop head (retail enters a rotated loop with a
   jmp) and the racer loop's esi/edi swap. Next: the loop-entry shape, which
   decides the register assignment for the rest of the loop
   (docs/QUARRYSTUNTEVENT.md).

8. **0x00463a30 FollowCamera::ProcessCameraInput** (927/4058).
   Confirmed: same call, store and branch counts, 1088 against 1080
   instructions, 34 masked diffs across the keyboard/joystick arms. Next:
   locate the eight extra instructions (probably one reloaded member that
   retail keeps in a register across an arm; provisional) and fix that before
   anything else. Only skimmed.

9. **0x00413200 KrustyBike::SimulateAIFrame** (2044/4114).
   Confirmed: frame 0x74 equal, 24 masked diffs: a/b/c slot offsets, operand
   order of the second lever cross product, lean clamp kept on the x87 stack.
   Next: swap the operands of the second cross product's multiplications to
   retail's load order (same value, since each product commutes), then revisit
   the a/b/c declaration order.

10. **0x00425900 BoxCapsuleOverlap** (2450/4321).
    Confirmed: retail calls DotProduct (0x0040ae30) in the seventh
    normalisation; the candidate expands it there (26 calls against 27). Next:
    find what distinguishes the seventh site in the source (argument types, a
    temporary operand, or the inline budget running out by that point). The
    permuted frame, which accounts for most of the gap, may follow from it.

11. **0x00459ce0 EcoSystem::GenerateObjects** (323/3300).
    Confirmed: the candidate materialises `word & 0x3e0` as a 0/1 value and
    tests it later; retail tests in place and branches (68 against 72
    conditional jumps). Next: remove the boolean local and write the mask test
    inside each `if` that uses it (strong inference that retail's source
    repeated the test, since retail has four more branches).

12. **0x00514550 UpdateSuspensionProbe** (2566/4258, medium risk).
    Confirmed: 1215/1215 instructions, frame 0xc0 equal, 16 masked diffs: four
    call/constructor temporaries land in `rolled`'s slot, the owner branch
    loads its pointer late, and the lateral cross product loads in another
    order. Next: narrow the scope of `rolled` so its slot is free for the
    temporaries, as retail's sharing implies. Vehicle.cpp's PlaceWheels calls
    the Tire.cpp body directly, so a fix there is what the game links.

13. **0x00405190 Bike::ApplyCollisionEvent** (1758/2874; risk now low).
    Not a matching priority: the remaining difference is slot offsets. It was
    listed for gameplay risk (collision response, locals left uninitialised on
    purpose, not scored by `linked_accuracy`). Done since the triage:
    `tools/emu_cases/bike_apply_collision_event.py` runs every event kind with
    stubbed solvers that log the values behind each pointer argument; retail
    and the source build agree in 1200 of 1200 runs at three control words,
    and a planted 0.6f -> 0.61f change is caught.

14. **0x004e9030 ResourceManager::AddArchive** (120/804, no differential case).
    Confirmed: 5 diffs, all from retail homing `archive` in the slot of the
    `new` temporary. Next: assign the `new` result directly to the variable
    that is used afterwards instead of through a second pointer (or the
    reverse; one of the two shares the slot).

15. **0x004da7b0 ProjectedShadow::Init** (153/778 masked).
    Confirmed: 239/239 instructions, 4 masked diffs in the sprintf/QueryFlag
    set-up. Next: resolve the `SetRenderContext` symbol `match_one` reports,
    then reorder the two argument evaluations to retail's.

## Not worth another attempt under the current rules

- Inline-asm rounding in retail (ebp frame, bare `fistp`): 0x00406ae0,
  0x0040c000, 0x00446f40, 0x004db8c0, 0x004dc2b0, 0x00509aa0, 0x00483d40.
  Confirmed by the `fistp` instructions and frame; the no-inline-assembly rule
  puts these out of reach.
- Zero/one-register over-caching (docs/VC6_ZERO_REGISTER.md): 0x00457480,
  0x005059d0, 0x004049d0. A fix would be a compiler-only form.
- Retail keeps dead code the optimiser removes from the source: 0x00407440
  (dead `cnt += 2`), 0x0047caa0 (unread fourth compare).
- Documented dead ends: 0x0044b200 (array slot sharing only under /GX-),
  0x0046a920 (unreferenced 0x80-byte local).

## Coverage gaps

No differential emulation case mentions these VAs: `0x004049d0`, `0x00406ae0`, `0x00407440`, `0x0040c000`, `0x00448560`, `0x0044b200`, `0x004513a0`, `0x004738a0`, `0x00478570`, `0x00486170`, `0x004a2560`, `0x004c69c0`, `0x004c7b40`, `0x004ca5a0`, `0x004cab00`, `0x004e14a0`, `0x004e9030`, `0x0050dad0`.

`match_one.py` cannot link 0x004db8c0, 0x004dc2b0 and 0x00483d40 (unresolved
rounding helper), nor 0x004da7b0 (one unresolved symbol); their table scores
are masked.
