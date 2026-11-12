# Naming

This file records the reconstruction names that replaced address placeholders
(`Fn_XXXXXXXX`, `Method_0xXXXXXXXX`). They describe the behaviour of the
retail code and make no claim about the original identifiers. Renames do not
change code generation: calibration, physics and object fixtures must hold
the same bytes before and after.

Each name has an evidence tier, following [AGENTS.md](../AGENTS.md):

1. **Confirmed.** A literal string in the function or its source file names the
   thing (for example a `__FILE__` path or a resource name).
2. **Strong inference.** RTTI or a vtable slot, an API role, or a behaviour
   that the decoded body and its named callers or callees show without
   ambiguity.
3. **Provisional.** The name comes only from context, such as the call sites of
   an empty body.

A function whose behaviour does not support a name unambiguously keeps its
placeholder. Old names appear only in this table; the rest of the tree uses
the new name and keeps the VA. Later waves append rows.

## Wave 1

| Old name | New name | VA | Tier | Evidence |
| --- | --- | --- | --- | --- |
| `KbBikeLoader::Fn_004079C0` | `KbBikeLoader::LoadBike` | `0x004079c0` | 1 | Bike.cpp `__FILE__` and the node strings "Bike", "Handlebars", "TireFront", "TireRear", "SwingArm", "Shocks", "InlineFront"; same VA as the canonical `Bike::LoadBike` |
| `Bike::Method_0x0040a520` | `Bike::IntegrateSteerTorque` | `0x0040a520` | 2 | integrates `steerTorque` from `steerTorqueGains` and the wheel contacts; `ApplySteerRate` returns `1.0f - IntegrateSteerTorque()` |
| `KrustyBike::Fn_00413200` | `KrustyBike::SimulateAIFrame` | `0x00413200` | 2 | slot 49 `SimulateFrame` calls it in place of `Bike::SimulateFrame` when `aiKind` or `race->forceAiStep` is set; lies in the BikeAI.cpp range |
| `KrustyBike::Fn_00414370` | `KrustyBike::UpdateAIRacingLine` | `0x00414370` | 2 | BikeAI.cpp `__FILE__` at `0x00414846`; `ReadControlAxes` calls it for AI bikes; rebuilds `aiPath` and picks the AI trick |
| `BikeA38::Method_0x00435fe0` | `BikeA38::SnapPlacement` | `0x00435fe0` | 2 | unused view declaration at the VA of `CollisionObject::SnapPlacement` (17 bytes, two `UpdatePlacement` calls) |
| `Fn_00439600` | `CapsuleTouchesObject` | `0x00439600` | 2 | sibling of `SphereTouchesObject` (`0x004394f0`) and `SegmentTouchesObject` (`0x00439820`); calls `CapsuleTreeQueryWithVertices` and `BoxCapsuleOverlap` |
| `KrustyBike::Fn_0048D780` | `KrustyBike::PickTrick` | `0x0048d780` | 2 | chooses a trick id from the trick tables at `0x0056cca0`/`0x0056cc60` that fits the remaining air time |
| `KrustyBike::Fn_0048D8B0` | `KrustyBike::ResetTrickState` | `0x0048d8b0` | 2 | resets the trick multiplier to 1 and clears the trick points, timers, count and chain flags |
| `KrustyBike::Fn_0048D910` | `KrustyBike::StartTrick` | `0x0048d910` | 2 | starts `riderTrickMotions[trick]`/`bikeTrickMotions[trick]` (FindMotion trick clips) |
| `KrustyBike::Fn_0048D990` | `KrustyBike::StartAltTrick` | `0x0048d990` | 2 | starts the alternate ("Alt") trick clips, chained with motion index `trick + 16` |
| `KrustyBike::Fn_0048E280` | `KrustyBike::LookBackAtNearestRival` | `0x0048e280` | 2 | calls `FindNearestRival` and applies pose handle 14, the "LookBack"/"bLookBack" motions |
| `KrustyBike::Fn_004925A0` | `KrustyBike::SendFollowedRacer` | `0x004925a0` | 2 | sends network message 0x13 (request) or 0x87 (announce, recorded) naming the followed racer |
| `KrustyBike::Fn_00492670` | `KrustyBike::SendFullState` | `0x00492670` | 2 | packs position, roll and the rest of the bike state into a message when networked or recording |
| `KrustyBike::Fn_00492AD0` | `KrustyBike::SendStateDelta` | `0x00492ad0` | 2 | sends only the state fields that changed since the last send |
| `KrustyBike::Fn_004933E0` | `KrustyBike::ApplyStateDelta` | `0x004933e0` | 2 | scales the quantised deltas of a received message and adds them to the base velocity, position and angles |
| `KrustyBike::Fn_00493660` | `KrustyBike::InterpolateNetworkState` | `0x00493660` | 2 | blends a remote bike from its received state over `dt` |
| `KrustyBike::Fn_00495C00` | `KrustyBike::ReceivedPositionChanged` | `0x00495c00` | 2 | returns 1 when a remote bike's two newest received positions differ in integer x or z |
| `KrustyBike::Fn_00495FF0` | `KrustyBike::AwardTrickScore` | `0x00495ff0` | 2 | formats `"%s %.0f.00"` (KrustyBike.cpp `__FILE__`) and calls `ReportTrickScore` |
| `KrustyBike::Fn_00496D20` | `KrustyBike::UseBikeCollision` | `0x00496d20` | 2 | swaps `collisionObject` to the "Bike.col" object; slot 101 `SwitchToBikeCollision` tail-jumps here |
| `KrustyBike::Fn_00496DA0` | `KrustyBike::UseBikeAndRiderCollision` | `0x00496da0` | 2 | swaps `collisionObject` back to the "BikeAndRider.col" object |
| `KrustyBike::Fn_00496E20` | `KrustyBike::SetNetRecorder` | `0x00496e20` | 2 | stores the recorder in `netRecorder`, which the send methods consult |
| `KrustyBike::Fn_00496E30` | `KrustyBike::ApplySpawnCollisionIgnores` | `0x00496e30` | 2 | called by `SetSpawnProtection`; ignores (1) or restores (0) collisions with every other bike |
| `KrustyBike::Fn_00496F90` | `KrustyBike::IgnoreCollisionsWith` | `0x00496f90` | 2 | adds the other bike's objects with `AddIgnoredOwner` |
| `KrustyBike::Fn_00497370` | `KrustyBike::RestoreCollisionsWith` | `0x00497370` | 2 | removes them again with `RemoveIgnoredOwner` |
| `Character::Fn_004A8BF0`, `Method_0x004a8bf0` | `Character::ApplyMotionAtPhase` | `0x004a8bf0` | 2 | sets the current motion and its frame at a phase |
| `Character::Method_0x004a8c50` | `Character::ApplyMotionBlendAtPhase` | `0x004a8c50` | 2 | the same for two motions blended by a weight |
| `Character::Method_0x004a9050` | `Character::ApplyFourMotionBlendAtPhase` | `0x004a9050` | 2 | the same for four motions blended by weight and lean (KrustyBike slot 102) |
| `KbPlayer::Fn_004A9E80` | `KbPlayer::SetFollowedRacer` | `0x004a9e80` | 2 | stores the followed racer, moves collision ignores from the previous one and optionally tells the network |
| `KbRaceHandler::Fn_004DE580` | `KbRaceHandler::NotifyContactImpulse` | `0x004de580` | 3 | empty `ret 0xc` body shared with other handlers; the name comes only from the KrustyBike contact-impulse call sites |
| `Tire::Fn_00515c90` | `Tire::NormalizedCross` | `0x00515c90` | 2 | returns the normalised cross product of two vectors, with a fallback when it is degenerate |

