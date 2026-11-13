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
| `UnknownVcr` | `VCR` | 2 | The nonpolymorphic 0x101c-byte ring of `D:\aardvark\VC\krusty2\VCR.cpp` (literal `0x00575848`): its constructor (`0x00524110`) and destructor (`0x005241b0`) push the literal, and the unit holds every method up to `0x00524a30` ([VCR](VCR.md)). The name follows the unit; no RTTI names the class. |
| `UnknownVcrSlot`, `UnknownVcrBlock` | `VCRSlot`, `VCRPositionBlock` | 2 | The ring's 16-byte slot (`VCR` +0x0000, 256 of them) and the chained 0x804-byte block of (file position, time) pairs (`VCR` +0x100c). |
| `UnknownVcrFile` | `VCRFile` | 2 | The nonpolymorphic 0x248-byte recording file of `D:\aardvark\VC\krusty2\VCRfile.cpp` (literal `0x00575868`): its constructor (`0x00524a50`) and destructor (`0x00524b10`) push the literal, and the unit holds every method up to `0x005254a0`. The name follows the unit. |
| `UnknownVcrFileBlock`, `UnknownVcrFileEntry` | `VCRFileBlock`, `VCRFileEntry` | 2 | `VCRFile`'s 0x19000-byte in-memory data block (+0x234) and its per-handle entry (+0x004). |
| `UnknownTgaFile` | `TgaFile` | 2 | The struct the `D:\aardvark\VC\krusty2\Tgafile.cpp` readers fill and the writers emit ([TGAFILE](TGAFILE.md)). Its first 0x12 bytes are the TGA header fields in file order (ID length, colour-map type, image type, colour-map spec, origin, size, depth, descriptor), unpacked. `ReadTgaHeader` (`0x00511b40`) allocates it under the Tgafile.cpp literal (`0x00511b54`) and `FreeFile` (`0x00512dd0`) frees it. |
| `UnknownGuid` | `Guid` | 2 | The 16-byte Win32 GUID layout (`Data1`, `Data2`, `Data3`, `Data4[8]`) in `Guid.h`. The DirectX, DirectInput, DirectPlay and media IIDs/CLSIDs (`DirectXGuids.cpp`, `tests/test_directx_guids.py`) and every `QueryInterface` argument use it. The SDK `GUID` (`struct _GUID`) would change every mangled name, so the project keeps its own struct. The media-control fixture's `Guid()` helper became `GuidIndex()` to free the name; that fixture's objects differ only in that symbol. |
| `UnknownSurfaceInterface` | `DirectDrawSurface` | 2 | The COM surface interface. Every decoded slot index and argument count lines up with the IDirectDrawSurface4/7 vtable (Blt 5, BltFast 7, Flip 11, GetAttachedSurface 12, Lock 25, Unlock 32; `RenderInterfaces.h`). It is obtained both as IDirectDrawSurface7 (PCRenderTarget) and through `IID_IDirectDrawSurface4` (MediaControl, `0x005560f0`). The two share these slots, so the name carries no version. The VC98 headers stop at DirectX 5 and cannot supply it. |
| `UnknownSurfaceDesc` | `DDSurfaceDesc2` | 2 | The 0x7c-byte DDSURFACEDESC2 layout: size, flags, height, width, pitch, the back-buffer and mip/refresh unions, `lpSurface` at +0x24, the pixel format at +0x48, the caps at +0x68 and the texture stage at +0x78. `CreateSurface`, `Lock`, `GetSurfaceDesc` and `EnumDisplayModes` fill or read it. |
| `UnknownSurfaceCaps` | `DDSCaps2` | 2 | The 16-byte DDSCAPS2 layout (four caps words), passed to `GetCaps` and `GetAttachedSurface` (surface slots 14 and 12). |
| `UnknownPixelFormat` | `DDPixelFormat` | 2 | The 0x20-byte DDPIXELFORMAT layout (size, flags, FourCC, bit count, four masks), at `DDSurfaceDesc2` +0x48 and filled by surface slot 21 `GetPixelFormat` and the device's `EnumTextureFormats` callback. |
| `UnknownPixel32`, `UnknownPixel24` | `RgbaPixel`, `RgbPixel` | 2 | The 4-byte RGBA (format 0x22b8) and 3-byte RGB (format 0x378) pixels, red first in memory. Pixtrans.cpp takes them by value and Tgafile.cpp swaps red and blue on load. |
| `UnknownDialogEvent` | `DialogEvent` | 2 | The one argument of every dialog's message procedure (UIDialog vtable slot 29, `ret 4`; `DialogProc.h`): notification code, control name, kind, dialog, GUI, sending control, parameter, key and a handled flag. `UIDialog::NotifyParent` builds kind 9. |
| `UnknownDialogEventKind` | `DialogEventKind` | 2 | The enum of `DialogEvent` +0x08 kinds, in `DialogEventKind.h`, which already carried the name. |
| `UnknownInputEntry` | `InputButtonState` | 2 | The 20-byte per-key and per-button state in the `keyStates`/`buttonStates` arrays at KeyboardDevice+0x260 and MouseDevice+0x260: a state and four times. The constructors stamp the times with `ReadClock` (`0x004bfa80`), and the PC keyboard and joystick devices copy them into the caller's entry. |
| `UnknownControlEvent` | `ControlEvent` | 2 | The 20-byte input event (control, device kind, device, pressed, modifiers). `ControlInterface` (RTTI) queues it with `QueueEvent` (`0x0043cea0`) into `events[160]` and hands it out in `DispatchInput` (`0x0043cf00`). `TestInputEvent` (`0x0043caa0`) tests one. |
| `UnknownControlBinding` | `ControlBinding` | 2 | The 0x3c-byte binding of a control to an input device (owner device, binding id, axis or keys, value range, dead zone; `0x0043cc40..0x0043cde0`). The joystick lists at JoystickDevice+0x4fc hold them, and `Unbind` makes the owner device drop the id. |
| `UnknownControlMapping` | `ControlMapping` | 2 | The table at ControlInterface+0xcbc: the key, the joystick button per joystick and the mouse button of each of 50 controls. Its constructor and `Set*`/`Get*` methods are at `0x0043cae0..0x0043cc10`. |
| `UnknownMessage` | `TextQueueMessage` | 2 | The 0x8c-byte text line (text, show time, elapsed time, next) queued on `TextQueueOverlay` (RTTI). Callers build one on the stack (constructor `0x0051b200`), and the overlay keeps heap copies (copy constructor `0x0051b250`) in the list at +0x50. |
| `UnknownTextureMapList` | `TextureMapList` | 2 | The intrusive list of `TextureMap`s (RTTI) through their +0x08/+0x0c links, with a cursor and a count. Its methods (`0x00510a50..0x00510b70`) lie among TextureMapManager.cpp's code. |
| `UnknownParser`, `UnknownParserList`, `UnknownParserNode` | `Parser`, `ParserList`, `ParserNode` | 2 | Unit name: the literal `__FILE__` "D:\aardvark\VC\krusty2\Parser.cpp" (`0x0056f864`) is referenced inside `0x004b8510` (the list append), so the code `0x004b8470..0x004b89d0` is Parser.cpp. The parser reads `<Tag>value</Tag>` pairs (`0x004b87f0`) into a singly linked list (head at parser +0x04) of 0x404-byte tag/value nodes linked at +0x400. |

Left as placeholders in wave 3:

- Views of named classes: `UnknownTexturePalette` (`ColorMapper`, Quantize.h),
  `UnknownGuiControl` (`UIControl` as GUIUser and ToolTip see it),
  `UnknownQuarryRace` (`BikeRace` as the quarry loader sees it), and the bringup
  views `UnknownBikeRaceModel`, `UnknownBikeRaceRacer`, `UnknownEventRacer` and
  `UnknownKrustyBikeView` (RaceView.h and EventManager.h types). Naming them
  means merging them into the canonical class, which is a type change.
- `UnknownOverlayRect`, `UnknownMakeOverlayRect` and `UnknownTrackOverlayRect`:
  a RECT-shaped left/top/right/bottom quad, but nothing ties it to an API call,
  and `OverlayRect.h` is a repository file name, not a retail one.
- `UnknownTextureFormatChoice`: only a forward declaration and a pointer use.
- `UnknownParameterStream` stays a typedef of `FileStream`. Removing it moves
  lines, so it waits for a pass that checks the `__LINE__` users.

## Wave 4

Wave 4 names virtual slots of views and the COM interface types. The proof is
the one wave 3 used: every affected translation unit was compiled with VC6
from the base commit and from the renamed tree, and the objects compare equal
once the old names are mapped to the new ones (IDENTICAL, NAME-ONLY, or
NAME-ONLY-COMDAT-ORDER where VC6 emits the RTTI COMDATs in another order).

### View slots

A view is a stand-in declaration of a class that a caller reaches through a
pointer; its padding slots exist only to number the one slot it calls. Where
the object behind the view is established (RTTI, a vptr write, the
constructor that builds it, or a retail VA already bound to a named method),
slot N takes the canonical class's slot-N name. Every view is a single
primary vtable at object_offset 0, and the canonical names are those of the
class's own header. A slot that is itself a placeholder in the canonical
class (GameObject 9 and 19, Game 12, SoultreePhysicsBaseObject 26 and 27)
and every destructor slot keep their placeholder. Only the names change; the
views stay separate types (merging them is a type change).