## Wave 2

Wave 2 names enum values, view methods, fields and parameters as well as
functions. Field and parameter renames leave the mangled names unchanged.
View-method renames change them; the bindings and fixture keys were updated to
match. Every affected TU was compiled with VC6 from the base and from the renamed
tree. All non-debug sections and symbols compare equal once the renamed
identifiers are mapped.

| Old name | New name | VA or offset | Tier | Evidence |
| --- | --- | --- | --- | --- |
| `riderPoseHandles[N]`, `bikePoseHandles[N]`, pose literals 1..17 | `enum RiderPose`: `RiderPose_LeanR01` (0) .. `RiderPose_Turn3U` (8), `RiderPose_Wheelie` (10), `RiderPose_CrosOv1..3` (11..13), `RiderPose_LookBack`, `RiderPose_Victory`, `RiderPose_BackUpTop`, `RiderPose_BackUpBottom` (14..17) | Bike +0x66c / +0x6b4 index | 1 | `Bike::LoadBike` (`0x004079c0`) fills slot N with `FindMotion("LeanR01".."BackUpBottom")`. The bike clips use the same slots ("BLeanR02", "BTurn2D/U", "BWheelie", "CrosBik1..3", "bLookBack", "bBackUpTop"). Slot 9 is never loaded. |
| `UnknownBikeRaceRacer::UnknownFunction495c00` | `UnknownBikeRaceRacer::ReceivedPositionChanged` | `0x00495c00` | 2 | Same VA and `this` as `KrustyBike::ReceivedPositionChanged` (wave 1). |
| `UnknownBikeRaceRacer::UnknownFunction4933e0` | `UnknownBikeRaceRacer::ApplyStateDelta` | `0x004933e0` | 2 | Same VA as `KrustyBike::ApplyStateDelta`. |
| `UnknownBikeRaceRacer::UnknownFunction496e20` | `UnknownBikeRaceRacer::SetNetRecorder` | `0x00496e20` | 2 | Same VA as `KrustyBike::SetNetRecorder`. The bring-up view forwards to it. |
| `UnknownEventRacer::UnknownFunction4925a0` | `UnknownEventRacer::SendFollowedRacer` | `0x004925a0` | 2 | Same VA as `KrustyBike::SendFollowedRacer`. |
| `UnknownEventRacer::UnknownFunction496f90` | `UnknownEventRacer::IgnoreCollisionsWith` | `0x00496f90` | 2 | Same VA as `KrustyBike::IgnoreCollisionsWith`. |
| `Fn_0043caa0` (collision `targets.json` note) | `TestInputEvent` | `0x0043caa0` | 2 | This note was the last place still using the old name. |
| `UnknownBikeRaceRacer::Load` `a`, `c`, `d`, `e` | `engineName`, `gasGyroOption`, `brakeGyroOption`, `withRider` | `0x0048fc80` | 2 | Forwarded position for position to `KrustyBike::Load`. |
| `KbBikeLoader::LoadBike` `a2`, `desc`, `a5`, `a6`, `a12`, `noSetup`, `setupName`, `s50`/`s54`/`s58`, `s0c`..`s20`, `gasGyro`, `brakeGyro` | `engineName`, `riderModel`, `desc`, `info`, `name`, `defaultEngine`, `torqueTable`, `rpmLow`/`rpmHigh`/`rpmStep`, `frontSpring`, `frontDamper`, `rearSpring`, `rearDamper`, `frontRatio`, `rearRatio`, `gasGyroOption`, `brakeGyroOption` | `0x004079c0` | 2 | Same VA as `Bike::LoadBike`. Each argument goes unchanged to the parameter in the same position, and the names are `Bike::LoadBike`'s. `a13` (its unused `a19`) is kept. |
| `KrustyBike::Load` `a2`, `desc`, `a5`, `a6`, `a15` | `engineName`, `riderModel`, `desc`, `info`, `riderName` | `0x0048fc80` | 2 | Forwarded to `LoadBike` in the same positions. `a15` is `LoadBike`'s `name`, which is copied to `Bike::riderName`. `a16` (always 0) is kept. |
| `KrustyBike::Load` `a24` | `aiSkill` | `0x0048fc80` | 3 | BikeRace passes the racer's `skill` there (0 for players). It is stored at +0x15d8. |
| `KrustyBike::field_0x778` | `aiCrashImpactLimit` | +0x778 | 2 | The loader sets 100.0f. The AI step `0x00413200` crashes the bike when the impact exceeds it (`fcomp` at `0x00413f3a`), and nothing else reads it. |
| `KrustyBike::field_0x15d8` | `aiSkill` | +0x15d8 | 3 | Write-only. Holds the loader argument above. |
| `KbRacer::field_0xa8` | `tagFollowedRacer` | +0xa8 | 2 | `KbGame::tagEvent` is TrackGame+0x568, a `TrackGameViewOwner`, whose +0xa8 is `followedRacer`. `KbRacer` also views the race camera, whose +0x3b4 is `followedRacer`. |
| `KbNetDelta::field_0x0e`, `field_0x12` | `motionFlags`, `poseBytes` | +0x0e, +0x12 | 2 | `KbBikeDeltaMessage` (message 13) puts the motion/crash/flag bits at +0x0e..+0x11 and the pose bytes at +0x12..+0x15. |
| `KbNetState::field_0x50`, `field_0x48` | `motionFlags`, `poseBytes` | +0x50, +0x48 | 2 | `KbBikeMessage` (message 1) layout. `ApplyStateDelta` copies the delta's two ints into them. |
| `KbNetPacket::field_0x4`, `field_0x8` | `request`, `followedRacerId` | +0x4, +0x8 | 2 | `SendFollowedRacer` stores its `request` flag and the racer's netId. BikeRace.h names +0x8 `followedRacerId`. |
| `KbBikeMessage::field_0x02` | `elapsedMs` | +0x02 | 2 | `SendFullState` stores `(short)time - (short)state->time`. |
| `KbTrackEntry::field_0x0`, `field_0x28` | `currentDevice`, `throttleCode` | +0x00, +0x28 | 2 | TrackGame+0x33fc is `UnknownTrackGameObject33fc` (`currentDevice`, then `assignments[8][14]`). +0x28 of the device row is `assignments[d][4].code`, and DeviceSetup.cpp binds row 4 to `throttleBinding`. |
| `D3DIMSoultreeCharacter::chr_field_0x10`, `KbA5C4::field_0x10` | `motionTime` | +0x10 | 2 | The motion clock: `AdvanceMotion` adds dt to it and `BlendToMotion` copies it. SceneManager.h's Character view already calls +0x10 `motionTime`. |
| `KbBikeSetup::field_0x8` | `weight` | +0x08 | 2 | The 0x5c-byte record has the `UnknownOptGarageSettings` layout (OptionProcs.h, TrackGame+0x1550): KrustyUI "Weight". The loader passes it as `LoadBike`'s `weight`. |
| `KbBikeSetup::field_0xc`..`field_0x20` | `frontComp`, `frontDecomp`, `rearComp`, `rearDecomp`, `frontDamp`, `rearDamp` | +0x0c..+0x20 | 2 | Garage `suspension[0..5]`, bound to the sliders "SldFrontComp" .. "SldRearDamp" (OptionProcs.cpp `0x004b3e70`). |
| `KbBikeSetup::setupName` (`char[0x2c]`) | `eqBands` (`int[11]`) | +0x24 | 2 | Garage `eqBands` ("SldEQ1..11"), passed by address as `LoadBike`'s `torqueTable`. Same size. |
| `KbBikeSetup::field_0x50`, `field_0x54`, `field_0x58` | `firstBand`, `lastBand`, `bandStep` | +0x50..+0x58 | 2 | Garage `firstBand`/`lastBand` ("RPMUpperLimit")/`bandStep`, which are `LoadBike`'s `rpmLow`/`rpmHigh`/`rpmStep`. |

Left as placeholders in wave 2:

- KrustyBike +0x794, +0x798, +0x15cc, +0x1534 and +0x15e5: a retail displacement scan finds only stores, so nothing shows what they mean. +0x810 is set together with `aiTricksEnabled`, and the scan finds only those stores.
- `KbChild::field_0x4`, `field_0xa4`: their role is unclear.
- `KbGame::field_0x2d74`: a dead compare in `KrustyBike::Load`.
- `Scene::UnknownFunction4de580`, `UnknownFunction464e80`/`464e90` views and `BaseQuarryEvent::UnknownVirtualSlot34`: they share tiny `ret`/`ret N` bodies with named methods. A shared address does not prove the same method.
- `UnknownQuarryOverlayHooks::UnknownFunction51d730` (`ChatOverlay::CollectRacers`): it stays address-named because VisualCue's `CollectRacers` has the same view signature (docs/TRACKOVERLAY.md).
- Tire `field_0x` members: the constructor arguments are literals in `Bike::LoadBike`, not parameter-file keys, so there is no loader-key evidence.

## Wave 3

Wave 3 names types: the most-used `Unknown*` classes and structs. A type name
changes the mangled name of every method and of every function that takes the
type, so the bindings, calibration cases and fixture keys were updated with
it. Every affected translation unit (each `.cpp` under `src/`, `samples/` and
`tests/fixtures/` that includes a changed file) was compiled with VC6 from the
base commit and from the renamed tree, from the tree root with relative paths
so that `__FILE__` is the same. With sections matched by name and COMDAT
symbol, all non-debug section bytes, relocations and symbol values compare
equal once the old names are mapped to the new ones. In some units VC6 emits
the COMDAT sections (RTTI records, vtables) or the symbol table in another
order after a rename; their contents do not change.