| Old name | New name | Class / slot / object_offset | Tier | Evidence |
| --- | --- | --- | --- | --- |
| `UnknownTrackRecordListBox::UnknownVirtualSlotN` (TrackRecordDlg.h) | UIListBox's names for slots 1-65 (63 slots, e.g. 1 `AddRef`, 65 `SetSelectedRow`) | `UnknownTrackRecordListBox` primary vtable (object_offset 0) as `UIListBox` | 2 | slot 66 is UIListBox::ReportClick; only UIListBox/UIDDLListBox have a slot 66 |
| `TireVehicle::UnknownVirtualSlotN` (Tire.h) | Vehicle's names for slots 0-47 (46 slots, e.g. 0 `UpdateMass`, 47 `GetDriveShareScale`) | `TireVehicle` primary vtable (object_offset 0) as `Vehicle` | 2 | Vehicle.cpp passes this to 0x00513f90; slot 47 is Vehicle's pure GetDriveShareScale |
| `KbGame::UnknownVirtualSlotN` (KrustyBikeTypes.h) | Game's names for slots 1-20 (19 slots, e.g. 1 `Startup`, 20 `GetRegistryInt`) | `KbGame` primary vtable (object_offset 0) as `Game` | 2 | slot 20 takes (setting name, default) like Game::GetRegistryInt |
| `KbPad::UnknownVirtualSlotN` (KrustyBikeTypes.h) | JoystickDevice's names for slots 0-12 (12 slots, e.g. 0 `RemoveBindings`, 12 `StartEffect`) | `KbPad` primary vtable (object_offset 0) as `JoystickDevice` | 2 | ControlInterface activeJoystick (+0x0c); force-feedback slots 6/7/11/12 |
| `VehicleValueSource::UnknownVirtualSlotN` (Vehicle.h) | JoystickDevice's names for slots 0-16 (16 slots, e.g. 0 `RemoveBindings`, 16 `ResetForceFeedback`) | `VehicleValueSource` primary vtable (object_offset 0) as `JoystickDevice` | 2 | ControlInterface activeJoystick (+0x0c) |
| `VehicleKeyTable::UnknownVirtualSlotN` (Vehicle.h) | 0 `RemoveBindings`, 2 `CreateDevice`, 3 `DetectSubtype`, 4 `IsControlDown`, 5 `IsButtonDown` | `VehicleKeyTable` primary vtable (object_offset 0) as `KeyboardDevice` | 2 | ControlInterface keyboard (+0x34) |
| `VehicleInputMap::UnknownVirtualSlotN` (Vehicle.h) | 1 `CreateDevices`, 2 `IsControlDown`, 3 `IsDeviceButtonDown` | `VehicleInputMap` primary vtable (object_offset 0) as `ControlInterface` | 2 | Vehicle+0x468 is the ControlInterface |
| `VehicleSparkEmitter::UnknownVirtualSlotN` (Vehicle.h) | SparkParticleEmitter's names for slots 1-26 (24 slots, e.g. 1 `AddRef`, 26 `MarkPendingRelease`) | `VehicleSparkEmitter` primary vtable (object_offset 0) as `SparkParticleEmitter` | 2 | SparkParticleEmitter (vtable 0x00555c1c, ctor 0x004b9830) |
| `BikeParticleEmitterView::UnknownVirtualSlotN` (Bike.h) | DustParticleEmitter's names for slots 1-26 (24 slots, e.g. 1 `AddRef`, 26 `MarkPendingRelease`) | `BikeParticleEmitterView` primary vtable (object_offset 0) as `DustParticleEmitter` | 2 | Dust/DirtChunk/Steam particle emitters (ParticleEmitters.h, GameObject slots) |
| `UnknownEcoCamera::UnknownVirtualSlotN` (EcoSystem.h) | Camera's names for slots 1-26 (24 slots, e.g. 1 `AddRef`, 26 `MarkPendingRelease`) | `UnknownEcoCamera` primary vtable (object_offset 0) as `Camera` | 2 | Camera at renderContext+0x08; slots 27-32 already Camera's |
| `UnknownEcoTrackGame::UnknownVirtualSlotN` (EcoSystem.h) | Game's names for slots 1-19 (18 slots, e.g. 1 `Startup`, 19 `SwitchDisplayMode`) | `UnknownEcoTrackGame` primary vtable (object_offset 0) as `Game` | 2 | slots 20-22 already Game's GetRegistryInt/Float/Flag |
| `UnknownKeyboardBoolView::UnknownVirtualSlotN` (ControlInterface.h) | 0 `RemoveBindings`, 2 `CreateDevice`, 3 `DetectSubtype`, 4 `IsControlDown`, 5 `IsButtonDown` | `UnknownKeyboardBoolView` primary vtable (object_offset 0) as `KeyboardDevice` | 2 | ControlInterface keyboard (+0x34); slot 5 IsButtonDown(DIK code) |
| `UnknownJoystickBoolView::UnknownVirtualSlotN` (FollowCamera.h) | 0 `RemoveBindings`, 2 `IsButtonDown`, 3 `GetPovAngle`, 4 `GetPovDirection` | `UnknownJoystickBoolView` primary vtable (object_offset 0) as `JoystickDevice` | 2 | ControlInterface activeJoystick (+0x0c); slot 4 (index, *x, *y) = GetPovDirection |
| `UnknownPodiumArcade::UnknownVirtualSlotN` (Group1_PodiumScene.cpp) | 1 `AddRef`, 2 `Release`, 3 `GetRefCount`, 4 `Disable`, 5 `Enable` | `UnknownPodiumArcade` primary vtable (object_offset 0) as `GameObject` | 2 | ArcadeObject : GameObject (RTTI) |
| `UnknownPodiumArcade::UnknownVirtualSlotN` (EventManagerNearMisses.cpp) | 1 `AddRef`, 2 `Release`, 3 `GetRefCount`, 4 `Disable`, 5 `Enable` | `UnknownPodiumArcade` primary vtable (object_offset 0) as `GameObject` | 2 | ArcadeObject : GameObject (RTTI) |
| `UnknownGuiControl::UnknownVirtualSlotN` (GUIManager.h) | UIControl's names for slots 27-33 (7 slots, e.g. 27 `AdvanceImages`, 33 `KillFocus`) | `UnknownGuiControl` primary vtable (object_offset 0) as `UIControl` | 2 | dialog control; 31 int (focus), 32/33 take a control like EnterHover/LeaveHover/KillFocus |
| `ShadowTexture::UnknownVirtualSlotN` (ProjectedShadow.h) | PCTextureMap's names for slots 5-12 (8 slots, e.g. 5 `LoadSurface`, 12 `RecreateIfLost`) | `ShadowTexture` primary vtable (object_offset 0) as `PCTextureMap` | 2 | built by PCTextureMap's constructor 0x004c5f00 |
| `ParticleTexture::UnknownVirtualSlotN` (ParticleManager.h) | 4 `CreateSurface`, 5 `LoadSurface`, 6 `Clone`, 7 `IsResident`, 8 `CreateTextureSurface` | `ParticleTexture` primary vtable (object_offset 0) as `TextureMap` | 2 | TextureMap returned by 0x0050a590 |
| `ShadowCameraProxy::UnknownVirtualSlotN` (ProjectedShadow.h) | 27 `TakeScreenshot`, 28 `UpdateMatrices`, 29 `LookAt` | `ShadowCameraProxy` primary vtable (object_offset 0) as `Camera` | 2 | ShadowCamera ctor 0x004da520 (ShadowCamera : PCCamera : Camera) |
| `VisibilityView::UnknownVirtualSlotN` (VisibilityQuadTree.h) | RenderTarget's names for slots 1-6 (6 slots, e.g. 1 `BeginScene`, 6 `GetTextureStageState`) | `VisibilityView` primary vtable (object_offset 0) as `RenderTarget` | 2 | renderer at camera+0x18; slot 7 SetTextureStageState |
| `VisibilityRenderer::UnknownVirtualSlotN` (VisibilityQuadTree.h) | RenderTarget's names for slots 1-15 (15 slots, e.g. 1 `BeginScene`, 15 `DrawIndexedPrimitive`) | `VisibilityRenderer` primary vtable (object_offset 0) as `RenderTarget` | 2 | renderer at camera+0x18; slot 16 draws a primitive |
| `GridRenderDevice::UnknownVirtualSlotN` (Griddraw.h) | RenderTarget's names for slots 1-15 (15 slots, e.g. 1 `BeginScene`, 15 `DrawIndexedPrimitive`) | `GridRenderDevice` primary vtable (object_offset 0) as `RenderTarget` | 2 | renderer at Terrain+0x18; slots 7/8/15 match RenderTarget |
| `CollisionDebugDevice::UnknownVirtualSlotN` (CollisionObject.cpp) | RenderTarget's names for slots 1-10 (10 slots, e.g. 1 `BeginScene`, 10 `SetTextureBlend`) | `CollisionDebugDevice` primary vtable (object_offset 0) as `RenderTarget` | 2 | renderer at renderContext; slots 7/8/10 match RenderTarget |
| `UnknownScenePhysicsBody::UnknownVirtualSlotN` (SceneManagerNearMisses.cpp) | SoultreePhysicsObject's names for slots 0-40 (39 slots, e.g. 0 `UpdateMass`, 40 `LoadPhysicsModel`) | `UnknownScenePhysicsBody` primary vtable (object_offset 0) as `SoultreePhysicsObject` | 2 | ctor 0x005037c0 writes the SoultreePhysicsObject vtables (object_offset 0) |
| `UnknownTrackGameObject56cItem::UnknownVirtualSlotN` (TrackGame.h) | UIDialog's names for slots 1-26 (24 slots, e.g. 1 `AddRef`, 26 `MarkPendingRelease`) | `UnknownTrackGameObject56cItem` primary vtable (object_offset 0) as `UIDialog` | 2 | its EndDialog is UIDialog::EndDialog 0x0046ff30 |
| `UIStatic::UnknownVirtualSlotN` (CompilerShapeProbe.cpp) | 30 `IsInteractive` | `UIStatic` primary vtable (object_offset 0) as `UIControl` | 2 | same retail VA 0x00478fe0 as UIStatic::IsInteractive (GameUi.h) |
| `UIMultiState::UnknownVirtualSlotN` (CompilerShapeProbe.cpp) | 34 `GetControlText`, 35 `GetTextLength`, 36 `GetTextLines`, 37 `GetTextExtent` | `UIMultiState` primary vtable (object_offset 0) as `UIControl` | 2 | same retail VAs 0x00478260..0x004782c0 as UIMultiState slots 34-37 (GameUi.h) |
| `ParticleDirect3D::UnknownVirtualSlotN` (ParticleManager.h) | 3 `EnumDevices`, 4 `CreateDevice` | `ParticleDirect3D` primary vtable (object_offset 0) as `IDirect3D7` | 2 | slot 5 is CreateVertexBuffer(desc, &vb, flags): only IDirect3D7 puts it at 5; its 3/4 are EnumDevices/CreateDevice |

### COM interface types

Each type is a slot-ordered view of a COM interface (`this` on the stack,
`__stdcall`). Every decoded slot index and argument count matches the
DirectX 7 / DirectShow SDK interface named, and the IID the game passes (in
`.rdata`; DirectXGuids.cpp holds the ones the bundled DXGUID.LIB lacks) or
the creating call identifies the interface. The names drop the SDK's `I`
prefix, as `DirectDrawSurface` did, so no SDK type is used and the mangled
names stay ours. Uncalled slots keep `UnknownMethodN`.

| Old name | New name | Interface | Tier | Evidence |
| --- | --- | --- | --- | --- |
| `UnknownDirectSound` | `DirectSound` | IDirectSound | 2 | Result of the `DirectSoundCreate` import (thunk `0x005330a0`); slot 3 CreateSoundBuffer(desc, &buffer, outer), 4 GetCaps (DSCAPS size 0x60), 5 DuplicateSoundBuffer, 6 SetCooperativeLevel |
| `UnknownSoundBuffer` | `DirectSoundBuffer` | IDirectSoundBuffer | 2 | Created by `DirectSound` slot 3 with a 0x24-byte DSBUFFERDESC; GetCurrentPosition 4, GetStatus 9, Lock 11 (7 arguments), Play 12, SetCurrentPosition 13, SetVolume 15, SetFrequency 17, Stop 18, Unlock 19, Restore 20 |
| `UnknownSound3DBuffer` | `DirectSound3DBuffer` | IDirectSound3DBuffer | 2 | Queried from a buffer with the bytes of IID_IDirectSound3DBuffer (`0x00556ca0`); SetAllParameters 12 through SetVelocity 20 in SDK order |
| `UnknownSoundListener` | `DirectSound3DListener` | IDirectSound3DListener | 2 | Queried from the primary buffer with IID_IDirectSound3DListener (`0x00556c90`); SetDistanceFactor 11 through CommitDeferredSettings 17 |
| `UnknownSoundNotify` | `DirectSoundNotify` | IDirectSoundNotify | 2 | Queried with IID_IDirectSoundNotify (`0x00556cd0`); slot 3 SetNotificationPositions(count, positions) |
| `UnknownPropertySet` | `KsPropertySet` | IKsPropertySet | 2 | Queried with IID_IKsPropertySet (`0x00556ce0`); Set 4 and QuerySupport 5 with the EAX listener property set (`0x00556d20`) |
| `UnknownSoundDevice` | `SoundDeviceRecord` | (not COM) | 2 | The 0xf0-byte record (GUID, 0x80-byte description, DSCAPS) that the `DirectSoundEnumerateA` callback `0x004be7b0` fills for up to four devices through `ProbeSoundDevice` (`0x004be2d0`) |
| `UnknownDirectDrawInterface` | `DirectDraw` | IDirectDraw7 | 2 | IID_IDirectDraw7 (`0x005560b0`) with the `DirectDrawCreateEx` import; CreateClipper 4, CreatePalette 5, CreateSurface 6, EnumDisplayModes 8, GetCaps 11, GetDisplayMode 12, GetGDISurface 14, SetCooperativeLevel 20, SetDisplayMode 21 (5 arguments), WaitForVerticalBlank 22, GetAvailableVidMem 23, GetDeviceIdentifier 27 |
| `UnknownDirect3DInterface` | `Direct3D` | IDirect3D7 | 2 | Queried from `DirectDraw` with IID_IDirect3D7 (`0x00556180`); CreateDevice 4 (GUID, surface, &device), EnumZBufferFormats 6 |
| `UnknownRenderInterface` | `Direct3DDevice` | IDirect3DDevice7 | 2 | Created by `Direct3D` slot 4; BeginScene 5, EndScene 6, Clear 10 (6 arguments), SetTransform 11, SetViewport 13, SetMaterial 16, SetRenderState 20, DrawPrimitive 25, DrawIndexedPrimitive 26, DrawIndexedPrimitiveVB 32, SetTexture 35, SetTextureStageState 37, Load 43 |
| `UnknownClipperInterface` | `DirectDrawClipper` | IDirectDrawClipper | 2 | Created by `DirectDraw` slot 4 CreateClipper; slot 8 SetHWnd(flags, window) |
| `UnknownPaletteInterface` | `DirectDrawPalette` | IDirectDrawPalette | 2 | Created by `DirectDraw` slot 5 CreatePalette; released through slot 2 |
| `UnknownGammaControlInterface` | `DirectDrawGammaControl` | IDirectDrawGammaControl | 2 | Queried from the primary surface with IID_IDirectDrawGammaControl (`0x00556140`); slot 4 SetGammaRamp(flags, ramp) |
| `UnknownInputInterface` | `DirectInputDevice` | IDirectInputDevice7A | 2 | Created by IDirectInput7A::CreateDeviceEx (the `DirectInputCreateEx` import with IID_IDirectInput7A `0x005569e0`; IID_IDirectInputDevice7A at `0x00556a50`); Unacquire 8 before Release, CreateEffect 18, SendForceFeedbackCommand 22 |
| `UnknownEffectInterface` | `DirectInputEffect` | IDirectInputEffect | 2 | Created by `DirectInputDevice` slot 18 CreateEffect; Start 7, Stop 8, GetEffectStatus 9 |
| `UnknownMultiMediaStreamInterface` | `AMMultiMediaStream` | IAMMultiMediaStream | 2 | Created with CLSID_AMMultiMediaStream / IID_IAMMultiMediaStream (`0x00558ec0`/`0x00558ed0`); GetMediaStream 4, GetState 6, SetState 7, GetTime 8, GetDuration 9, Seek 10, Initialize 12, AddMediaStream 15, OpenFile 16 |
| `UnknownMediaStreamInterface` | `DirectDrawMediaStream` | IDirectDrawMediaStream | 2 | Queried with IID_IDirectDrawMediaStream (`0x00558e90`); GetFormat 9, CreateSample 13 |
| `UnknownStreamSampleInterface` | `DirectDrawStreamSample` | IDirectDrawStreamSample | 2 | Created by `DirectDrawMediaStream` slot 13 CreateSample; Update 6 (4 arguments), CompletionStatus 7 |

Left as placeholders in wave 4:

- `GameObject` slots 9 (`0x00469430`) and 19 (`0x00469530`) and `Game` slot 12
  (`0x004688e0`, which returns the root object's slot 19): child-walking
  dispatchers (child flags 0x01 and 0x100) whose event is not identified by a
  literal, caller or override.
- `FollowCamera` slots 55 and 56: the base bodies test DirectInput left Alt
  and left Shift, but `KrustyBikeCamera` tests controls 0x0b and 0x0a, whose
  names are not established; slot 56 enters camera state 7. Slot 50 returns a
  point on the subject (the rider's head, or the vehicle raised by 5).
- `Vehicle` slots 52, 77, 78 and 79, `KrustyBike` slot 27 (starts the tumble
  or the animated wreck) and `SoultreePhysicsBaseObject` 26/27: roles come
  only from call sites, and the base bodies are not known well enough.
- `PhysicsBody` slots 31/32: setters of the unnamed fields +0x180 and +0x184.
- `UIDialog` slot 30 (`0x00470040`): calls the optional callback at +0xd4,
  whose role is unknown.
- Shared stubs: `ShadowReceiver`, `D3DIMSoultreeShadow` and `TerrainShadow`
  slots 27-30, `BackgroundImage` 27, `BaseQuarryEvent` 28/32/34,
  `ConnectionInfoType` 1 (`ret`), `ShadowCamera` 19, `Terrain` 19,
  `ConstraintMethodCollisionModel` 2, and `CollisionPoint`/`SoultreeContact`
  slot 1.
- Views whose object is not established: `KbSink`, `UnknownGameObjectOwnerPart`,
  `UnknownFollowedRacerSlots` and the vbase racer views (`UnknownEventRacer`,
  `UnknownVisualCueRacer`, `UnknownBikeRaceView6c`, `UnknownSceneObject`),
  `VehicleTicker`, `BikeBodyObj` and the quadtree-head views.
  `UnknownBikeRaceCharacter` is a `D3DIMSoultreeCharacter`, whose own slots are
  still placeholders (`CharacterVirtualSlotN`), and `VehicleContactPoint` is a
  `CollisionPoint`, whose slot 1 is a placeholder.
- `UnknownSceneBuffer`: not an interface, only a count and an array of
  `UnknownSceneCaster` at Scene+0xb8.

## Wave 5

Wave 5 names fields of the race and network views and two DirectSound
structures. The 99 affected translation units (`src/`, `samples/` and
`tests/fixtures/`) were compiled with VC6 from the base commit and from the
renamed tree in the same directory: 95 are NAME-ONLY and 4 (`DlgProcs.cpp`,
`DlgProcsNearMisses.cpp`, `Group4_DlgMain.cpp`, `Group6_BikeRaceCreate.cpp`)
are NAME-ONLY-COMDAT-ORDER. Every non-debug section, relocation and symbol
value is equal.

`SendRaw` (`0x004ad280`) stores the type byte at +0x00 of every message it
sends, and the sequence at +0x02 when the type has bit 0x80. `NetMessage::Set`
(`0x004aacc0`) hands the receiver the same bytes. Its type is the first byte,
or the whole first dword when the sender is 0 (a DirectPlay system message).
No `Send` call in the retail image passes type 5, so message 5 is a system
message.

| class | offset | old | new | tier | evidence |
| --- | --- | --- | --- | --- | --- |
| `UnknownKrustyVcrRecord` | +0x04 | `field_0x04` | `points` | 2 | Same 8-byte grid entry as `UnknownTrackGameModeEntry` (`KrustyVCR::gridEntries` copies race settings +0x14c), whose +0x04 is `points`. |
| `UnknownBikeRaceNetPlayer` | +0x04 | `field_0x04` | `points` | 2 | View of the same grid entry (TrackGame+0x2ebc). |
| `UnknownBikeRaceNetState` | +0x00 | `field_0x00` | `type` | 2 | Message 1: `SendRaw` writes the type byte. |
| `UnknownBikeRaceNetState` | +0x01 | `field_0x01` | `finishPosition` | 2 | The sender's message 1 layout (`KbBikeMessage::finishPosition`). |
| `UnknownBikeRaceNetScore` | +0x00 | `field_0x00` | `type` | 2 | Message 10, as `KbBikePing`. |
| `UnknownBikeRaceNetStamp` | +0x00 | `field_0x00` | `header` | 2 | Messages 0x84/0x85: type byte and sequence (`SelectGamePicProcs` convention). |
| `UnknownBikeRaceVcrFollow` | +0x00 | `field_0x00` | `header` | 2 | Message 0x87, sent by `0x004a9d20`. |
| `UnknownBikeRaceVcrFollow` | +0x04 | `field_0x04` | `request` | 2 | `KbNetPacket::request`: `SendFollowedRacer` sends 0x13 or 0x87 with this layout; `0x004a9d20` stores 0. |
| `KbNetPacket` | +0x00 | `field_0x0` | `header` | 2 | Messages 0x13/0x87 (`SendFollowedRacer` `0x004925a0`). |
| `KbBikeMessage` | +0x00 | `field_0x00` | `type` | 2 | Message 1 (`0x004928e8` sends it). |
| `KbBikePing` | +0x00 | `field_0x00` | `type` | 2 | Message 10. |
| `KbBikeDeltaMessage` | +0x00 | `field_0x00` | `type` | 2 | Message 13. |
| `KbNetDelta` | +0x00 | `field_0x00` | `type` | 2 | Received message 13; `BikeRace` passes the message itself to `ApplyStateDelta`. |
| `KbTrack` | +0xac | `field_0xac` | `gateCount` | 2 | Modulus of the lap gates (`nextLapGate = (lapGate + 1) % gateCount`). `UnknownRadarGateOwner` and `UnknownBikeRaceNodeOwner` name +0xac `gateCount`. |
| `UnknownChatMessage` (TrackOverlay.h) | +0x00 | `field_0x00` | `header` | 2 | Message 0x85 (`0x0051db4b`), as `SelectGamePicProcs`' `UnknownChatMessage`. |
| `UnknownLoadFailedMessage` | +0x00 | `field_0x00` | `header` | 2 | Message 0xcc (`LoadingDlg::Tick`). |
| `UnknownPlayerStateMessage` | +0x00 | `field_0x00` | `type` | 2 | Message 2 (`0x004f5703`). |
| `UnknownQuarryLeaveMessage` | +0x00 | `field_0x00` | `type` | 2 | Message 5 is a system message, so the first dword is its type. |
| `UnknownPlayerLeftMessage` (SelectGamePicProcs.cpp) | +0x00 | `header` | `type` | 2 | Correction: the same message 5, which `SendRaw` never writes. |
| `UnknownQuarryQuadTreeNode` (two views) | +0x04, +0x06, +0x08 | `int field_0x04`, `int field_0x08` | `queryStamp`, `sortKey`, `objectTypeId` | 2 | The `CollisionObject` base (`QuadTreeObject` in CollisionObject.h); the derived view's methods are `CollisionObject`'s. Merged into `CollisionObject` in wave 8. |
| `UnknownSoundCaps` | (type) | `UnknownSoundCaps` | `DirectSoundCaps` | 2 | `GetCaps` is passed size 0x60. The 24 dwords are DSCAPS in order. |
| `UnknownSoundBufferDesc` | (type) | `UnknownSoundBufferDesc` | `DirectSoundBufferDesc` | 2 | Size 0x24: the DirectX 7 DSBUFFERDESC fields in order. |

Left as placeholders in wave 5:

- `UnknownWaveFormat`: its `sizeof` is 20, while the SDK's packed WAVEFORMATEX
  is 18, so it is not the SDK layout.
- `TrackGameViewOwner::field_0xac`: one struct stands in for three owners.
  +0xac is a gate count in two of them, and the tag owner stores 5.0f there
  (`0x004a9f01`).
- `TrackGameMode::field_0xa20` (and `UnknownTrackGameModeOptionsA20::field_0x00`,
  `UnknownOptSoundSettings::field_0x00`):
  - EventManager `0x0045e7b6` calls `StopCDAudio` (`0x00521a40`) when it is 0.
  - `ResetSoundOptions` (`0x005227d0`) clears it.
  - No dialog control or INI key is bound to it, so "CD music on" is only tier 3.
- `TrackGameMode` fields:
  - +0x6a4 and +0x1bdc are only written.
  - +0x25e8 is only created and destroyed.
  - +0xa2c, +0xa44, +0xa58 and +0xa60 are never addressed; only bulk copies touch them.
- `UnknownTrackGameViewOwnerDc::field_0x10a`: `0x004a9ef0` clears it on the tag
  ball. It may be the `SoultreePhysicsBaseObject` asleep flag, but the class
  is not established.
- `UnknownBikeRaceColliderBase` (+0x04/+0x08): its shape is a `CollisionObject`
  base, but the object (30SecMan +0x210) is not established as one.
- Ghost records (`UnknownBikeRaceGhostRecord::field_0x00`,
  `UnknownBikeRaceGhostDelta::field_0x00`): replay kinds 0x11/0x10 are not
  network messages, so the `SendRaw` evidence does not apply.
- `KbNetState::field_0x00` is a decoded state, not a message.
- `UnknownQuarryLeaveMessage::field_0x04` and
  `UnknownPlayerLeftMessage::field_0x04`: the SDK's DPMSG_DESTROYPLAYERORGROUP
  puts dwPlayerType there. That is external context only.
- `UIDialog` +0x180/+0x184/+0x188: loader `0x0046a920` fills them from
  section lookups whose defaults are the `MouseAnim`, `FXAnimIn` and
  `FXAnimOut` buffers. The buffer flow is not proven.
- `UIEditBox` +0x1f4/+0x1f8: no access in the edit box methods.
- `UIControl` +0x1c8: the writers found (`0x00467990`, `0x004685d0`) use
  offsets beyond `UIControl`, so they belong to another class.
- `DebugSocket::field_0x08`: the constructor sets it to 1000, and nothing reads it.
- Padding (`UnknownZone*::field_0x03`, the quarry camera's +0x393) and the
  padding arrays in RaceView.h and Krusty3DObjects.h.

## Wave 6

Wave 6 names the `Tire` members by physics role, the `CollisionPoint` fields
that `Tire` and `SoulTreePhysics.cpp` use, and parameters whose callee already
names them. The 70 affected translation units (`src/`, `src/bringup/` and
`tests/fixtures/`) were compiled with VC6 from the base commit and from the
renamed tree in the same directory. All 70 are IDENTICAL: the renamed members,
parameters and locals appear only in debug sections. The 71st unit
(`tests/fixtures/bvh_vector_compare.cpp`) needs a header that its test
generates; the suite builds it. No line moved, so `__LINE__` values are
unchanged.

The constructor arguments come from `Bike::LoadBike` and the `VehicleWheel`
view (the same constructor, `0x00512f10`). The `aN` numbers follow that
constructor's argument order. `VehicleWheel` (Vehicle.h) and `BikeWheel`
(Bike.h) are views of `Tire`. `TireVehicle` is a view of `Vehicle` at offset 0,
and `SoultreeContact` is a view of `CollisionPoint`.

| class | offset or param | old | new | tier | evidence |
| --- | --- | --- | --- | --- | --- |
| `Tire` | +0x1d8 | `field_0x1d8` | `driveTorqueScale` | 2 | Constructor argument a11 (Bike passes 0.5/1.0). ApplyDrive `0x00514170` adds `cross(relativePosition, push) * it` to the torque. |
| `Tire` | +0x1dc | `field_0x1dc` | `levelRiseRate` | 2 | Constructor argument a12. Vehicle `0x00529280`: `rampLevel += stepTime * it`, clamped. |
| `Tire` | +0x1e0 | `field_0x1e0` | `levelFallRate` | 2 | Constructor argument a13. Vehicle `0x00529280`: `rampLevel -= stepTime * it`, clamped. |
| `Tire` | +0x20c | `wheelUpAxis` | `wheelForwardAxis` | 2 | Correction. UpdateSuspensionProbe stores the node-rotated `kVec3ZAxis`, else the caller's `savedForward` (body +0xa0). It is the `rollDirection` fallback. |
| `Tire` | +0x224 | `wheelVelocity` | `bodyPosition` | 2 | Correction. UpdateSuspensionProbe copies its position argument here. Both callers (`0x00528f65`, `0x00528fd8`) pass body +0x0c, `position`. |
| `Tire` | +0x248 | `field_0x248` | `driveForce` | 2 | Force output of the engine's `DriveForce` (Vehicle). UpdateDriveShare scales it by `driveShare`. |
| `Tire` | +0x268 | `field_0x268` | `shockEngaged` | 2 | UpdateShock sets it after a shock solve. UpdateSuspensionProbe clears it. |
| `Tire` | +0x26c | `field_0x26c` | `shockExtending` | 2 | UpdateShock copies the shock's flag (+0x54) here. |
| `Tire` | +0x280 | `field_0x280` | `driveMagnitude` | 2 | Magnitude output of `DriveForce`. UpdateDriveShare: `driveMagnitude * invMass - abs(acceleration)`. |
| `Tire` | +0x290 | `field_0x290` | `driveDistance` | 2 | UpdateRoll: `driveShareComplement * driveMagnitude * dt * driveScale`. |
| `Tire` | +0x294 | `field_0x294` | `restLoadWeight` | 2 | Same field as `VehicleWheel::restLoadWeight`. Constructor argument a14. |
| `Tire` | +0x2a0 | `field_0x2a0` | `driveStrength` | 2 | Constructor argument a10. ApplyDrive scales the push by it (`surfaceGrip * driveStrength * rampLevel`). |
| `Tire` | +0x2a8 | `field_0x2a8` | `engine` | 2 | Vehicle calls `UnknownEngineGearbox::DriveForce` on it. Bike's loader stores `engineState` there. |
| `Tire` | +0x2ac | `field_0x2ac` | `rotatingShock` | 2 | Its shock solve is RotatingShock `0x004fac60`. The destructor releases it when `inlineShock` is null. |
| `Tire` | +0x2b0 | `field_0x2b0` | `inlineShock` | 2 | Its shock solve is InlineShock `0x004fa400`. Released first by the destructor `0x004f9f80`. |
| `Tire` | +0x2b4 | `field_0x2b4` | `slipThreshold` | 2 | Constructor argument a7 (Bike passes 45.0f). UpdateDriveShare `0x00513f90` compares `slipSpeed` with it. |
| `Tire` | +0x2b8 | `field_0x2b8` | `driveShare` | 2 | UpdateDriveShare `0x00513f90` sets it; 1.0 otherwise. |
| `Tire` | +0x2bc | `field_0x2bc` | `driveShareComplement` | 2 | UpdateDriveShare: `1 - driveShare`. |
| `Tire` | ctor | `field_0x2b4_`, `field_0x2a0_`, `field_0x1d8_`, `field_0x1dc_`, `field_0x1e0_` | `slipThreshold_`, `driveStrength_`, `driveTorqueScale_`, `levelRiseRate_`, `levelFallRate_` | 2 | The fields they are stored into. Also in `VehicleWheel` (Vehicle.h, Group2_VehicleWheel.cpp). |
| `Tire` | UpdateSuspensionProbe | `velocity`, `angle`, `radiusScale`, `a5`, `a6`, `a7`, `a8` | `position`, `pitch`, `sinRoll`, `yaw`, `forward`, `up`, `node` | 2 | Callers `0x00528f65`/`0x00528fd8` pass body +0x0c, +0x4c, +0x54, +0x50, +0xa0, +0xac and +0x42c, whose `SoultreePhysicsBaseObject`/`Vehicle` names these are. `velocity` was a misreading of `position`. |
| `Tire` | UpdateContactPatch | `pos`, `axis`, `ref`, `minLength`, `outValue`, `outSign` | `center`, `angularVelocity`, `velocity`, `speed`, `outSpeed`, `outForward` | 2 | Single caller `0x00529aa7` passes `centerOfMass`, `worldAngularVelocity`, `velocity`, `linearSpeed`. |
| `TireAux` | +0x00 | `value` | `throttleFraction` | 2 | Tire+0x2a8 is the engine gearbox, and its +0x00 is `UnknownEngineGearbox::throttleFraction` (GearRatios.h). |
| `TireVehicle` | +0x24 | `field_0x24` | `invMass` | 2 | View of `SoultreePhysicsBaseObject` at offset 0. |
| `TireVehicle` | +0x60 | `field_0x60` | `savedCosPitch` | 2 | Same view. |
| `TireVehicle` | +0x70 | `field_0x70` | `acceleration` | 2 | Same view. |
| `TireVehicle` | +0xa4 | `field_0xa4` | `savedForwardY` | 2 | y of `savedForward` (+0xa0). |
| `TireVehicle` | +0x444 | `field_0x444` | `crashState` | 2 | `Vehicle::crashState`. |
| `VehicleWheel` | ApplyDrive | `force`, `torque` | `torque`, `force` | 2 | Correction: the last two names were swapped. `Tire::ApplyDrive` adds the moment to its sixth argument and the push to its seventh, and Vehicle passes `(..., worldTorque, force)`. |
| `BikeWheel` | +0x20c | `wheelUpAxis` | `wheelForwardAxis` | 2 | View of `Tire`. |
| `Bike` | LoadBike locals | `roll`, `grip`, `a12`, `a13` | `sideFrictionScale`, `driveTorqueScale`, `levelRiseRate`, `levelFallRate` | 2 | The constructor parameters they are passed to. |
| `CollisionPoint` | +0x5c | `field_0x5c` | `rotationalVelocity` | 2 | Tire `0x005135f0`: `angularVelocity x relativePosition`, where the caller passes `worldAngularVelocity`. |
| `CollisionPoint` | +0x68 | `field_0x68` | `normalLoadVector` | 2 | Vehicle `0x00527a20`: `surfaceNormal * normalLoad`. |
| `CollisionPoint` | +0xa8 | `field_0xa8` | `impactPosted` | 2 | Vehicle slot 18 sets it when it posts the contact effect (wheel +0x160), and slot 30 clears it. |
| `CollisionPoint` | +0xac | `field_0xac` | `slidePosted` | 2 | Vehicle slot 19 sets it when it posts the slide effect (`0x0052a07e`, wheel +0x164), and slot 30 clears it. |
| `CollisionPoint` | +0xb0 | `field_0xb0` | `scrapePosted` | 2 | Vehicle slot 20 sets it when it posts the scrape effect (`0x0052a33f`, wheel +0x168), and slot 30 clears it. |
| `CollisionPoint` | +0xb4 | `field_0xb4` | `contactState` | 2 | `SoultreeContact` view: slot 31 stores 0 or 1 (it keeps 2), and slot 13 reads it. Stays `float`-typed (no type changes). |
| `SoultreeContact` | +0x20 | `field_0x20` | `surfacePosition` | 2 | `CollisionPoint::surfacePosition`. Slot 18 places the contact effect there. |
| `SoultreeContact` | +0x98 | `field_0x98` | `penetration` | 2 | `CollisionPoint::penetration`. |
| `SoultreeContact` | +0xac/+0xb0 | `field_0xac`, `field_0xb0` | `slidePosted`, `scrapePosted` | 2 | As `CollisionPoint`. |
| `SoultreePhysicsBaseObject` | +0x20e | `field_0x20e` | `settledLastFrame` | 2 | Slot 21 stores `(char)settled` at the end. The type-4 Restart runs only on the rise (`settled && !settledLastFrame`). The constructor clears it. |
| `SoultreeGlobals` | +0x574 | `field_0x574` | `sceneObject` | 2 | View of `TrackGame`, whose +0x574 is `sceneObject` (TrackGame.h). |
| `SoultreePhysicsObject` | LoadPhysicsModel | `a4` | `options` | 2 | Forwarded to slot 9 (`0x0043f4b0`) as its fourth argument. |
| `D3DIMSoultreeObject` | slot 9 | `a1`..`a5` | `owner`, `name`, `lights`, `options`, `useOptions` | 2 | Names of `D3DIMSoultreeObject::LoadModel` (tier 2). Also the views in Group5_D3DIMForwarders.cpp, D3DIMSoultreeMotnctrl.cpp (`Load`) and CarProcedural.h (`LoadModel`, whose `carModel` is `new D3DIMSoultreeObject(1)`). |
| `CarProcedural` | Create | `a3`, `a4` | `lights`, `options` | 2 | Forwarded to `carModel->LoadModel` (slot 9) unchanged. |
| `SoultreePhysicsCharacter` | LoadPhysicsModel | `a5` | `loadInfo` | 2 | Forwarded to `D3DIMVirtualSlot11`. Bike passes its info record. |
| `Vehicle` | LoadVehicle | `a5` | `loadInfo` | 2 | Forwarded unchanged to `SoultreePhysicsCharacter::LoadPhysicsModel`. |
| (free) | SoultreeRefreshContacts | `a1` | `enabled` | 2 | The SoulTreePhysics.cpp declaration names it `int enabled`. The caller passes `ShouldRefreshContacts()`. |

Left as placeholders in wave 6:

- `Tire` +0x254, +0x2a4 and +0x298 (with the constructor parameter
  `field_0x298_`): they are only written.
- `TireContactOwner` +0x40, +0x64 and +0x224, `MovingPart` +0x00 and +0x48,
  and `CollisionObject::field_0x5c` as `Tire` uses it: no role was proven.
- `CollisionPoint` +0xb8: only the constructor writes it (1.0).
- `SoultreePhysicsBaseObject` +0x28 and `field_0x164[9]`: only the
  constructor writes them.
- `SoultreeLoadDesc` +0x25: only the bit test `& 1` is known.
- `D3DIMVirtualSlot11`'s fifth argument and `CollisionCharacter`'s `a6`: they
  are stored at +0x68, and nothing shows what reads them.
- The `aN` parameters of `Scene::Load`/`ReadStaticModels` (a3 to a5) and
  `Grid` a1, and the chain `KrustyBike` a16 to `Bike` a19 to `KbBikeLoader`
  a13: no callee or arithmetic names them.
- `Tire::UpdateShock`'s `offset`: the caller passes a float (`linearSpeed`)
  through a pointer-typed parameter. A name would need a type change, so it
  waits.
- `aN` mentions inside comments that map argument positions (the
  `LoadPhysicsModel` grouping notes): they number the stack dwords.

## Wave 7: UI, text and front-end types

Wave 7 names the text service, the pro circuit career, the KrustyUI
records and the TrackGame objects behind the menus. Type renames change
mangled names, so the bindings, calibration cases, emulator cases and
fixture keys were updated with them. Each affected unit was compiled with
VC6 before and after (057d2c9e): every one is identical, or differs only
in the renamed symbols (some with COMDAT sections in a different order);
four test fixtures that need their own flags or generated headers fail to
compile the same way on both sides.
No line moved.

| Old name | New name | Tier | Evidence |
| --- | --- | --- | --- |
| `UnknownOverlayText` | `TextService` | 2 | The constructor `0x0050ac30` pushes the TextService.cpp `__FILE__` literal (`0x00574b30`, line 22) and `Create` `0x0050ae80` uses it at line 113. The unit holds every method, as for `VCR` and `ResourceManager` in wave 3 ([TEXTSERVICE](TEXTSERVICE.md)). |
| `UnknownOverlayVertex` | `TLVertex` | 2 | The pre-transformed, lit vertex drawn with FVF `0x1c4` (D3DTLVERTEX: x, y, z, rhw, color, specular, tu, tv). |
| `TextService` +0x10, +0x2c | `bounds`, `vertices` | 2 | The constructor stores `(x, y)-(x + width, y + height)` in the +0x10 rectangle; `LayoutText` fills and returns the +0x2c quads. |
| `Rectangle2D` +0x04, +0x10, `UnknownSet` | `topLeft`, `bottomRight`, `SetCorners` | 2 | The text blitters use +0x04 as the glyph origin and `+0x10 - +0x04` as its extent; the inline setter stores both corners. |
| `UnknownTrackGameObject3444` | `ProCircuit` | 2 | The constructor `0x004d3480` pushes the ProCircuit.cpp `__FILE__` literal (`0x005718ac`, xref `0x004d34a1`); the unit `0x004d3480..0x004d4b1f` holds only its methods ([PROCIRCUIT](PROCIRCUIT.md)). |
| `UnknownProCircuitStats`, `...Racer`, `...Track`, `...Schedule` | `ProCircuitStats`, `ProCircuitRacer`, `ProCircuitTrack`, `ProCircuitSchedule` | 2 | The career records `ProCircuit` reads from PCTables.pb/PCSched.pb and saves. |
| `UnknownSetText` (ProCircuitProcs.cpp) | `SetResourceText` | 2 | Its body is `SetTextFromResource(g_TrackGame->resourceInstance, id)`. |
| `UnknownKrustyUIModel` (SelectGamePicProcs.h) | `KrustyUIModelRecord` | 1 | One `KrustyUI::LoadBikesAndRiders` record of bikes.pb `"Manufacturer_%02d"` or riders.pb `"Rider_%02d"`. |
| `KrustyUIModelRecord` +0x40, +0x80, +0xc4 | `modelFile`, `uiModelFile`, `availability` (were `modelName`, `field_0x80`, `modelKind`) | 1 | The keys `"ModelFile"` (a rider's `"File"`), `"UIModelFile"` and `"Availability"` stored there (KrustyUI.cpp). |
| `UnknownKrustyUIBike` (SelectGamePicProcs.h) | `KrustyUIBikeRecord` | 1 | The `"BikeName/BikeTexture/BikeEngineSize/BikeFourStroke_%02d"` record. |
| `KrustyUIBikeRecord` +0x00 | `manufacturer` (was `model`) | 2 | Index into KrustyUI+0x48 (the manufacturers); KrustyUI.cpp's and QuarryStuntEvent.cpp's views already call it that. |
| `ProCircuit` +0x48 | `maxAvailability` (was `maxModelKind`) | 2 | Compared with `availability` when the racers' bikes and riders are picked (and by QuarryStuntEvent). |
| `UnknownTrackGameObject33fc` | `ControlSetup` | 3 | TrackGame+0x33fc (`controlSetup`). It loads and saves control.ctl under `"Setup"`/`"CurrentInputDevice"` and `"Controller%d"`/`"Key%d"`. No RTTI or `__FILE__` names it, so the name is descriptive ([DEVICESETUP](DEVICESETUP.md)). |
| `UnknownControlAssignment` | `ControlAssignment` | 2 | The `"%d,%d"` kind and code of one `ControlSetup` entry. |
| `UnknownTrackGameObject574` | `SceneArchiveFinder` | 3 | TrackGame+0x574 (`sceneObject`): opens the scene archive and finds files in it. The name its header (SceneArchiveFinder.h) and the collision notes already used; no RTTI or `__FILE__` names the class. |
| `UnknownEventRacerPart` | `RaceStatusNode` | 2 | The 0x54-byte node RaceStatus.cpp allocates with DebugCalloc under its `__FILE__` literal (`0x00572990`, lines 214/235/257) and frees; the unit's list helpers build, order and free it ([RACESTATUS](RACESTATUS.md)). |
| `BikeRace::UnknownFunctionCameraView` | `RaceCameraView` | 2 | Inline accessor: returns `raceCamera` as the camera view. |

Left as placeholders in wave 7:

- `TextService::field_0x08` (0.2): only the constructor writes it.
- `EventManager` +0x43c: a sweep of the unit (`0x0045b000..0x0045f800`)
  finds no access.
- `UnknownEventStanding` +0x04/+0x08: only `CompareStandings`
  (`0x0045d3d0`) compares them, and `RankRacers` never writes them.
- `GUIUser` +0xc4 and +0x1dc, `GUIManager` +0x1f8 and +0x304 (and
  `UnknownFunction4864f0`, which returns +0x1f8): the constructors clear
  them and one site reads or compares each; no writer of a value was found.
- `KrustyUI` +0x38 (OpenMenu's argument when the 0x911 track record closes;
  no writer found), +0x49c, +0x4a0, +0x4a4 (cleared on a control press),
  +0x4ac (set when leaving the menus for a race) and the +0x634 slots.
- `GameUi.h`: `UnknownGameUiListRow` +0x2c/+0x30, `UIControl` +0x1c8 and
  the edit box's +0x1f4/+0x1f8 are only cleared. The int members of the
  unions (`field_0x1ec`, `field_0x204`, `field_0x208`, `field_0x218`) are
  the views the code reads; merging them is a type change.
- Griddraw.h's terrain views (`GridGameSettings` +0x94 and the padding):
  not UI, and no role was proven.
- KrustyUI.cpp's `UnknownKrustyUIModelEntry`/`UnknownKrustyUIBikeEntry` and
  QuarryStuntEvent.cpp's `UnknownQuarryBikeModel`/`UnknownQuarryBikeChoice`
  were views of the two KrustyUI records (merged in wave 8).
  `ProCircuitRank`, `ProCircuitResult` and `ProCircuitSkinned` stay views.

## Wave 8: view merges

Wave 8 folds per-unit views into the class they mirror where the merge is
provably codegen-neutral. Each merge was checked with a before/after VC6
SP3 `/O2` object comparison (`work/naming/objcmp.py`, ignored) of every
unit whose include closure reaches a changed file, against 029dd2a8. It
compares all non-debug section bytes, relocations and symbols. It
normalises the `__FILE__` tree root, anonymous-namespace stamps and the
compiler's local label counters (`$T`, `$L`, `$SG`, ...), and applies an
explicit old-to-new mangled-name map to the base side. A unit passes when
it is identical, or identical apart from the mapped names and possibly
COMDAT order. Where a merge deletes a bring-up forwarder, the forwarder's
COMDAT is named with `--drop` and must be the only other difference
(NAME-ONLY+DROPPED). Negative controls: changing one float constant in
the loader (`1024.0f` to `1023.0f`) and leaving out the name map both
report DIFF. Calibration was unchanged (3058 cases, 3005/3014 strict
pairs, no per-case difference).
The whole-object pin for QuarryStuntEventLoaderNearMiss.cpp in
tests/fixtures/eco_light_objects.json hashes symbol names, so it was
repinned (same 50 sections and 435 relocations; the comparer reports
NAME-ONLY).

| View | Canonical | Files | Evidence |
| --- | --- | --- | --- |
| `UnknownKrustyUIModelEntry`, `UnknownKrustyUIBikeEntry` (KrustyUI.cpp), `UnknownQuarryBikeModel`, `UnknownQuarryBikeChoice` (QuarryStuntEvent.cpp) | `KrustyUIModelRecord`, `KrustyUIBikeRecord` (SelectGamePicProcs.h) | KrustyUI.cpp, QuarryStuntEvent.cpp, SelectGamePicProcs.h, BikeRace.h (comment) | Same offsets (+0x00 name, +0x40 model file, +0x80 UI model file, +0x48 texture, +0x8c engine size, +0x90 four-stroke). 38 units: 26 identical, 12 identical apart from COMDAT order. `uiModelFile` became `char[0x40]` because a rider's file may run past `modelFile` into it. |
| `UnknownQuarrySkyCube` | `SkyCube` (SkyCube.h) | QuarryStuntEventLoaderNearMiss.cpp (+ bindings), Group3_QuarryLoad.cpp, Group3_QuarryLinkEco.cpp, Group3_QuarryLoadLinks.h | Constructor 0x004fb1e0 and Create 0x004fb230, 0x48 bytes. Sample NAME-ONLY; loader and link unit NAME-ONLY+DROPPED (the view forwarder and `G3Q_SkyCubeCreate`). |
| `UnknownQuarryEcoSystem` | `EcoSystem` (QuarryEvent.h now declares `Create`, 0x004594d0, with EcoSystem.h's signature) | as above, QuarryEvent.h; Group3_QuarryLinkEco.cpp deleted | Constructor 0x00457250, 0x5c4 bytes. All ten units that include QuarryEvent.h or the link header: seven identical, sample NAME-ONLY, loader NAME-ONLY+DROPPED. |
| `UnknownQuarryShadow` | `ProjectedShadow` (krusty2 shadow/ProjectedShadow.h) | sample (+ bindings), Group3_QuarryLoad.cpp, Group3_QuarryLinkKrusty.cpp, Group3_QuarryLoadLinks.h, tools/emu_cases/projected_shadow_init.py (docstring) | Constructor 0x004da570, Init 0x004da7b0, AddCaster 0x004dab00, SetShadowStrength 0x004dace0, SetShadowGrey 0x004dae30, SetLight 0x004dae50; 0x134 bytes. Arguments are cast to the canonical parameter types. Sample NAME-ONLY; loader and link unit NAME-ONLY+DROPPED (five forwarders, five `G3Q_ProjectedShadow*` helpers). |
| `UnknownQuarryQuadTreeNode`, `UnknownQuarryCollision` | `CollisionObject` (krusty2 collision/CollisionObject.h: `QuadTreeObject` at +0, `GraphicsTest : GameObject` at +0xc) | sample (+ bindings), Group3_QuarryLoad.cpp, Group3_QuarryLinkKrusty.cpp, Group3_QuarryLoadLinks.h | SetUseBroadphase 0x00432120, SnapPlacement 0x00435fe0, SetIgnoreListMode 0x00439400, AddIgnoredOwner 0x00439410 (takes `void*`, as canonical), SetOwner 0x004394d0; `Release` goes through the +0xc GameObject as before. Sample NAME-ONLY; loader and link unit NAME-ONLY+DROPPED. |

Blocked merges:

- BikeRace.h's `UnknownBikeRaceUiModel` and `UnknownBikeRaceUiChoice` (to
  `KrustyUIModelRecord`/`KrustyUIBikeRecord`). Removing the two structs from
  BikeRace.h changes `BikeRace::CreateRacers` in
  Group6_BikeRaceCreateRacers.cpp: `xor edi, edi` and `mov ebx, 1` at +0x21a
  swap. Keeping them declared and switching only the uses moves the same
  swap to samples/race/BikeRaceNearMisses.cpp instead. VC6's scheduling
  there depends on the set of declared types, so no neutral form was found.
- `UnknownPodiumSound` (Group1_PodiumScene.cpp, Group1_PodiumViews.cpp,
  EventManagerNearMisses.cpp) to `Sound`. Sound is declared only in
  PCAudio.h, which includes `<windows.h>`. Group1_PodiumScene's code bytes
  stayed identical, but its `.drectve` gains `-defaultlib:uuid.lib` twice.
  In the sample, IMM32's prototype clashes with the local
  `ImmAssociateContext(void*, void*)` declaration (C2733, through
  LeaveRaceForMenu.cpp).
- `UnknownQuarryRace` to `BikeRace`. Including BikeRace.h in the loader
  sample adds an `eh vector constructor iterator` (`??_H`) COMDAT to the
  object. The view's racer and character types also differ from
  BikeRace's.
- Not attempted:
  - `UnknownTexturePalette` to `ColorMapper`. About 40 consumers, including
    the TextureMap/PCTextureMap slot signatures, calibration and emulator
    case symbols and the whole-object palette fixtures.
    [PALETTE_GETTER_CONNECTIONS](PALETTE_GETTER_CONNECTIONS.md) keeps the
    prefix view on purpose.
  - `UnknownGuiControl` to `UIControl`. `LeaveHover`/`KillFocus` take the
    view where UIControl takes `int`, and GUIUser's API uses the view.
  - `UnknownBikeRaceModel` to `D3DIMSoultreeObject`. It lives in BikeRace.h,
    which is scheduling-sensitive as shown above.

## Wave 9: view merges

Wave 9 folds the remaining Group3 quarry-loader views with the Wave 8
method and comparer. Each merge was compared against the commit before it.
A cumulative run from 96b4bcc8 to the final tree applies the full name map
to all six affected units: the sample is NAME-ONLY, Group3_QuarryLoad and
Group3_QuarryLinkKrusty are NAME-ONLY+DROPPED, and Group3_QuarryLinkCamera,
Group3_QuarryLinkRace and KrustyUI are IDENTICAL. Group3_QuarryLinkKrusty
loses its `__fltused` reference together with its last float forwarder
(`G3Q_VisibilitySetup`). The eco-light whole-object pin for the sample was
repinned after each NAME-ONLY result (still 50 sections and 435
relocations). Calibration was unchanged (3058 cases, 3005/3014 strict
pairs, no per-case difference).

| View | Canonical | Files | Proof |
| --- | --- | --- | --- |
| `UnknownQuarryParticles` | `ParticleManager` (krusty2 effects/ParticleManager.h) | sample (+ bindings), Group3_QuarryLoad.cpp | Constructor 0x004ba390, slot 27 Create 0x004ba4d0 (ret 0x10), 0x2058 bytes. Loader IDENTICAL, sample NAME-ONLY. |
| `UnknownQuarryVisibility` (bring-up only) | `VisibilityQuadTree` (krusty2 visibility/VisibilityQuadTree.h: QuadTree at +0, GameObject at +0x874) | Group3_QuarryLoad.cpp, Group3_QuarryLinkKrusty.cpp, Group3_QuarryLoadLinks.h, tools/emu_cases/quarry_load_event_objects.py, GAME_BUILD.md | Constructor 0x0052d340, Setup 0x0052d460 (ret 0x18, returns the GameObject part). Loader and link unit NAME-ONLY+DROPPED (Setup forwarder, `G3Q_VisibilitySetup`). |
| `UnknownQuarryTerrainObject` | `Terrain` (krusty2 broadphase/Terrain.h) | sample (+ bindings), Group3_QuarryLoad.cpp, Group3_QuarryLinkKrusty.cpp, Group3_QuarryLoadLinks.h, tools/emu_cases/terrain_load.py (docstring) | Constructor 0x00505830, Load 0x005059d0, 0xcc4 bytes. Sample NAME-ONLY; loader and link unit NAME-ONLY+DROPPED (Load forwarder, `G3Q_TerrainLoad`). |
| `UnknownQuarryTerrainShadow` | `TerrainShadow` (src/reconstructed/TerrainShadow.h) | sample (+ bindings), Group3_QuarryLoad.cpp, Group3_QuarryLinkKrusty.cpp (its local TerrainShadow view too), Group3_QuarryLoadLinks.h | Constructor 0x00508ae0, Attach 0x00508b80, 0x6780 bytes. Sample NAME-ONLY; loader and link unit NAME-ONLY+DROPPED (Attach forwarder, `G3Q_TerrainShadowAttach`). |

Blocked merges:

- The sample's `UnknownQuarryVisibility`. The near miss passes the scene
  as a seventh argument to the `ret 0x18` Setup. Calling the canonical
  six-argument Setup changes `LoadEventObjects`' bytes and relocations.
  The bring-up already corrects this.
- `UnknownQuarryRaceObject` to `SoultreeQuadTreeRenderer`.
  - SoultreeQuadTreeRenderer.h declares its own TU-local `SoultreeObject`
    and `D3DIMSoultreeObject`.
  - The loader and the sample include EventManager.h, LightEmitter.h and
    SceneManager.h, which reach D3DIMSoulTree.h's definitions of the same
    classes, so the compiler reports C2011.
  - `G3Q_QuadTreeRendererInsertNodes` therefore keeps
    Group3_QuarryLinkKrusty.cpp alive.
- `UnknownQuarryBikeCamera` to `KrustyBikeCamera`.
  - KrustyBikeCamera.h itself shares the loader unit without errors.
  - The loader writes nine VehicleCamera, BikeCamera and KrustyBikeCamera
    members directly: `vehicle`, the two cached flags, `vehicleMode`, the
    three targets, `bike` and `krustyBike`, plus FollowCamera's
    `isSettled`. The canonical headers declare these members `protected`.
  - The loader stores one racer view where the canonical headers declare
    three pointer types (`UnknownCameraVehicle*`, `UnknownCameraBike*`,
    `UnknownKrustyBike*`). It reads `bike->modelNode` and
    `bike->riderCharacter` through the racer view's types.
  - The merge needs an access decision or a recovered inline setter chain
    (the store order looks like inlined VehicleCamera, BikeCamera and
    KrustyBikeCamera setters). It is not a name-only change.
- KrustyUI.cpp's `UnknownKrustyUIShadow` to `ProjectedShadow`. The
  function-local static `shadow` is mangled with its pointee type. Renaming
  that type moves the static within `.bss` (0x2ec to 0x44; `.bss` shrinks
  from 0x2f0 to 0x2ec), which changes `BuildGarageScene`'s relocation
  addends. Neither layout follows retail's order (0x0067c3f8..0x0067c4a4).
  The view and the `config/game_build.json` Init link alias therefore stay.
- `UnknownQuarryRace` to `BikeRace` was not retried (Wave 8 blocker).

## Wave 10: placeholder types

Wave 10 names mid-tier `Unknown*` types that are not views of a named
class. Each rename went through every tracked consumer, including source,
bindings keys, calibration and emulator case symbols, tests and docs. It
also covered mangled names, where the identifier follows a type code and
ends at `@`. The proof is the Wave 8 comparer, run against a snapshot of
09687116 with the cumulative old-to-new map. It covers every unit in src,
samples and tests/fixtures whose include closure reaches a changed file.
Every unit is IDENTICAL, IDENTICAL-COMDAT-ORDER, NAME-ONLY or
NAME-ONLY-COMDAT-ORDER. Fixtures that make VC6 `/O2` fail with C1001 are
compiled with `/Od`, as their tests do (follow_camera_tick_compare.cpp).

Negative controls:

- Leaving out the map reports DIFF for the nine PeakHold NAME-ONLY units.
- Changing a live float compare in Terrain.cpp (`dir.y > 0.0f` to `0.25f`
  in `TerrainClipRayToPlaneY`) reports DIFF: a new `__real` constant and
  changed relocations.
- Renaming Game.cpp's file static `s_UnknownFile65b4ac` *with* the map
  applied still reports DIFF. VC6 places `.bss` statics by name, not by
  declaration order, and the rename moved it from 0x2c to 0x4c. Swapping
  the declaration order of two statics leaves every offset unchanged. This
  is the mechanism behind the Wave 9 `UnknownKrustyUIShadow` blocker. None
  of the Wave 10 renames moved a `.bss` symbol.
- Rebuilding unchanged units gives IDENTICAL objects. COMDAT reorders
  therefore come from the renamed identifiers.

Whole-object and partial pins hash symbol names. After the comparer passed,
they were repinned by rerunning their tests. Only digest lines changed;
section and relocation counts were unchanged. Pins whose rows sort COMDATs
did not move. Calibration was unchanged (3058 cases, 3005/3014 strict
pairs, no per-case difference), as were the physics samples (1085/1152
strict) and the unit layout sweep (130 identical).

| Old name | New name | Tier | Evidence |
| --- | --- | --- | --- |
| `UnknownPeakHold` | `PeakHold` | 3 | 12-byte value (0x004cb670..0x004cb6d8). `Record` raises the peak or restarts it once `holdTime` has passed; `Peak` returns 0 when stale. Used for Game.cpp's ten profile-page peaks and the terrain, eco and visibility cache peaks. No string names it. 50 units: 38 identical, 3 COMDAT order, 9 name-only. |
| `UnknownEcoDefinition`, `UnknownEcoCollisionDefinition` | `VegetationDefinition`, `VegetationCollisionDefinition` | 3 | One "Vegetation_%d" section of the .est file (0x210 bytes, constructor 0x00455de0, keys "BillboardName", "MeanHeight", ...), and one of its "CollisionObject%i" entries (0x24 bytes). `Vegetation` is the placed instance. 9 units: 5 identical, 4 name-only. |
| `UnknownEngineGearbox`, `UnknownGearShiftRange`, `UnknownTorque()`, `UnknownPower()` | `EngineGearbox`, `GearShiftRange`, `TorqueAtCurrentRpm()`, `PowerAtCurrentRpm()` | 3 | 0x1e4 bytes, 0x004d2940..0x004d333f, allocated by Vehicle.cpp. It builds the rpm/torque curve ("Rpm Limits (%d,%d) invalid in %s") and the per-gear shift windows ("Gear Ratios invalid in %s", "Unable to find optimum shift range for GEAR=%d"). The two inline lookups interpolate the torque and power curves at `currentRpm`. 38 units: 30 identical, 1 COMDAT order, 7 name-only. |
| `UnknownSoultreeLod`, `UnknownSoultreeSurface` | `SoultreeLod`, `SoultreeSurface` | 2 | The .slt reader's section names "LOD %i" and "LOD %i - Surface %i" (with "- Vertices", "- Faces" and "- Object Pointer List") name the 8-byte LOD and the 0x38-byte surface records. |
| `UnknownSoultreeVertex`, `UnknownSoultreeFaceGroup`, `UnknownSoultreeUV` | `SoultreeVertex`, `SoultreeVertexGroup`, `SoultreeUV` | 3 | The 0x20-byte vertex in the D3DLVERTEX layout (FVF 0x1e2). The 0x14-byte per-node vertex group transformed by `TransformVertexGroups`. The u/v pair. The soultree families together: 80 units, 65 identical, 11 COMDAT order, 4 name-only. |
| `UnknownTextureRegion` | `CacheTextureRegion` | 3 | 0x30-byte quadtree node of a CacheTexture page (RTTI `CacheTexture`), taken from TextureMapManager+0x70's pool; a leaf holds at most one ManagedTexture. 36 units: 25 identical, 11 name-only. |
| `UnknownGameUiListRow` | `UIListBoxRow` | 3 | 0x38-byte row of UIListBox's (RTTI) `rowTable` (+0x214). GameUi.cpp allocates, grows, sorts and clears the table in rows of this size. 70 units: 57 identical, 13 COMDAT order (the type is in no symbol). |
| `UnknownPCAudioObject`, `UnknownPCAudioObject::UnknownThreadProc` | `SoundMemoryManager`, `SoundMemoryManager::LoaderThreadProc` | 3 | The 0x54-byte object at PCSoundInterface+0x46c (the field is already `soundMemoryManager`). Its thread (0x004bdc00, started by `StartLoaderThread`) creates and fills queued streamed sounds and evicts idle ones to stay within the byte budget. No string or RTTI names it. `UnknownSoundNotifier::UnknownThreadProc` (0x004bb630) is a different function and stays. 40 units: 32 identical, 3 COMDAT order, 5 name-only. |
| `UnknownTrackGameObject3400` | `TrackRecord` | 2 | The 0xe8-byte high-score table at TrackGame+0x3400. Its constructor 0x0051ef30 pushes the TrackRecord.cpp `__FILE__` literal (0x0057536c), as for `ProCircuit` and `TextService`. The unit also holds `TrackRecordDlg` (RTTI), so the class name is tier 2 at most ([TRACKRECORD](TRACKRECORD.md)). |
| `UnknownTrackRecord` | `TrackRecordEntry` | 3 | The table's 0x14-byte entry (16-byte name and a float time or score; constructor 0x0051ee90). TrackRecord.h's families: 173 units, 146 identical, 12 COMDAT order, 15 name-only (cumulative map). |
| `UnknownBikeRaceNode` | `RaceGate` | 3 | The 0x3c-byte box chained from BikeRace+0xc8. `AppendNode` DebugCallocs one per course gate, and RaceStatus counts laps through it. "Gate" is the game's own term ("LstNumGates", "ChkRandomGates", "StartGate.slb"). The start and finish probes (+0xcc, +0x108) share the layout. 180 units: 151 identical, 10 COMDAT order, 16 name-only, 3 name-only with COMDAT order (cumulative). |

Left as placeholders:

- EcoSystem `field_0x588[8]` and `field_0x598`. The only write is
  `field_0x598 = 0xcd` in the constructor, and no reader was found.
- EngineGearbox `field_0x74`, `field_0x80`, `field_0x14c` and
  `field_0x1d8`. A scan of retail operands finds them written only inside
  0x004d2940..0x004d3340 (the constructor), so no consumer gives a role.
- UIListBoxRow `field_0x2c` and `field_0x30`.
- `UnknownSoultreeMesh` (MorphBastardModifier.h). It is a view of
  `SoultreeSurface` with MorphBastard member types (`MorphBastardVertex*`,
  `MorphBastardFace*`, `int* uvs`), so merging it is a type change, not a
  rename.
- `UnknownSoultreeScreenRect` and `UnknownSoultreeCameraView`. The first
  holds two 16-byte blocks, and only the one at +0x298 is shown to be a
  rectangle. The second is a partial camera view that ObjectPicker.h's
  `UnknownPickCamera` mirrors.
- `UnknownBikeRaceBikeSetup` (BikeRace.h). It is a view of `KbBikeSetup`
  (krusty2 vehicle/KrustyBikeTypes.h), but `fourStroke` is `int` here and
  `char` there. BikeRace.h is scheduling-sensitive (Wave 8), so the merge
  needs its own byte proof.
- `UnknownBikeRaceNodeOwner`, `UnknownBikeRaceNodeSource` and
  `UnknownBikeRaceNodeEntry`. These are the event-side gate records that
  `BuildNodeList` reads, which are different types from `RaceGate`.
- Out of scope by request: the views of named classes (`UnknownEventRacer`,
  `UnknownGuiControl`, `UnknownBikeRaceRacer`, `UnknownKrustyBikeView`,
  `UnknownQuarryRace`, `UnknownTexturePalette`) and the shared stubs
  `UnknownFunction4de580`, `UnknownFunction464e80` and
  `UnknownFunction464e90`.

## Wave 11: hotspot fields and the gate event records

Wave 11 targets the headers with the most `field_0x` placeholders (BikeRace.h,
TrackGame.h, Krusty3DObjects.h, RaceView.h, TrackOverlay.h, Griddraw.h) and
the event-side gate records left in Wave 10. Most hotspot hits are padding
arrays (`unsigned char field_0xNN[...]`) between named members, not fields.
Krusty3DObjects.h and RaceView.h hold only padding. Of the typed
placeholders, only the ones below have a reader that gives them a role. The
rest are listed under "Left as placeholders". `git grep -c field_0x -- src
samples` fell from 2034 to 2016 lines.

Method: a new before/after comparer compiles every unit in src, samples and
tests/fixtures whose include closure reaches a changed file. It compiles
each unit twice, from an archive of b991bf7c and from a copy of the renamed
tree. The two roots have equal length, so `__FILE__` literals keep their
size. The comparer compares non-debug section bytes, relocations and
symbols, applying the old-to-new map
(`UnknownBikeRaceNode{Owner,Source,Entry}` to `RaceGate{Event,Pose,Object}`).
It normalises the tree root inside literals and the string-literal name
hash that covers it, anonymous-namespace stamps and `$T`/`$L`/`$SG`
counters. Units under `src/` use the game build's include directories.
Samples and fixtures compile as their tests do. Three fixtures stop VC6
`/O2` with C1001 (follow_camera_controls_compare.cpp,
follow_camera_tick_compare.cpp and profile_displays_compare.cpp), so they
are compiled with `/Od`, as their tests do.

Result: 237 units. 226 are IDENTICAL. 6 are IDENTICAL-COMDAT-ORDER: the RTTI
`.rdata$r`/`??_R0` COMDATs of the dialog units (DlgProcs.cpp, KrustyUI.cpp,
Group4_DlgMain.cpp and three UI near-miss samples) move, as in earlier
waves. 5 are NAME-ONLY: BikeRace.cpp, BikeRaceNearMisses.cpp,
Group3_QuarryLinkRace.cpp, Group6_BikeRaceCreate.cpp and
Group6_BikeRaceCreateRacers.cpp, whose symbols mangle the gate types.

Negative controls:

- Without the map, BikeRace.cpp reports DIFF (`AppendNode`'s section, whose
  mangled name holds `UnknownBikeRaceNodeSource`, is only in the old object).
- Swapping CompareDisplayModes' subtraction in VideoCard.cpp
  (`a->bufferCount - b->bufferCount` to `b->bufferCount - a->bufferCount`)
  reports DIFF: the bytes of `?CompareDisplayModes@@YAHPBX0@Z` change.

| Old name | New name | Tier | Evidence |
| --- | --- | --- | --- |
| `UnknownTrackGameModeOptions6ac360::field_0x00`..`field_0x10` | `curve125cc`, `curve250cc`, `curve400cc`, `curve500cc`, `curve600cc` | 2 | The five ints at Options6ac+0x360 (TrackGameMode+0xa0c, `garageCurveChoice[5]`), which OptionProcs.h reads as `curveRows[BikeClassOf(cc)]`. Class i is `g_BikeClassEngineSizes[i]`, read from retail .data at 0x0056cb6c as 125, 250, 400, 500, 600. |
| `UnknownDisplayMode::field_0x10` | `bufferCount` | 2 | `CreateModeSurfaces` (0x004ca520) budgets `w*h*bpp/8*buffers` and passes `buffers - 1` to `CreateFlipChain`, which sets `backBufferCount` with DDSD_BACKBUFFERCOUNT. Enumeration stores 2. `CompareDisplayModes`, `FindDisplayMode` and PCGame's duplicate filter compare it. |
| `UnknownBikeRaceNodeOwner` | `RaceGateEvent` | 3 | The event as `BuildNodeList` (0x0041d170) reads it. `BikeRace::Create` (0x00417ed0) receives the race's event (`BaseQuarryEvent::LoadEventObjects` 0x004de590 pushes `this`) and appends one `RaceGate` per gate: `gateCount` at +0xac, placements at +0xd8, gate objects at +0x420. "Gate" is the game's term ("LstNumGates", "ChkRandomGates", "StartGate.slb"). It is a view: BaseQuarryEvent is 0xa4 bytes and NationalRace's +0xac is `finishTextShown`, and no retail allocation of an event with gates was found. |
| `UnknownBikeRaceNodeSource` | `RaceGatePose` | 3 | The 0x18-byte record at RaceGateEvent+0xd8 + 0x18*i. `AppendNode` (0x0041d0d0) copies its center and direction into the new RaceGate. RadarOverlay reads the same records as `UnknownRadarGate` {position, direction}. |
| `UnknownBikeRaceNodeEntry` | `RaceGateObject` | 3 | The per-gate object at RaceGateEvent+0x420. `BuildNodeList` passes its +0x50 width to `AppendNode`. SelectLapGates (0x00404df0) flags the object at its +0x58. |

The gate types remain distinct from `RaceGate` (the 0x3c-byte box chained from
BikeRace+0xc8) and from the other views of the same records
(`UnknownRadarGateOwner`/`UnknownRadarGateEntry` in TrackOverlay.h,
`TrackGameViewOwner`/`UnknownTrackGameViewEntry` in TrackGame.h). Merging
them would be a type change, not a rename.

Left as placeholders:

- Padding arrays in all six hotspot headers. Krusty3DObjects.h and RaceView.h
  contain nothing else.
- RaceGateObject `field_0x54`. It goes to AppendNode's last argument and
  then to `RaceGate::field_0x28`, and no reader was found.
- TrackGameViewOwner `field_0xac`. It is a gate count in the gated owners,
  but the tag owner stores a float there (Wave 5). UnknownTrackGameViewEntry
  `field_0x58` and its target's `field_0x44`. SelectLapGates writes 1 to the
  +0x44 flag for the two gates it leaves and 0 for the two it picks. No
  reader of the flag was found, so "hidden" or "inactive" is a guess.
- UnknownTrackGameViewOwnerDc `field_0x10a`. It may be the physics `asleep`
  byte, but the ball's class is unproven.
- TrackGameMode `field_0x6a4`, `field_0xa20`, `field_0xa2c`, `field_0x1bdc`
  and `field_0x25e8`. Options6ac `field_0x34`..`field_0x44`, OptionsA20
  `field_0x00`/`0x0c`/`0x24` and OptionsA4c `field_0x0c`/`0x14`. Each is
  only cleared, bulk-copied, or has an unexplained single test.
- `UnknownEventStanding::field_0x04`/`field_0x08` (EventManager.h).
  `CompareStandings` breaks ties on them, but the reconstructed `RankRacers`
  (0x0045e9d0) fills only `points` and `racer` of its stack array.
- BikeRace.h `UnknownBikeRaceViews::field_0x40` and RaceSound's matching
  `field_0x40`/`field_0x48`. The group is created or zeroed, then only
  paused and unpaused. `UnknownBikeRaceNetState::field_0x04` spans
  KbBikeMessage's two shorts (`lapCount`, `gatesPassed`), so naming it would
  need a type split. Ghost `field_0x5e0` and the ghost record's
  `field_0x04`: delta1 feeds +0x5e0, the slot KrustyBike calls
  `deltaBasePosition`, which may mean the existing ghost names are swapped.
- Griddraw.h. `field_0x2d54` (GridVertexCache), GridNodeExtra `field_0x10`,
  GridBlockRecord `field_0x04`, `field_0x12e`, `field_0x17a`, `field_0x2b`
  and `field_0x94` are only written. Terrain `field_0xc9c`/`field_0xca0`
  are the float views of the `unlitColor`/`unlitSpecular` unions.
- TrackOverlay.h `field_0x170` is only written.
- VCRfile.h `field_0x00`. Open/Load claim an entry when it and `size` are 0,
  and Close clears it, but nothing sets it.

## Wave 12: read-backed placeholder fields

Wave 12 names scalar `field_0xNN` members that retail code reads, instead of
the members of the densest headers. Leads came from three places: the
readers left open in Wave 11, a scan of each class's bound methods for
non-store reads of `[reg + 0xNN]` at the placeholder's offset, and members
next to literals (registry keys, `.dtm` section markers). A scan hit is only a
lead, because the register can hold another object. Each name below was
checked against the decoded readers. `git grep -c field_0x -- src samples`
fell from 2016 to 1907 lines. No `UnknownVirtualSlotN` was renamed (see the
end of this section).

Method: as in Wave 11. The comparer built every unit in src, samples and
tests/fixtures whose include closure reaches a changed file (359 units, plus
57 for the last three families), once from an archive of b6adb807 and once
from a copy of the renamed tree. The map holds the one mangled rename,
`UnknownMethod4c3ae0` to `SetJoystickFilter`. Two changes to the comparer:

- VC6 also names anonymous namespaces `?%<source path><path hash>@`. The path
  holds the tree root, so Group2_ShadowCameraLookAt.cpp first reported DIFF
  on `StoreNormalised`. The comparer now normalises that form too, and the
  unit compares IDENTICAL.
- bvh_vector_compare.cpp includes `bvh_vector_patches.h`, which its test
  writes from the retail image. Both trees were compiled with the same stub
  header (`static void InstallPatches() {}`), and the objects are IDENTICAL.

Result: of the 359 units, 348 are IDENTICAL (with the two above). 6 are
IDENTICAL-COMDAT-ORDER: the RTTI COMDATs of the dialog and race units move
(DlgProcsNearMisses.cpp, SelectGamePicProcsNearMisses.cpp,
Group4_DlgMain.cpp, Group6_BikeRaceCreate.cpp, BikeRace.cpp,
ProCircuitProcs.cpp), as in earlier waves. 4 are NAME-ONLY:
PCJoystickDevice.cpp, OptionProcs.cpp and the fixture
krusty_ui_create_compare.cpp, plus KrustyUI.cpp, which is NAME-ONLY with the
same COMDAT move.
The 57 units that reach the last three families (BackgroundImage,
D3DIMSoultreeModifier, MorphBastardModifier) were compiled again after them:
51 IDENTICAL and the same 6 IDENTICAL-COMDAT-ORDER.

Negative controls:

- Without the map, PCJoystickDevice.cpp and krusty_ui_create_compare.cpp
  report DIFF (`?UnknownMethod4c3ae0@PCJoystickDevice@@QAEXE@Z` is only in the
  old object). OptionProcs.cpp and KrustyUI.cpp report DIFF on the
  relocations of the callers (`OptAdvancedGraphicsDlg::HandleEvent`,
  `KrustyUI::Create`).
- Live changes report DIFF. `joystickFilter = !value` in SetJoystickFilter
  changes the bytes of `?SetJoystickFilter@PCJoystickDevice@@QAEXE@Z`, and
  `texelSize = 0.015625f` in TerrainShadow's constructor changes the bytes
  of `??0TerrainShadow@@QAE@H@Z`.

tests/fixtures/eco_light_objects.json pins a digest of the whole
KrustyUI.cpp object. After the NAME-ONLY result it was repinned: the section
and relocation counts (317, 1847) are unchanged and only the digest moves. The
calibration case for 0x004c3ae0 (tools/run_calibration.py), the bindings in
KrustyUI.bindings.json and OptionProcs.bindings.json, the fixture
krusty_ui_create_compare.cpp and test_joystick_buffered_behavior.py's layout
probe use the new names.

| Old name | New name | Tier | Evidence |
| --- | --- | --- | --- |
| `PCJoystickDevice::field_0x5d5` | `joystickFilter` | 1 | KrustyUI.cpp and OptionProcs.cpp pass the registry flag `"JoystickFilter"` to 0x004c3ae0, which stores it here. FilterAxisSpike (inline in ReadBufferedState) keeps the previous axis value when the jump exceeds 10000 only when it is set. |
| `PCJoystickDevice::UnknownMethod4c3ae0` | `SetJoystickFilter` | 1 | The same: its only callers pass the `"JoystickFilter"` registry flag, and its body stores the byte at +0x5d5. |
| `UnknownGameUiSection::field_0x38` | `sectionKind` | 2 | ReadDialogResource stores 0 for `"Set_Info"`, 3 for `"Set_Default"`, and otherwise the kind of the last marker section (`"Set_Anim"` 1, `"Set_Sound"` 2, `"Set_Control"` 4). Each pass switches on it. |
| `KrustyUI::field_0x38` | `trackRecordReturnMenu` | 3 | Its only reader, DlgProcs at 0x0045314d, reopens it with `OpenMenu` when the 0x911 track-record dialog closes. Only the constructor (0x00498824) writes it, with 0. |
| `VCRDlg::field_0x7f58` | `leftForMenu` | 2 | HandleEvent sets it after `LeaveRaceForMenu(kMenuMain)` and `ShowCursors(1)`. On kDialogClose, `ShowCursors(0)` is skipped when it is set. |
| `Overlay::field_0x118` | `layOutOnRender` | 3 | The constructor's second argument. Every reconstructed caller passes 1. Render3D (0x004b61a0) calls LayOutQuad before drawing when it is set. The constructor parameter has the same name. |
| `GUIUser::field_0xc4` | `keepAnimationPhase` | 3 | SetCursorAnimation (0x00487fb0) copies `currentFrame`, `playForwards` and `lastStepTime` from the current animation to the new one when it is set. The constructor clears it; no other writer is known. |
| `BackgroundImage::field_0x5c` | `drawnBefore` | 3 | The constructor clears it. RenderPost3D (0x00403ec0) passes it as DrawImage's `dirty` argument and then sets it, so the first draw skips the region restore. |
| `UnknownBikeRaceGhost::position` (+0x5d4) | `velocity` | 3 | ApplyReplayDelta (0x004a9aa0) adds delta0 here. KrustyBike's ApplyStateDelta (0x004933e0) adds the same message field to `deltaBaseVelocity`, and SendStateDelta encodes `message.velocity` there. |
| `UnknownBikeRaceGhost::field_0x5e0` | `position` | 3 | delta1 goes here, the field KrustyBike adds to `deltaBasePosition` and SendStateDelta encodes from the position. This resolves the swap noted in Wave 11. |
| `UnknownBikeRaceGhostRecord::position` (+0x28), `field_0x04` | `velocity`, `position` | 3 | The record fields that the ghost's +0x5d4 and +0x5e0 are copied from (RecordFrame 0x00421d50) and into (0x004a9aa0). |
| `TerrainShadow::field_0x6668`..`field_0x6674` | `screenMinX`, `screenMaxX`, `screenMinY`, `screenMaxY` | 2 | Slot 27 resets them to FLT_MAX/-FLT_MAX and keeps the min/max of each projected hit's screen x and y. The visibility test compares them with the viewer's width and height and requires a 10 x 5 extent. |
| `TerrainShadow::field_0x677c` | `texelSize` | 2 | Slot 28 sets it to 0.5 or 1.0 over the texture size, the value D3DIMSoultreeShadow's slot 28 keeps in `g_d3dimShadowTexel`. Slot 30 scales tu and tv by it. |
| `ProjectedShadow::field_0x98` | `downsampleOnPresent` | 2 | The constructor sets 1 and Init clears it for surface format 0x613. Present (0x004dbd30) returns unless it is set, then halves the clip rectangle and box-filters 2x2 texels. The receivers' slot 28 uses 0.5 / size when it is set. |
| `ShadowCaster::field_0x27c` | `currentLod` | 2 | ShadowCaster is a D3DIMSoultreeObject view: its methods are 0x00444fe0 and 0x00445030 (`GetVertexCount`, `GetSurfaceCount`), and D3DIMSoulTree.h names +0x27c `currentLod`. 0x004db8c0 calls ReleaseParts unless it is -1. |
| `SoultreeObject::field_0x150` (core view) | `boundsFound` | 2 | D3DIMSoulTree.h's name at +0x150. UpdateSubtreeBounds (0x004fe0f0) and AccumulateBounds (0x004fe2e0) include the local bounds only when it is set. |
| `SoultreeObject::field_0x190` (core and quadtree views), `D3DIMSoultreeObject::field_0x190` | `drawsItself` | 3 | 1 after construction. InsertNodesInQuadtree (0x005046e0) clears it and RemoveNodesFromQuadtree (0x005047f0) sets it again. D3DIMSoultreeObject's slot 12 (PrepareGeometry) does its work only when it is set. |
| `D3DIMSoultreeObject::field_0x28c`, `field_0x2d4` (quadtree view) | `lodTable`, `visible` | 2 | D3DIMSoulTree.h's names at the same offsets of the same class. The view keeps its `int` types. |
| `D3DIMSoultreeModifier::field_0x3c` | `onFirstModifierList` | 2 | The constructor sets 1 and MorphBastardModifier clears it. MorphBastardModifier is added with AddSecondModifier (KrustyBike.cpp). DetachFromAll removes the modifier from the first list (+0x264) when it is set and from the second (+0x26c) otherwise. |
| `ParticleManager::field_0x2054` | `lightManager` | 2 | Slot 27 stores its last argument here. The render slot passes it to LightManager::LightVertices for the corner normals. The argument is renamed `lights`. |
| `ParticleManager::field_0x1f8c` | `quadVertexBuffer` | 2 | An FVF 0x1c4 vertex buffer of 4000 vertices. The render slot locks it, writes four transformed vertices per visible particle and draws it with DrawIndexedPrimitiveVB. |
| `PCGame::field_0x544` | `bladeLoaded` | 2 | Main passes it as GetDXVersion's third output, which is 1 when `LoadLibrary("Blade.dll")` succeeds. PCGame and PickDevice skip the z-buffer and pick devices differently when it is set. |

Left as placeholders:

- HiResMeter `field_0x38` (GameTick). Registration and resets write it
  (0x00468668, 0x00468687, 0x00488576, 0x004885b1, 0x00467de4). The reads at
  +0x38 in Tick are of another object (0x0056e26c), so no reader is known.
- Recorder.h `UnknownRecorderRecord::field_0x04` and TrackRecordDlg.h
  `UnknownTrackRecordRow::field_0x04`. No reader. 0x0051fbe7 reads only the
  row's +0x00.
- UnknownTrackGameViewEntry +0x58, its target's +0x44 flag and RaceGateObject
  `field_0x54` (and so `RaceGate::field_0x28`). A search along the pointer
  chain found no reader.
- FollowCamera `field_0x33c`. It is the tenth `CAMERA` float of a `sscanf`
  and is never read.
- UIScrollBar and UIEditBox `int` aliases of named unions. They are
  intentional views.
- ConstraintMethodCollisionModel +0xbc, +0xd8, +0xdc and +0xe0. The scan hits
  are reads through `this` adjusted by `lea ecx, [esi - 0xc]`, so they belong
  to other offsets.
- Particle emitter `field_0x38`/`field_0x3c`. +0x3c is copied to +0x5c every
  tick, but the role is unclear.
- Tire `field_0x2a4` (a 1.0 divisor in the shock retraction), UIDialog
  `field_0x180` (passed to PlayControlSound's unused parameter), GUIManager
  `field_0x1f8` (only read by the getter 0x004864f0, no writer known), Scene
  `field_0x88` (a flag passed to LoadPhysicsModel), PCVideoCard
  `field_0x198` (only cleared, then passed as CreateSurface's palette).
- SoultreeMaterial `field_0xb8`, LightEmitter `field_0xa8`, ObjectPicker
  `field_0x2c`, CarProcedural `field_0x58`, Terrain `field_0x94` and
  TrackGameMode `field_0x25e8`. Each is only cleared or has one unexplained
  test.

`UnknownVirtualSlotN`: of the 58 remaining names, none sits at a vtable index
and `object_offset` whose canonical class already names the slot. The game
views' slot 0 is Game's destructor (a different kind of member) and the
canonical Game and GameObject slots 9, 12 and 19 are themselves unnamed. The
shadow receivers' slots 27-30 and the physics slots 26/27 would need
behavioural names, which this wave's rule excludes.

## Wave 13: replay deltas, the tag ball and shadow slot 30

Wave 13 follows up the leads Wave 12 left: the byte-delta arrays shared by
the network and replay messages, the "ghost" types in BikeRace.h, the shadow
receivers' virtual slots 27-30 and a stale SoulTreePhysics.cpp comment. It
also rewrote the lead finder (work/naming/classreads.py) to type registers
from compiled layouts (see "Lead finder" below), and named the fields that
its scan exposed. `git grep -c field_0x -- src samples` fell from 1907 to
1881 lines. `UnknownVirtualSlot` lines fell from 352 to 342.

The "ghost" was misnamed. The object at TrackGameViewOwner+0xdc, which the
replay recorder callback reaches as TrackGame+0x568's +0xdc, is the Tag
event's ball: StatsOverlay (0x0051aa40) shows string 0x14b7 `"FREE BALL!"`
while it is set and no racer "is IT!". The VCR ghost
bike is `aiRacers[0]`, a KrustyBike. The Wave 12 rows for
`UnknownBikeRaceGhost` and `UnknownBikeRaceGhostRecord` above describe the
same members under their new names.

Method: as in Wave 12. The comparer built every unit in src, samples and
tests/fixtures whose include closure reaches a changed file (196 units), once
from an archive of 1fe5ed5a and once from a copy of the renamed tree. The map
holds the four type renames (`UnknownBikeRaceGhostRecord`, `...Delta`,
`...Part` and `UnknownBikeRaceGhost@`) and the three slot 30 symbols
(`UnknownVirtualSlot30@ShadowReceiver@@`, `@D3DIMSoultreeShadow@@`,
`@TerrainShadow@@`).

Result: of the 196 units, 186 are IDENTICAL. 2 are IDENTICAL-COMDAT-ORDER
(Group6_BikeRaceCreate.cpp and KrustyUI.cpp, the RTTI COMDAT move of earlier
waves). 8 are NAME-ONLY: BikeRace.cpp, FollowRacer.cpp,
D3DIMSoultreeShadow.cpp, src/reconstructed/TerrainShadow.cpp,
BikeRaceNearMisses.cpp, FollowRacerNearMisses.cpp,
D3DIMSoultreeShadowNearMisses.cpp and samples/physics/shadow/TerrainShadow.cpp.
SoulTreePhysics.cpp, whose comments changed, is IDENTICAL. The 64 units that
reach the last two families (the focal length and the sound lists) were
compiled again after them, against the same archive and map: 54 IDENTICAL
(PCAudio.cpp and AuralScape.cpp among them), the same 2
IDENTICAL-COMDAT-ORDER and the same 8 NAME-ONLY.

Negative controls:

- Without the map, BikeRace.cpp reports DIFF on the relocations of
  `?RecordFrame@BikeRace@@...`, TerrainShadow.cpp on the relocations of its
  vtable `??_7TerrainShadow@@6B@`, and FollowRacerNearMisses.cpp has
  `?ApplyReplayDelta@UnknownBikeRaceGhost@@...` only in the old object.
- A live change reports DIFF: `delta->velocity[0] * 0.25f` in
  `TagBall::ApplyReplayDelta` changes that function's section.

Fixture pins: five whole-object digests of NAME-ONLY units were repinned with
section and relocation counts unchanged. They are D3DIMSoultreeShadow.cpp and
D3DIMSoultreeShadowNearMisses.cpp in clipper_global_objects.json,
FollowRacer.cpp and FollowRacerNearMisses.cpp in
followed_racer_collisions_objects.json, and BikeRaceNearMisses.cpp in
soultree_position_objects.json. No other digest of a NAME-ONLY unit appears
under tests/. work/naming/repin.py does this: it repins only a failure whose
counts are unchanged and whose unit the proof report marks NAME-ONLY. The
calibration case for 0x004a9aa0 (tools/run_calibration.py), the bindings,
the shadow samples' targets.json, test_shadow_getter_connections.py and the
emu cases d3dim_shadow_slot30.py and terrain_shadow_slot30.py use the new
names.

| Old name | New name | Tier | Evidence |
| --- | --- | --- | --- |
| `KbNetDelta::delta0`..`delta3` (+0x0b, +0x01, +0x08, +0x05) | `velocity`, `position`, `angularVelocity`, `angles` | 2 | ApplyStateDelta (0x004933e0) adds them, at 15/64 and pi/64 per unit, to KrustyBike's `deltaBaseVelocity` (+0x1358), `deltaBasePosition` (+0x1364), `deltaBaseAngularVelocity` (+0x1370) and roll/pitch/yaw (+0x137c..+0x1384). KbBikeDeltaMessage encodes the same fields at the same offsets. |
| `UnknownBikeRaceGhostDelta::delta0`..`delta3` | the same four names | 2 | The replay record kind 0x10 has the KbNetDelta layout. ApplyReplayDelta (0x004a9aa0) applies it with the same scales into the matching ball fields. |
| `UnknownBikeRaceGhost` | `TagBall` | 2 | TrackGameViewOwner+0xdc is the Tag event's ball (StatsOverlay 0x0051aa40 shows `"FREE BALL!"` while it is set). The VCR ghost is `aiRacers[0]`, a KrustyBike. The accessor `GhostBike()` becomes `Ball()`. |
| `UnknownBikeRaceGhostRecord`, `...Delta`, `...Part` | `TagBallState`, `TagBallStateDelta`, `TagBallReceivedState` | 2 | The ball's record kind 0x11 state, its kind 0x10 byte deltas and the four received states the ball keeps (newest first, rotated by 0x00421d50). |
| `TagBallState::field_0x00` (`short`) | `header` | 2 | SendRaw (0x004ad280) writes the message type byte here, as in the other kind records. |
| `UnknownVirtualSlot30` of ShadowReceiver, D3DIMSoultreeShadow, TerrainShadow | `BuildShadowVertices` | 3 | The base (0x0044d710) is empty. Both overrides (0x00446f40, 0x00509aa0) turn the receiver geometry into the textured vertices their slot 14 draws. They map positions to the shadow texture's tu/tv, scale by `texelSize` and set `vertexCount`. ProjectedShadow's update loop calls it on every receiver. No literal names it. |
| `ShadowCameraProxy::field_0x198`, `TerrainShadowCamera::field_0x198` | `focalLength` | 2 | The object is a ShadowCamera: its constructor 0x004da520 calls PCCamera's (0x004bed80), which calls Camera's (0x0042e340), and the vtable 0x00557540 has ShadowCamera's RTTI. Camera.h names +0x198 `focalLength`. TerrainShadow's slot 28 passes it to atan2 as x. |
| `SoundInterface::field_0x04`, `field_0x18` (`ContainerList<void*>`) | `loadedSounds`, `loaded3DSounds` | 2 | PCSoundInterface's start-up Inits both to 8 entries growing by 8. LoadWave (0x004bbef0) adds every Sound to the first and 3D ones to the second too; ~Sound (0x004bbb60) removes them. AuralScape's debug page prints the first count as `"Sounds %d"`. |

SoulTreePhysics.cpp line 108 (and nearby comments) still named `field_0x150`
and other offsets that earlier waves had renamed. The comments now use
`totalWeight` (+0x150), `stepTime`/`invStepTime`, `attachments`,
`collisionObject`, `restitution`, `invMass` and the rest state fields.

Lead finder: work/naming/classreads.py now compiles every src and samples
unit with the VC6 profile vc6_o2_mt (which emits `.debug$T`) and reads the class layouts from `.debug$T` (LF_FIELDLIST
members, base classes and nested arrays; a member of a forward-declared class
gets its size by name). 1187 classes compile; 55 names have several layouts
(local views that share a name across units), and each keeps every variant. A
bound function resolves through the layout of its own bindings unit, and
anything else through the canonical one. The scan starts `this` at the
`object_offset` of every vtable entry and adds the adjustment of each thunk
in vtable_thunks.json, so an MI method sees its secondary view's offsets.
It follows registers through `mov`/`lea`/`add`/`sub`, loads of pointer
members (pointer-chain views) and typed globals, and kills eax/ecx/edx at
calls. It typed `this` in 2732 functions and found 372 placeholder paths
accessed, 122 of them read. Wave 12's ConstraintMethodCollisionModel false
positives (+0xbc, +0xd8, +0xdc, +0xe0 through `lea ecx, [esi - 0xc]`) no
longer appear. Two units do not compile on their own
(samples/allocation/test_accounting.cpp, samples/category_pilots/test_models.cpp).
Usage: `python work/naming/classreads.py layouts`, then
`python work/naming/classreads.py scan [--class C] [--field F] [--every C]`.

Left as placeholders:

- `TagBallState::field_0x38`. Nothing reads or writes it outside
  whole-record copies.
- The shadow receivers' slots 27-29. The two overrides do different work.
  Slot 27 is a visibility test (a shadow-camera box test in
  D3DIMSoultreeShadow, a screen footprint of at least 10 x 5 in
  TerrainShadow). Slot 28 computes the texel size in both, but
  TerrainShadow's also fits the shadow camera and light matrix to the screen
  footprint and returns whether it did. Slot 29 is a camera-view test in one and collects cell keys in
  the other.
- UnknownBikeRaceViews (RaceSound) +0x40. Only the constructor writes it,
  with 0.
- UnknownFollowCameraSubject (Terrain) +0xbe8. A header flag that chooses
  the constants at 0x005507f8 or 0x00550bac. The role is unclear.
- UIDialog +0xd4. UIControl::SetTextDrop (0x00470d60) writes +0xd4 on
  UIControl, so the two views disagree and the base must be settled first.
- KbBikeNetState `field_0x5c`. The readers test it as "lap fields valid",
  but the receive path's writer was not found.
- UnknownAuralScapeReverbOwner +0xa4. The owner class is unknown.
- TrackGameViewOwner +0xac. Its readers vary by event.
- Cube +0x10, GridNodeExtra +0x10, GridTerrain +0xc9c/+0xca0, PCTextureMap
  +0x40, UIListBox +0x218, UIListBoxRow +0x30 and GameCursor +0x50. One or
  two reads each with no role. VCRFile `entries[].field_0x00` and the pad
  arrays (`field_0xNN[...]` byte runs) need their structure first, which
  this wave's rule excludes.
- The UIScrollBar and UIEditBox `int` aliases of named unions stay as
  intentional views (Wave 12).

`UnknownVirtualSlotN`: slot 30 of the shadow receivers was the only family
whose behaviour is the same in every override. 57 distinct `UnknownVirtualSlotN`
identifiers remain (slot 30 survives on UIDialog and other classes), and the
bindings hold 17 distinct `?UnknownVirtualSlotN@Class@@` symbols (20 before).