A view type that mirrors a named class keeps its placeholder: merging views is
a type change, not a rename. A unit's file name names the unit, not the class
in it, so a name taken from a `__FILE__` path is tier 2 at most.

| Old name | New name | Tier | Evidence |
| --- | --- | --- | --- |
| `UnknownTextureStream` | `FileStream` | 2 | The 0x134-byte stream at `0x00460d10..0x00461d57` (src/reconstructed/FileStream.cpp, [FILESTREAM](FILESTREAM.md)). `Open` calls `fopen`; `Read`, `Write` and `Seek` go through `fread`, `fwrite` and `fseek`; `GetFileTimes` calls `_fstat`. Archive slices and "FAOE"-encoded files are read through the same object. It loads textures, scenes, terrain, motions, sounds and parameter files, not only textures. No RTTI or `__FILE__` names it, so the name is descriptive. The `UnknownParameterStream` and `CollisionFileStream` typedefs still alias it. |
| `UnknownResourceManager` | `ResourceManager` | 2 | The `__FILE__` literal `D:\aardvark\VC\krusty2\ResourceManager.cpp` (`0x00572b64`) is used by the class's own destructor (`0x004e8ea0`, at `0x004e8ee1`/`0x004e8f1d`) and by `AddArchive`, `AddLooseFile` and `AddObject`. The unit (`0x004e8d30..0x004e9960`) holds the constructor, the destructor and every method, and its static initializer builds the global at `0x00689c78`. The class keeps `ResourceItem`s (RTTI) and opens "RS2" archives ([RESOURCEMANAGER](RESOURCEMANAGER.md)). The alias header `UnknownResourceManager.h`, which only holds the `UnknownResourceEntry` typedef of `ResourceItem`, is now `ResourceEntry.h`. |
| `UnknownParameterBlock` | `ParameterBlock` | 2 | The 0x5c4-byte INI reader of `D:\aardvark\VC\krusty2\Parameterblocks.cpp` (`0x004b6f30..0x004b8465`, [PARAMETERBLOCKS](PARAMETERBLOCKS.md)). Its destructor (`0x004b7190`) and section loaders push that `__FILE__` literal (`0x0056f808`), and every method lies in the unit. The partial declaration in `core/SoultreeObject.h` is the same class (same mangled names) and was renamed with it. |
