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
  - No dialog control or INI key is bound to it, so "CD music on" is only tier 3
    (Wave 20 names it `cdAudioOn` at that tier).
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

## Wave 14: the most-used placeholder types

Wave 14 takes the most-used remaining `Unknown*` types and sorts them into
local views of an already-named class, which stay (merging them is a type
change), and distinct types, which are renamed. Members keep their names
unless noted. `git grep -ohE '\bUnknown[A-Z][A-Za-z0-9_]*' -- src samples`
fell from 661 unique names and 4427 uses to 626 and
3841. `field_0x` lines fell from 1881 to 1880 (one comment).

Method: as in Wave 13. work/naming/rename.py rewrites a whole identifier,
and an identifier inside a decorated name, in every tracked text file except
this one. That covers source, bindings keys, calibration and emulator case
symbols, tests and docs. The comparer built every unit in src, samples and
tests/fixtures whose include closure reaches a changed file (408 units),
once from an archive of 48df4fbc and once from the renamed tree, with the
cumulative map work/naming/map-wave14.json.

Result: of the 408 units, 330 are IDENTICAL, 2 IDENTICAL-COMDAT-ORDER
(Group4_DlgMain.cpp, surface_map_smoke.cpp), 71 NAME-ONLY and 4
NAME-ONLY+COMDAT-ORDER (BikeRaceNearMisses.cpp, DlgProcsNearMisses.cpp,
ProCircuitProcsNearMisses.cpp, KrustyUI.cpp). tests/fixtures/bvh_vector_compare.cpp
fails to compile in both trees on its own: it includes a header its test
generates (bvh_vector_patches.h), so the suite covers it. The final tree was
proved after it was split into one commit per family, and it equals the tree
proved in two earlier batches (404 and 243 units).

`UnknownParameterStream` is a typedef of `FileStream`, so every declaration
now spells `FileStream`. The typedef lines stay in SoultreeObject.h and
FileStream.h, because the first proof run removed them and reported 39 DIFF
units. 37 differed only in VC6's internal `$label$N` and `$TN` numbers (one
fewer symbol). Two, tests/fixtures/character_motion_routes.cpp and
bike_motion_routes.cpp, swapped the order of two loads in `_main` /
`CharacterMotionCases` (`mov eax, [ebp-0x2cc]` and `mov edx, [ebp-0x790]`).
With the typedefs kept, all 40 of those units are IDENTICAL or NAME-ONLY.
An unused typedef is therefore not codegen-neutral under VC6 SP3 `/O2`.

Statics: `g_DrawImageRectSourceRect` (BackgroundImage.cpp, now `Win32Rect`)
keeps `.bss` offset 0x40 in the object, and the fixtures' `static DisplayMode
modes[...]` arrays (mode_surfaces_compare.cpp, partial_texture_uploads_compare.cpp,
profile_displays_compare.cpp) are IDENTICAL or NAME-ONLY. No renamed type
moved a `.bss` or `.data` symbol.

Negative controls:

- Without the map, BackgroundImage.cpp reports DIFF: its `.bss` section key
  holds `?g_DrawImageRectSourceRect@@3UUnknownRect@@A` only in the old object.
- A live change reports DIFF: `regionCapacity = 5` (was 4) in
  `BackgroundImage::SetRenderContext` changes that function's section.
- The typedef removal above.

Fixture pins: 52 digest lines were repinned: 51 in 23 files under
tests/fixtures, plus the `Terrain::Load` partial row in
tests/test_terrain_connections.py. Every changed digest belongs to a NAME-ONLY unit, and its section,
relocation, size and matching-position counts did not change. The units
are EcoSystem.cpp, KrustyUI.cpp, TrackOverlay.cpp, DlgProcs.cpp,
ProCircuitProcs.cpp, QuarryStuntEvent.cpp, SceneManager.cpp, TrackGame.cpp,
ZoneReport.cpp, DirectoryList.cpp, UiInfo.cpp, PCTextureMap.cpp, Terrain.cpp,
and the samples DlgProcsNearMisses.cpp, ProCircuitProcsNearMisses.cpp,
SceneManagerNearMisses.cpp, BikeRaceNearMisses.cpp, TrackOverlayNearMisses.cpp,
QuarryStuntEventLoaderNearMiss.cpp, PCTextureMapNearMisses.cpp,
StatsStandingsDraw.cpp and GameUiNearMisses.cpp. Some units are pinned in several files, and those
copies were repinned together.

work/naming/repin.py now accepts a digest pinned under the same unit in
several fixtures. The new work/naming/repin2.py repins the forms repin.py
does not parse: aggregate pins (`"source"`), partial rows (unit, symbol,
size, digest) and tuple diffs whose other elements must be equal.
tests/test_motion_stream_connections.py asserts that no Parameterblocks
symbol contains `UnknownParameterStream`, so the rename skips it.

| Old name | New name | Tier | Evidence |
| --- | --- | --- | --- |
| `UnknownOverlayRect`, `UnknownMakeOverlayRect` | `OverlayRect`, `MakeOverlayRect` | 3 | The RECT-shaped int quad that Overlay keeps at +0xe8/+0xf8 and TextQueueOverlay at +0x30/+0x40. ChatOverlay::Setup and `Overlay::RestoreRect` take it. No string names it. |
| `UnknownTrackOverlayRect` | `TrackOverlayRect` | 3 | TrackOverlay.h's 16-byte rectangle with constructors 0x00518d50/0x00518d60. |
| `UnknownChatInput`, `UnknownChatEntry` | `ChatInput`, `ChatHistoryEntry` | 3 | ChatOverlay+0x13c (field `chatInput`; 0x0051ea50..0x0051eb40) holds the typed line, and its 0x5c-byte history lines. |
| `UnknownTextureFormatChoice` | `TextureContext` | 3 | The six-field block (manager, opaque/alpha groups and formats, halvings) at Game+0x1c..+0x30, whose field is `textureContext`. The texture loader 0x0050a590 reads it. `UnknownKrustyUIContext` (KrustyUI.cpp) is a same-layout view and stays. |
| `UnknownTrackGameObject3410` | `MSZoneInterface` | 2 | TrackGame+0x3410 (field `networkGameObject`). Its methods push the MSZoneInterface.cpp `__FILE__` literal (0x0056e1b4; xrefs 0x004aa36c..0x004aa64d in GetLobbyPreset, GetLobbyRank and SubmitScores). ZoneReport.cpp (0x0049c2f0..) drives it. |
| `UnknownTrackGameObject3410Base` | `MSZoneInterfaceBase` | 3 | Its base, whose constructor body 0x004676a0 is shared by identical code folding. |
| `UnknownBikeNumberPainter` | `BikeNumberPainter` | 2 | Constructor 0x00417500 loads the "BikeNumbers128.tga" literal. The painter draws the plate digits into the "PROCEDURAL" textures. |
| `UnknownRect`, `UnknownOSVersionInfo` | `Win32Rect`, `OSVersionInfo` | 2 | The Win32 RECT layout PCGame passes to GetWindowRect (`windowRect`, +0x308), and the 0x94-byte OSVERSIONINFOA. |
| `UnknownDisplayMode` | `DisplayMode` | 2 | The 36-byte record EnumDisplayModes fills and the "%d x %d %d bit" page lists. |
| `UnknownVectorDifference`, `UnknownVectorSum`, `UnknownVectorLength` | `VectorDifference`, `VectorSum`, `VectorLength` | 2 | The static inline helpers of DlgProcs.cpp, ProCircuitProcs.cpp, SelectGamePicProcs.cpp and the bike-rider dialogs. The names state their decoded bodies. |
| `UnknownRecordFileHeader` | `KrustyVCRHeader` | 2 | The 0xeb0-byte header (`push 0xeb0` at 0x0044ec50, 0x004507a0, 0x00454030, 0x00454640, 0x00454a50, 0x00455040) that GhostFilesDlg, ReplayFilesDlg and SPEventDlg read. It is KrustyVCR+0xdc's block: magic "MCMVCR" (0x0056da6c) and version at +0x00, description +0x10, length +0x30, race settings +0x34. KrustyVCR (0x0049bf10, 0x0049c010) hands it to VCRInterface::Start. `field_0x00[0x10]` stays one array (splitting it is a shape change). |
| `UnknownDriveList`, `UnknownDriveEntry`, `UnknownDirectoryEntry` | `DriveList`, `DriveEntry`, `DirectoryEntry` | 2 | The dirlist.cpp unit (literal 0x00569204). TrackGame's `driveList` walks GetLogicalDriveStrings, GetDriveTypeA and GetVolumeInformationA to find the "MCM2" CD. |
| `UnknownTokenizer` | `Tokenizer` | 3 | The strtok-style splitter 0x00515dc0/0x00515df0. Its header moved to src/reconstructed/Tokenizer.h. |
| `UnknownSound3DParameters`, `UnknownNotifyPosition`, `UnknownPcmFormat`, `UnknownWaveHeader` | `DS3DBuffer`, `DSBPositionNotify`, `PcmWaveFormat`, `WaveFileHeader` | 2 | The DS3DBUFFER passed to IDirectSound3DBuffer::SetAllParameters, the DSBPOSITIONNOTIFY and PCMWAVEFORMAT layouts, and the 44-byte RIFF/WAVE header. |
| `UnknownFollowCameraValue` | `FollowCameraEase` | 3 | A value FollowCamera eases toward a target. Its owners are `fieldOfViewEase`, `distanceEase`, `targetEaseX/Y/Z` and `positionEaseX/Y/Z`. |
| `UnknownParameterStream` (uses) | `FileStream` | 1 | A typedef of `FileStream`. The two typedef lines stay (see above). |
| `UnknownBitmapFile`, `UnknownBitmapFileHeader`, `UnknownBitmapInfoHeader` | `BitmapFile`, `BitmapFileHeader`, `BitmapInfoHeader` | 2 | bmpfile.cpp (`__FILE__` 0x00568290) reads and writes 0x4bc-byte records. 0x004245f0 stores "BM", 0x28 and 0x436, which fit the BITMAPFILEHEADER/BITMAPINFOHEADER layout. |
| `UnknownFontTextureNode` | `FontTextureNode` | 2 | The 8-byte list node FontTextureManager.cpp (`__FILE__` 0x0056b2fc) allocates at lines 25 and 29. The list itself (`UnknownStatic65b478`) stays. |
| `UnknownLobbySlotTable`, `UnknownLobbySlot` | `LobbySlotTable`, `LobbySlot` | 3 | MultiPlayerDlg+0x7f68 (constructor 0x004f2f00) and its eight player/requested/granted slots, which the lobby settings message carries at +0x54. |
| `UnknownGameUiTextRun` | `UITextRun` | 3 | The 0x18-byte colored run that `UIDialog::DrawTextRuns` (0x0046f420) draws. GameUi.cpp splits "~N" and "\|x" text into these runs. |
| `UnknownTrackGameModeSettings` | `RaceSettings` | 3 | The 0x1ec-byte block TrackGameMode keeps as `raceSettings` (+0x27f8) and two saved copies. |
| `UnknownTrackGameRacerSlot`, `UnknownTrackGameRacerChoice` | `RacerSlot`, `RacerChoice` | 3 | TrackGameMode's `racers[8]` entry (0xf8 bytes, constructor 0x00521f30) and the player's bike and rider choice (`racerChoice`, +0x1974). |

Left as placeholders (views of a named class, so merging them changes types):

- `UnknownEventRacer` (RaceView.h) and `UnknownBikeRaceRacer` (BikeRace.h)
  are KrustyBike as EventManager and BikeRace see it. Both have a virtual
  GameObject base at vbptr +4, as KrustyBike's RTTI shows.
  `UnknownBikeRaceRacer` declares 95 `UnknownRacerVirtualSlotN`.
- `UnknownGuiControl` (GUIManager.h) is a UIControl as GUIUser and ToolTip
  see it, and it is never constructed.
- `UnknownTexturePalette` is TextureMap+0x2c's palette. It is a view of
  ColorMapper (Quantize.h): the getters 0x004de270..0x004de290 and the
  members already carry ColorMapper's names and offsets.
- `UnknownEcoTrackGame` (EcoSystem.h) is TrackGame as EcoSystem reads it.
- `UnknownSoultreeNodeView` is a SoultreeObject view. It is kept out of
  D3DIMSoulTree.h because more declarations there change SceneManager.cpp's
  0x004eb570.
- `UnknownBikeRaceModel` is the D3DIMSoultreeModifier detail-level view at
  the racer's +0x3bc and the rider's +0x1a0.
- `UnknownTimer` (PeakHold), `UnknownCursorAnimation` (UIAnim), `UnknownStream`
  (FileStream), `UnknownKrustyUIContext` (TextureContext),
  `UnknownKrustyUIModelObject` (a model object), and `UnknownRunwayTerrain`
  and `UnknownBikeRaceProjector` (Terrain::QueryGround 0x00507c10).
- `UnknownSceneEntry`, `UnknownSceneEntry2` and `UnknownSceneTable`
  (SceneManager.h). These are distinct, but "Animation<n>" and
  "RandomSet<n>" alone do not settle the names.
- Typedef aliases (`UnknownParameterStream`, ResourceEntry.h's
  `UnknownResourceEntry`). Removing the typedef line is not neutral (see
  above).
- Skipped by request: `UnknownKrustyBikeView`, `UnknownQuarryRace`,
  `UnknownBikeRaceBikeSetup`, `UnknownSoultreeMesh`,
  `UnknownSoultreeCameraView`, `UnknownPodium*`, `UnknownStatic65b478`, the
  shared stubs `UnknownFunction4de580/464e80/464e90` and
  `UnknownVirtualSlotN`.

## Wave 15: racer view slots, error reporters and scene records

Wave 15 names the virtual slots of the racer views and a few placeholder
functions and types with literal or vtable evidence. The views stay
separate types. `git grep -ohE '\bUnknown[A-Z][A-Za-z0-9_]*' -- src samples`
fell from 626 unique names and 3841 uses to 504 and 3610.
`UnknownRacerVirtualSlotN` fell from 95 names and 112 uses to 6 and 6, and
`UnknownVirtualSlotN` from 58 names and 374 uses to 33 and 325.
`field_0x` lines fell from 1880 to 1879 (one comment).

Method: as in Wave 14. work/naming/rename.py renamed the unique
identifiers in every tracked text file except this one. The view slots
named `UnknownVirtualSlotN`, which other classes share, were renamed line by
line with work/naming/sub.py (work/naming/w15_views.py). The comparer built
every unit in src, samples and tests/fixtures whose include closure reaches a
changed file (227 units), once from an archive of e983d669 and once from the
final tree, with the map work/naming/map-wave15.json (146 entries, the
shared slot names scoped by class).

Result: 227 units, 214 IDENTICAL, 4 IDENTICAL-COMDAT-ORDER
(samples/ui/DlgProcsNearMisses.cpp, src/bringup/Group4_DlgMain.cpp,
src/bringup/Group6_BikeRaceCreate.cpp, src/reconstructed/KrustyUI.cpp) and 9
NAME-ONLY (the units that define or reach `ReportDirectInputError` or the
scene records: JoystickBufferedState.cpp, PCJoystickDeviceNearMisses.cpp,
PCControl.cpp, PCJoystickDevice.cpp, PCKeyboardDevice.cpp, PCMouseDevice.cpp,
PCTextureMap.cpp, tests/fixtures/joystick_buffered_compare.cpp and
tests/fixtures/scene_motion_compare.cpp). There was no DIFF and no compile
failure, and no .bss or .data offset moved.

Negative controls:

- Without the map, PCControl.obj gives DIFF (the `.text` COMDAT
  `?UnknownReportError@@YAXJPBDH@Z` exists only in the old object).
- A live change (`char name[256]` to `[260]` in the new tree's
  PCControl.cpp) gives DIFF in the bytes of `?ReportDirectInputError@@YAXJPBDH@Z`.

Fixture pins: one digest, the PCTextureMap.cpp entry of
tests/fixtures/texture_lzw_objects.json (aa90ac201d76 to 0239eba56a15). It is
NAME-ONLY; its 157 sections and 462 relocations are unchanged.

### Racer view slots

The racer views are KrustyBike as BikeRace and EventManager see it.
`UnknownBikeRaceRacer` is built by KrustyBike's constructor 0x0048fa60, and
`UnknownEventRacer`'s methods are KrustyBike.cpp code (0x004925a0,
0x00496f90). Both have their own vfptr at +0 and reach the virtual GameObject
base through a vbptr at +4, as KrustyBike's RTTI shows, so their own slot N is
KrustyBike's primary-vtable slot N (vtable 0x00554548, object_offset 0, 103
slots). The canonical names were read from the linked build: work/naming/w15_slots.py
lists, for each slot, the retail entry and the symbol the build puts there
(the vtable audit reports every slot ok). The retail call sites agree with the
names: BikeRace 0x004205e7 calls +0xb0 (slot 44, `RemoveFromRace`) for a
player who left, 0x004209d9 calls +0xac (43, `SnapshotRespawnPose`) on a
restart, 0x0041dcb7/0x0041dcc9 pass +0xb4's float (45,
`GetSpawnProtectionDuration`) to +0xc8 (50, `SetSpawnProtection`), and
0x0041d32c calls +0x178 (94, `UpdateForceFeedback`) each frame.

The slots keep their placeholder where KrustyBike's slot is itself a
placeholder: 26 and 27 (SoultreePhysicsBaseObject), 52 and 77-79 (Vehicle).
Slot 87's built and retail entries are the folded empty stub 0x00464e90,
which many names share, so its name comes from VehicleCore.h's declaration
order (`ApplyCrashedControl`). The views keep their own signatures.

| Old name | New name | Class / slot / object_offset | Tier | Evidence |
| --- | --- | --- | --- | --- |
| `UnknownBikeRaceRacer::UnknownRacerVirtualSlotN` (BikeRace.h) | KrustyBike's names for slots 0-94 (89 slots): 0 `UpdateMass`, 1 `ResetDynamicState`, 2 `InitializePhysics`, 3 `ApplyStaticContactImpulse`, 4 `ApplyBodyContactImpulse`, 5 `ShouldRefreshContacts`, 6 `ApplyDrag`, 7 `DistributeContactForce`, 8 `UpdateLocalCenterOfMass`, 9 `ComputeFrameSteps`, 10 `IsAtBaseWeight`, 11 `FindPlacement`, 12 `ClassifyOutOfBounds`, 13 `AccumulateContactForces`, 14 `IntegrateMotion`, 15 `ApplyWorldTorque`, 16 `LocalTorqueToAngularAcceleration`, 17 `AverageAttachmentPosition`, 18 `UpdateContactEffect`, 19 `UpdateSlideEffect`, 20 `UpdateScrapeEffect`, 21 `UpdateAttachmentEffects`, 22 `ShouldUpdateShadowLight`, 23 `ShouldEmitMovingEffects`, 24 `ShouldRestartSteamEffect`, 25 `ShouldBurstSteamEffect`, 28 `ResolveSubstepCollisions`, 29 `SaveBodyOrientation`, 30 `ClearContactEffectFlags`, 31 `UpdateContactStates`, 32 `EffectiveLoadWeight`, 33 `ResetToPlacement`, 34 `LoadBodyAxes`, 35 `StoreBodyAxes`, 36 `ResetBodyOrientation`, 37 `AddAttachment`, 38 `ApplyCollisionEvent`, 39 `ApplyPendingRespawn`, 40 `LoadPhysicsModel`, 41 `EndMotion`, 42 `CheckMotionComplete`, 43 `SnapshotRespawnPose`, 44 `RemoveFromRace`, 45 `GetSpawnProtectionDuration`, 46 `ComputeRotationPivot`, 47 `GetDriveShareScale`, 48 `AdvancePosition`, 49 `SimulateFrame`, 50 `SetSpawnProtection`, 51 `IsSpawnProtected`, 53 `ApplySteerRate`, 54 `GetContactGroundNormal`, 55 `GetPivotAndForwardAxis`, 56 `UpdateLeanFromNormal`, 57 `GetLeanAngle`, 58 `SettleBodyToWheels`, 59 `GetMaxSteerAngle`, 60 `ComputeSteerRate`, 61 `ComputeSlideAngle`, 62 `ClassifyGroundContact`, 63 `ReadControlAxes`, 64 `UpdateControlInput`, 65 `ApplyLaunchImpulse`, 66 `IsRiderPoseLocked`, 67 `ClearCrashState`, 68 `GetRespawnKey`, 69 `ResetCrashTimer`, 70 `IsSpeedGainBelowLimit`, 71 `UpdateAirborneState`, 72 `ComputeGroundNormalAndTurn`, 73 `GetTurnRadius`, 74 `DeflectVelocity`, 75 `ComputeWheelForwardAlignment`, 76 `ComputeDriveTorque`, 80 `IsThrottleRequested`, 81 `IsClutchRequested`, 82 `IsReverseRequested`, 83 `IsBrakeRequested`, 84 `QueryControlInput`, 85 `UpdateWheelRolls`, 86 `UpdateWheelDriveForces`, 87 `ApplyCrashedControl`, 88 `ClassifyInputDirection`, 89 `SelectRiderPose`, 90 `ClampSubStepsOnCrash`, 91 `ApplyAirControl`, 92 `ApplyLeanCorrection`, 93 `InitForceFeedback`, 94 `UpdateForceFeedback` | `UnknownBikeRaceRacer` own vtable (object_offset 0) as `KrustyBike` | 2; 3 for 47, 65, 66, 81, 87, 89, 90 and 91 | constructor 0x0048fa60 is KrustyBike's; the call sites above. The tier-3 slots are the ones VehicleCore.h marks as provisional names. |
| `UnknownEventRacer::UnknownVirtualSlot0` (RaceView.h) | `UpdateMass` | `UnknownEventRacer` own vtable (object_offset 0) as `KrustyBike` | 2 | its methods are KrustyBike.cpp code (0x004925a0, 0x00496f90); FollowRacer.cpp 0x004a9e80 calls its slot 50 with (1, 5.0f, 0) like `SetSpawnProtection` |
| `UnknownFollowedRacerSlots::UnknownVirtualSlotN` (FollowRacerNearMisses.cpp) | KrustyBike's names for slots 0-49 (48 slots, the same names as above) | `UnknownFollowedRacerSlots` (the `UnknownEventRacer` vtable) as `KrustyBike` | 2; 3 for 47 | its slot 50 is already `SetSpawnProtection` |

### Functions and types

| Old name | New name | VA | Tier | Evidence |
| --- | --- | --- | --- | --- |
| `UnknownReportError` | `ReportDirectInputError` | `0x004bf6a0` | 1 | formats "DirectInput Error %s in file %s at line %d\n" (0x0056fc84, pushed at 0x004bf975) with the DIERR_* name of the result and the caller's `__FILE__` and line |
| `UnknownReportDirectDrawError` | `ReportDirectDrawError` | `0x004c86e0` | 2 | the same (result, `__FILE__`, line) arguments; formats the DDERR_* name of a DirectDraw result (literals such as 0x00571270 "DDERR_NOGDI"). No format string survives, so the "Report" part is by analogy with 0x004bf6a0. |
| `UnknownEnumEffectsCallback` | `EnumEffectsCallback` | `0x004c4260` | 2 | `PCJoystickDevice::EnumEffects` (0x004c4190) passes it to IDirectInputDevice2::EnumEffects (method 19); it copies each effect into the caller's array |
| `UIDialog::UnknownInlineField18` | `UIDialog::GetRenderContext` | inline | 2 | returns GameObject+0x18 `renderContext`; every caller casts the result to RenderTarget or PCRenderTarget |
| `UnknownTrackGameModeEntry` | `GridEntry` | | 3 | the 8-byte record of `RaceSettings::gridOrder[8]` (+0x14c): player id and championship points. `CompareGridEntries` (0x004f2080) sorts them by points, and the BikeRace loader seats the racers from them. No string names it. |
| `UnknownSceneTable` | `SceneAnimationTable` | | 2 | Scene+0xb4 `animationTable`. Scene::ReadAnimations (0x004edfe0) fills it from the "Animations" section: "NumberOfAnimations" (0x00573954) and "NumberOfRandomSets" (0x0057374c). |
| `UnknownSceneEntry` | `SceneAnimation` | | 2 | the table's 0x44-byte records, one per "Animation%d" section (0x00573910, pushed at 0x004ee153) |
| `UnknownSceneEntry2` | `SceneRandomSet` | | 2 | the table's 0x10-byte records, one per "RandomSet%d" section (0x00573740, pushed at 0x004eeff0) |

Left as placeholders:

- `UnknownRacerVirtualSlot26`, `27`, `52`, `77`, `78`, `79` and the matching
  `UnknownFollowedRacerSlots` slots 26 and 27: KrustyBike's slots are
  placeholders.
- The racer views themselves (`UnknownBikeRaceRacer`, `UnknownEventRacer`,
  `UnknownFollowedRacerSlots`): merging them into KrustyBike is a type change.
- `UnknownInlineSetField18` (GameUi.h) stores `renderContext`. The natural
  name, `SetRenderContext`, is GameObject's slot 8 and would hide it.
- `Scene::UnknownFunction4ea7e0`, `GUIManager::UnknownFunction4864f0`,
  `UnknownRecorderThread` and `UnknownSoundNotifier::UnknownThreadProc`: no
  string names them.
- `UnknownEffectInfo` (the DIEFFECTINFO layout) and the statics
  `s_UnknownEffectCount`: out of this wave's scope (a type, and a static).

## Wave 16: crash, shadow-receiver and camera slots

Wave 16 names the canonical virtual slots whose retail overrides and call
sites agree on a role, and carries the names to the views that share the
slot index and object_offset. `git grep -ohE '\bUnknown[A-Z][A-Za-z0-9_]*' --
src samples` fell from 504 unique names and 3610 uses to 496 and 3526.
Counted with the same command, `UnknownVirtualSlotN` fell from 32 names and
322 uses to 27 and 241, and `UnknownRacerVirtualSlotN` from 6 to 3.
`field_0x` lines stay at 1879.

Method: the overrides of each slot were listed across every class that
contains the base, from analysis/vtables.json and the RTTI base offsets, with
the symbol the linked build puts at the same (class, object_offset, slot)
(work/naming/w16_overrides.py); the retail call sites of the slot's vtable
offset were listed with the bound name of the calling function
(work/naming/w16_callers.py). A slot was named only when its overrides and
callers agree on one role. The shared names were renamed per class with
work/naming/w16_scoped.py: `Class::Old` and the decorated `Old@Class`
everywhere, the unqualified declarations and calls as count-checked edits
(spec files work/naming/w16_spec[ABC].json); the unique names with
work/naming/rename.py. The comparer built every unit in src, samples and
tests/fixtures whose include closure reaches a changed file (104 units), once
from an archive of 5b987282 and once from the final tree, with the map
work/naming/map-wave16.json (24 entries, every one scoped by class).

Result: 104 units, 85 IDENTICAL, 2 IDENTICAL-COMDAT-ORDER
(src/bringup/Group6_BikeRaceCreate.cpp, src/reconstructed/KrustyUI.cpp) and 17
NAME-ONLY (the units that define or call a renamed slot:
FollowCameraConstructor.cpp, FollowCameraNearMisses.cpp, LayoutProbe.cpp,
KrustyBikeNearMisses.cpp, D3DIMSoultreeShadowNearMisses.cpp,
samples/physics/shadow/TerrainShadow.cpp, D3DIMSoultreeShadow.cpp,
SoulTreePhysics.cpp, Bike.cpp, KrustyBike.cpp, Vehicle.cpp, BikeCamera.cpp,
FollowCamera.cpp, KrustyBikeCamera.cpp, src/reconstructed/TerrainShadow.cpp,
VehicleCamera.cpp and VehicleMethods.cpp). There was no DIFF and no compile
failure, and no .bss or .data offset moved.

Negative controls:

- Without the map, VehicleMethods.obj gives DIFF (the `.text` COMDAT
  `?UnknownVirtualSlot52@Vehicle@@UAEHXZ` exists only in the old object), and
  D3DIMSoultreeShadow.obj gives DIFF in the relocations of its vtable
  `??_7D3DIMSoultreeShadow@@6B@`.
- A live change (`crashState == 0` to `== 1` in the new tree's
  `Vehicle::CanCrash`) gives DIFF in the bytes of `?CanCrash@Vehicle@@UAEHXZ`.

Fixture pins: whole-object and aggregate digests of NAME-ONLY units only, each
with unchanged section and relocation counts: Bike.cpp (two compile forms),
KrustyBike.cpp (two), Vehicle.cpp (two), SoulTreePhysics.cpp, FollowCamera.cpp,
VehicleMethods.cpp, D3DIMSoultreeShadow.cpp, KrustyBikeNearMisses.cpp (two),
FollowCameraNearMisses.cpp (three), FollowCameraConstructor.cpp and
D3DIMSoultreeShadowNearMisses.cpp, in 14 fixture files and in
tests/test_rider_motion_start_connections.py. Every occurrence of each old
digest was replaced (work/naming/repin.py, then work/naming/w16_repin3.py for
the pins repin.py cannot attribute: aggregate keys and digests inside test
modules). tests/test_vehicle_method_providers.py maps the new slot name
`CanCrash` to slot 52, as it does for the other named Vehicle slots.

### Slots

| Old name | New name | Class / slot / object_offset | Tier | Evidence |
| --- | --- | --- | --- | --- |
| `SoultreePhysicsBaseObject::UnknownVirtualSlot27`, `KrustyBike::UnknownVirtualSlot27` | `StartWreck` | SoultreePhysicsBaseObject slot 27, object_offset 0 (KrustyBike 0x0048da00; the base entry is the folded empty stub 0x00464e90) | 2 | KrustyBike is the only override: it starts the wrecker's tumble (crash reason 9 or a running motion) or its animated wreck in the crash direction, then switches to the bike collision. Slot 28 (base and Vehicle) calls it once when the +0x138 flag is set and clears the flag; Bike's UpdateCrashState and Vehicle::CheckCrash set the flag on a crash. |
| `slot27Pending` (+0x138) | `wreckPending` | SoultreePhysicsBaseObject field | 2 | the flag above |
| `Vehicle::UnknownVirtualSlot52`, `KrustyBike::UnknownVirtualSlot52` | `CanCrash` | Vehicle slot 52, object_offset 0 (0x0040c8a0, KrustyBike 0x00495800) | 2 | Vehicle returns `crashState == 0`, KrustyBike also 0 for a remote bike. Its callers (Bike's collision callbacks 0x00500c00/0x00500c30 and Bike::ApplyCollisionEvent, KrustyBike's callback) use it to decide whether the hit runs UpdateCrashState. |
| `Vehicle::UnknownVirtualSlot77` | `IsPovHatPressed` | Vehicle slot 77, object_offset 0 (0x00529200, not overridden) | 2 | true when the argument is set and the active joystick's POV hat 0 angle is not -1 (centred); the throttle, brake and slot 78/79 input checks skip their joystick buttons while it holds |
| `ShadowReceiver::UnknownVirtualSlot27` and the D3DIMSoultreeShadow / TerrainShadow overrides | `IsShadowVisible` | ShadowReceiver slot 27, object_offset 0 (base 0x00468c90 return 1; 0x004468f0; 0x00508bc0) | 3 | ProjectedShadow::PrepareGeometry (0x004db000) asks each receiver first and skips it on 0. D3DIMSoultreeShadow tests the caster's bounds against the shadow camera's box (inShadowView); TerrainShadow tests the footprint's screen rectangle against the viewer (at least 10 x 5 pixels). |
| `ShadowReceiver::UnknownVirtualSlot28` and overrides | `UpdateTexelSize` | ShadowReceiver slot 28, object_offset 0 (base 0x004aa190 return 0; 0x00446bf0; 0x005097d0) | 3 | called next, result ignored; both overrides compute the shadow texture's texel size (g_d3dimShadowTexel, TerrainShadow::texelSize) |
| `ShadowReceiver::UnknownVirtualSlot29` and overrides | `NeedsShadowRender` | ShadowReceiver slot 29, object_offset 0 (base 0x00468c90 return 1; 0x00446c30; 0x005099c0) | 3 | its result is OR'd into dirtyFlags, which makes PrepareGeometry render the shadow and run slot 30 (BuildShadowVertices). D3DIMSoultreeShadow tests the render camera (inCameraView); TerrainShadow returns whether it gathered any height-field cells. |
| `FollowCamera::UnknownVirtualSlot50` (pure), `VehicleCamera::`, `BikeCamera::` | `GetOnboardEyePosition` | FollowCamera slot 50, object_offset 0 (__purecall; 0x0052cb80; 0x00417290, which KrustyBikeCamera inherits) | 2 | ComputeCameraPosition (0x00464b30) takes it as the camera position in state 3, the state in which Tick hides the subject and applies GetRoll. BikeCamera returns the rider's "Head" node position (literal "Head"), VehicleCamera the vehicle's position raised by 5. |
| `FollowCamera::UnknownVirtualSlot55`, `KrustyBikeCamera::` | `IsLookBackHeld` | FollowCamera slot 55, object_offset 0 (0x00464ea0, 0x00498080) | 2 | while it returns nonzero, FollowCamera::Tick (0x00465c20, call at 0x00466859) saves the view and switches to state 6 with ApplyReverseViewPreset, restoring it on release. FollowCamera reads keyboard code 0x38, KrustyBikeCamera control 0x0b. |
| `FollowCamera::UnknownVirtualSlot56`, `KrustyBikeCamera::` | `IsTargetViewHeld` | FollowCamera slot 56, object_offset 0 (0x00464ec0, 0x004980a0) | 3 | while it returns true, Tick (call at 0x00466740) saves the view and switches to state 7 with LookAtTargetPoint, restoring it on release. FollowCamera reads keyboard code 0x2a, KrustyBikeCamera control 0x0a with extra airborne conditions. |

Views (same slot index and object_offset 0, names only):

| Old name | New name | Class / slot / object_offset | Tier | Evidence |
| --- | --- | --- | --- | --- |
| `UnknownBikeRaceRacer::UnknownRacerVirtualSlot27`, `52`, `77` (BikeRace.h) | `StartWreck`, `CanCrash`, `IsPovHatPressed` | `UnknownBikeRaceRacer` own vtable as `KrustyBike` | 2 | Wave 15's racer view rule |
| `UnknownFollowedRacerSlots::UnknownVirtualSlot27` (FollowRacerNearMisses.cpp) | `StartWreck` | as `KrustyBike` slot 27 | 2 | Wave 15's racer view rule |
| `UnknownScenePhysicsBody::UnknownVirtualSlot27` (SceneManagerNearMisses.cpp), `TireVehicle::UnknownVirtualSlot27` (Tire.h) | `StartWreck` | SoultreePhysicsBaseObject slot 27 views at object_offset 0 | 2 | their slots 0-25 and 28 on already carry SoultreePhysicsBaseObject's names in the same order |

Left as placeholders:

- GameObject slots 9 and 19 and Game slot 12: no class overrides slot 9 and
  only GameObject's own child walk calls it; slot 19's only overrides are the
  `return 0` bodies of Terrain (0x00507920) and ShadowCamera (0x004da550),
  and its only caller is Game slot 12, which no retail code calls (neither
  MainWindowProc 0x004a0d50 nor ControlInterface::DispatchInput 0x0043cf00).
  The GameObject and game views keep them too, and slot 0 stays a placeholder
  in the views (Waves 4 and 12).
- SoultreePhysicsBaseObject slot 26: empty in every class (0x00464e90); its
  two call sites (after IntegrateMotion, and after placement in the AI frame)
  name no single role. So `UnknownRacerVirtualSlot26` and the views' slot 26.
- Vehicle slots 78 and 79: no retail caller; they read keys 0x1f/0x2d and
  joystick buttons 3/4, which name no action. So `UnknownRacerVirtualSlot78`
  and `79`.
- PhysicsBody slots 31 and 32: not overridden, no caller found; they store
  `field_0x180` and `field_0x184`, which have no reader evidence.
- UIDialog slot 30 (calls the +0xd4 callback), BackgroundImage slot 27 (the
  shared `return 1` body) and BaseQuarryEvent slots 28, 32 and 34 (shared
  empty bodies): not overridden, and the call sites name no role.
- `UnknownBikeRaceCharacter` slots: D3DIMSoultreeCharacter's own slots were
  still `CharacterVirtualSlotN` / `D3DIMVirtualSlot11` placeholders (named in
  Wave 17).
- The GridEntry views (`UnknownKrustyVcrRecord`, `UnknownBikeRaceNetPlayer`):
  their `networkPlayerId` already says what GridEntry's `playerId` says, and
  `UnknownGridEntry` already uses GridEntry's names.
- `UnknownMethodN` (COM interfaces in GUIManager.h and MSZoneInterface.cpp;
  all but MSZoneInterface's named in Wave 17)
  and the folded-stub names (`UnknownFunction4de580`, `UnknownFunction464e80`,
  `UnknownFunction464e90`): no literal names them.

## Wave 17: Character slots, COM method names and view merges

Wave 17 names D3DIMSoultreeCharacter's primary slots and carries them to the
character views. It gives the COM interface views their SDK method names and
tries eight view merges, smallest first. `git grep -ohE
'\bUnknown[A-Z][A-Za-z0-9_]*' -- src samples` fell from 496 unique names and
3526 uses to 466 and 3350. Counted the same way, `UnknownMethodN` fell from
154 uses to 17, the MSN Zone score interface's. `CharacterVirtualSlotN`,
`ModelVirtualSlotN`, `D3DIMVirtualSlot11` and `ApplyPoseListSlotN` fell from
173 uses to none. `field_0x` lines stay at 1879.

Method: the slot renames are scoped by class and use the Wave 16 tools
(work/naming/w17_char.json, w17_editsA.py, w17_editsA2.py), and the COM names
use work/naming/w17_com.py. Each merge is one script (work/naming/w17_m1.py to
w17_m4.py) that edits the view out, drops casts made redundant and renames the
decorated consumers. The comparer builds every unit in src, samples and
tests/fixtures whose include closure reaches a changed file. It builds each
unit once from the previous commit (work/naming/mkold.py, which restores the
changed files from git with the checkout's CRLF) and once from the edited
tree.

The comparer gained one verdict, NAME-ONLY-LABELS. VC6 names a goto label
`$<label>$<counter>` (storage class 6, no bytes), and the counter moves when a
header declares one name more or fewer. When an object is equal only after
these counters are renumbered by first appearance on both sides, the verdict
is NAME-ONLY-LABELS. Every byte, section flag and relocation target still has
to match. The whole-object test pins hash only external and section symbols,
so these units need no repin.

Result (cumulative, every unit whose closure reaches one of the 81 changed
files, from bb33e20f to the final tree, map work/naming/map-final.json with 18
entries): 318 units, 263 IDENTICAL, 5
IDENTICAL-COMDAT-ORDER (DlgProcsNearMisses.cpp, Group4_DlgMain.cpp,
Group6_BikeRaceCreate.cpp, DlgProcs.cpp, ProCircuitProcs.cpp), 15 NAME-ONLY
(the units that define or call a renamed Character slot: LayoutProbe.cpp,
CharacterMotionStart.cpp, samples Motnctrl.cpp, WreckerNearMisses.cpp,
CollisionCharacter.cpp, D3DIMSoultreeMotnctrl.cpp, Motnctrl.cpp,
SoulTreePhysics.cpp, Bike.cpp, KrustyBike.cpp, Vehicle.cpp,
CharacterDefaults.cpp, Wrecker.cpp and the fixtures character_defaults.cpp and
character_motion_start_compare.cpp) and 35 NAME-ONLY-LABELS (the units that
include Track.h, and KrustyUI.cpp). There was no DIFF and no compile failure, and no .bss or
.data offset moved.

Negative controls:

- Without the map, samples/physics Motnctrl.obj gives DIFF in the relocations
  of the vtable `??_7Character@@6B0@@`, and Wrecker.obj gives DIFF in the
  relocations of `?EndWreck`. Without the map, the Track.obj of the FileStream
  merge gives DIFF: the COMDAT `?FreeNodeArray@...PAVUnknownStream@@...` exists
  only in the old object.
- A live change (`motionTime = 0.0f` to `1.0f` in the new tree's
  Motnctrl.cpp) gives DIFF in the bytes of `??0Character@@QAE@H@Z`. The same
  control was rerun after the label pass was added and still gives DIFF.

Fixture pins: whole-object and aggregate digests of NAME-ONLY units only, each
with unchanged section and relocation counts: Bike.cpp, KrustyBike.cpp,
Vehicle.cpp, SoulTreePhysics.cpp, D3DIMSoultreeMotnctrl.cpp, Wrecker.cpp,
KrustyBikeNearMisses.cpp, WreckerNearMisses.cpp, samples Motnctrl.cpp and
CharacterMotionStart.cpp. They live in 12 fixture files and in the digest
tuples of tests/test_rider_motion_start_connections.py (work/naming/repin.py,
then work/naming/w17_repinA.py and w17_repinA2.py). The two merges changed no
pinned digest.

### Slots

| Old name | New name | Class / slot / object_offset | Tier | Evidence |
| --- | --- | --- | --- | --- |
| `CharacterVirtualSlot0` | `BuildModel` | Character slot 0, object_offset 0 (0x00464e90 empty stub; D3DIM 0x00445a70) | 2 | D3DIM loads "Base Character Frame" and the character model; slot 1 calls it first |
| `CharacterVirtualSlot1` | `LoadFromSlt` | Character slot 1 (0x004a6930, not overridden) | 2 | reads the .slt "General info" keys, then calls slot 0, slot 2, LoadMotions and ApplyRestPose |
| `CharacterVirtualSlot2` | `CaptureRestPose` | Character slot 2 (0x00464e90; D3DIM 0x00445fc0) | 2 | D3DIM stores every node's current pose in poseList, the rest pose |
| `CharacterVirtualSlot3` | `CaptureFramePoses` | Character slot 3 (0x00464e80; D3DIM 0x00446110) | 2 | CaptureMotion's per-frame call; D3DIM overwrites the frame's records with the live node poses |
| `CharacterVirtualSlot4` | `ApplyPose` | Character slot 4 (0x004a6ba0; D3DIM 0x00446210) | 2 | applies one pose record to its node under the per-row enables |
| `CharacterVirtualSlot5` | `ApplyMirroredPose` | Character slot 5 (0x004a6ba0; D3DIM 0x00446300) | 2 | the same, through mirrorMap |
| `CharacterVirtualSlot6` | `ApplyFullPose` | Character slot 6 (0x004a6ba0; D3DIM 0x00446480) | 2 | ignores applyRotation/applyTranslation; the rest-pose path |
| `CharacterVirtualSlot7` | `AdvanceMotionInterpolated` | Character slot 7 (0x004a70c0, not overridden) | 2 | per-frame playback; the float argument is added to motionTime (+0x10) and the frames are interpolated |
| `CharacterVirtualSlot8` | `CopyStateFrom` | Character slot 8 (0x004a98b0; D3DIM 0x00445e70) | 2 | copies another character's motion state, names and rest pose, then calls slot 9 |
| `CharacterVirtualSlot9` | `CopyModelFrom` | Character slot 9 (0x00464e80; D3DIM 0x00445ec0) | 2 | D3DIM clones the other character's node model |
| `CharacterVirtualSlot10` | `DumpNodeTransforms` | Character slot 10 (0x00464e90; D3DIM 0x00446520) | 2 | D3DIM formats a "Position/Orientation Data" line per node (the output is stripped) |
| `CharacterVirtualSlot11`, `D3DIMVirtualSlot11` | `LoadCharacter` | D3DIMSoultreeCharacter slot 11, object_offset 0 (0x00445680) | 2 | loads the character resource and returns it, or releases itself and returns 0 when the resource is missing |
| `ApplyPoseListSlot4`, `ApplyPoseListSlot6` | `ApplyPoseList`, `ApplyFullPoseList` | non-virtual Character helpers 0x004a8a80, 0x004a8ac0 | 2 | they apply a pose list through slot 4 or slot 6 |

Views (same slot index and object_offset 0, names only; the objects are
D3DIMSoultreeCharacters): `UnknownBikeRaceCharacter` (BikeRace.h),
`UnknownKrustyUICharacter` and KrustyUI's model view (`ModelVirtualSlotN`).

### COM methods

| Interface view | SDK interface | Slots declared | Tier | Evidence |
| --- | --- | --- | --- | --- |
| `DirectDraw` (RenderInterfaces.h) | IDirectDraw7 | 28 | 1 | IID_IDirectDraw7 and the DirectDrawCreateEx import; every decoded index and argument count agrees |
| `Direct3D` | IDirect3D7 | 7 | 1 | IID_IDirect3D7 |
| `UnknownDirectInput`, `DirectInputDevice` (PCInputDevice.h) | IDirectInput7A, IDirectInputDevice7A | 10, 26 | 1 | IID_IDirectInput7A 0x005569e0, IID_IDirectInputDevice7A 0x00556a50 |
| `AMMultiMediaStream`, `DirectDrawMediaStream` (MediaControl.h) | IAMMultiMediaStream, IDirectDrawMediaStream | 26, 14 | 1 | IID_IAMMultiMediaStream 0x00558ed0, IID_IDirectDrawMediaStream 0x00558e90 |
| `UnknownGuiDirectDraw`, `Direct3DDevice`, `DirectDrawSurface`, `UnknownGuiSurface`, `DirectDrawClipper`, `UnknownGuiClipper`, `DirectDrawPalette`, `DirectDrawGammaControl`, `DirectDrawStreamSample` | IDirectDraw7 (GUIManager's two-method view), IDirect3DDevice7, IDirectDrawSurface7, IDirectDrawClipper, IDirectDrawPalette, IDirectDrawGammaControl, IDirectDrawStreamSample | the remaining `UnknownMethodN` | 2 | created by a confirmed interface's method; there is no own IID literal, but the indices and argument counts agree with the SDK order |

All 137 renamed slots keep their index. Every slot that was already named
agrees with the same SDK tables.

### Merges

| View | Into | Result |
| --- | --- | --- |
| `UnknownKrustyUIContext` (KrustyUI.cpp) | `TextureContext` | merged (6604ab6b). It is KrustyUI's copy of Game+0x1c..+0x33, with TextureContext's six fields at the same offsets. 1 unit, NAME-ONLY: only the label `$found$` in PickRandomRacers moved. |
| `UnknownStream` (Track.h) | `FileStream` | merged (e77fe250). Track::FreeNodeArray (0x00515e70) receives the loader's FileStream, and the forward-declared view only forced 14 casts in LoadTrack. 181 units: 143 IDENTICAL, 3 IDENTICAL-COMDAT-ORDER, 35 NAME-ONLY-LABELS. |
| `UnknownCursorAnimation` (GUIManager.h) | `UIAnim` | blocked: samples/race/BikeRaceNearMisses.cpp gives DIFF in the bytes of `?CreateRacers@BikeRace@@...`. That sample includes GUIManagerCore.h before BikeRace.cpp, and BikeRace's register allocation is sensitive to the declarations before it (Wave 8). The other 85 units were IDENTICAL or name-only. The diff is work/naming/w17_m3_blocked.diff. |
| `UnknownTexturePalette` (TextureMap.h) | `ColorMapper` | blocked: 148 units, 3 DIFF and 3 COMPILE-FAIL. Terrain.cpp's `Render3D` swaps the order of its `g_terrainRender3DTicks` and `g_terrainPrepareGeometryTicks` references, ManagedTextureGroup.cpp's `DrawSelection` grows 4 bytes before its `__ftol` calls, and BikeRaceNearMisses' CreateRacers changes as above. PCTextureMap.cpp, PCTextureMapNearMisses.cpp and TextureClone.cpp read `colorMapper->indexFrom555`/`entries` and would need Quantize.h. The new BikeRaceNearMisses object is identical to the one from the CursorAnimation attempt, so CreateRacers reacts to one struct fewer, not to which struct. The diff is work/naming/w17_m4_blocked.diff. |
| `UnknownTimer` (TextureMapManager.h) | `PeakHold` | blocked: [TEXTURE_HISTORY_TIMER](TEXTURE_HISTORY_TIMER.md) records that both the alias and the moved view changed strict Scene::ReadLight from 2102 to 2065 matching positions. |
| `UnknownGuiControl` (GUIManager.h) | `UIControl` | blocked: the view's +0x210/+0x214 (acceptedCharacters, caretRect) are UIEditBox fields, and `LeaveHover`/`KillFocus` take `int` in UIControl and its six overrides but the view in GUIUser. |
| `UnknownEcoTrackGame` (EcoSystem.h) | `TrackGame` | blocked: TrackGame's display is a PCVideoCard. The view's +0x28 colorKeyFormat is element 5 of `VideoCard::field_0x14[16]` and +0x1c0 capabilityBits is inside `PCVideoCard::field_0x1c0[]`, so the PCVideoCard fields must be split first. The EcoSystem units would also need TrackGame.h. |
| `UnknownSoultreeNodeView` (bringup, D3DIMSoulTreeNearMisses) | `SoultreeObject` | blocked: the view takes and returns Vector3 by pointer where SoultreeObject uses `Vec3&` and returns Vec3, so the call sites would need conversions. Its comment also records that more declarations in D3DIMSoulTree.h change SceneManager.cpp 0x004eb570. |
| `UnknownEventRacer`, `UnknownBikeRaceRacer` | `KrustyBike` | not attempted (out of scope unless the rest was done). Both have a virtual GameObject base and BikeRace.h is scheduling-sensitive. |

Left as placeholders:

- PhysicsBody `field_0x180`/`field_0x184` and slots 31 and 32: still no
  reader of the fields and no caller of the slots.
- UIDialog slot 30: it calls the +0xd4 callback, and no caller of the slot
  names a role.
- MSZoneInterface's `UnknownZoneScore` methods 3 to 5: its IID
  3FAF0AFD-B48B-11D2-8A51-00C04F8EF4E9 is not a public SDK interface.

## Wave 18: the declaration count behind the blocked view merges

Wave 18 asked why removing one view struct changes
`?CreateRacers@BikeRace@@...` (0x00419970) and whether a readable source form
keeps it stable, so the three view merges blocked in Wave 17 can proceed.
The answer is the declaration-count tie-break of
[VC6_OPERAND_ORDER.md](VC6_OPERAND_ORDER.md) section 2, and no merge could be
kept: every candidate also moves at least one other exact or pinned function.
No source file changed. `git grep -ohE '\bUnknown[A-Z][A-Za-z0-9_]*' -- src
samples` stays at 466 unique names and 3350 uses.

### Characterisation

Method: work/w18/probe.py compiles a wrapper that is a prefix file followed by
`#include "<unit>"`, with the unit's own profile. work/w18/gen.py writes
prefixes of k declarations of one kind. work/w18/bscan.py `base UNIT` keeps
the unit's whole object for k = 0..63 typedefs. `fit UNIT MAP` then finds
the k at which a candidate tree's object equals the base object under the
proof map, which is the candidate's counter shift.

Measured on samples/race/BikeRaceNearMisses.cpp, which includes
GUIManagerCore.h and the canonical headers before BikeRace.cpp:

- The order at body offset 0x21a is the only difference. That is
  `xor edi,edi; mov ebx,1` (retail, 13,396 of 13,428 strict bytes) against
  `mov ebx,1; xor edi,edi` (the current sample, 13,390). It depends only on
  k mod 64. Over 131 typedef counts the outcome has exact period 64:
  `MMMMMMXXXMXXMMMMMMXMMXMXXXMMMMXXXXMXMXMMMMXXMXMMMXXMMMXXMMMMMXMM`
  (k = 0 first; M is the current order, X the retail order). The
  whole-object scan gives the same pattern, so CreateRacers is the only
  sensitive function in that object.
- Weights, in typedefs:
  - typedef, forward declaration of a new name, function declaration, extern
    variable, class member, enumerator: 1 each;
  - one-enumerator enum: 2;
  - class or struct definition: 8 plus 1 per member;
  - first class with an inline constructor: 11, later ones: 10;
  - redeclaration of an existing name: 0.

  These are the half-units of section 2.1. Name length, the environment,
  `/Z7`, `/Zi` and no debug information do not matter, and neither does
  anything after the function. Shifts add across header positions.
- BIKE_RACE_STATE_CONNECTIONS.md said the order depends only on how many
  inline-bearing classes the unit declares. That is a special case: any
  declaration counts by its weight.
- Wave 17's two merges shift the sample as follows:
  - the CursorAnimation merge (`UnknownCursorAnimation` -> `UIAnim`) by -15:
    UIDialog.h's forward declaration -1 and GUIManager.h's 6-member struct
    -14;
  - the TexturePalette merge by -15: the 8-member struct -16 and
    `class ColorMapper;` +1.

  Alone, either one lands k = 49, which is X. Together they land k = 34,
  which is M.

The forms tried for the CursorAnimation merge all give X, as the weights
predict:

- a `typedef UIAnim UnknownCursorAnimation;` alias;
- a `class UIAnim;` forward declaration;
- keeping `struct UnknownCursorAnimation;`;
- dropping GUIManagerCore.h's forward declaration.

Only keeping a struct definition gives M, even an empty
`struct UnknownCursorAnimation {}`, and that would be a padding declaration.

The counter is per unit and per function, so one edit is a different lottery
in every unit it reaches. Base tables at 28aaba41 (whole object equal to the
k = 0 object, `=`):

| Unit (sensitive function) | k = 0..63 |
| --- | --- |
| BikeRaceNearMisses.cpp (CreateRacers) | `======DDD=DD======D==D=DDD====DDDD=D=D====DD=D===DD===DD=====D==` |
| Group6_BikeRaceCreateRacers.cpp (CreateRacers) | `===DDDDDDD====DD===DDDD=DDD===D==DD==DDDD=DDDD=====DD====DDD==D=` |
| Terrain.cpp (Render3D 0x00506220, exact) | `=DDDDD==DDDDDDD====D==DDDD==DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD=` |
| ManagedTextureGroup.cpp (DrawSelection 0x0050ef70, exact) | `======DDD===========DDDD==========DDDDDDDDDDDDDDDDDDDDDDDDDDD===` |
| tests/fixtures/edit_box_draw_compare.cpp (Setup) | `===================================DDDDDDDDDDDDDD=D=D==D========` |
| GameUi.cpp, GameUiNearMisses.cpp | all `=` |

Terrain's Render3D is exact only at k in {63, 0, 6, 7, 15..18, 20, 21, 26,
27}. That set replaces the windows that VC6_OPERAND_ORDER.md section 2.3
recorded before later header changes. Any edit before it that shifts its
count by 1 to 5 or by -2 to -36 swaps its two tick loads.

Allowed levers are the ones section 2.3 already names: function order that
follows retail addresses, and dropping includes a unit does not use. Neither
applies here:

- Terrain.cpp's two definitions before Render3D are already in retail order.
- Terrain.cpp reads `PCVideoCard::driverGuidText` and includes Quantize.h
  itself.

So no meaningful source form keeps every unit stable, and padding
declarations are excluded (AGENTS.md).

### Merges

Proof: work/naming/proof.py over every unit whose include closure reaches a
changed file, old tree 28aaba41 (work/w18/runproof.sh).

| View | Into | Result |
| --- | --- | --- |
| `UnknownCursorAnimation` (GUIManager.h) | `UIAnim` | blocked. 86 units: 63 IDENTICAL, 6 IDENTICAL-COMDAT-ORDER, 4 NAME-ONLY, 12 NAME-ONLY-LABELS, 1 DIFF. The DIFF is BikeRaceNearMisses' CreateRacers (shift -15, k = 49). That DIFF is the retail order: match_one gives 13,396 strict bytes instead of 13,390. The merge improves the unadmitted sample, but it changes its pinned CreateRacers fingerprint (test_race_state_connections.py), so the strict rule keeps it out. Diff: work/naming/w17_m3_blocked.diff. |
| `UnknownTexturePalette` (TextureMap.h) | `ColorMapper` | blocked. Alone, it fails 149 units: DIFF in the sample's CreateRacers, Terrain's Render3D relocations (shift -16, k = 48), ManagedTextureGroup's DrawSelection bytes and GameUi/GameUiNearMisses' `?Tick@UIDialog@@` relocations. Together with the CursorAnimation merge (work/w18/m3m4.diff) the sample and both GameUi units pass. 192 units: 138 IDENTICAL, 5 IDENTICAL-COMDAT-ORDER, 11 NAME-ONLY, 30 NAME-ONLY-LABELS. The combination still fails: Group6_BikeRaceCreateRacers.cpp's CreateRacers bytes (shift -30), Terrain's Render3D (still -16) and DrawSelection (-16 or -30) give DIFF, and so does edit_box_draw_compare's Setup. PCTextureMap.cpp, PCTextureMapNearMisses.cpp and TextureClone.cpp fail to compile without Quantize.h. Terrain already includes Quantize.h, so its -16 is the same whether TextureMap.h forward-declares ColorMapper or includes Quantize.h, and a typedef alias gives -15. No form reaches Terrain's window. |
| `UnknownEcoTrackGame` (EcoSystem.h) | `TrackGame` | blocked at the prerequisite split. +0x1c0 is DDCAPS `dwCKeyCaps`: the DDCAPS block starts at +0x1b4, and the block's `dwVidMemTotal` lands on `driverVideoMemory` +0x1f0. Splitting `PCVideoCard::field_0x1c0[]` into `unsigned int driverColorKeyCaps; unsigned char field_0x1c4[...]` adds one member. 236 units: 190 IDENTICAL, 7 IDENTICAL-COMDAT-ORDER, 38 NAME-ONLY-LABELS, and DIFF in Terrain's Render3D relocations (shift +1). Diff: work/w18/b3split.diff. +0x28 cannot be split at all: it is element 5 of `VideoCard::field_0x14[16]`, which TextureMapManager.cpp indexes by texture kind. The view's `colorKeyFormat` name also conflicts with that per-kind role, so +0x28 needs its own evidence first. |
| `UnknownEventRacer`, `UnknownBikeRaceRacer` | `KrustyBike` | not attempted (the brief's condition was not met). |

Negative controls (CursorAnimation merge, old 28aaba41, units GUIManager.cpp
and UIDialogTick.cpp):

- With work/w18/map-m3.json both are NAME-ONLY.
- Without the map, GUIManager.cpp gives DIFF: the COMDAT
  `?SetAnimation@GUICursor@@QAEXPAUUnknownCursorAnimation@@@Z` exists only
  in the old object. UIDialogTick.cpp gives DIFF in the relocations of
  `?Tick@UIDialog@@UAEHM@Z`.
- A live change in the new tree (`currentFrame = 0` to `1` in
  GUICursor::SetAnimation) gives DIFF in the bytes of
  `?SetAnimation@GUICursor@@QAEXPAVUIAnim@@@Z`. UIDialogTick.cpp stays
  NAME-ONLY.

No fixture was repinned.

Follow-up (2026-11-13): the `PCVideoCard::field_0x1c0[]` split is on main as
`unsigned int driverColorKeyCaps; // +0x1c0` (DDCAPS `dwCKeyCaps`, tier 2:
the DDCAPS block starts at +0x1b4 and EcoSystem.cpp tests bit 8 there). The
Render3D change it causes is the one retail needs: the physics runner row
0x00506220 (src/krusty2/broadphase/targets.json) went from 3171/3173 to
strict exact, and the Terrain.cpp calibration rows stayed exact. Other units
that include the header differ only in `$L`/`$T` label numbers or RTTI
COMDAT order. `UnknownEcoTrackGame::display->capabilityBits` can now become
`driverColorKeyCaps`; the +0x28 `colorKeyFormat` part of that merge is still
blocked as described above.

## Wave 19: SDK layouts, scene sections, helpers and messages

Wave 19 names the long tail of distinct `Unknown*` types and helpers that are
not views of named classes. It skips `UnknownVirtualSlotN`, `UnknownMethodN`,
the shared stubs, `UnknownPodium*`, statics and the types of statics. Names
only: no declaration, typedef, forward declaration or member was added or
removed. `git grep -ohE '\bUnknown[A-Z][A-Za-z0-9_]*' -- src samples` fell
from 466 unique names and 3350 uses to 394 unique names and 2947 uses. `field_0x` lines fell
from 1879 to 1872 (1998 to 1991 occurrences).

Method: work/naming/w19/triage.py lists each name's definitions and the
comments before them, show.py prints the bodies, strs.py finds a literal in
mcm2.exe's .rdata/.data and the retail instructions that reference it, and
coll.py counts a candidate name across the tracked tree. The renames are the
maps work/naming/w19/map-A.json, map-B.json, map-D.json, map-E.json and
map-E2.json, applied with work/naming/rename.py (map-E2 excludes
InGameProcs.cpp), and the field renames are work/naming/w19/fields.py. A new
name that two units define locally replaces one old name shared by exactly
those definitions, so no unit gains a second definition. The field leads came
from work/naming/classreads.py v2 (`layouts`, then `scan`).

Result (every unit whose closure reaches one of the 96 changed files,
from 664e1883 to the final tree, map work/naming/w19/map-wave19.json with 73
entries): 307 units, 292 IDENTICAL, 6 IDENTICAL-COMDAT-ORDER
(DlgProcsNearMisses.cpp, Group4_DlgMain.cpp, Group5_PCBailoutDlg.cpp,
Group6_BikeRaceCreate.cpp, KrustyUI.cpp, OptionProcs.cpp) and 9 NAME-ONLY
(MSZoneInterface.cpp, PCControl.cpp, PCJoystickDevice.cpp,
Parameterblocks.cpp, TrackGame.cpp, TrackOverlay.cpp and the fixtures
media_control_open_compare.cpp, primary_sound_format_compare.cpp and
race_sound_refill_compare.cpp: the units whose decorated names carry a
renamed type). There was no DIFF and no compile failure, and no .bss or
.data offset moved. No unit is NAME-ONLY because of a helper or field
rename.

Negative controls:

- Without the map, PCControl.obj gives DIFF in the relocations of
  `?CreateDevices@PCControlInterface@@UAEHXZ`.
- A live change (`return 1` to `return 2` after "GUID_ConstantForce" in the
  new tree's PCControl.cpp, work/naming/w19/live.py) gives DIFF in the
  bytes of `?GetForceFeedbackEffectName@@YAHUGuid@@PAD@Z`.

Fixture pins: whole-object digests of NAME-ONLY units only, each with
unchanged section and relocation counts: TrackOverlay.cpp in
chat_name_tag_objects.json, stats_standings_objects.json,
clipper_global_objects.json and visibility_connections_objects.json, and
TrackGame.cpp in startup_list_objects.json (work/naming/repin.py; the two
canonical TrackOverlay records by hand after the same check). The
mangled names in the bindings of MSZoneInterface, TrackGame, PCControl,
PCJoystickDevice, Parameterblocks and TrackOverlay and in
tools/run_calibration.py were renamed with the types.

### Types

| Old name | New name | Tier | Evidence |
| --- | --- | --- | --- |
| `UnknownDirectInput` | `DirectInput7` | 1 | IID_IDirectInput7A `0x005569e0`, passed to `DirectInputCreateEx`; the view's ten slots are the SDK's IDirectInput7A order (Wave 17). |
| `UnknownDirectPlay4A`, `UnknownDirectPlayLobby3A` | `DirectPlay4A`, `DirectPlayLobby3A` | 1 | IID_IDirectPlay4A `0x005567b0` and IID_IDirectPlayLobby3A `0x00556870`; MSZoneInterface::SetDirectPlay (`0x004aa350`) stores both. |
| `UnknownEnumerateEx` | `DirectDrawEnumerateExFunction` | 1 | The typedef of the pointer `GetProcAddress(library, "DirectDrawEnumerateExA")` returns (PCVideoCard.cpp). |
| `UnknownDeviceObjectData`, `UnknownEffectParams`, `UnknownEffectInfo`, `UnknownDeviceInstance`, `UnknownObjectInstance`, `UnknownDataFormat`, `UnknownDeviceCaps`, `UnknownInputProperty`, `UnknownInputPropertyRange`, `UnknownEnvelope`, `UnknownPeriodic`, `UnknownJoystickState` | `DIDeviceObjectData`, `DIEffect`, `DIEffectInfo`, `DIDeviceInstance`, `DIDeviceObjectInstance`, `DIDataFormat`, `DIDevCaps`, `DIPropDword`, `DIPropRange`, `DIEnvelope`, `DIPeriodic`, `DIJoyState` | 2 | PCInputDevice.h and PCJoystickDevice.h: each layout and size is the DirectInput 7 structure the call that fills it takes (GetDeviceData, CreateEffect, GetEffectInfo, EnumDevices, EnumObjects, SetDataFormat with `c_dfDI*`, GetCapabilities, Get/SetProperty with DIPROP_* headers, GetDeviceState). `c_dfDIKeyboard`/`c_dfDIMouse`/`c_dfDIJoystick` are `extern "C"`, so their decorated names do not carry the type. |
| `UnknownDeviceIdentifier`, `UnknownBltFx`, `UnknownEnumCallback`, `UnknownViewport`, `UnknownScreenVertex`, `UnknownMediaSurfaceDesc` | `DDDeviceIdentifier2`, `DDBltFx`, `DDEnumCallback`, `D3DViewport7`, `D3DTLVertex`, `DDSurfaceDesc` | 2 | The 0x430-byte GetDeviceIdentifier record, the Blt effects record, the DirectDrawEnumerateA callback, the SetViewport record, the FVF 0x1c4 vertex and MediaControl's 0x6c-byte (DirectX 6) surface description. Earlier waves' convention: `DDSurfaceDesc2`, `DDSCaps2`, `DDPixelFormat`. |
| `UnknownFogBackdropVertex` | `FogBackdropVertex` | 3 | Fog.cpp's backdrop vertex; it is the D3DTLVERTEX layout, but PCVideoCard.cpp's `D3DTLVertex` is a separate local definition, so the role name avoids two definitions of one name. |
| `UnknownLogFont`, `UnknownTextExtent`, `UnknownMemoryStatus`, `UnknownWaveFormat`, `UnknownPaletteEntry` | `LogFont`, `Win32Size`, `MemoryStatus`, `WaveFormatEx`, `PaletteEntry` | 2 | The CreateFontIndirectA record (0x3c bytes, three local definitions), GetTextExtentPoint32A's SIZE, GlobalMemoryStatus's record (two local definitions), the WAVEFORMATEX the primary buffer's SetFormat takes, and the PALETTEENTRY table. Win32Size follows Wave 14's `Win32Rect`. |
| `UnknownEaxListenerParameters` | `EaxReverbProperties` | 2 | The 16-byte {environment, volume, decayTime, damping} record RaceSound passes to PCSoundInterface `0x004bed40` (for example environment 9, volume 0.361, decay 7.0, damping 0.332): EAX 1.0's EAX_REVERBPROPERTIES. |
| `UnknownLobbyGetProperty`, `UnknownLobbyGetPropertyResponse` | `DPLMsgGetProperty`, `DPLMsgGetPropertyResponse` | 2 | MSZoneInterface.cpp's lobby messages type 7 and type 8: DPLMSG_GETPROPERTY and DPLMSG_GETPROPERTYRESPONSE. |
| `UnknownQuarryLeaveMessage` (QuarryStuntEvent.cpp), `UnknownPlayerLeftMessage` (SelectGamePicProcs.cpp only) | `DPMsgDestroyPlayerOrGroup` | 2 | System message 5 is DPSYS_DESTROYPLAYERORGROUP; both views read the player id at +0x08 (dpId) and leave +0x04 (dwPlayerType). Neither unit includes the other. |
| `UnknownChatMessage`, `UnknownKickMessage`, `UnknownLobbyStartMessage`, `UnknownLoadFailedMessage` | `ChatMessage`, `RemovePlayerMessage`, `LobbyStartMessage`, `LoadFailedMessage` | 3 | Application messages 0x85 (chat text), 0x8e (the removed player's id), 0x83 (the 0x8d8-byte race start with the racer slots) and 0xcc (a race failed to load), named after the handlers that read them. |
| `UnknownSceneLight`, `UnknownSceneEnvironment`, `UnknownSceneFog`, `UnknownSceneBuffer`, `UnknownSceneCaster` | `SceneLight`, `SceneEnvironment`, `SceneFog`, `SceneStaticModels`, `SceneStaticModel` | 2 | The .scn sections the loaders read into Scene+0xac, +0xa4, +0xb0 and +0xb8: "Light%d" (`0x00572f54`), "Environment" (`0x00573300`), "Fog" (`0x00573060`), "StaticModels" (`0x005735f8`) with one "Model%d" (`0x0057359c`) element each. |
| `UnknownParameterValue`, `UnknownParameterSection` | `ParameterValue`, `ParameterSection` | 2 | Parameterblocks.cpp's parsed row value (0 int, 1 float, 2 string) and its 0x40c-byte section index (stream offset, line count, name). |
| `UnknownHiResMeterEntry` | `HiResMeterTimer` | 2 | HiResMeter's `timers[50]`; the columns are slot 10's "MS Timers [...]" literal (`0x0056c4ec`). |
| `UnknownGameUiSection` | `DialogSection` | 2 | UIDialog+0x9cc: one 0x3c-byte .dtm section per name, its kind set by the "Set_*" markers (Wave 12's `sectionKind`). |
| `UnknownTrackRecordRow`, `UnknownRaceSoundChannel`, `UnknownSoundFade`, `UnknownRecorderThreadParameters`, `UnknownGauge` | `TrackRecordRow`, `EngineSoundChannel`, `SoundFade`, `RecorderThreadParameters`, `GaugeNeedle` | 3 | The track-record list row, RaceSound's eleven per-racer engine channels, PCAudio's volume fade, what 0x004e7a90 passes to the recorder's worker thread, and InstrumentOverlay's dial needle (+0x11c). |

### Helpers

Static inline helpers named after their decoded bodies (tier 3, the Wave 14
precedent):

| Old name | New name | Unit |
| --- | --- | --- |
| `UnknownMin`, `UnknownMax` | `MinFloat`, `MaxFloat` | GearRatios.cpp |
| `UnknownFunctionMax` | `MaxFloat` | BikeRace.cpp |
| `UnknownMinInt`, `UnknownMaxInt` | `MinInt`, `MaxInt` | GameUi.cpp |
| `UnknownSquareMagnitude` | `EcoSquareMagnitude` | EcoSystem.cpp (Math3D.h already has a `SquareMagnitude` with another summation order) |
| `UnknownSetIdentity`, `UnknownEcoNormalizeXZ`, `UnknownEcoScaleXZ`, `UnknownEcoMin1`, `UnknownEcoZ`, `UnknownEcoOffset`, `UnknownEcoRandom`, `UnknownEcoTgaIsSet`, `UnknownEcoLean`, `UnknownEcoLitColor`, `UnknownEcoLightOn` | `EcoSetIdentity`, `EcoNormalizeXZ`, `EcoScaleXZ`, `EcoMinOne`, `EcoComponentZ`, `EcoSum`, `EcoRandomUnit`, `EcoTgaIsSet`, `EcoLean`, `EcoLitColor`, `EcoLightIntensity` | EcoSystemNearMisses.cpp, Group4_EcoSystem.cpp |
| `UnknownScaled`, `UnknownRandomSkill` | `ScaledVector`, `RandomSkill` | BikeRaceNearMisses.cpp, Group6_BikeRaceCreate*.cpp |
| `UnknownFileExists` | `FileExists` | ProCircuitProcs.cpp |
| `UnknownDropDownList` | `DropDownListBox` | OptionProcs.cpp |
| `UnknownAsVec3` | `Vector3AsVec3` | Group6_BikeRaceViews.cpp (other bringup units already have an `AsVec3`) |

### Fields

| Class | Offset | Old name | New name | Tier | Evidence |
| --- | --- | --- | --- | --- | --- |
| `ObjectPicker` | +0x2c | `field_0x2c` | `pickedObject` | 2 | PickerHitCallback (`0x00417b00`) stores the hit CollisionObject there, and PickAt (`0x004b0500`) returns it when QueryCollisions hits. |
| `GameCursor` | +0x50 | `field_0x50` | `drawScale` | 3 | Draw (`0x0043ed40`) multiplies the destination's right and bottom by it; the constructor stores 1.0 and nothing else writes it. |

Left as placeholders:

- `UnknownBltRect` and `UnknownBltEffects` (ManagedTextureGroup.cpp): copies
  of `Win32Rect` and `DDBltFx`. Using those names would be a merge, not a
  rename.
- `UnknownPlayerLeftMessage` (InGameProcs.cpp): application message 0x89,
  a different record from the type-5 view renamed above (Wave 20 names it
  `LeaveRaceMessage`).
- `UnknownSceneKeyword`: the type of the static `s_lightTypes`.
  `UnknownSceneShapeKeyword` and `UnknownSceneEmitterKeyword` (samples) wait
  with it.
- The `UnknownOpt*Settings` types (globals), `UnknownFunction4ea7e0`,
  `UnknownFunction4864f0`, and the `UnknownCompare*` comparators: no literal
  names them, and the sort keys were not decoded this wave (Wave 20 names
  the comparators and `IsPaletteLocked`).
- `TrackGameMode::field_0xa20` and `UnknownOptSoundSettings::field_0x00`
  (TrackGame+0xf98): the default is 0, and LeaveRaceForMenu and EventManager
  (`0x0045e7b6`) call StopCDAudio when it is clear. That suggests a CD music
  option, but no dialog control or literal names it (Wave 20: tier 3
  `cdAudioOn`).
- `Scene::field_0x88`: Scene's initialiser (`0x004ea7e0`) stores its char
  argument, and 0x004ecd60 passes it to a constructor whose parameter is not
  decoded.
- `UIDialog::field_0x180` (7 by default): it is passed as the second argument
  of PlayControlSound, which the base implementation ignores.
- The union arms `UIScrollBar::field_0x1ec/0x204/0x218`,
  `UIEditBox::field_0x204` and `UIListBox::field_0x218` are already named by
  their typed arms.

## Wave 20: comparators, race result records, lobby messages and read-backed fields

Wave 20 decodes the sort keys of the `UnknownCompare*` comparators, names
the race result and lobby records and the messages Wave 19 left, and names
fields whose only reader or writer decides their role. Names only: no
declaration, typedef, forward declaration or member was added or removed,
and no static or type of a static was renamed.
`git grep -ohE '\bUnknown[A-Z][A-Za-z0-9_]*' -- src samples` fell from 394
unique names and 2946 uses to 377 unique names and 2840 uses. `field_0x`
lines fell from 1872 to 1844 (1991 to 1973 occurrences).

Method: work/naming/w20/storers.py lists the stores to `[reg + offset]` in a
VA range with their containing function. work/naming/xref.py and writers.py
were used as in Wave 19, and so was classreads.py v2 (`layouts`, then `scan`).
The type and comparator renames are work/naming/w20/map-A.json, applied with
work/naming/rename.py. InGameProcs.cpp is the only unit where
`UnknownPlayerLeftMessage` was renamed. The field renames are the scripts
work/naming/w20/t1.py, cdaudio.py and fields.py, plus the four
`fullState` uses in Group3_KrustyBike.cpp. The method rename is
work/naming/w20/map-B.json.

Result: two proofs, each run over every unit whose closure reaches a changed
file.

- Batch A runs from 0eb90ea3 to 98ffa4f3 with map-A (16 entries). It has
  250 units: 242 IDENTICAL, 3 IDENTICAL-COMDAT-ORDER and 5 NAME-ONLY.
  - The COMDAT-ORDER units are DlgProcsNearMisses.cpp, Group4_DlgMain.cpp
    and KrustyUI.cpp, as in Wave 19.
  - The NAME-ONLY units are AuralScape.cpp, EventManager.cpp,
    ManagedTextureGroup.cpp, ManagedTextureGroupNearMisses.cpp and
    Group2_ManagedTextureGroup.cpp. These are the units whose decorated names
    carry a renamed comparator or record.
- Batch B runs from 98ffa4f3 to 04cbe105 with map-B (1 entry). It
  has 139 units: 125 IDENTICAL, 9 IDENTICAL-COMDAT-ORDER and 5 NAME-ONLY.
  - The COMDAT-ORDER units are BikeRaceNearMisses.cpp,
    DlgProcsNearMisses.cpp, SelectGamePicProcsNearMisses.cpp,
    Group4_DlgMain.cpp, Group6_BikeRaceCreate.cpp, DlgProcs.cpp, KrustyUI.cpp,
    OptionProcs.cpp and SelectGamePicProcs.cpp.
  - The NAME-ONLY units are GUIManager.cpp, GameUi.cpp,
    GUIManagerNearMisses.cpp, GameUiNearMisses.cpp and
    Group2_UIDialogResource.cpp. These are the units that define or call
    `IsPaletteLocked`.
  - The first run gave one compile failure: Group3_KrustyBike.cpp still read
    `field_0x5c`. Those four uses were renamed, and the unit is IDENTICAL.

There was no DIFF, and no .bss or .data offset moved. No unit is NAME-ONLY
because of a field rename.

Negative controls:

- Without the map:
  - EventManager.obj gives DIFF: `??0UnknownEventEntry@@QAE@XZ` is in the
    old `.text` only.
  - GUIManager.obj gives DIFF in the same way through
    `?UnknownFunction4864f0@GUIManager@@QAEHXZ`.
- A live change gives DIFF in the bytes of
  `?CompareEmittersFarthestFirst@@YAHPBX0@Z`. The change is `return -1` to
  `return -2` in the new tree's CompareEmittersFarthestFirst
  (work/naming/w20/live.py). After the change is reverted, the unit is
  IDENTICAL again.

Fixture pins: only whole-object digests of NAME-ONLY units were repinned. In
each case the pinned section and relocation counts still held.

- ManagedTextureGroup.cpp in file_printf_objects.json and
  texture_history_timer_objects.json, and ManagedTextureGroupNearMisses.cpp
  in texture_history_timer_objects.json.
- GUIManager.cpp and GUIManagerNearMisses.cpp in
  uianim_connection_objects.json.
- GameUiNearMisses.cpp's aggregate view in edit_box_draw_aggregate.json,
  multi_state_draw_aggregate.json (the same digest) and
  edit_box_on_char_aggregate.json.

work/naming/repin.py made the `{relocations, sections, sha256}` pins. The
aggregate-view pins were changed by hand after the same check
(work/naming/w20/swap.py).

The renamed mangled names were also updated in the bindings of AuralScape,
EventManager, ManagedTextureGroup (source and sample), GUIManager and GameUi
(source and sample) and in tools/run_calibration.py.
docs/EVENTMANAGER.md and the layout snippet in
tests/test_stats_standings_draw.py follow the record names.

### Types

| Old name | New name | Tier | Evidence |
| --- | --- | --- | --- |
| `UnknownEventEntry` | `RacerResult` | 2 | EventManager's `results[11]`, one per racer; CopyFromRacer fills it from the racer's score and times. |
| `UnknownEventRanking` | `RacerTimeRank` | 2 | {racer, totalRunningTime}, sorted by CompareRankings. |
| `UnknownEventScore` | `RacerScoreRank` | 2 | {value, racer} (8 bytes), sorted by CompareScoresDescending. |
| `UnknownEventStanding` | `RacerStanding` | 2 | The points row that CompareStandings sorts. |
| `UnknownProCircuitRank` | `CircuitPointsRank` | 2 | {points, racer index}, sorted by CompareRankPoints (`0x004d8c20`). |
| `UnknownTrackNameEntry`, `UnknownTrackNameList` | `LobbyTrackEntry`, `LobbyTrackList` | 2 | ZoneReport's ConvertLobbyNumbering tables, which map lobby track numbers to the game's tracks. |
| `UnknownEventRacerMessage` | `RacerResultsMessage` | 3 | Application message 0x86: one racer's score and times, which EventManager copies into its results. |
| `UnknownPlayerStateMessage` | `PlayerStateMessage` | 3 | Application message type 2, the player state. |
| `UnknownLobbySettingsMessage` | `LobbySettingsMessage` | 3 | Application message 0xf: the 0xb4-byte lobby settings. |
| `UnknownPlayerLeftMessage` (InGameProcs.cpp) | `LeaveRaceMessage` | 3 | Application message 0x89: ExitDlg's leave button (ButLeft) sends it, and EventManager marks the sender done. This is not the system message 5 that Wave 19 named `DPMsgDestroyPlayerOrGroup`. |

### Comparators

These are qsort callbacks named after their decoded keys.

| Old name | New name | Tier | Key |
| --- | --- | --- | --- |
| `UnknownCompare402d40` | `CompareWaitingEmitters` | 3 | AuralScape (static): emitters with SoundEmitter+0x1a0 bit 2 set sort first. It sorts `waitingEmitters` in PickEmittersToPlay (`0x00403150`). |
| `UnknownCompare402d70` | `CompareEmittersFarthestFirst` | 2 | `listenerDistance` descending. |
| `UnknownCompare50ee70` | `CompareTexturesByPlannedLevel` | 2 | ManagedTextureGroup: `plannedLevel` descending, then address. |
| `UnknownCompare50eeb0` | `CompareTexturesByLevelSurplus` | 2 | `allocatedLevel - plannedLevel` descending, then address. |
| `UnknownCompare50ef00` | `CompareTexturesByUnhappiness` | 2 | Unhappiness() descending, then address descending (the "unhappy=%.2f" literal). |

### Methods

| Old name | New name | Tier | Evidence |
| --- | --- | --- | --- |
| `GUIManager::UnknownFunction4864f0` | `IsPaletteLocked` | 3 | 0x004864f0 returns +0x1f8; see `paletteLocked` below. |

### Fields

| Class | Offset | Old name | New name | Tier | Evidence |
| --- | --- | --- | --- | --- | --- |
| `UnknownEcoTrackGame::display` view | +0x1c0 | `capabilityBits` | `driverColorKeyCaps` | 2 | The DDCAPS `dwCKeyCaps` that PCVideoCard keeps as `driverColorKeyCaps`. EcoSystem tests bit 8. This is a member rename; the view was not merged. |
| `TrackGameMode` | +0xa20 | `field_0xa20` | `cdAudioOn` | 3 | The first word of the 0x2c-byte sound block. Its only reader is LeaveRaceForMenu (`0x0045e7b6`, `cmp [eax+0xf98]`), which calls StopCDAudio (`0x00521a40`) when it is clear. Its writers are the defaults (ResetSoundOptions `0x00522780`, 0), the options file (UiInfo) and the options dialog's block copies. The CDAudio class (`0x00431050..0x004312a8`) sends only open, set, status, stop and close. The same name is used for the sound block views `UnknownTrackGameModeOptionsA20::field_0x00` and `UnknownOptSoundSettings::field_0x00`. |
| `LightEmitter` | +0xa8 | `field_0xa8` | `lastColor` | 3 | SetColor (`0x0049e020`) returns early when the colour equals it. Only the constructor (`0x0049deb0`) writes it (0). |
| `GUIManager` | +0x1f8 | `field_0x1f8` | `paletteLocked` | 3 | When it is set, an 8-bit UIDialog skips its "PaletteFile" load (the code near `0x0046a920`), and UIDialog's destructor (`0x0046a070`) skips the palette restore. Only the constructor (`0x00485219`) writes it (0). |
| `KbBikeNetState` | +0x5c | `field_0x5c` | `fullState` | 2 | BikeRace sets the matching +0x5c to 1 on message 1 (full state) and to 0 on message 13 (delta). KrustyBike reads the lap and race fields only when it is set. |
| `UnknownAuralScapeReverbOwner` | +0xa4 | `field_0xa4` | `environment` | 2 | The owner is read as a Scene: Scene+0xa4 is the SceneEnvironment. |
| `UnknownAuralScapeReverbTable` | +0x410 | `field_0x410` | `enviroID` | 2 | SceneEnvironment's `enviroID[4]` (the "EnviroID" key), indexed by reverb zone. |
| `GridNodeExtra` | +0x10 | `field_0x10` | `builtTexture` | 2 | Grid1.h's view of the same record names it `builtTexture`. SelectTexture (`0x0047caa0`) stores the result of AcquireOwnedObject (`0x00505600`) there. |

Left as placeholders:

- `UnknownEventPlayerMessage`: an overlay of several message types. Naming it
  would be a merge.
- `UnknownBikeRaceNetMessage13`, `UnknownProCircuitResult` and
  `UnknownProCircuitSkinned`: views of `KbNetDelta`, `RacerResult` and
  `D3DIMSoultreeObject`. Merging them would add or remove declarations
  (Wave 18). The tests also name `UnknownProCircuitSkinned`.
- SoundEmitter+0x1a0 bit 2 (`field_0x1a0_bit2`): Create sets it when
  `!(flags & 2) || (flags & 4) || is2D`, and it is counted as `waiting3D`.
  Its meaning is not settled, so its comparator stays tier 3.
- `TrackGameViewOwner::field_0xac`: it has two roles, an int gate count and,
  in the bike race owner, a float.
- The Terrain/FollowCamera subject +0xbe8: the first dword of the TRN header.
  A value of 1 selects crash limit 0.766 and camera rate 0.25. What it means
  needs track data to settle.
- Sound block +0x0c/+0x24 and graphics block +0x0c/+0x14: nothing reads them.
- `RaceSound::field_0x40`: only ever 0.
- `Cube::field_0x10`: only 0 is written, and bit 1 is tested.
- `RacerStanding::field_0x04/0x08`: nothing sets them.
- `GridTerrain::field_0xc9c/0xca0`: union arms.
- `UIDialog::field_0xd4`: the slot 30 callback; its writer is not known.
- `FollowCamera::field_0x33c`: the tenth CAMERA value; its meaning is not
  known.
- From Wave 19:
  - `UnknownBltRect` and `UnknownBltEffects`: views.
  - `UnknownSceneKeyword`: the type of a static.
  - The `UnknownOpt*Settings` types: globals.
  - `UnknownFunction4ea7e0` and `Scene::field_0x88`: Scene's initialiser
    argument is not decoded.
  - `UIDialog::field_0x180`.
  - The union arms.

## Wave 21: score interface slots, the race view hook and generic parameters

Wave 21 names the placeholder methods that the retail call sites can name.
It also renames generic parameter and local names (`aN`, `valueN`) whose
role is clear from the body or from the decoded callers. Names only: no
declaration, typedef, forward declaration or member was added or removed,
and no static or type of a static was renamed. No line count changed, so the
`__LINE__` values that DebugAlloc.h's macros take did not move.
`git grep -ohE 'UnknownVirtualSlot[0-9]+|UnknownMethod[0-9]+|Method_[0-9A-Fa-f]+|\bFn_[0-9A-Fa-f]+' -- src samples`
fell from 283 uses to 270. Of those, `UnknownMethodN` fell from 17 to 9.
work/naming/w21/generic.py strips comments and strings, then counts the
generic names in tracked src, samples and fixture sources:

- `aN` fell from 110 to 18 (from 104 to 12 outside the files this wave
  could not touch);
- `valueN` fell from 22 to 0;
- `argN` stayed at 6;
- `temp` stayed at 0.

Method:

- work/naming/w16_overrides.py and w16_callers.py resolve each slot's
  overrides and retail call sites.
- The method renames are work/naming/w21/editA.py, applied with
  work/naming/sub.py.
- The parameter and local renames are work/naming/w21/specB.py, applied with
  work/naming/w21/pren.py. pren.py renames whole words within a line range
  and skips member accesses. It stops if an old name is missing from the
  range or if the new name is already used there, so a parameter cannot
  capture a member or global that the body reads. The one allowed existing
  use is `g_TrackGame->textureManager` in Scene::Load, which is a member
  access.

Result: two proofs, each run over every unit whose closure reaches a changed
file.

- Batch A runs from d39d250a to 70698a05 with map-A (1 entry,
  `?UnknownVirtualSlot32@BaseQuarryEvent@@` to
  `?OnRaceViewAttached@BaseQuarryEvent@@`). It has 21 units: 19 IDENTICAL
  and 2 NAME-ONLY.
  - The NAME-ONLY units are QuarryStuntEvent.cpp and NationalRace.cpp. These
    are the units that emit BaseQuarryEvent's vtable or NationalRace's
    vtable, which holds the slot.
  - MSZoneInterface.cpp is IDENTICAL: the score interface is never defined,
    and its calls go through the vtable.
- Batch B runs from 70698a05 to 3c92fa73 with an empty map. It has 86 units:
  85 IDENTICAL and 1 IDENTICAL-COMDAT-ORDER (Group6_BikeRaceCreate.cpp, as in
  Wave 20; a rerun gives the same verdict). No mangled name changed.

There was no DIFF and no compile failure, and no .bss or .data offset moved.

Negative controls:

- Without the map, QuarryStuntEvent.obj and NationalRace.obj give DIFF. Their
  vtable `.rdata` (`??_7BaseQuarryEvent@@6B@`, `??_7NationalRace@@6B@`)
  differs in its relocations.
- Live changes give DIFF in the changed function's `.text` bytes. After each
  change is reverted, the new tree matches the work tree again.
  - In the new tree's SubmitScores, `? 4 : 16` became `? 4 : 17`
    (work/naming/w21/live.py). This gives DIFF in
    `?SubmitScores@MSZoneInterface@@QAEHIPAX0@Z`.
  - In DistanceToStripEdge, `mode == 17` became `mode == 18` before the
    second edge (work/naming/w21/liveB.py). This gives DIFF in
    `?DistanceToStripEdge@Track@@QAEHUTrackVec3@@PAUTrackSegment@@PAMH@Z`.

Fixture pins:

- QuarryStuntEvent.cpp's whole-object digest is pinned in two fixtures,
  eco_light_objects.json and race_detail_objects.json. Because it occurs
  twice, repin.py's one-occurrence check does not apply, so the digest was
  replaced by hand in both. The pinned section and relocation counts
  (86, 393) still held.
- NationalRace.cpp has no whole-object pin. Batch B needed no repin.
- The renamed mangled name was also updated in
  QuarryStuntEvent.bindings.json and race_event_partials.json.
- tools/run_calibration.py does not spell any renamed name.

### Methods

| Old name | New name | Tier | Evidence |
| --- | --- | --- | --- |
| `UnknownZoneScore::UnknownMethod3` | `Initialize` | 3 | SubmitScores (`0x004aa670`) calls it first, with the application GUID, the player count, 0, 0 and 0x1000. The interface's IID (3FAF0AFD-B48B-11D2-8A51-00C04F8EF4E9) is not public, so the name comes from the arguments. |
| `UnknownZoneScore::UnknownMethod4` | `SetPlayerResult` | 3 | It is called once per player with the index, name and score, 0, and the place flags (2 first, 4 placed, 16 no position). |
| `UnknownZoneScore::UnknownMethod6` | `SetResultData` | 3 | Its arguments are a block and its size: ZoneReport passes `&raceStatus, sizeof(raceStatus)`. |
| `UnknownZoneScore::UnknownMethod15` | `SendThroughLobby` | 3 | It is called last, with the DirectPlayLobby3A. |
| `BaseQuarryEvent::UnknownVirtualSlot32` | `OnRaceViewAttached` | 3 | The quarry loader calls slot 32 (`call [edx+0x80]`, `0x004dff03`) with the race view right after AppendChild of the view at +0x34. The base body is the shared empty stub (`0x00464e80`), and no class overrides it. |

### Parameters and locals

| Function | Old | New | Tier | Evidence |
| --- | --- | --- | --- | --- |
| `Scene::Load` (and the ReadStaticModels/ReadAnimations declarations and the sample bodies) | `a3`, `a4`, `a5` (`a2`-`a4`, `a3`-`a4`) | `particleManager`, `textureManager`, `modelTextures` | 2 | Group3_QuarryLoad passes `(int)particleManager`, the TrackGame texture manager and `&modelTextures`. ReadStaticModels hands the first to the particle emitters and the last to the model loads. |
| `Scene::Load` | local `camera` | `flareTextureManager` | 2 | It is the texture manager when the light has a lens flare, and 0 otherwise. It is LightEmitter::Create's ninth argument. |
| SceneManagerNearMisses.cpp's LoadPhysicsModel view | `a4`, `a8`, `a10`, `a14`-`a17`, `a21`, `a22` | `options`, `terrainProbe`, `weight`, `fixedStep`, `maxSteps`, `drag`, `elasticity`, `probeMask`, `createCollision` | 3 | The names SoultreePhysicsObject.h gives the same parameters. `a9` stays: the scene passes the particle manager there, which does not fit the canonical `linkedObject`. |
| SceneManagerNearMisses.cpp's AddCollisionPoint view | `a4`, `a7` | `threshold`, `surfaceOwner` | 3 | CollisionPoint.h's names. |
| SceneManager.h's character Load and CarProcedural Create views | `a4`, `a6` / `a4`, `a5` | `loadInfo`, `unused` / `options`, `ground` | 3 | CollisionCharacter.h's and CarProcedural.h's names. |
| `FollowCamera::SetupFollowCamera` and the Vehicle, Bike and KrustyBike camera setups | `value`, `rate294`, `rate298`, `value228`, `value2d0`, `value2e8` | `context`, `fieldOfViewEaseRate`, `distanceEaseRate`, `initialLookAhead`, `initialFollowHeight`, `initialYawOffset` | 2 | `0x00463140` passes the context to SetRenderContext. It builds the field-of-view and distance eases from the two rates, then stores the look-ahead distance, the default and current follow height, and the yaw offset. The wrappers forward the arguments unchanged. |
| `BikeCamera::SteerToKeepTargetOnScreen` | locals `value10`, `value14` | `raiseRate`, `lowerRate` | 2 | SetTableEntry's raise-rate and lower-rate arguments. |
| `DrawableGridNode` constructor, `ReadNode`, `CreateChildNode`, `DrawableGridNodeSharedTextures` | `a1` | `loadChildren` | 2 | ReadNode creates the children only when it is set, and passes it down. |
| `TestVisibility`, `WalkVisibilityQuadrants` (also its sample), `UpdateLevelOfDetail` | `a1` | `screenMatrix` | 2 | It is TestBox's matrix. Terrain passes `visibilityCamera->screenMatrix`. |
| `GridVisibilityClipper::TestBox` | `a6` | `unused` | 2 | The body ignores it. This is VisibilityClipper.h's name. |
| `RadarOverlay::LoadMap` | `a3` | `camera` | 2 | It is stored as `radarCamera`. QuarryStuntEvent passes `raceCamera`. |
| `RadarOverlay::TakeRacers` | `a2` | `chat` | 3 | It is unused, and the callers pass the chat overlay. |
| QuarryEvent.h's `VisualCue::LoadVisualCue` view | `value`, `a9`, `a10`, `a11` | `options`, `size`, `screenXFraction`, `screenYFraction` | 3 | Krusty3DObjects.h's names. |
| `FontTexture::Load` | `a5` | `textureContext` | 2 | It is cast to TextureContext* for TerrainLoadTexture. |
| `Track::DistanceToStripEdge` | locals `a`, `d`, `a2`, `d2` | `startA`, `directionA`, `startB`, `directionB` | 2 | The chosen edge's start and direction, and the second edge's start and direction (mode 17). |

Left as placeholders:

- Not touched because another agent owns the file:
  - Out of bounds for this wave: src/krusty2/ and samples/physics/.
  - GameObject slot 19: the only override is Terrain (src/krusty2), and the
    only caller is Game slot 12, which nothing calls. Game slot 12 itself
    stays a placeholder too.
  - PhysicsBody slots 31 and 32, Vehicle slots 78 and 79,
    SoultreePhysicsBaseObject slot 26 and its views, KrustyBikeTypes slots
    1-5 and 12, Bike slot 43, ConstraintMethodCollisionModel slot 2,
    CollisionPoint/SoultreeContact slot 1 and VehicleContactPoint slot 1.
  - `Method_00478FE0` (declared in VehicleCore.h) and `Method_00528EB0`.
  - The `Fn_*` names, which appear only in krusty2 targets.json notes and
    samples/physics.
  - `Method_00527A20`, `Method_00529450` and `Method_005293E0`, which another
    agent is matching.
  - The generic names left in src/krusty2 (CollisionCharacter,
    D3DIMSoultreeMotnctrl, Vehicle, VehicleCore).
  - `D3DIMSoultreeObject::GetPart`'s `a6`, which sits next to the SoulTree
    work (D3DIMSoulTreeNearMisses.cpp).
- Not enough evidence for a name:
  - GameObject slot 9: nothing overrides it, and its only caller is its own
    child walk.
  - BaseQuarryEvent slots 28 and 34: shared stubs, and no caller passes
    three arguments to 34.
  - UIDialog slot 30.
  - BackgroundImage slot 27: a shared `return 1` stub with no override.
  - DrawableGridNode slot 5: empty in the base and the override. ReadNode
    calls it with the stream.
  - ConnectionInfoType slot 1: a `ret` stub.
  - UnknownGameObjectOwnerPart slots 1-4: the object is not identified.
  - The score interface's `UnknownMethod5` and `UnknownMethod7` to
    `UnknownMethod14`: nothing calls them.
- Destructor slots keep their placeholders:
  - slot 0 in the views;
  - slot 1 in the keyboard and joystick InputDevice views (InputDevice slot 1
    is `~InputDevice`);
  - UnknownEcoTrackGame slot 12, which is Game's placeholder.
- Generic parameters with no use that decides a role:
  - LightEmitter::Create's `a8` to `a10`: the body ignores them. The scene
    passes 0, the flare texture manager and the flare texture name.
  - TrackOverlay CastSegment's `a4` to `a6`: the canonical names are tier 3.
  - The `a5` of SceneManager.h's character Load view.
  - The `a9` of the LoadPhysicsModel view (see above).
  - GearRatios' `arg6`, which is stored at +0x1d8 and never read.
  - The D3DIMSoulTreeNearMisses.cpp `a7`.

## Wave 22: krusty2 slots, the vehicle helpers and the motion parameters

Wave 22 takes up Wave 21's list of names it could not touch in src/krusty2
and samples/physics. It names the slots and helpers whose bodies, overrides
or callers fix a role, and renames the generic parameters whose use decides
one. Names only: no declaration, typedef, forward declaration or member was
added or removed, and no static or type of a static was renamed. No line
count changed, so the `__LINE__` values that DebugAlloc.h's macros take did
not move.
`git grep -ohE 'UnknownVirtualSlot[0-9]+|UnknownMethod[0-9]+|Method_[0-9A-Fa-f]+|\bFn_[0-9A-Fa-f]+' -- src samples`
fell from 270 uses to 146. In src/krusty2 and samples/physics alone it fell
from 112 to 45. No `Method_` name is left, and the only `Fn_` name left is
the historical one in samples/physics/hierarchy/MIGRATION.md.
work/naming/w21/generic.py counts `aN` falling from 18 to 8 and `argN` from
6 to 3; `valueN` and `temp` stay at 0.

Method:

- work/naming/w16_overrides.py resolves each slot's overrides.
  work/naming/w22_callers.py is w16_callers.py with any base register, and
  small displacements (`+ 8]`) were grepped in work/naming/retail.asm.
- The unique identifiers (`UnknownVirtualSlot19`, `12`, `26`, `43`,
  `CollisionPointVirtualSlot1` and the `Method_` names) were renamed across
  the tracked tree with work/naming/rename.py and work/naming/w22/renameA.json.
  The name of a slot that other views share (`UnknownVirtualSlot0` and `1`)
  was changed only at its own declaration and call sites, with
  work/naming/w22/editB.py and work/naming/sub.py. The stale
  "UnknownVirtualSlot12" comment in D3DIMSoulTreeNearMisses.cpp, which
  describes PrepareGeometry, was corrected first (work/naming/w22/pre.py).
- The parameter renames are work/naming/w22/specB.py, applied with
  work/naming/w21/pren.py.

Result: one proof over every unit whose closure reaches a changed source
(461 units, run in two halves), from 1a8c08c9 to the renamed tree, with
work/naming/w22/map.json (14 entries: the renamed slots and methods of
GameObject, Game, ShadowCamera, Terrain, SoultreePhysicsBaseObject,
BikeBodyObj, CollisionPoint, Tire, SoultreeContact, VehicleContactPoint,
VehicleTicker, KbSink and Vehicle). Parameter names are not mangled, so the
map needs no entry for them.

- 351 IDENTICAL, 104 NAME-ONLY and 5 NAME-ONLY+COMDAT-ORDER.
  - The COMDAT-ORDER units are DlgProcsNearMisses.cpp, Group4_DlgMain.cpp,
    KrustyUI.cpp and ProCircuitProcs.cpp, where VC6 emits the dialog RTTI
    COMDATs in another order once TrackRecordDlg's view slot is renamed, and
    SoulTreePhysics.cpp, where the vtordisp thunks of SoultreePhysicsObject
    and SoultreePhysicsBaseObject change places. Every section, relocation
    and symbol is equal after the map; only the COMDAT order differs, as in
    Waves 4 and 14.
- tests/fixtures/bvh_vector_compare.cpp fails to compile in the old tree as
  well: it includes the header its test generates (Wave 14).

There was no DIFF, no other compile failure, and no .bss or .data offset
moved. Every edited file keeps its line count.

Negative controls:

- Without the map, GameObject.obj, Tire.obj and Vehicle.obj give DIFF: the
  vtable `.rdata` of GameObject and CollisionPoint, and SimulateFrame's
  `.text`, differ in their relocations.
- A live change gives DIFF in the changed function's `.text` bytes. In the
  new tree's ApplyShockDamping, `lastStepTime` became `stepTime`
  (work/naming/w22/live.py). This gives DIFF in
  `?ApplyShockDamping@Vehicle@@QAEXPAM@Z`. After the change is reverted,
  Vehicle.cpp is NAME-ONLY again and the new tree matches the work tree.

Fixture pins: only whole-object digests of NAME-ONLY units were repinned,
and every pinned section and relocation count still held.

- work/naming/w22/repin_all.py ran work/naming/repin.py, repin2.py and
  w16_repin3.py on the 41 failing modules (log: work/naming/w22/repin.log).
  37 passed. w16_repin3.py does not check the verdict, so its units were
  checked against the proof by hand: FollowCameraNearMisses.cpp,
  FollowCameraConstructor.cpp, Terrain.cpp and VisibilityQuadTree.cpp are
  all NAME-ONLY. The "FollowCameraControls.cpp" keys in
  follow_camera_constructor_objects.json and the "?" key in
  follow_camera_load_file_objects.json name the FollowCameraNearMisses.cpp
  aggregate (66, 337) and the FollowCameraConstructor.cpp candidate
  (31, 139).
- The other four modules were repinned by hand:
  - test_collision_point_storage (CollisionPoint.cpp, 32, 128) and
    test_race_sound_refill (the RaceSoundNearMisses.cpp aggregate, 107, 580):
    work/naming/w22/repin_manual.py.
  - test_rider_motion_start_connections pins Bike.cpp (243, 1105),
    KrustyBike.cpp (248, 1082) and KrustyBikeNearMisses.cpp (271, 1443) in
    the test itself. test_chat_name_tag_draw's "canonical" digest is
    TrackOverlay.cpp (215, 920), pinned in both chat_name_tag_objects.json
    and stats_standings_objects.json: work/naming/w22/repin_bike.py.
- test_soultree_physics_methods lists the slots renamed away from
  `UnknownVirtualSlotN`. `'OnPhysicsStep': 26` was added to it
  (work/naming/w22/testfix.py).
- The renamed mangled names were also updated in tools/run_calibration.py
  (the ShadowCamera, Game and GameObject OnInputEvent rows),
  tools/emu_cases/vehicle_wheel_impulse.py, test_vector_add_assign.py,
  test_vector_constructors.py, the bindings, the targets.json files and
  docs/PHYSICS_VALIDATION.md, VC6_OPERAND_ORDER.md and VECTOR_ADD_ASSIGN.md.

### Methods

| Old name | New name | Tier | Evidence |
| --- | --- | --- | --- |
| `GameObject::UnknownVirtualSlot19` (with the ShadowCamera and Terrain overrides and the Bike, Vehicle, EcoSystem, TrackGame and TrackRecordDlg views) | `OnInputEvent` | 3 | The base body (`0x00469530`) runs only when childFlags has bit 0x100. It passes its int to each enabled child that is not pending release, and returns 1 at the first child that returns nonzero, like OnChar and OnKeyDown, which follow it. ShadowCamera (`0x004da550`) and Terrain (`0x00507920`) override it with `return 0`, as they do OnControlRelease. Its only caller is Game slot 12. |
| `Game::UnknownVirtualSlot12` (with the UnknownEcoTrackGame and KbGame views) | `OnInputEvent` | 3 | `0x004688e0` returns `rootObject->OnInputEvent(value) != 0`. It sits before Game's OnControlRelease and OnControlPress. Nothing in retail calls it. The `call [eax+0x4c]` sites in LeaveMenusForRace and KrustyUI::OpenMenu are g_TrackGame's slot 19, not this one. |
| `SoultreePhysicsBaseObject::UnknownVirtualSlot26` (with the Tire, FollowRacer and SceneManager views) | `OnPhysicsStep` | 3 | The entry is the shared empty stub (`0x00464e90`) and nothing overrides it. It is called once in each step of the three step loops: RunSteps (after IntegrateMotion, before the position update), Vehicle::SimulateFrame (before AdvancePosition) and KrustyBike::SimulateAIFrame. |
| `CollisionPoint::CollisionPointVirtualSlot1` (and `Tire`) | `ComputeFrictionForce` | 2 | The base (`0x0043b240`) computes frictionForce (+0x78) from the normal load. The Tire override (`0x00515b50`) computes an anisotropic friction. Both retail callers add frictionForce right after the call. |
| `SoultreeContact::UnknownVirtualSlot1`, `VehicleContactPoint::UnknownVirtualSlot1` | `ComputeFrictionForce` | 2 | Both are views of CollisionPoint. RunSteps' contact loop and ApplyWheelDriveAndFriction call slot 1 and then read frictionForce. |
| `BikeBodyObj::UnknownVirtualSlot43` (Bike.cpp view) | `GetWorldAngularVelocity` | 2 | The view's fields match PhysicsBody (angularVelocity at +0x16c, invMass at +0x17c, invInertia at +0x1d0), and PhysicsBody slot 43 (`0x004cbed0`) is GetWorldAngularVelocity. The call fills a Vec3 out-parameter, which the collision response uses as the other body's angular velocity. |
| `KbSink::UnknownVirtualSlot0`, `UnknownVirtualSlot1` | `Reset`, `ClearForces` | 2 | KrustyBike::InterpolateNetworkState (`0x00493660`) calls slot 0 of frontWheel+0x2b0 and rearWheel+0x2ac (`0x00494890`-`0x004948a8`), which hold the InlineShock and the RotatingShock. Shock's vtable has two entries: Reset (`0x004f9ba0`) and ClearForces (`0x004f9c70`). |
| `VehicleTicker::UnknownVirtualSlot0` | `Reset` | 2 | Bike's loader passes the RotatingShock (constructor `0x004fa700`) to AddLateTicker and the InlineShock (`0x004f9ee0`) to AddEarlyTicker, so RunTickers calls Shock::Reset. |
| `Vehicle::Method_00478FE0` | `DetectCrash` | 3 | The shared `xor eax,eax; ret` stub. CheckCrash (`0x00526830`) calls it directly and stores the result as its crash state. |
| `Vehicle::Method_00527A20` | `ApplyWheelDriveAndFriction` | 2 | It applies each wheel's drive share (Tire's per-wheel drive step `0x00514170`), then the contact friction impulses limited by frictionMagnitude. |
| `Vehicle::Method_005293E0` | `ApplyShockDamping` | 2 | It calls Shock::ApplyDamping for each wheel whose shock is engaged. |
| `Vehicle::Method_00529450` | `ApplyShockForces` | 2 | It projects each engaged wheel's spring force on the ground normal and adds the force, the torque and the displacement impulse. |

The `Fn_` and `Method_00528EB0` names in the targets.json notes became the
bound names of the same addresses: FindObjectPlacement (`0x004b0df0`),
CastSegment (`0x00506e90`), RemoveNodesFromQuadtree (`0x005047f0`),
ResolveContactImpulse (`0x00500220`) and PlaceWheels (`0x00528eb0`).

### Parameters

| Function | Old | New | Tier | Evidence |
| --- | --- | --- | --- | --- |
| `Vehicle::LoadVehicle` | `arg18` | `suspensionScale` | 2 | It is stored in suspensionWeightScale, which slot 0 multiplies into the suspension mass. Bike passes 0.9f. |
| `D3DIMSoultreeCharacter::LoadCharacter` | `a5` | `useOptions` | 2 | It is stored at character +0x20c (d3d_field_0x1a4+0x68). BuildModel (`0x00445c17`) passes it to LoadModel as useOptions. Every caller passes 1. |
| `CollisionCharacter::Load`, SceneManager.h's character Load view | `a6`, `a5` | `useOptions` | 2 | Both forward it to LoadCharacter unchanged. |
| `D3DIMSoultreeObject::GetPart` | `a6` | `unused` | 2 | The body ignores it, and the callers pass 0. |
| The GetPart views in Group5_ShadowBridges.cpp and ProjectedShadow.h | `c`, `d` | `unused`, `lod` | 2 | D3DIMSoultreeObject::GetPart's names. |
| D3DIMSoulTreeNearMisses.cpp's TestBox view | `a7` | `unused` | 2 | VisibilityClipper.h's name. |

Left as placeholders:

- PhysicsBody slots 31 and 32: they set +0x180 (an int) and +0x184 (a Vec3),
  which nothing reads. No call site uses vtable offset 0x7c or 0x80, and
  nothing overrides them.
- Vehicle slots 78 and 79 (`0x00492220`, `0x00492270`): nothing calls or
  overrides them. 78 reads joystick button index 3 or DIK_S, and 79 reads
  button index 4 or DIK_X. The booklet's control list does not pair these
  keys and buttons with one action, so no name is defensible.
- ConstraintMethodCollisionModel slot 2: the shared bare `ret`
  (`0x0044d710`), added on the QuadTreeObject primary vtable. No caller was
  found.
- KbSink slots 2-5: they lie past the end of Shock's two-entry vtable, and
  nothing calls them.
- GameObject slot 9 (see Wave 21).
- Destructor slots keep their placeholders: slot 0 in the views
  (VisibilityQuadTree.h, CollisionObject.cpp, VehicleInputMap,
  VehicleContactPoint, the Bike and Vehicle GameObject views, KbGame) and
  slot 1 in the KbPad and Vehicle.h InputDevice views (`~InputDevice`).
- Generic parameters with no use that decides a role:
  - GearRatios' `arg6` (and LoadVehicle's `gearArg`): stored at gearbox
    +0x1d8 and never read. Bike passes 0.18f.
  - LightEmitter::Create's `a8` to `a10`, TrackOverlay CastSegment's `a4`
    to `a6` and the LoadPhysicsModel view's `a9` (see Wave 21).
- RunTickers could be renamed after Shock::Reset; it was left for a later
  wave.

## Wave 23: GameObject slot 9, the render target's display view and message fields

Wave 23 takes up the slots and fields that Waves 21 and 22 left as
placeholders, and names those whose body, caller or the class they view
fixes a role. Names only: no declaration, typedef, forward declaration or
member was added or removed, and no static or type of a static was renamed.
Every edited file keeps its line count, so the `__LINE__` values that
DebugAlloc.h's macros take did not move.
`git grep -ohE 'UnknownVirtualSlot[0-9]+|UnknownMethod[0-9]+|Method_[0-9A-Fa-f]+|\bFn_[0-9A-Fa-f]+' -- src samples`
fell from 146 uses to 85. `\bfield_0x[0-9a-fA-F]+` fell from 1920 uses to
1899, and `\bUnknown[A-Za-z0-9_]*` from 2768 uses (370 names) to 2704 (366
names). The `flagN` bit-fields of the Kb bike messages are all named.

Method:

- work/naming/w16_overrides.py and work/naming/w22_callers.py resolved the
  overrides and callers, as in Wave 22.
- The unique identifiers (`UnknownVirtualSlot9`, `UnknownVirtualSlot27` and
  `UnknownGameObjectOwnerPart`) were renamed across the tracked tree with
  work/naming/rename.py and work/naming/w23/renameA.json, and the message
  bits with work/naming/w23/renameB.json. Shared names (DrawableGridNode
  slot 5, the display view's slots, the `field_0xNN` names and the
  UIEditBox parameters) were changed only at their own declaration and uses,
  with work/naming/w23/editB.py and work/naming/sub.py.
  work/naming/w23/align.py realigned the bit-field comments (whitespace).

Result: one proof over every unit whose closure reaches a changed source
(463 units), from ae01415a to the renamed tree, with work/naming/w23/map.json
(8 entries: GameObject slot 9, BackgroundImage slot 27, DrawableGridNode
slot 5, and the display view's type and slots 1-4). Field and parameter names
are not mangled into these objects, so the map needs no entry for them.

- 357 IDENTICAL, 95 NAME-ONLY and 10 NAME-ONLY+COMDAT-ORDER.
  - The COMDAT-ORDER units are DlgProcs.cpp, DlgProcsNearMisses.cpp,
    Group4_DlgMain.cpp, KrustyUI.cpp, OptionProcs.cpp, ProCircuitProcs.cpp,
    ProCircuitProcsNearMisses.cpp, SelectGamePicProcs.cpp,
    SelectGamePicProcsNearMisses.cpp and VisibilityQuadTree.cpp. VC6 emits
    their RTTI and vtable COMDATs in another order once the GameObject views'
    slot 9 is renamed. Every section, relocation and symbol is equal after
    the map; only the COMDAT order differs, as in Waves 4, 14 and 22.
- tests/fixtures/bvh_vector_compare.cpp fails to compile in the old tree as
  well: it includes the header its test generates (Wave 14).

There was no DIFF, no other compile failure, and no .bss or .data offset
moved.

Negative controls:

- Without the map, GameObject.obj and Griddraw.obj give DIFF in the
  relocations of the GameObject and DrawableGridNode vtables, and
  BackgroundImageHooks.obj gives DIFF because the old object's slot 27
  `.text` has no counterpart.
- A live change gives DIFF in the changed function's `.text` bytes. In the
  new tree's Tire::HandleContact, the car branch read `physicsVelocity`
  instead of `carPathVelocity` (work/naming/w23/live.py). This gives DIFF
  in `?HandleContact@Tire@@QAEXHHPAVCollisionObject@@@Z`. After the new
  tree's Tire.cpp is restored, Tire.cpp is NAME-ONLY again and the new tree
  matches the work tree.

Fixture pins: only whole-object digests of NAME-ONLY units were repinned,
and every pinned section and relocation count still held.

- 149 test modules reach a changed file, a renamed name or a NAME-ONLY unit
  (work/naming/w23/pick_tests.py). 31 failed on digests only.
- work/naming/w22/repin_all.py ran work/naming/repin.py, repin2.py and
  w16_repin3.py on them (logs: work/naming/w23/repin1.log and repin2.log).
  29 passed: 22 through repin.py, 3 through repin2.py and 4 through
  w16_repin3.py. w16_repin3.py does not check the verdict, so its units were
  checked against the proof by hand: VisibilityQuadTree.cpp,
  FollowCameraNearMisses.cpp, Terrain.cpp and FollowCameraConstructor.cpp
  are all NAME-ONLY (or NAME-ONLY+COMDAT-ORDER). As in Wave 22, the
  "FollowCameraControls.cpp" keys of follow_camera_constructor_objects.json
  and the "?" key of follow_camera_load_file_objects.json name the
  FollowCameraNearMisses.cpp aggregate and the FollowCameraConstructor.cpp
  candidate.
- The other two were repinned by hand with work/naming/w23/repin_manual.py:
  - test_chat_name_tag_draw's "canonical" digest, TrackOverlay.cpp
    (215, 920), pinned in both chat_name_tag_objects.json and
    stats_standings_objects.json; and test_race_sound_refill's
    RaceSoundNearMisses.cpp aggregate (107, 580).
  - test_rider_motion_start_connections pins Bike.cpp (243, 1105),
    KrustyBike.cpp (248, 1082) and KrustyBikeNearMisses.cpp (271, 1443) in
    the test itself.
- The 31 modules pass after the repins. So do the seven modules whose
  fixtures share a repinned digest (test_multi_state_draw,
  test_race_detail_connections, test_stats_standings_draw,
  test_vegetation_collision_object, test_vegetation_connections,
  test_vehicle_input_subtype and test_visibility_connections).

### Methods and types

| Old name | New name | Tier | Evidence |
| --- | --- | --- | --- |
| `GameObject::UnknownVirtualSlot9` (with the Bike, Vehicle, EcoSystem, TrackGame and TrackRecordDlg views) | `PreTick` | 3 | The body (`0x00469430`) is Tick's (`0x004693d0`) child walk without the display hook: it runs only when childFlags has bit 0x01, the bit below Tick's 0x02, and passes its argument to each enabled child that is not paused or pending release. It is the slot just before Tick. Nothing overrides or calls it, so the name rests on the shape alone. |
| `UnknownGameObjectOwnerPart` (GameObject.cpp) | `RenderTargetDisplayView` | 2 | Tick reads it from renderContext +0x04, which is RenderTarget's targetDisplay, a VideoCard. It tests +0x70 bit 2, VideoCard's lastFlipFailed. |
| Its `UnknownVirtualSlot1` to `UnknownVirtualSlot4` | `ReleaseResources`, `SetDisplayMode`, `Present`, `RetryFlip` | 2 | VideoCard's slots 1-4. Tick calls RetryFlip(0) when lastFlipFailed is set. |
| `BackgroundImage::UnknownVirtualSlot27` | `DrawBackground` | 3 | The shared `return 1` body (`0x004627f0`), slot 27 of the primary vtable `0x005506d8`, with no override. RenderPre3D (`0x00403dc0`) calls it with the off-screen copy and then sets copyValid, or with the render surface for a full restore, so it is where a background is drawn into a surface. |
| `DrawableGridNode::UnknownVirtualSlot5` | `ReadExtraData` | 3 | The shared empty body (`0x004806f0`), empty in the override too. The node reader calls it last, with the stream it read the node from. |

### Fields and parameters

| Owner | Old | New | Tier | Evidence |
| --- | --- | --- | --- | --- |
| `KbBikeMessage`, `KbBikeDeltaMessage` | `flag4` to `flag9` | `spawnProtected`, `finished`, `onTrack`, `throttleOn`, `driveDisengaged`, `trickPoseHeld` | 2 | The message builder copies them from IsSpawnProtected() (slot 51), hasFinished (+0x7a4) and the KrustyBike members onTrack (+0x78c), throttleOn (+0x478), driveDisengaged (+0x479) and trickPoseHeld (+0x153c). |
| `KbBikeDeltaMessage` | `field_0x11`, `field_0x16` | `lapGate`, `loaderKind` | 2 | KbBikeMessage's names for the same values: the delta builder stores lapGate (lapCount in event types 2 and 3) and loaderKind (KrustyBike +0x11c0). |
| `LobbySettingsMessage` | `field_0x2d78` (3 bits) | `series` | 2 | The sender stores `mode.raceSettings.series` in it, and the receiver stores it back there. |
| `TireContactOwner` (Tire.cpp) | `field_0x40`, `field_0x64`, `field_0x224` | `carPathVelocity`, `physicsVelocity`, `characterVelocity` | 2 | HandleContact picks one by the owner's tag: 0x69 (CarProcedural, whose +0x40 is pathVelocity), 0x64/0x68 (physics bodies, velocity at +0x64) and 0x2711 (CollisionCharacter, nodeVelocity at +0x224). |
| `UIEditBox::UIEditBox` | `a`, `b` | `keyClickSound`, `fullTextSound` | 2 | They are stored at +0x204 and +0x208, whose union members are clickSound (the key click) and fullSound (the "full" sound). |

DlgProcsNearMisses.cpp's comment now calls the podium camera slot by its
Wave 21 name, `LookAt(podiumFocus)`. docs/STARTUP_DEFAULT_METHODS.md,
tools/run_calibration.py, test_startup_default_methods.py and the
particle_emitter_objects.json symbols use the new mangled names.

Left as placeholders:

- PhysicsBody slots 31 and 32: still no caller and no reader of +0x180 or
  +0x184 (see Wave 22).
- Vehicle slots 78 and 79 (`0x00492220`, `0x00492270`): no caller or
  override, and the booklet does not pair joystick button 3 with DIK_S or
  button 4 with DIK_X (see Wave 22).
- UIDialog slot 30: the callback at +0xd4 it uses is set only by the
  constructor (to 0), and no caller was found.
- BaseQuarryEvent slots 28 and 34, ConstraintMethodCollisionModel slot 2,
  KbSink slots 2-5, ConnectionInfoType slot 1 and the score interface's
  `UnknownMethod5` and `UnknownMethod7` to `UnknownMethod14`: shared stubs
  or no caller.
- RenderTargetDisplayView slot 0 and the other destructor slots (see Wave 22).
- Fields whose uses do not decide a role: RaceSound `field_0x40`,
  TrackGame `field_0xac` and `field_0x25e8`, VideoCard `field_0x14[16]`
  (read, but no writer found), PCTextureMap `field_0x38`, and
  KrustyBikeCore's write-only fields.
- The generic parameters listed in Wave 22.

## Wave 24: fields named by the SDK, a retail string and the values they copy

Wave 24 takes up the `field_0xNN` placeholders and names those whose value
goes straight to a DirectX or DirectInput parameter, is printed with a retail
string, or is copied to or from a member that already has a name. It also
renames the generic parameters and one struct field whose use fixes the
meaning. Names only: no declaration, typedef, forward declaration or member
was added or removed, no array was split, and no static or type of a static
was renamed. Every edited file keeps its line count, so the `__LINE__` values
that DebugAlloc.h's macros take did not move.
`git grep -ohE '\bfield_0x[0-9a-fA-F]+' -- src samples` fell from 1899 uses
to 1786. `\bUnknown[A-Za-z0-9_]*` stays at 2704 uses (366 names), and the
placeholder method names (`UnknownVirtualSlotN`, `UnknownMethodN`,
`Method_`, `Fn_`) stay at 85: this wave renamed no type or method.

Method:

- work/naming/w24_survey.py lists every `field_0xNN` declaration with its
  struct (1001 declarations: 629 padding arrays and 372 scalars), and
  work/naming/classreads.py (layouts in work/naming/cv/layouts.json) maps
  the retail reads and writes of each placeholder (work/naming/w24_reads.txt).
  Fields with only proximity, only constructor stores, or no reader were
  left alone.
- The retail string resources are in LANG.DLL and UILANG.DLL (mcm2.exe has
  no string table). `LoadResourceString(0x13ad)` is "Gate: ".
- The renames were applied at their own declaration and uses with
  work/naming/w24/editA.py, editB.py and editC.py (work/naming/sub.py,
  count-checked, line endings kept).

Result: one proof over every unit whose closure reaches a changed source
(313 units), from afc56028 to the renamed tree, with an empty map
(work/naming/w24/map.json): no mangled name changed.

- 305 IDENTICAL and 8 IDENTICAL-COMDAT-ORDER (BikeRaceNearMisses.cpp,
  DlgProcsNearMisses.cpp, ProCircuitProcsNearMisses.cpp, Group4_DlgMain.cpp,
  Group4_DlgPCBunny.cpp, Group6_BikeRaceCreate.cpp, KrustyUI.cpp and
  SelectGamePicProcs.cpp). Every section, relocation and symbol is equal;
  only the COMDAT order differs, as in earlier waves.

There was no DIFF, no compile failure, and no .bss or .data offset moved.

Negative control: the first attempt gave the Sound bit 6 member the name
`preferHardware`, which PlayWithOptions' argument already has. Inside
PlayWithOptions the member store became a self-assignment of the argument and
the test read the argument, and the proof gave DIFF in the `.text` of
`?PlayWithOptions@Sound@@QAEHHKH@Z` (work/naming/w24/first-run-report.txt).
The member is `hardwarePreferred` instead. The same run also caught the
SoultreeSurface and UnknownTimer uses that the first script missed (compile
failures in the new tree), which work/naming/w24/editC.py renamed.

Fixture pins: no digest changed, so nothing was repinned. 76 test modules
reach a changed file or a renamed name (work/naming/w24/pick_tests.py); all
84 of their tests pass.

### Fields and parameters

| Owner | Old | New | Tier | Evidence |
| --- | --- | --- | --- | --- |
| `TrackGameViewOwner` (TrackGame.h) | `field_0xac` | `gateCount` | 1 | TrackOverlay's instrument rows print `LoadResourceString(0x13ad)` (LANG.DLL: "Gate: ") with `"%s %d / %d"`, `lapGate + 1` and this value. RaceStatus.cpp (`0x004e6a50`) wraps a racer's next gate by it. The bike race owner still stores a float there through a cast (FollowRacerNearMisses.cpp, see Wave 5); that role keeps the cast and has no name of its own. |
| `InputDevice` | `field_0x14_axis0` to `field_0x14_axis5` | `hasXAxis`, `hasYAxis`, `hasZAxis`, `hasRxAxis`, `hasRyAxis`, `hasRzAxis` | 2 | PCJoystickDevice's EnumAxisObjectsCallback (`0x004c2cb0`) sets each bit when the DirectInput object's guidType is GUID_XAxis, GUID_YAxis, GUID_ZAxis, GUID_RxAxis, GUID_RyAxis or GUID_RzAxis. Each bit then gates the DIPROP_RANGE GetProperty at DIJOYSTATE offsets 0, 4, 8, 12, 16 and 20 (lX to lRz), and JoystickDevice reads that axis only when its bit is set. |
| `Sound` (PCAudio.h) | `field_0x1f4_bit6` | `hardwarePreferred` | 2 | PlayWithOptions stores its `preferHardware` argument there and then plays with flag 2 (DSBPLAY_LOCHARDWARE) when it is set and hardware 3D buffers are free, else with flag 4 (DSBPLAY_LOCSOFTWARE). The constructor and ReleaseBuffers clear it. The argument already has the name `preferHardware`, so the member could not take it (see the negative control). |
| `Sound` (PCAudio.h) | `field_0x1f4_bit7` | `loadQueued` | 2 | PlayWithOptions sets it when it hands a sound whose buffer is not ready to the loader queue (QueueSoundLoad), and only then. IsPlaying reports the sound as playing while it is set and the buffer is not ready, and the loader thread (samples/audio/AudioLoaderThread.cpp) clears it when the buffer is filled. |
| `SoundGroup` | `field_0x2c_bit0` | `playEnabled` | 3 | The constructor sets it, and PlayWithOptions returns without playing a member when it is clear. No other writer was found, so the name rests on that one test. |
| `TrackGameMode` raceSettings (TrackGame.h) | `field_0x18c[0x60]` | `lobbySlots` | 2 | SelectGamePicProcs.cpp copies it into `lobbySlots.slots` (8 LobbySlot of 12 bytes: 0x60), and the lobby start message senders copy `lobbySlots.slots` into it (Group5_MultiPlayerDlg.cpp, SelectGamePicProcsNearMisses.cpp). The array type is kept. |
| `UnknownBlitSource` (PCRenderTarget.h) | `field_0x70` | `systemSurface` | 2 | PCRenderTarget's Blit (`0x004c53f0`) passes it to Blt as the source surface. The callers pass PCTextureMaps, whose +0x70 is `systemSurface`. |
| `PCVideoCard` | `field_0x198` | `surfacePalette` | 2 | The constructor stores 0, and every read (`0x004cacbb`, `0x004cade4`, GameCursor.cpp, PartialTextureUploads.cpp) passes it as CreateSurface's `surfacePalette` argument, which other callers fill with a palette. |
| `DDSurfaceDesc2` (RenderInterfaces.h) | `field_0x14` (union with `backBufferCount`) | `depth` | 2 | DDSURFACEDESC2's union of `dwBackBufferCount` and `dwDepth`. Nothing reads it. |
| `DPMsgDestroyPlayerOrGroup` (QuarryStuntEvent.cpp, SelectGamePicProcs.cpp) | `field_0x04` | `playerType` | 2 | DPMSG_DESTROYPLAYERORGROUP's `dwPlayerType`, between `dwType` (5, DPSYS_DESTROYPLAYERORGROUP) and `dpId`. Nothing reads it (SDK layout, as in Wave 19). |
| `UnknownGuiRgb` (GUIManager.cpp) | `field_0x00` to `field_0x02` | `red`, `green`, `blue` | 2 | CreateFilledTexture (`0x00486740`) fills them with `color >> 16`, `color >> 8` and `color`, then loads the pixels as source format 0x378, whose pixel is RgbPixel {red, green, blue}. |
| `UnknownTimer` (TextureMapManager.h) | `field_0x04` | `peakSample` | 2 | LargestRecentSample (`0x004cb690`) returns it while the interval runs, and RecordSample (`0x004cb6b0`) stores a larger sample in it. |
| `SoultreeSurface` (D3DIMSoulTree.h) | `field_0x2c`, `field_0x30`, `field_0x34` | `mappingScale`, `mappingOffsetU`, `mappingOffsetV` | 2 | The draw passes their addresses to ManagedTexture::RemapTexCoordsIfMoved as `scale`, `offsetU` and `offsetV`, and restoring the coordinates resets them to 1.0, 0 and 0, as Griddraw.h's records named `mappingScale`, `mappingOffsetU` and `mappingOffsetV` are. The two offsets keep their int type. |
| `GridManagedTexture::RemapTexCoordsIfMoved` (Griddraw.h, Group5_GridManagedTexture.cpp) | `a`, `b`, `c` | `scale`, `offsetU`, `offsetV` | 2 | The view forwards them to ManagedTexture::RemapTexCoordsIfMoved, whose parameters have these names. |
| `CarProceduralSensorContact` (Group2_CarProcedural.cpp) | `field_0x08` | `normal` | 2 | The car copies it into the ground normal in place of QueryGround's normal, and the value feeds `smoothedNormal`. |
| `Scene` (SceneManager.h) | `field_0x88` | `groundProbeMask` | 3 | 0x004ea7e0 stores its last argument (`flag`, now `probeMask`) there; Quarry passes 8. The scene loader passes it to LoadPhysicsModel as `probeMask`, which the physics object keeps as `groundProbeMask`. The canonical names are tier 3. |

Left as placeholders:

- TrackGameViewOwner +0xac in the bike race owner: a float stored through
  a cast (5.0 when the followed racer changes); its own role has no name.
- Overlay `field_0x40` (a copy of the render target's surfacePixelFormat),
  HiResMeter `field_0x38` (the last selected timer) and TrackGame
  `field_0x1bdc` (saved in the profile): no reader that decides a role was
  found.
- Overlay::Attach's `a12` and TextService::Create's `a10`: forwarded, never
  interpreted.
- D3DIMSoultreeCharacter `d3d_field_0x1a4[0x6c]`: a 0x50-byte name followed
  by ints; naming it would need a split.
- The sound and graphics option blocks' `field_0x0c`/`field_0x24`/`field_0x14`,
  RaceSound `field_0x40`, Cube `field_0x10`, RacerStanding +0x04/+0x08,
  UIDialog `field_0x180` and `field_0xd4`, GridTerrain `field_0xc9c`/`0xca0`,
  the Terrain/FollowCamera header word +0xbe8 and FollowCamera
  `field_0x33c`: as in Wave 23, their uses do not decide a role.
- The union int aliases of named UI members (UIScrollBar +0x1ec/+0x204/+0x218,
  UIListBox +0x218, UIEditBox +0x204/+0x208): kept for codegen; a name would
  add nothing to the member they alias.
- `Scene::UnknownFunction4ea7e0` (it stores the owner and settings and adds
  the sound group) and the generic parameters listed in Wave 22.

## Wave 25: the view types named after the class they view

Wave 25 renames 149 `Unknown*` types, typedefs and inline methods. The views
are partial declarations of a retail class, seen by one consumer. Each one
is named `<Consumer><Class>View`, and the class is the one the view's
constructor, vtable write, RTTI or member offsets identify. Several
consumers view the same class (KrustyBike is seen from EventManager,
BikeRace, the visual cue, the number, the runway, the bonus, the cue gate,
the instrument and the chat overlays), so the consumer prefix keeps the
views apart. Wave 18's declaration count still forbids merging them. A
record or a network message takes its role name instead
(`RacerStateMessage`, `KrustyVcrGridEntry`, `VcrTickRecord`). Tier 2 means
the class or role is fixed by a constructor, vtable write, RTTI, the
member offsets shared with the named class, or an SDK layout. Tier 3 means
the role rests on how one consumer uses the object.

Names only: no declaration, typedef, forward declaration or member was
added or removed, and no static, global or type of a static or global was
renamed. Every edited file keeps its line count. The excluded progress
documents (NEAR_MISS_INDEX.md, REBUILD_GAPS.md, DECOMPILATION_PROGRESS.md,
config/decompilation_progress.json) spell none of the renamed names.

`git grep -ohE '\bUnknown[A-Za-z0-9_]*' -- src samples` fell from 2704 uses
(366 names) to 1317 uses (217 names). `field_0xNN` stays at 1786 uses and
the placeholder method names (`UnknownVirtualSlotN`, `UnknownMethodN`,
`Method_`, `Fn_`) stay at 85.

Method:

- work/naming/w25/triage.txt lists every remaining name with its uses and
  definitions. The specs work/naming/w25/specA.py, specB.py and specC.py
  hold each row with its tier and evidence. work/naming/w25/check.py flags
  names with no use, new names that already exist as an identifier or
  inside a decorated name, duplicates, and variables whose type is the old
  name (static or global hazard).
- check.py stopped two renames. `UnknownProgressCallback` could not take
  `LoadProgressCallback`, which is already the name of the function
  0x0045cb20 that is passed through it, so the typedef is `LoadProgressFn`.
  `UnknownSoultreeClipper` is the type of the extern `g_visibilityClipper`
  in D3DIMSoulTreeNearMisses.cpp, so it keeps its name.
- work/naming/rename.py rewrote each name as an identifier and inside
  decorated names (`PAVOld@@`, `?Method@Old@@`, `??0Old@@`) across the
  tracked tree, .bindings.json keys, tools/run_calibration.py, tests and docs
  included (work/naming/w25/map.json).

Result: one proof over every unit whose closure reaches a changed source
(272 units), from dc8253e8 to the renamed tree. The map
(work/naming/w25/proofmap.json) holds `Old@` -> `New@` for every rename.

- 199 IDENTICAL, 71 NAME-ONLY, 1 IDENTICAL-COMDAT-ORDER (Group4_DlgMain.cpp)
  and 1 NAME-ONLY+COMDAT-ORDER (DlgProcsNearMisses.cpp).

There was no DIFF in the final tree, no compile failure, and no .bss or
.data offset moved.

- The first run renamed `UnknownSoultreeQuadTreeObject` as well and gave
  DIFF in D3DIMSoulTree.cpp. The type descriptor
  `??_R0?AVSoultreeQuadTreeObjectView@@@8` is emitted in `.data`, so the
  name's length changes the section. The rename was reverted. A 70-unit
  rerun over the files that spell the name gave 45 IDENTICAL, 23 NAME-ONLY,
  1 IDENTICAL-COMDAT-ORDER and 1 NAME-ONLY+COMDAT-ORDER.
- Negative control: GameUiNearMisses.cpp and BikeRace.cpp proved with an
  empty map give DIFF in the relocations of `?EnterHover@UIControl@@UAEHXZ`
  and `?PickerHitCallback@@YAXPAVCollisionObject@@0@Z`. These are the
  renamed symbols they reference.

Fixture pins: the whole-object digests of 48 NAME-ONLY units changed with
the decorated names, in 45 fixture files and three test modules
(test_race_sound_refill's RaceSoundNearMisses.cpp aggregate and
test_rider_motion_start_connections' KrustyBike.cpp and
KrustyBikeNearMisses.cpp tuples). They were repinned with
work/naming/repin.py, repin2.py and w16_repin3.py, and by hand for the
three test modules. Each one is checked against the proof's NAME-ONLY
verdict for the unit, and no IDENTICAL or DIFF unit was repinned.
115 test modules reach a changed file, a renamed name or a repinned digest
(work/naming/w25/pick_tests.py, pinmods.py). All of them pass
(160 tests, 1 skipped). tools/static_check.py passes. tools/audit_bindings.py reports
0 mismatches and the same 6 `_kVec3*`/`_g_gridRow*` symbol conflicts that
the base dc8253e8 already reports without a linked build.

Left for later:

- Statics, globals and the types of statics: `UnknownStatic65b478`,
  `UnknownFontTextureManager`, `UnknownSoultreeStaticVector`,
  `UnknownKrustyUIVector`, `UnknownVisibilityStatic`, the `UnknownOpt*Settings`
  blocks, `UnknownBikeAIFilter` (0x00577ac0), `UnknownCubeTextureMapping`
  (0x0057ea10), `UnknownEcoDetailBand` (0x0056a600/0x0056a740),
  `UnknownBonusKey` (0x0056c820), `UnknownFilterKeys` (0x00689940),
  `UnknownSceneKeyword` and its sample siblings, and `UnknownSoultreeClipper`
  (the extern `g_visibilityClipper`).
- `UnknownSoultreeQuadTreeObject`: its RTTI type descriptor is emitted, so a
  rename changes `.data` (see above).
- Shared stubs and folded helpers (`UnknownFunction4de580`,
  `UnknownFunction464e80`, `UnknownFunction464e90`, `UnknownFolded4de580`)
  and `Scene::UnknownFunction4ea7e0`.
- Views whose class is not yet fixed: `UnknownTexturePalette` and
  `UnknownCursorAnimation` (blocked merges), `UnknownSoultreeNodeView`,
  `UnknownSoultreeMesh`, `UnknownSoultreeCameraView`, `UnknownKrustyUIModelObject`,
  `UnknownResourceEntry`, `UnknownGridEntry`, `UnknownGridDraw`, `UnknownTimer`,
  `UnknownCubeTextureContext`, `UnknownTrackGameModeOptions*`,
  `UnknownSoundNotifier`, `UnknownInlineButton`, `UnknownBikeAIPath`, the
  Wrecker, Recorder, Background, Scene emitter and Podium views, and the
  remaining single-file helpers in work/naming/w25/triage.txt.
- The fields and parameters listed at the end of Waves 23 and 24. Their uses
  still do not decide a role, and this wave found no new reader for them.

### Race, bike race, 3D object and overlay views

| Old | New | Tier | Rewrites | Evidence |
| --- | --- | --- | --- | --- |
| `UnknownEventRacer` | `EventKrustyBikeView` | 2 | 166 | RaceView.h: a vfptr at +0 and GameObject through a vbptr at +4, as KrustyBike's RTTI (virtual GameObject base, pdisp 4); its methods (0x004925a0, 0x00496f90) are KrustyBike.cpp code and its members sit at KrustyBike offsets (aiKind +0x734, isRemote +0x735, isGhost +0x736). |
| `UnknownKrustyBikeView` | `EventBikeRaceView` | 2 | 129 | TrackGameViewOwner+0x34: its methods are bikerace.cpp code (NextCameraTarget 0x0041f1d0, RestartRace 0x00420650, NextBike 0x004204e0; 0x004210f0 is bound as BikeRace::WriteRelativePositions) and its members are BikeRace's (localRacer +0x38, raceSound +0x44, raceCamera +0x50, youIndicator +0x60). |
| `UnknownKrustyBikeViewReplay` | `EventKrustyVCRView` | 2 | 2 | BikeRace+0x1a0 is the replay being recorded or played, a KrustyVCR (BikeRace.h). |
| `UnknownEventRacerModel` | `EventBikeWheelView` | 2 | 2 | A racer's +0x5f0 is Bike::frontWheel; its +0x200 is VehicleWheel nodePosition. |
| `UnknownBikeRaceRacer` | `BikeRaceKrustyBikeView` | 2 | 147 | BikeRace.h: its constructor is KrustyBike's (0x0048fa60), its slots are KrustyBike's (vtable 0x00554548, object_offset 0) and it reaches GameObject through the vbptr at +4. |
| `UnknownBikeRaceRacerInfo` | `LodBikeRaceView` | 2 | 3 | A racer's +0x740 is its BikeRace (RaceView.h bikeView); +0x18c is BikeRace's "ForceHighLOD". |
| `UnknownBikeRaceRacerPart604` | `BikeRaceWreckerView` | 2 | 2 | A racer's +0x604 is the Wrecker (Bike SetupWrecker); +0x38 is Wrecker::collisionModel. |
| `UnknownBikeRaceViews` | `BikeRaceRaceSoundView` | 2 | 5 | BikeRace+0x44 is the RaceSound (RaceView.h raceSound); +0x3c and +0x44 are RaceSound's engineGroup and effectsGroup. |
| `UnknownBikeRaceCharacter` | `BikeRaceD3DIMSoultreeCharacterView` | 2 | 13 | Its slots 0-11 are D3DIMSoultreeCharacter's primary slots (LoadCharacter is slot 11) and 0x004a8b10 is the character's SetMotionByName. |
| `UnknownBikeRaceMesh` | `StartD3DIMSoultreeObjectView` | 2 | 13 | The model at +0x1a0 of the start character and of the BikeRace+0x64/+0x6c objects; the methods called on it are D3DIMSoultreeObject's (0x004444c0, 0x004fc630, 0x004fbd70) and +0x28c is the LOD table. |
| `UnknownBikeRaceModel` | `RacerD3DIMSoultreeObjectView` | 2 | 34 | A racer's +0x3bc model and its rider's +0x1a0: 0x00444d00, 0x00444d40 and 0x00444c70 are D3DIMSoulTree/D3DIMSoultreeModifier code, and +0x274/+0x27c/+0x280/+0x28c are the LOD count, level, distances and table. |
| `UnknownBikeRaceMeshGroup` | `BikeRaceSoultreeLodView` | 2 | 5 | An entry of the +0x28c LOD table (8 bytes): D3DIMSoulTree's SoultreeLod, item count and surfaces. |
| `UnknownBikeRaceMeshItem` | `BikeRaceSoultreeSurfaceView` | 3 | 3 | One of a LOD's surfaces (0x38 bytes); ReplaceMeshId walks its ids. |
| `UnknownBikeRaceRider` | `BikeRaceRiderView` | 3 | 6 | A racer's +0x5c4 rider character, seen only for its +0x1a0 model. |
| `UnknownBikeRaceNetPlayer` | `BikeRaceGridEntryView` | 2 | 7 | TrackGame+0x2ebc (race settings +0x14c) is the grid; +0x04 is GridEntry::points (TrackGame.h). |
| `UnknownKrustyVcrRecord` | `KrustyVcrGridEntry` | 2 | 3 | KrustyVCR+0x25c holds the race settings' grid (+0x14c) copied with the settings: player id and points per slot. |
| `UnknownKrustyVcrRacer` | `KrustyVcrRacerRecord` | 2 | 2 | KrustyVCR+0x300: the 0x124-byte record KrustyVCR::SetRacer (0x0049c070) writes and GetRacer (0x0049c1e0) reads. |
| `UnknownBikeRaceEventAi` | `EventAiRacerRecord` | 2 | 5 | EventManager+0x444: one 0xcc-byte record per AI racer (textures, model path, engine, skill) that the loader reads. |
| `UnknownBikeRaceView6c` | `StartObjectCharacterView` | 3 | 9 | BikeRace+0x64/+0x6c ("StartGate.slb", "30SecMan.slt"): vbptr GameObject at +4, model at +0x1a0 and CollisionCharacter::collisionObject at +0x210; the class is not established. |
| `UnknownBikeRaceColliderBase` | `StartObjectColliderBase` | 3 | 2 | The object at +0x210 of the 30SecMan: a base with its own vfptr in front of the GameObject part 0x00421050 releases. |
| `UnknownBikeRaceCollider` | `StartObjectCollider` | 3 | 2 | The same object with its GameObject base. |
| `UnknownBikeRaceProjector` | `BikeRaceTerrainView` | 2 | 26 | BikeRace+0x4c: 0x00507c10 is Terrain::QueryGround and +0x40 is Terrain::gridCellSize. |
| `UnknownBikeRaceObjectFlags` | `BikeRaceGameObjectFlags` | 2 | 10 | GameObject's +0x25 flags (GameObject.h keeps them protected). |
| `UnknownBikeRaceSceneCharacter` | `BikeRaceSceneCharacterView` | 3 | 13 | A Scene+0xb4 entry with bit 3 set, reaching its GameObject flags through a vbptr at +4. |
| `UnknownBikeRaceCameraView` | `BikeRaceKrustyBikeCameraView` | 2 | 8 | BikeRace+0x50 is the KrustyBikeCamera; the members are FollowCamera's +0x244/+0x274, VehicleCamera's +0x384..+0x394, BikeCamera's +0x3b0 and KrustyBikeCamera's +0x3b4. |
| `UnknownBikeRaceShadow` | `BikeRaceProjectedShadowView` | 2 | 18 | BikeRace+0x5c: 0x004dab00 and 0x004dab90 are ProjectedShadow.cpp code (RTTI ProjectedShadow). |
| `UnknownBikeRacePickModel` | `PickPhysicsObjectView` | 2 | 2 | A scene caster's +0x08 physics object: +0x128 is SoultreePhysicsBaseObject::collisionObject. |
| `UnknownBikeRacePickCaster` | `PickSceneStaticModelView` | 2 | 3 | A Scene+0xb8 caster: SceneStaticModel's flags, physicsObject (+0x08) and collisionObject (+0x0c). |
| `UnknownBikeRacePickCharacter` | `PickCollisionCharacterView` | 2 | 2 | +0x210 is CollisionCharacter::collisionObject. |
| `UnknownBikeRacePickObject` | `PickProceduralObjectView` | 3 | 2 | A Scene+0xb4 procedural entry's object; +0x3c is its collision. |
| `UnknownBikeRaceVcrTick` | `VcrTickRecord` | 2 | 4 | Replay record kind -1: the time played and the tick count. |
| `UnknownBikeRaceVcrFollow` | `VcrFollowRecord` | 2 | 9 | Record and message kind 0x87: FollowRacer sends the followed racer's networkPlayerId, RecordFrame matches it to recorderId. |
| `UnknownBikeRaceNetState` | `RacerStateMessage` | 2 | 7 | Network message 1 (0x58 bytes): the KbBikeMessage layout the sender builds (position, angles, velocities, time). |
| `UnknownBikeRaceNetMessage13` | `RacerDeltaMessage` | 2 | 9 | Network message 13, which KrustyBike 0x004933e0 applies as deltas (KbNetDelta). |
| `UnknownBikeRaceNetScore` | `RacerScoreMessage` | 2 | 5 | Network message 10: racer index and KrustyBike score (+0x768). |
| `UnknownBikeRaceNetStamp` | `TimeStampMessage` | 2 | 5 | Network messages 0x84 (a time stamp at +4) and 0x85 (text at +4). |
| `UnknownBikeRaceRacerPart` | `ReceivedRacerState` | 2 | 14 | One of a racer's received states (+0x11c8, newest first): a message 1 state, its flags and a fresh flag (as TagBallReceivedState). |
| `UnknownBikeRaceBikeSetup` | `BikeEngineSetup` | 2 | 33 | The 0x5c-byte engine setup (engine size, four-stroke, KrustyUI "Weight", preset curve, "RPMLowerLimit"/"RPMUpperLimit") KrustyBike::Load takes as `setup`. |
| `UnknownBikeRaceUiModel` | `BikeRaceManufacturerRecord` | 3 | 10 | KrustyUI+0x48 records (0xc8 bytes, "ModelFile" at +0x40); the bike records' +0 indexes them. |
| `UnknownBikeRaceUiChoice` | `BikeRaceBikeRecord` | 3 | 6 | KrustyUI+0x50 records (0x94 bytes): manufacturer index, bike name, engine size, four-stroke flag. |
| `UnknownFunctionFollow` | `StartFollowing` | 2 | 7 | Inline (three times in 0x0041eb20): follows `racer`, restoring the previous racer's LOD and setting the camera's followed bike and racer. |
| `UnknownFunctionTargetCaster` | `TargetShadowCaster` | 2 | 3 | Inline: stores a scene shadow caster as the target and clears the others. |
| `UnknownFunctionTargetCharacter` | `TargetSceneCharacter` | 2 | 4 | Inline: stores a scene character as targetCharacter and clears the others. |
| `UnknownFunctionTargetObject` | `TargetSceneObject` | 2 | 4 | Inline: stores a scene procedural object as targetCar and clears the others. |
| `UnknownFunctionFreeNodes` | `FreeGateList` | 2 | 4 | Inline: frees each RaceGate of gateList (DebugFree at bikerace.cpp line 0xd3) and clears it. |
| `UnknownVisualCueRacer` | `VisualCueKrustyBikeView` | 2 | 11 | The racers BikeRace::NextBike (0x004204e0) returns: vfptr at +0, GameObject through the vbptr at +4 (KrustyBike RTTI). |
| `UnknownNumberCamera` | `NumberCameraView` | 2 | 11 | NumberObjectManager+0x34: Camera's +0x170 position, +0x17c/+0x188 axes and +0x198 focal length. |
| `UnknownNumberRacerState` | `NumberKrustyBikeView` | 2 | 2 | BikeRace's localRacer; +0x76c is the racer's offTrackTime. |
| `UnknownNumberRacer` | `NumberBikeRaceView` | 2 | 10 | NumberObjectManager+0x2c: BikeRace's localRacer (+0x38), 30SecMan (+0x6c) and startCountdown (+0x15c). |
| `UnknownRunwayTrackPos` | `RunwayTrackPos` | 2 | 11 | Track.h's TrackPos (node, segment, t), as Track::PointAt (0x00518080) takes it. |
| `UnknownRunwayTrack` | `RunwayTrackView` | 2 | 9 | BikeRace's raceTrack (+0x48); 0x00518080 is Track::PointAt. |
| `UnknownRunwayView` | `RunwayBikeRaceView` | 2 | 2 | A racer's +0x740 BikeRace, read only for raceTrack (+0x48). |
| `UnknownRunwayRacerPart` | `RunwayRaceStatusNodeView` | 2 | 2 | A racer's +0x744 RaceStatusNode: placedPos (+0x38) and progressPos (+0x44). |
| `UnknownRunwayRacer` | `RunwayKrustyBikeView` | 2 | 13 | The racer NationalRace passes (its view's +0x38): trackPoint, bikeView, statusNode, onTrack, hasFinished at the KrustyBike offsets of RaceView.h. |
| `UnknownRunwayTerrain` | `RunwayTerrainView` | 2 | 25 | NationalRace+0x40; 0x00507c10 is Terrain::QueryGround. |
| `UnknownVisualCueCamera` | `VisualCueKrustyBikeCameraView` | 2 | 2 | BikeRace's raceCamera (+0x50), a KrustyBikeCamera; +0x3b0 is BikeCamera's followed bike. |
| `UnknownVisualCueView` | `VisualCueBikeRaceView` | 2 | 17 | VisualCue+0xa8: BikeRace::NextBike (0x004204e0), localRacer +0x38, raceTrack +0x48, raceCamera +0x50, racing +0x18a. |
| `UnknownBonusRacerPart` | `BonusSoultreeObjectView` | 2 | 8 | A racer's +0x3bc model node; 0x004fc970 is SoultreeObject::GetPosition. |
| `UnknownBonusRacerModel` | `BonusBikeWheelView` | 2 | 2 | The racer's rearWheel (Bike +0x5f4); +0x230 is BikeWheel rollDirection. |
| `UnknownBonusRacerState` | `BonusKrustyBikeView` | 2 | 2 | BikeRace's localRacer: model node +0x3bc, crashState +0x444, rearWheel +0x5f4. |
| `UnknownBonusRacer` | `BonusBikeRaceView` | 2 | 9 | BonusObjectManager+0x2c, read only for BikeRace's localRacer (+0x38). |
| `UnknownBonusCamera` | `BonusCameraView` | 2 | 11 | The render target's current camera: Camera's +0x16c field of view, +0x170 position, +0x198 focal length. |
| `UnknownBonusRenderTarget` | `BonusRenderTargetView` | 2 | 2 | g_TrackGame+0x10, the render target; +0x08 is its current camera. |
| `UnknownKrustyRacerRef` | `CueGateKrustyBikeView` | 2 | 3 | The camera's followed racer; +0x7b8 is the racer's lapGate. |
| `UnknownKrustyCamera` | `CueKrustyBikeCameraView` | 2 | 2 | BikeRace's raceCamera (KrustyBikeCamera); +0x3b4 is its followed racer. |
| `UnknownKrustyCameraHolder` | `CueBikeRaceView` | 2 | 2 | The course event's +0x34 race view (BikeRace), read for raceCamera (+0x50). |
| `UnknownKrustyGate` | `CueCourseGate` | 2 | 2 | One 24-byte gate record at the course event's +0xd8 (RaceGatePose: centre and direction). |
| `UnknownKrustyCourse` | `CueCourseEventView` | 3 | 2 | TrackGame+0x560 (the Baja/Enduro event): raceView at +0x34, gate records at +0xd8. Its class is not established. |
| `UnknownKrustyFollowed` | `CueTagCameraView` | 3 | 2 | The tag event's raceCamera (+0x3c): GameObject flags at +0x25 and KrustyBikeCamera's followed racer at +0x3b4. |
| `UnknownKrustyNode` | `CueTagBallView` | 2 | 2 | TrackGameViewOwner::ball (the tag ball); its SoultreeObject subobject at +0x21c. |
| `UnknownKrustyPlayer` | `CueTagEventView` | 3 | 2 | TrackGame+0x568 (the Tag event): raceCamera +0x3c, followedRacer +0xa8, ball +0xdc. Its class is not established. |
| `UnknownKrustyGame` | `CueTrackGameView` | 2 | 15 | The game global 0x0056e26c (g_TrackGame): render target +0x10, events +0x560/+0x568, cameraMode +0x2d74. |
| `UnknownInstrumentState` | `InstrumentKrustyBikeView` | 2 | 3 | InstrumentOverlay+0x184's +0x3b4 (the followed racer): airborne +0x108, isGhost +0x736, lastLapTime +0x74c, fastestLap +0x750, finishPosition +0x784 at KrustyBike offsets. |
| `UnknownInstrumentSource` | `InstrumentKrustyBikeCameraView` | 2 | 10 | InstrumentOverlay+0x184, read only for +0x3b4, KrustyBikeCamera's followed racer. |
| `UnknownTerrain` | `OverlayTerrainView` | 2 | 31 | 0x00506e90 casts a segment against the terrain and 0x00507960 is Terrain::SelectQuality (src/krusty2/broadphase/Terrain.h). |
| `UnknownNameOverlayWorld` | `NameOverlayEventView` | 3 | 11 | NameOverlay+0x12c, read only for its terrain at +0x4c. |
| `UnknownStatsSource` | `StatsBikeRaceView` | 2 | 7 | StatsOverlay+0x124: BikeRace::NextBike (0x004204e0), localRacer +0x38, replayTime +0x1b8. |
| `UnknownRadarCamera` | `RadarKrustyBikeCameraView` | 2 | 3 | The camera RadarOverlay's loader is given (KrustyBikeCamera); +0x3b0 is the followed bike. |
| `UnknownRadarGate` | `RadarGatePose` | 2 | 2 | The gate event's +0xd8 records: position and direction (BikeRace.h's RaceGatePose). |
| `UnknownRadarGateEntry` | `RadarGateObjectView` | 2 | 2 | The gate event's +0x420 entries (BikeRace.h's RaceGateObject): draw flag +0x30, half width +0x50. |
| `UnknownRadarGateOwner` | `RadarGateEventView` | 2 | 3 | TrackGame+0x560 as RadarOverlay reads it (BikeRace.h's RaceGateEvent): gate count +0xac, current/next gate, records +0xd8, entries +0x420. |
| `UnknownChatRacerState` | `ChatSoultreeObjectView` | 3 | 2 | A racer's +0x3bc model, read for +0x14c (hides the name tag). |
| `UnknownChatRacer` | `ChatKrustyBikeView` | 2 | 22 | The racers BikeRace::NextBike returns: Vehicle driveForceScale +0x4a4, name +0x5e0, finishPosition +0x784. |
| `UnknownChatCamera` | `ChatKrustyBikeCameraView` | 2 | 3 | ChatOverlay+0x128: KrustyBikeCamera's followed racer at +0x3b4. |
| `UnknownChatView` | `ChatBikeRaceView` | 2 | 15 | ChatOverlay+0x12c: BikeRace::NextBike (0x004204e0) and localRacer +0x38. |

### Quarry loader, camera, GUI, ground fog and ecosystem views

| Old | New | Tier | Rewrites | Evidence |
| --- | --- | --- | --- | --- |
| `UnknownQuarryPhysicsObject` | `QuarryLoadPhysicsObjectView` | 2 | 12 | PhysicsObject members the loader reads |
| `UnknownQuarryTerrain` | `QuarryLoadTerrainView` | 2 | 4 | Terrain (QueryGround 0x00507c10) |
| `UnknownQuarryCar` | `QuarryLoadCarProceduralView` | 2 | 4 | CarProcedural fields the loader sets |
| `UnknownQuarryRacerRider` | `QuarryLoadRiderView` | 3 | 4 | rider member of the racer as the loader reads it |
| `UnknownQuarryRacerState` | `QuarryLoadWreckerView` | 2 | 4 | Wrecker offsets on the racer |
| `UnknownQuarryRaceView` | `QuarryLoadLodBikeRaceView` | 2 | 4 | LodBikeRace members |
| `UnknownQuarryRacer` | `QuarryLoadKrustyBikeView` | 2 | 21 | KrustyBike members |
| `UnknownQuarryRaceCharacter` | `QuarryLoadStartObjectView` | 3 | 6 | start-object character the loader places |
| `UnknownQuarryRace` | `QuarryLoadBikeRaceView` | 2 | 78 | ctor 0x00417c30 = BikeRace, Create 0x00417ed0 |
| `UnknownQuarryBikeCamera` | `QuarryLoadKrustyBikeCameraView` | 2 | 21 | ctor 0x00497cb0 = KrustyBikeCamera |
| `UnknownQuarryFog` | `QuarryLoadFogView` | 2 | 7 | 0x0046262f writes Fog vtable 0x005526dc |
| `UnknownQuarryFogObject` | `QuarryLoadFogOffView` | 2 | 6 | 0x00462e1d writes FogOff vtable 0x0055274c |
| `UnknownQuarryFogLink` | `QuarryLoadFogOnView` | 2 | 7 | 0x00462e9d writes FogOn vtable 0x005527bc |
| `UnknownQuarryRaceObject` | `QuarryLoadSoultreeQuadTreeRendererView` | 2 | 11 | 0x005044b9 writes vtable 0x00558168 |
| `UnknownQuarryVisibility` | `QuarryLoadVisibilityQuadTreeView` | 2 | 7 | ctor 0x0052d340; VisibilityQuadTree : QuadTree, GameObject |
| `UnknownQuarryQuadTreeHead` | `QuarryLoadQuadTreeView` | 2 | 2 | QuadTree head the loader resets |
| `UnknownQuarryOverlayHooks` | `QuarryLoadOverlayHooks` | 3 | 18 | overlay calls the loader makes |
| `UnknownFunction51d730` | `CollectChatRacers` | 2 | 6 | ChatOverlay::CollectRacers 0x0051d730 |
| `UnknownProgressCallback` | `LoadProgressFn` | 2 | 8 | BaseQuarryEvent.h typedef; LoadProgressCallback 0x0045cb20 is the function passed |
| `UnknownQuarryCamera` | `QuarryEventKrustyBikeCameraView` | 2 | 14 | KrustyBikeCamera offsets |
| `UnknownQuarryCameraSubject` | `QuarryEventKrustyBikeView` | 2 | 2 | KrustyBike offsets |
| `UnknownArcadeView` | `ArcadeCameraView` | 2 | 20 | arcade camera fields |
| `UnknownRaceSoundCamera` | `RaceSoundKrustyBikeCameraView` | 2 | 13 | KrustyBikeCamera offsets |
| `UnknownRaceStatusCamera` | `RaceStatusKrustyBikeCameraView` | 2 | 2 | KrustyBikeCamera offsets |
| `UnknownCameraBikeState` | `BikeCameraWreckerView` | 2 | 5 | Wrecker offsets |
| `UnknownCameraBikeRider` | `BikeCameraRiderView` | 3 | 4 | rider member |
| `UnknownCameraBikeBody` | `BikeCameraPhysicsRigidBodyView` | 2 | 2 | PhysicsRigidBody offsets |
| `UnknownCameraBike` | `BikeCameraBikeView` | 3 | 2 | bike the camera follows |
| `UnknownCameraVehicle` | `VehicleCameraVehicleView` | 2 | 4 | Vehicle offsets (+0x53c wheelList) |
| `UnknownCameraRider` | `VehicleCameraWheelView` | 2 | 3 | Vehicle+0x53c wheelList entries are VehicleWheel |
| `UnknownInlineRider` | `PickCameraWheel` | 3 | 2 | wheel entry the inline pick reads |
| `UnknownCameraTargetA` | `VehicleCameraCharacterTargetView` | 2 | 3 | character target offsets |
| `UnknownCameraTargetB` | `VehicleCameraCarProceduralView` | 2 | 3 | CarProcedural offsets |
| `UnknownGuiControl` | `GuiControlView` | 2 | 136 | dialog control as GUIUser/ToolTip see it; UIEditBox +0x214 caretRect |
| `UnknownGuiClipper` | `GuiDirectDrawClipper` | 2 | 5 | IDirectDrawClipper slot layout (SetClipList 7, SetHWnd 8) |
| `UnknownGuiDirectDraw` | `GuiDirectDraw` | 2 | 2 | IDirectDraw slot 4 CreateClipper |
| `UnknownGuiSurface` | `GuiDirectDrawSurface` | 2 | 3 | IDirectDrawSurface slot 28 SetClipper |
| `UnknownGroundFogCamera` | `GroundFogCameraView` | 2 | 3 | Camera position +0x170 (same as EcoSystem camera view) |
| `UnknownGroundFogOwner` | `GroundFogRenderTargetView` | 2 | 2 | RenderTarget at GameObject+0x18, camera at +0x08 |
| `UnknownGroundFog` | `GroundFogView` | 3 | 6 | found by name "GroundFog" (0x00484f10); class not established |
| `UnknownGroundFogShader` | `GroundFogCacheOwner` | 3 | 9 | object whose first member caches the fog (0x00484f10) |
| `UnknownFogVertex` | `GroundFogVertex` | 2 | 11 | D3DLVERTEX layout the fog pass edits |
| `UnknownEcoQuadTree` | `EcoQuadTreeView` | 2 | 15 | QuadTree global 0x0068aba4; Insert 0x004dcac0 |
| `UnknownEcoTerrain` | `EcoTerrainView` | 2 | 24 | Terrain QueryGround 0x00507c10 |
| `UnknownEcoHullShape` | `EcoHullShape` | 3 | 6 | CollisionObject shapeType 0 data |
| `UnknownEcoCapsuleShape` | `EcoCapsuleShape` | 3 | 2 | CollisionObject shapeType 3 data |
| `UnknownEcoSphereShape` | `EcoSphereShape` | 3 | 2 | CollisionObject shapeType 4 data |
| `UnknownEcoLight` | `EcoLightEmitterView` | 2 | 16 | LightManager::FindLightOfType 0x004a0190 result |
| `UnknownEcoCamera` | `EcoCameraView` | 2 | 13 | Camera slots 27-32, RenderTarget+0x08 |
| `UnknownEcoRenderTarget` | `EcoRenderTargetView` | 2 | 14 | GameObject+0x18 render target; IDirect3DDevice7 forwards |
| `UnknownEcoTexture` | `EcoTextureMapView` | 2 | 9 | TextureMap slots 4-19; 0x0050a590 returns TextureMap |
| `UnknownEcoModelVertex` | `EcoModelVertex` | 2 | 5 | .slt loader vertex (8 floats) |
| `UnknownEcoVertex` | `EcoLitVertex` | 2 | 9 | D3DLVERTEX FVF 0x1e2 |
| `UnknownEcoVisibilityClipper` | `EcoVisibilityClipperView` | 2 | 9 | visibility clipper 0x00575a98, SphereInFrustum 0x0052fbb0 |
| `UnknownEcoDisplay` | `EcoVideoCardView` | 3 | 2 | TrackGame+0x10; +0x1c0 PCVideoCard::driverColorKeyCaps |
| `UnknownEcoDebugOverlay` | `EcoDebugOverlayView` | 2 | 8 | DebugOverlay Print/Title 0x00447f40/0x00447fa0 |
| `UnknownEcoTrackGame` | `EcoTrackGameView` | 2 | 48 | cast from g_TrackGame; Game registry slots 20-22 |

### Scene, follow camera and other views

| Old | New | Tier | Rewrites | Evidence |
| --- | --- | --- | --- | --- |
| `UnknownFollowCameraSubject` | `FollowCameraTerrainView` | 2 | 15 | Terrain QueryGround 0x00507c10 |
| `UnknownSceneCollisionObject` | `SceneCollisionObjectView` | 2 | 16 | CollisionObject members SceneManager reads |
| `UnknownSceneAnimatedObject` | `SceneCarProceduralView` | 2 | 20 | CarProcedural members |
| `UnknownSceneObject` | `SceneCollisionCharacterView` | 2 | 29 | CollisionCharacter ctor 0x004318d0 (0x294 bytes); SetMotion 0x004a8b40 |
| `UnknownKrustyUICharacter` | `KrustyUID3DIMSoultreeCharacterView` | 2 | 13 | RTTI D3DIMSoultreeCharacter |
| `UnknownWreckerParticles` | `WreckerParticleManagerView` | 2 | 7 | RTTI ParticleManager |
| `UnknownCubeVertex` | `CubeLitVertex` | 2 | 7 | D3DLVERTEX layout |

## Wave 26: fields named by the layout they share, and stale comment references

Wave 26 names `field_0xNN` members whose struct is a second view of a layout
that already has names (the soultree surface and vertex group, the BVH box
node, the grid block record, the quadtree object), whose value is copied
from a named record, or whose one reader fixes the role. It also rewrites
the comments and notes that still spell a member by the placeholder it had
before an earlier wave named it. Names only: no declaration, typedef or
member was added, removed or reordered, no array was split, and no static or
type of a static was renamed. Every edited file keeps its line count.
`git grep -ohE '\bfield_0x[0-9a-fA-F]+' -- src samples` fell from 1786 uses
(377 distinct spellings) to 1499 (345): 69 uses went with the 28 renamed
member declarations and the parameter, 218 with the comment edits. `\bUnknown[A-Za-z0-9_]*`
stays at 1317 uses: no type or method was renamed.

Method:

- work/naming/w26/viewmatch.py and pairmatch.py read the compiled layouts
  (work/naming/cv/layouts.json) and list each placeholder whose offset and
  size match a named member of another class that agrees with it on other
  named offsets. Most leads are padding blocks of the Wave 25 views; the
  scalar leads were checked against their uses and retail by hand.
  work/naming/classreads.py lists the retail accesses (work/naming/w26/reads.txt).
- The renames were applied with work/naming/w26/editA.py, editB.py and
  editC.py (work/naming/sub.py, count-checked, line endings kept).
- work/naming/w26/comments.py lists every comment occurrence of a placeholder
  spelling. Each was resolved to the class it names (from its `Class::`
  qualifier, the `->` operand, the enclosing method or the surrounding text)
  and the member now declared at that offset in the canonical header
  (work/naming/w26/comment_edits.json, 192 proposed line edits, 118 left with
  a reason). work/naming/w26/editD.py replaces whole lines at their line
  numbers. It skips Quadtree.cpp, CollisionObject.cpp, FollowCamera.cpp,
  TrackOverlayNearMisses.cpp and BikeRaceNearMisses.cpp (21 edits), whose near
  misses are being matched at the same time, and applies 171 edits in 60 files.

Result: one proof over every unit whose closure reaches a changed file
(472 units), from 9fe68b72 to the renamed tree, with an empty map
(work/naming/w26/map.json): no mangled name changed.

| Verdict | Units |
| --- | --- |
| IDENTICAL | 465 |
| IDENTICAL-COMDAT-ORDER | 7 (DlgProcsNearMisses.cpp, SelectGamePicProcsNearMisses.cpp, Group4_DlgMain.cpp, DlgProcs.cpp, KrustyUI.cpp, OptionProcs.cpp, SelectGamePicProcs.cpp) |
| NAME-ONLY, DIFF, compile failure | 0 |

tests/fixtures/bvh_vector_compare.cpp was compiled in both trees with the
stub `bvh_vector_patches.h` (`static void InstallPatches() {}`), as in
earlier waves; it is IDENTICAL. No .bss or .data offset moved.

Negative control: in the new tree only, the GameObject bit was given the
name `flags`, which the constructor's argument already has. The constructor's
store became `flags = flags;`, a self-assignment of the argument, and the
proof gave DIFF in the `.text` of `??0GameObject@@QAE@H@Z`
(work/naming/w26/negcontrol.txt). The tree was then restored.

Fixture pins: no digest changed, so nothing was repinned.
138 test modules reach a changed file or a renamed name
(work/naming/w26/pick_tests.py); their 233 tests pass except test_emu_diff's vehicle_simulate_frame case (retail_faults 50 != 0), which fails the same way on the base tree 9fe68b72; 1 is skipped. tools/static_check.py
passes.

### Fields and parameters

| Owner | Old | New | Tier | Evidence |
| --- | --- | --- | --- | --- |
| `GameObject` | `field_0x25_bit1` | `receivesNetworkMessages` | 2 | Set and cleared with `isEnabled` (constructor, Enable `0x004690b0`, Disable `0x004690a0`), and the only test of it is OnNetworkMessage (`0x00469620`), which forwards the message to a child only when the bit is set. |
| `MorphBastardMeshGroup` (MorphBastardModifier.h) | `field_0x08`, `field_0x0c`, `field_0x10` | `sourceVertices`, `transformedVertices`, `normals` | 2 | The 0x14-byte group is D3DIMSoulTree.h's SoultreeVertexGroup (node, vertexCount, sourceVertices, transformedVertices, normals). CopyMesh (`0x004a5290`) points them into the copy's `vertices`, `drawnVertices` and `normals`, and ModifyMesh indexes the group by `sourceVertices - mesh->vertices`. |
| `MorphBastardMeshPair` (MorphBastardModifier.h) | `field_0x00`, `field_0x04` | `materialCount`, `materialIndices` | 2 | The 8-byte value at UnknownSoultreeMesh+0x20, which is SoultreeSurface's `materialCount` and `materialIndices` (+0x20, +0x24). The int type is kept. |
| `UnknownSoultreeMesh` (MorphBastardModifier.h) | `field_0x20` | `materials` | 2 | The pair above; CopyMesh copies it as one value. |
| `UnknownSoultreeMesh` (MorphBastardModifier.h) | `field_0x2c`, `field_0x30`, `field_0x34` | `mappingScale`, `mappingOffsetU`, `mappingOffsetV` | 2 | The 0x38-byte mesh is the SoultreeSurface that slot 27 receives (the draw casts the surface to it), and Wave 24 named SoultreeSurface's +0x2c..+0x34 so. The int types are kept. Nothing reads them through this view. |
| `MorphBastardChannel` (MorphBastardModifier.h) | `field_0x64`, `field_0x68` | `currentValue1`, `currentValue2` | 2 | ModifyMesh (`0x004a4c60`) reads `currentValue` for axis 0 and these two for axes 1 and 2; the controller update stores `-angleX`, `-angleZ` and `-angleY` into the three. |
| `UnknownSoultreeQuadTreeObject` (D3DIMSoulTree.h) | `field_0x04`, `field_0x06`, `field_0x08` | `queryStamp`, `sortKey`, `objectTypeId` | 2 | The RTTI QuadTreeObject base (vtable `0x005511c4`), whose CollisionObject.h declaration names the same offsets and sizes. The type keeps its name (Wave 25: its type descriptor is emitted). |
| `RaceGate` (BikeRaceNode.h) | `field_0x28` | `height` | 2 | The crossing test (GameUiHelpers.cpp) compares the crossing point's height above the gate centre with it, read as a float; AppendNode (`0x0041d0d0`) stores its int argument there. |
| `RaceGateObject` (BikeRace.h) | `field_0x54` | `height` | 2 | BuildNodeList (`0x0041d170`) passes it to AppendNode as the value stored at RaceGate+0x28. |
| `BikeRace::AppendNode` (BikeRace.cpp, BikeRaceCore.h) | `value` | `height` | 2 | The argument stored in RaceGate's `height`. |
| `LightManager` (LightEmitter.h) | `field_0x114` | `storeVertexColors` | 2 | LightVertices sets it when a colour array is passed and clears it otherwise, and the lighting loops keep the clamped per-vertex RGB in `vertexRed`/`vertexGreen`/`vertexBlue` only while it is set. |
| `Tire`, `VehicleWheel`, `BikeWheel`, `ShockCarrier` | `field_0x2a4` (`w_0x2a4` in BikeWheel) | `leanCos` | 2 | 1.0 after construction and ResetWheelContacts; Bike::UpdateLeanFromNormal stores the bike's `leanCos` in both wheels, and InlineShock::Retract (`0x004fa310`, called at `0x00528f1c` with `wheelList[i]` as the carrier) divides the retraction by it. |
| `CollisionBoxBounds` (CollisionShapeTests.h) | `field_0x00` | `volume` | 2 | The root is the 0x24-byte BVH node (center +0x04, halfExtents +0x10, children +0x1c/+0x20), BoundingBoxTreeBuild.h's BoxTreeNode, whose +0x00 is `volume`. The int type is kept. |
| `GridBlockRecord` (Griddraw.h) | `field_0x04` | `texture` | 2 | The record is Grid1.h's Grid1BlockTexture (same 0x2c bytes, same mapping fields), whose +0x04 is the texture used when the own bit is set; the allocation clears it. The int type is kept. |
| `CollisionCharacter` | `field_0x248`, `field_0x254` | `contactPoint`, `contactNormal` | 2 | Tick copies the contact record's +0x18 (the contact position CollisionObject copies into `hitPoint`) and +0x0c (`contactNormal`, CollisionSweepQuery) after QueryCollisions. |
| `SoultreeObject` (core/SoultreeObject.h, D3DIMSoulTree.h) | `field_0x170` | `subtreeBoundsValid` | 3 | It sits before the subtree bounds as `localBoundsValid` sits before the local bounds; the node record load reads it in that place, and BonusObjectManager::CreateFrameNode (`0x0048c8a0`) sets it to 1 with the subtree bounds as it sets `localBoundsValid` with the local bounds. No reader decoded. |

### Stale comment references

171 comment and note lines in 60 files spelled a member by the placeholder of
an earlier wave. They now use the member's name, resolved per mention (the
same offset can belong to different classes in one file: Tire.cpp mixes
CollisionPoint offsets of the +0xb8 base with Tire's own). Examples:
Tire.cpp's friction derivation (`frictionMagnitude = -friction(normalForce) *
surfaceGrip * frictionCoefficient`, `frictionDirection . rollDirection`),
the SoultreePhysicsBaseObject offsets in the physics headers (`sceneNode`,
`collisionPoints`, `terrainScale`, `asleep`), Terrain.h's `surfaceRegistry`,
`qualityIndex` and `drawDistance`, CollisionShapeTests.h's `swept`, Quantize.h's
ColorMapper tables, and samples/ui/DlgProcsNearMisses.cpp's `(results + i)->`.

Left as written: mentions of members that are still placeholders
(Vehicle `field_0x4e8`, FollowCamera `field_0x33c`, VisibilityCamera
`field_0x17c`, VCRFileEntry `field_0x00`), ranges and compound mentions
(`field_0x40..0x4c`), a Vec3 component (`field_0x504` is `controlInput.x`),
the bitfield byte `GameObject::field_0x25`, JSON files, the historical record
in samples/physics/hierarchy/MIGRATION.md (4 of its 44 mentions changed), and
the 21 edits in the files skipped above.

Left for later:

- The 21 comment edits in Quadtree.cpp, CollisionObject.cpp, FollowCamera.cpp,
  TrackOverlayNearMisses.cpp and BikeRaceNearMisses.cpp
  (work/naming/w26/comment_applied.json, `skipped_concurrent`), once their
  near-miss work has merged.
- The tag ball's byte +0x10a (TrackGame.h UnknownTrackGameViewOwnerDc), which
  0x004a9e80 clears when the followed racer changes: it would be
  SoultreePhysicsBaseObject's `asleep` if the ball is a physics object, which
  no constructor or RTTI shows yet.
- Members with only lifecycle or constructor stores: TrackGame
  `field_0x25e8` (a CombinedDirectoryList created and freed, never read),
  KrustyBike `field_0x810` (set with `aiTricksEnabled`), SoultreeObject's
  second screen rectangle `field_0x2a8`, the Terrain cell's `field_0x40`
  (TerrainShadow slot 30's gate, role not decided), Vehicle `field_0x4e8`,
  and the fields listed at the end of Waves 23 and 24.
- The padding arrays of the Wave 25 views that cover named members of their
  class (viewmatch.txt): naming them needs a split.

## Wave 27: fields named by the view they share, and the held-back comment edits

Wave 27 names `field_0xNN` members whose struct is a second view of a class
that already names the offset (CollisionPoint inside the vehicle wheel,
the soultree surface behind the BVH and shadow meshes, the Camera and
RenderTarget views, the RaceSettings copy in the VCR header), members that
sit at a known DirectX SDK struct position (D3DLVERTEX, DDCOLORKEY,
D3DDEVICEDESC7), and the exact-size padding members of the Wave 25 views. It
also applies the 21 comment edits Wave 26 held back. Names only: no
declaration, typedef or member was added, removed or reordered, no array was
split, and no static or type of a static was renamed. Every edited file
keeps its line count.

`git grep -ohE '\bfield_0x[0-9a-fA-F]+' -- src samples` fell from 1499 uses
(345 distinct spellings) to 1288 (326): 28 uses went with the held-back
comment edits and 183 with the renamed members, their uses and their
comments. 93 member renames changed 102 member declarations (some types have
two copies: the BVH tree types in krusty2 and reconstructed, the Quarry views
in bringup and samples, CarProceduralSensorContact in bringup and samples),
and 7 parameters were renamed. `\bUnknown[A-Za-z0-9_]*` stays at 1317 uses:
no type or method was renamed.

| Tier | Renames |
| --- | --- |
| 2 | 92 (85 members, 7 parameters) |
| 3 | 8 (`speed` and `previousSpeed` in the three Dirt/Spark particle emitters, CollisionModelBody `sceneNode`, VCRFileEntry `isOpen`) |

Method:

- work/naming/classreads.py rebuilt work/naming/cv/layouts.json and listed
  the retail accesses to each remaining placeholder (work/naming/w27/reads.txt,
  annotated in reads_annot.txt). Wave 26's viewmatch.py and pairmatch.py listed
  the placeholders whose offset and size match a named member of a class that
  agrees with them at other named offsets (viewmatch.txt, pairmatch.txt).
- Four groups worked the placeholders by area (A: game, UI, network and
  track; B1: vehicle, bike and particles; B2: terrain, collision, soultree
  and shadow; C: render, texture and camera). Each lead was checked against
  its uses and the retail accesses, and the groups listed what they left and
  why (work/naming/w27/leftA.txt, leftB1.txt, leftB2.txt, leftC.txt).
  Two names were reached independently by two groups and agree: the
  VisibilityGlobalEntry D3DLVERTEX names (A and B2, two copies of the
  struct) and `triTextureBlendCaps` at RenderTarget+0x1c8 (B2 and C).
- The exact-size padding members of the Wave 25 views (an array that covers
  exactly one member of the viewed class) took that member's name, with the
  byte-array type kept. Padding blocks that cover several members still need
  a split and were left.
- The renames were applied with work/naming/w27/editA.py, editB1.py,
  editB2.py, editC.py and editV.py (work/naming/sub.py, count-checked, line
  endings kept). work/naming/w27/editE.py applied Wave 26's held-back comment
  edits (comment_applied.json `skipped_concurrent`), matched by the whole
  line rather than the line number.
- Two test fixtures use renamed members and were updated with them:
  tests/fixtures/texture_loader_layout.cpp (its `SAME_OFFSET` pins now
  compare `alphaFormat` and `halvings` with themselves) and
  tests/fixtures/recorder_worker_compare.cpp (`entries[0].isOpen`).
- The functions being matched at the same time were left alone: no renamed
  member is used in Camera::UpdateMatrices `0x0042ee30`,
  GUIUser::SetPointerDevice `0x00488120`, NetworkInterface::DispatchMessages
  `0x004aced0`, the Vehicle constructor `0x005257a0`, PCGame::ProfileDisplays
  `0x004c0d10`, Wrecker::SprayContactParticles `0x00531740` or
  UIEditBox::OnChar `0x00474150`.

Result: one proof over every unit whose closure reaches a changed file
(388 units), from 4c945166 to the renamed tree, with an empty map
(work/naming/w27/map.json): no mangled name changed.

| Verdict | Units |
| --- | --- |
| IDENTICAL | 377 |
| IDENTICAL-COMDAT-ORDER | 11 (BikeRaceNearMisses.cpp, DlgProcsNearMisses.cpp, SelectGamePicProcsNearMisses.cpp, Group4_DlgMain.cpp, Group6_BikeRaceCreate.cpp, SoulTreePhysics.cpp, DlgProcs.cpp, KrustyUI.cpp, OptionProcs.cpp, ProCircuitProcs.cpp, SelectGamePicProcs.cpp) |
| NAME-ONLY, DIFF, compile failure | 0 |

The held-back comment edits were proved on their own first (6 units,
all IDENTICAL).

Negative control: in the new tree only, MovingPart's constructor parameter
was given the member's new name, `ownerRef`. The constructor's store became
`ownerRef = ownerRef;`, a self-assignment of the argument, and the proof gave
DIFF in the `.text` of `??0MovingPart@@QAE@PBDPAVSoultreeObject@@H@Z`
(work/naming/w27/negcontrol.txt). The tree was then rebuilt for the final proof.

Fixture pins: no digest changed, so nothing was repinned.
221 test modules reach a changed file or a renamed name
(work/naming/w27/pick_tests.py). Their 632 tests pass, 12 are skipped, and one
fails: test_emu_diff's vehicle_simulate_frame case (retail_faults 50 != 0), a
known failure of worktree checkouts that passes on main and does not involve
any renamed member.
tools/static_check.py passes.

### Fields and parameters

#### Game, UI, network and track (group A)

| Owner | Old | New | Tier | Evidence |
| --- | --- | --- | --- | --- |
| KrustyVCRHeader (DlgProcs.h) | `field_0x69` | `networkPlayerCount` | 2 | Header +0x34 is a RaceSettings copy: its named offsets line up (raceMode +0x34=RS+0, laps +0x54=RS+0x20 lapCount, randomGates +0x60=RS+0x2c, trackIndex +0x68=RS+0x34 trackNumber (TrackGame+0x2da4), trackName +0x6a=RS+0x36), so +0x69 = RaceSettings+0x35 networkPlayerCount (TrackGame+0x2da5). SinglePlayerDlg::HandleEvent 0x0044cf20 copies 0x1ec bytes from KrustyUI+0x4e8 (= header+0x34) into mode.raceSettings (rep movsd at 0x0044d456). Declaration only. |
| KrustyVCRHeader (DlgProcs.h) | `field_0x180` | `gridOrder` | 2 | 0x34 + 0x14c = 0x180 = RaceSettings::gridOrder of the header's RaceSettings copy; the existing comment TrackGame+0x2ebc = mode (0x578) + raceSettings (0x27f8) + 0x14c agrees. Declaration only. |
| KrustyUI (KrustyUI.h) | `field_0x634` | `gridOrder` | 2 | KrustyUI+0x4b4 holds the KrustyVCRHeader (DlgProcs.cpp:865 copies a whole one there); +0x634 = header +0x180 = the RaceSettings copy (KrustyUI+0x4e8, copied whole at 0x0044d456) + 0x14c gridOrder. Declaration only. |
| UnknownKrustyUISlot (KrustyUI.h) | `field_0x00` | `playerId` | 2 | The element of both gridOrder arrays above: the same 8-byte layout and role as GridEntry (TrackGame.h: int playerId; char points), RaceSettings::gridOrder's element. Ctor init list + declaration. |
| UnknownKrustyUISlot (KrustyUI.h) | `field_0x04` | `points` | 2 | As playerId: GridEntry +0x04 points. Ctor init list + declaration. |
| VisibilityGlobalEntry (Group1_Globals.cpp) | `field_0x00` | `reserved` | 2 | The entries alias g_visibilityBoxVertices +0xc (0x0068a774 vs 0x0068a768; 0x20-byte vertices); DrawBox draws the vertices with FVF 0x1e2 = D3DFVF_LVERTEX, whose +0x0c is dwReserved. Same names as group B's VisibilityQuadTree.cpp copy (already in the ROOT working tree). |
| VisibilityGlobalEntry (Group1_Globals.cpp) | `field_0x04` | `color` | 2 | D3DLVERTEX +0x10 color; the ctor's 0x2000ffff reset lands here (file comment, lines 38-40). |
| VisibilityGlobalEntry (Group1_Globals.cpp) | `field_0x08` | `specular` | 2 | D3DLVERTEX +0x14 specular. |
| UnknownScenePhysicsBody (SceneManagerNearMisses.cpp) | `field_0x04` | `vbptr` | 2 | Retail ctor 0x005037c0 stores the vbtable 0x557ff8 to [esi+4] (0x005037fb) under the most-derived flag; SoultreePhysicsBaseObject : virtual GameObject keeps its vbptr at +4 (SoultreePhysicsBaseObjectCore.h lines 6 and 174). Declaration only. |
| UnknownPodiumSoultree (EventManager.h) | `field_0x290` | `materialTable` | 2 | UnknownPodiumSoultree is a D3DIMSoultreeObject view (its methods bind to D3DIMSoultreeObject::SetUseLighting 0x004444c0 and ReplaceMaterialTexture 0x00444c70, SoultreeObject::SetPosition/SetAxesPtr); D3DIMSoulTree.h:307 names +0x290 materialTable. The podium code reads entry [0]'s +0x2c (SoultreeMaterial::textureName) and passes it as ReplaceMaterialTexture's name. |
| Overlay (Overlay.h) | `field_0x40` | `pixelFormat` | 2 | Overlay::Attach 0x004b5f50 copies renderContext->surfacePixelFormat (RenderTarget+0x28) into it (0x004b5f9e); ctor 0. Nothing in Overlay's code reads it. |
| TrackProbe (Track.cpp) | `field_0x28` | `height` | 2 | TrackProbe is the start/finish probe Track::LoadTrack fills; BikeRace passes &startProbe/&finishProbe, declared RaceGate (BikeRaceCore.h:136). RaceGate names +0x00 center, +0x0c direction, +0x18 halfExtents and +0x24 width (all agree with TrackProbe), and +0x28 height, read as a float by GameUiHelpers' crossing test. LoadTrack stores 150.0f. |
| VCRFileEntry (VCRfile.h) | `field_0x00` | `isOpen` | 3 | Readers decide the role: Load/Open claim an entry only when !field && !size; Close 0x00525000 acts only when it is set, then clears it and the position (0x0052502d). Nothing in retail sets it nonzero, so the name rests on the readers' intent. |
| NetFileChunk (Net.h) | `field_0x00` | `header` | 2 | NetFile::SendNextChunk 0x004ab1b0 sends the struct through Send(0xcf) -> SendRaw 0x004ad280, which writes the type byte at data+0 (the sequence at +2 when guaranteed); HandleReceivedMessage dispatches on that byte (0xcf) before casting the buffer. Same name as the other message structs' +0x00 (BikeRace.h, DlgProcs.h, SelectGamePicProcs.h, TrackOverlay.h). |
| NetFileHeader (Net.h) | `field_0x00` | `messageHeader` | 2 | The 0xcd message HandleReceivedMessage casts from the received buffer after dispatching on its byte 0: the message header. Named messageHeader, not header, because the struct is NetFileHeader and its users name their pointer `header` (NetThread.cpp:206, BeginReceive's parameter). |
| TrackGameMode::FindFileDirectory 0x00523a60 (TrackGame.h / UiInfo.cpp) | `value (param 1)` | `subdirectory` | 2 | Formatted as the directory before the file name ("%s\\%s.%s") when nonempty; callers pass seriesDirectories[...] / currentSeriesDirectory. `directory` is already a local of the function, hence subdirectory. |
| GUIManager::SwapModalDialog 0x00485ee0 (GUIManagerCore.h / GUIManager.cpp) | `value (param)` | `dialog` | 2 | Stored into modalDialog (+0x2c, the newest modal UIDialog); UIDialog passes (int)this (GameUi.cpp:4107). |
| UIDialog::SetShown 0x0046ea60 (UIDialog.h / GameUi.cpp) | `value (param)` | `shown` | 2 | Stored into isShown; selects Enable/Disable. UIDialog derives from GameObject, not UIControl, so UIControl::shown is not in scope. |
| UIDialog::SetPopupAlignment 0x0046ffc0 (UIDialog.h / GameUi.cpp) | `value (param)` | `alignment` | 2 | Stored into popupAlignment. |
| StatsOverlay::SetStatsSource 0x00519880 (TrackOverlay.h / TrackOverlay.cpp) | `value (param)` | `source` | 2 | Stored into statsSource. |

#### Vehicle, bike and particles (group B1)

| Owner | Old | New | Tier | Evidence |
| --- | --- | --- | --- | --- |
| DirtChunkParticleEmitter (ParticleEmitters.h) | `field_0x3c` | `speed` | 3 | Same ctor constants as DustParticleEmitter (+0x3c 1.0, +0x5c 0); the only non-ctor access is the Tick tail +0x5c = +0x3c (0x004b92f7), which is Dust's previousSpeed = speed. Nothing writes it after the ctor in this class, so the name rests on the Dust analogy. |
| DirtChunkParticleEmitter (ParticleEmitters.h) | `field_0x5c` | `previousSpeed` | 3 | Tick tail stores +0x3c here (0x004b92f7), exactly as Dust's slot 10 stores speed into previousSpeed; there is no reader in this class. |
| DirtSprayParticleEmitter (ParticleEmitters.h) | `field_0x3c` | `speed` | 3 | Same ctor constants as DustParticleEmitter (+0x3c 1.0, +0x5c 0); the only non-ctor access is the Tick tail +0x5c = +0x3c (0x004b9817), which is Dust's previousSpeed = speed. Nothing writes it after the ctor in this class, so the name rests on the Dust analogy. |
| DirtSprayParticleEmitter (ParticleEmitters.h) | `field_0x5c` | `previousSpeed` | 3 | Tick tail stores +0x3c here (0x004b9817), exactly as Dust's slot 10 stores speed into previousSpeed; there is no reader in this class. |
| SparkParticleEmitter (ParticleEmitters.h) | `field_0x3c` | `speed` | 3 | Same ctor constants as DustParticleEmitter (+0x3c 1.0, +0x5c 0); the only non-ctor access is the Tick tail +0x5c = +0x3c (0x004b9d8b/0x004b9da5), which is Dust's previousSpeed = speed. Nothing writes it after the ctor in this class, so the name rests on the Dust analogy. |
| SparkParticleEmitter (ParticleEmitters.h) | `field_0x5c` | `previousSpeed` | 3 | Tick tail stores +0x3c here (0x004b9d8b/0x004b9da5), exactly as Dust's slot 10 stores speed into previousSpeed; there is no reader in this class. |
| VehicleWheel (Vehicle.h) | `field_0x114` | `rotationalVelocity` | 2 | CollisionPoint base at +0xb8, and CollisionPoint+0x5c is rotationalVelocity; UpdateContactPatch 0x005135f0 writes it and UpdateShock 0x00513c70 takes its address; ResetWheelContacts 0x00525a90 zeroes it. |
| VehicleWheel (Vehicle.h) | `field_0x16c` | `contactState` | 2 | CollisionPoint+0xb4 contactState (base +0xb8; slot 31 stores 0/1 and keeps 2); AccumulateWheelContacts 0x00529c20 scales the torque when it is 2 and then clears it. |
| VehicleWheel (Vehicle.h) | `field_0x228` | `bodyPositionY` | 2 | Tire.h names +0x224 bodyPosition (UpdateSuspensionProbe 0x00514550 copies its position argument there); +0x228 is its y, which SettleBodyToWheels reads against wheelPosition.y. |
| VehicleTrackView (Vehicle.h) | `field_0x88` | `probeMask` | 2 | LoadVehicle passes it as SoultreePhysicsCharacter::LoadPhysicsModel's unsigned char probeMask parameter, which is stored as groundProbeMask. |
| Bike (BikeCore.h) | `field_0x5c8` | `loadInfo` | 2 | LoadBike copies *info here; the same info pointer is passed as D3DIMSoultreeCharacter::LoadCharacter's loadInfo (the 0x18-byte block it copies) and as LoadVehicle/LoadPhysicsModel's loadInfo. No Bike method has a loadInfo parameter. |
| BikeQuadTreeHead (Bike.cpp) | `field_0x08` | `objectTypeId` | 2 | The 0xc-byte QuadTreeObject head (+0x04 queryStamp, +0x06 sortKey, +0x08 objectTypeId char); this is the view's +0x08. The int type is kept, and it has no uses. |
| BikeBodyObj (Bike.cpp) | `field_0x178` | `mass` | 2 | BikeBodyObj is a PhysicsBody view: angularVelocity +0x16c, invMass +0x17c, invInertia +0x1d0, centerOfMass +0x228, node +0x234 and slot 43 GetWorldAngularVelocity all agree. PhysicsBody +0x178 is mass. No uses. |
| BikeKindInfo (Bike.cpp) | `field_0x24` | `fraction` | 2 | BikeKindParty is a ConstraintMethodCollisionModel, so +0x5c is CollisionObject::contactRecord, a CollisionSweepQuery whose +0x24 is fraction (ApplyContactImpulse's t between prev velocity and velocity); ApplyCollisionEvent case 0x3ea uses it the same way. |
| KbGame (KrustyBikeTypes.h) | `field_0xc50` | `wreckResetOnTrack` | 2 | KbGame is a TrackGame view (frameTime, bajaEnduroEvent, mode +0x578 and difficulty agree); TrackGame.h names mode +0x6d8 wreckResetOnTrack after the retail string ChkWreckResetOnTrack (Tier 1 backing). FindPlacement 0x00491e31 respawns at the stored position when it is set. |
| KbChild (KrustyBikeTypes.h) | `field_0x4` | `ownerNode` | 2 | The collisionPoints (+0x12c) elements are CollisionPoint objects; CollisionPoint+0x04 is ownerNode. ClearCrashState tests it before clearing the contact. |
| KbChild (KrustyBikeTypes.h) | `field_0xa4` | `inContact` | 2 | CollisionPoint+0xa4 inContact (SoultreeContact calls it touching, VehicleContact contactActive); ClearCrashState clears it. |
| CarProceduralNode (CarProcedural.h) | `field_0x08` | `objectTypeId` | 2 | The QuadTreeObject head (+0x08 objectTypeId char); the int type is kept. |
| CarProceduralSensorContact (Group2_CarProcedural.cpp, CarProceduralNearMisses.cpp) | `field_0x00` | `fraction` | 2 | The sensor collider's contact record is a CollisionBoxResult (fraction +0, point +4, normal +8); Tick reads 1 - fraction. |
| CarProceduralSensorContact (Group2_CarProcedural.cpp, CarProceduralNearMisses.cpp) | `field_0x04` | `point` | 2 | CollisionBoxResult +4 is point; the float type is kept. No uses. |
| CarProceduralSensorContact (CarProceduralNearMisses.cpp) | `field_0x08` | `normal` | 2 | The bringup view of the same struct already names +0x08 normal (CollisionBoxResult normal); the samples Tick copies it into its local normal. |
| MorphBastardFace (MorphBastardModifier.h) | `field_0x00` | `vertexIndices` | 2 | The element of SoultreeSurface+0x1c, unsigned short* indices (three per face); CopyMesh 0x004a5290 allocates faceCount of them and memcpy's them. |
| MovingPart (MovingPart.h) | `field_0x44` | `ownerRef` | 2 | The MovingPart views in Tire.h and Suspension.h name +0x44 ownerRef (ctor argument). The ctor parameter field_0x44_ becomes ownerRef_ to avoid a self-assignment. |
| TireAttachB (TireAttachmentViews.h) | `field_0x00` | `vtable` | 2 | The RotatingShock is a polymorphic Shock (slot 0 Reset), so +0x00 is its vptr; G3ShockView names the same offset vtable. No uses. |

#### Terrain, collision, soultree and shadow (group B2)

| Owner | Old | New | Tier | Evidence |
| --- | --- | --- | --- | --- |
| TerrainCell (src/krusty2/broadphase/Terrain.h) | `field_0x40` | `childMask` | 2 | The cell is a DrawableGridNode (Griddraw.h), whose +0x40 is childMask (Griddraw.h:370, updated by the visibility walk). TerrainShadow slot 30 (samples/physics/shadow/TerrainShadow.cpp) emits the cell's vertices only when it is nonzero, which is the Griddraw visibility gate. |
| TerrainHost (src/krusty2/broadphase/Terrain.h) | `field_0x00` | `vtable` | 2 | TerrainHost is the cast of the renderContext argument (Terrain.cpp:206/718, Group3_TerrainPrepareGeometry.cpp:119). That object is the polymorphic RenderTarget (virtual dtor 0x004e8cb0), so +0x00 is its vptr. +0x04 display lines up with RenderTarget::targetDisplay. Same spelling as G3ShockView/TireAttachmentViews' vtable. |
| CollisionModelShapeData (src/krusty2/collision/CollisionObject.cpp) | `field_0x04` | `elementEnabled` | 2 | This is the dtor's view of the type-1 payload. It is the same object as CollisionModelBody: the count at +0x00 and the elements/hulls at +0x14 agree, and both are 0x198-stride hull elements. CollisionModelBody +0x04 is elementEnabled: allocated elementCount*4 at lines 0xaa9/0xf4, and the model tests read it (ModelVsModel 0x00438280 etc.). The shape dtor frees it. |
| CollisionModelShapeData (src/krusty2/collision/CollisionObject.cpp) | `field_0x08` | `elementHighlight` | 2 | Same view match: CollisionModelBody +0x08 is elementHighlight, allocated at lines 0xaaa/0xf5 and read by the debug draw 0x00432b30 (a set element is drawn green). The shape dtor frees it. |
| CollisionModelBody (src/krusty2/collision/CollisionShapeTests.h) | `field_0x10` | `sceneNode` | 3 | ReadShape 0x00439e10 stores its node argument here for a type-1 model; for a type-0 hull it stores the same argument into CollisionHullBody::sceneNode. ReadModelShape 0x0043a050 stores the same node again. No reader was found, so this rests on the stores (tier 3). |
| D3DIMSoultreeObject desc view (src/krusty2/motion/D3DIMSoultreeCharacter.h) | `field_0x25` | `statusFlags` | 2 | GameObject +0x25 is the status byte (bit 0 isEnabled in GameObject.h), already named statusFlags at Krusty3DObjects.h:285. SoulTreePhysics.cpp (two readers) and Bike.cpp test desc->+0x25 & 1, the enabled bit. |
| TreeModelPart (BoundingBoxTreeBuild.h, krusty2 and reconstructed copies) | `field_0x0c` | `transformedVertices` | 2 | TreeModelPart is SoultreeVertexGroup (D3DIMSoulTree.h): 0x14 bytes, with node +0x00, vertexCount +0x04 and sourceVertices +0x08 matching frame/vertexCount/firstVertex. Its +0x0c is transformedVertices, written by TransformVertexGroups 0x00440d40. |
| TreeModelPart (BoundingBoxTreeBuild.h, krusty2 and reconstructed copies) | `field_0x10` | `normals` | 2 | Same view: SoultreeVertexGroup +0x10 is normals. |
| TreeModelMesh (BoundingBoxTreeBuild.h, krusty2 and reconstructed copies) | `field_0x14` | `drawnVertices` | 2 | TreeModelMesh is SoultreeSurface: 0x38 bytes, and groupCount/groups/vertexCount/faceCount/vertices/indices at +0x00..+0x10 and +0x1c agree. +0x14 is drawnVertices (D3DIMSoulTree.h:165; MorphBastardModifier.h uses the same name). 0x0042c260 gathers a single frame's positions from it. |
| TreeModelMesh (BoundingBoxTreeBuild.h, krusty2 and reconstructed copies) | `field_0x18` | `normals` | 2 | Same view: SoultreeSurface +0x18 is normals (freed by the surface dtor). |
| TreeModelSource (BoundingBoxTreeBuild.h, krusty2 and reconstructed copies) | `field_0x1a4` | `useLighting` | 2 | TreeModelSource is the D3DIMSoultreeObject (TransformVertexGroups 0x00440d40). Its +0x1a4 is useLighting (D3DIMSoulTree.h:284, which gates vertex lighting; copied by D3DIMSoulTree.cpp:1157). 0x0042c260 saves it, clears it around TransformVertexGroups(-1) and restores it, i.e. it transforms without lighting. |
| D3DIMShadowMesh (samples/physics/shadow/D3DIMSoultreeShadowNearMisses.cpp) | `field_0x14` | `drawnVertices` | 2 | The mesh view (vertexCount +0x08, triangleCount +0x0c, vertices +0x10, triangles +0x1c, size 0x38) is SoultreeSurface, whose +0x14 is drawnVertices. |
| D3DIMShadowMesh (samples/physics/shadow/D3DIMSoultreeShadowNearMisses.cpp) | `field_0x18` | `normals` | 2 | Same view: SoultreeSurface +0x18 is normals. |
| D3DIMShadowRenderTarget (src/krusty2/shadow/D3DIMSoultreeShadow.cpp) | `field_0x1c8` | `triTextureBlendCaps` | 2 | SDK position: the render target embeds D3DDEVICEDESC7, and +0x1c8 falls on dpcTriCaps.dwTextureBlendCaps. Slot 14 tests bit 2 (D3DPTBLENDCAPS_MODULATE) to choose the blend path. Group C gives TerrainShadowRenderTarget +0x1c8 the same name. |
| ProjectedShadow (src/krusty2/shadow/ProjectedShadow.h) | `field_0x60` | `texturePixels` | 2 | Init copies texture->pixels (a named member) here after CreateTextureSurface, and tests it before PageLock (ProjectedShadowNearMisses.cpp:60-61). |
| shadow camera view (src/bringup/Group2_ShadowCameraLookAt.cpp) | `field_0x194` | `roll` | 2 | This is a view of the Camera: lensScale +0x16c and eyePosition +0x170 agree with Camera.h. Camera +0x194 is roll (Camera.h:93; the setter's `const float* roll` argument). 0x0042e930 copies *c there. |
| shadow camera view (src/bringup/Group2_ShadowCameraLookAt.cpp) | `field_0x1d0` | `projectionChangeFrames` | 2 | Camera +0x1d0 is projectionChangeFrames (Camera.h:103). 0x0042e930 sets it to renderContext->frameModulus + 1. |
| LensRenderContextView (src/bringup/Group2_ShadowCameraLookAt.cpp) | `field_0x14` | `frameModulus` | 2 | RenderTarget +0x14 is frameModulus (RenderTarget.h:51; Camera slots 5/18 read it). 0x0042e930 reads renderContext+0x14 for the same projectionChangeFrames update. |
| VisibilityGlobalEntry (src/krusty2/visibility/VisibilityQuadTree.cpp) | `field_0x00` | `reserved` | 2 | SDK position: the entries at 0x0068a774 are the box vertices 0x0068a768 + 0xc. Those 0x20-byte vertices are D3DLVERTEX (FVF 0x1e2: x,y,z,dwReserved,color,specular,tu,tv), so entry +0x00 = vertex +0x0c dwReserved; the ctor stores 0. |
| VisibilityGlobalEntry (src/krusty2/visibility/VisibilityQuadTree.cpp) | `field_0x04` | `color` | 2 | Same SDK position: vertex +0x10 is the D3DLVERTEX color. The ctor stores 0x2000ffff, the colour of DrawBox's debug lines. |
| VisibilityGlobalEntry (src/krusty2/visibility/VisibilityQuadTree.cpp) | `field_0x08` | `specular` | 2 | Same SDK position: vertex +0x14 is the D3DLVERTEX specular (the ctor stores 0). |

#### Render, texture and camera (group C)

| Owner | Old | New | Tier | Evidence |
| --- | --- | --- | --- | --- |
| GameCursor (GameCursor.h) | `field_0x4c` | `redrawFrames` | 2 | RenderPost3D 0x0043ed40 passes &field_0x4c as BackgroundImage::DrawImageRect 0x00404700's `int* frames` (decremented, reset to Target()->frameModulus); UIControl passes its own redrawFrames (+0x1bc, GameUi.h) in the same argument position next to backgroundRegion. Otherwise only the ctor 0. |
| TextureMap (TextureMap.h) | `field_0x38` | `colorKeyHighValue` | 2 | SDK struct position: SetColorKey 0x004c7e30 and ColorKeyLevel store each key to +0x34 and +0x38 together, and &colorKeyValue (+0x34) is passed as the LPDDCOLORKEY of IDirectDrawSurface::SetColorKey(DDCKEY_SRCBLT) (PCTextureMap.cpp 538/717/718, ManagedTexture.cpp:121), so +0x38 is DDCOLORKEY.dwColorSpaceHighValue. colorKeyValue's comment updated. |
| UnknownPickCamera (ObjectPicker.h) | `field_0x19c` | `displayAspect` | 2 | Camera view at RenderTarget+0x08; agrees with Camera.h at eyePosition +0x170, focalLength +0x198, viewport +0x1a0.., +0x1c4; Camera.h names +0x19c displayAspect (written 0x0042e550). No use in the view; type int kept. |
| UnknownSoultreeCameraView (D3DIMSoulTree.h) | `field_0x19c` | `displayAspect` | 2 | Camera view (0x004439c0); agrees with Camera.h at position +0x170, viewDirection +0x17c, focalLength (imagePlaneDistance) +0x198, viewport +0x1a0; Camera.h names +0x19c displayAspect. No use; type int kept. |
| SoultreeTextureOptions (SoultreeMaterial.h) | `field_0x10` | `alphaFormat` | 2 | SoultreeMaterial.cpp:230 passes &textureOptions as TerrainLoadTexture's TextureContext*; TextureContext (TextureMap.h) names +0x10 alphaFormat (PCTextureMap slots 4/5) and agrees at +0x00 manager, +0x04/+0x08 opaque/alpha groups, +0x0c format. |
| SoultreeTextureOptions (SoultreeMaterial.h) | `field_0x14` | `halvings` | 2 | Same TextureContext view: +0x14 is TextureContext::halvings (PCTextureMap.cpp 72/85/217 halve wide textures that many times). |
| GroundFogableObject (GroundFogableObject.h) | `field_0x00` | `groundFog` | 2 | Griddraw calls 0x00484f10 with ecx = Terrain+0x2c (lea ecx,[eax+0x2c] at 0x00480dc1/0x00480ec9), the GroundFogableObject base; 0x00484f10 (GroundFogCacheOwner::FogVertices, GUIManager.cpp, member already named groundFog) caches FindByClassName(1, "GroundFog" 0x0056c338) or 1 in [this+0] and reads planeHeight through it. Class comment updated. |
| GridVertexLighter (Griddraw.h) | `field_0x00` | `groundFog` | 2 | GridVertexLighter is GridTerrain+0x2c, the same GroundFogableObject base, and declares FogVertices 0x00484f10; +0x00 is the cached GroundFog object as above. +0x04/+0x08 left. |
| GridRenderDevice (Griddraw.h) | `field_0x04` | `targetDisplay` | 2 | RenderTarget view (slots 7/8/15; +0x08 camera = currentCamera); RenderTarget.h declares PCVideoCard* targetDisplay at +0x04. No use; type void* kept. |
| Grid1RenderHost (Grid1.h) | `field_0x04` | `targetDisplay` | 2 | View of the RenderTarget at Terrain+0x18 (+0x08 camera); RenderTarget.h +0x04 is targetDisplay. No use; type int kept. field_0x00 (vptr word) left. |
| TerrainShadowRenderTarget (TerrainShadow.cpp) | `field_0x1c8` | `triTextureBlendCaps` | 2 | SDK struct position: PCRenderTarget's D3DDEVICEDESC7 at +0x164 puts dpcTriCaps at +0x1a0 and dwTextureBlendCaps at +0x1c8; PCRenderTarget.h already names triRasterCaps 0x1a8, triTextureCaps 0x1c0, triTextureFilterCaps 0x1c4. Slot 14 0x0050a1a0 tests bit 4 (D3DPTBLENDCAPS_DECALALPHA) at 0x0050a214 before picking the stage-0 blend. |
| Cube::SetVectors 0x0043d400 (Cube.h/Cube.cpp) | `a` | `newCenter` | 2 | Parameter copied to the named member center when nonzero; SkyCube.cpp:43 calls SetVectors(&center, 0, 0). |
| DrawableGridNode::SelectTexture slot (Griddraw.h, Grid1.h) | `a` | `distance` | 2 | The override DrawableGridNodeSharedTextures::SelectTexture 0x0047caa0 (samples/render/Grid1NearMisses.cpp:90) already names it distance and divides (16 << shift) * cameraFocalLength by it to get the on-screen detail; the caller passes ComputeViewerDetail 0x004815e0's viewer-to-bounds distance (local named detail at Griddraw.cpp:1161). |

#### Exact-size padding members of the Wave 25 views

Each array covers exactly one member of the class the view stands for, and the view
agrees with that class at two or more other named offsets (work/naming/w27/viewmatch.txt).
The `unsigned char` array type is kept and a trailing comment records it (`(bytes kept)`).
Nothing uses these members. All are tier 2.

| View (file) | Viewed class | Old | New | Viewed member |
| --- | --- | --- | --- | --- |
| `BikeRaceKrustyBikeView` (BikeRace.h) | KrustyBike | `field_0x124[0x128 - 0x124]` | `particleManager` | +0x124 `particleManager` (SoultreePhysicsBaseObject) |
| `BikeRaceKrustyBikeView` (BikeRace.h) | KrustyBike | `field_0x748[0x74c - 0x748]` | `finishTimeStamp` | +0x748 `finishTimeStamp` |
| `BikeRaceKrustyBikeView` (BikeRace.h) | KrustyBike | `field_0x76c[0x770 - 0x76c]` | `offTrackTime` | +0x76c `offTrackTime` |
| `BikeRaceKrustyBikeView` (BikeRace.h) | KrustyBike | `field_0x7a2[0x7a4 - 0x7a2]` | `prevLapCount` | +0x7a2 `prevLapCount` |
| `BikeRaceKrustyBikeView` (BikeRace.h) | KrustyBike | `field_0x11d8[0x1358 - 0x11d8]` | `stateBuffers` | +0x11d8 `stateBuffers` |
| `PickSceneStaticModelView` (BikeRace.h) | SceneStaticModel | `field_0x10[0x28 - 0x10]` | `fileName` | +0x10 `fileName` |
| `EventKrustyBikeView` (RaceView.h) | KrustyBike | `field_0x008[0x0c - 0x08]` | `sceneNode` | +0x08 `sceneNode` (SoultreePhysicsBaseObject) |
| `EventKrustyBikeView` (RaceView.h) | KrustyBike | `field_0x73c[0x740 - 0x73c]` | `plateNumber` | +0x73c `plateNumber` |
| `EventKrustyBikeView` (RaceView.h) | KrustyBike | `field_0x778[0x77c - 0x778]` | `aiCrashImpactLimit` | +0x778 `aiCrashImpactLimit` |
| `EventKrustyBikeView` (RaceView.h) | KrustyBike | `field_0x7a2[0x7a4 - 0x7a2]` | `prevLapCount` | +0x7a2 `prevLapCount` |
| `EventBikeRaceView` (RaceView.h) | BikeRace | `field_0x1a4[0x1a8 - 0x1a4]` | `ghostVcr` | +0x1a4 `ghostVcr` |
| `EventBikeRaceView` (RaceView.h) | BikeRace | `field_0x1e0[0x1e4 - 0x1e0]` | `appliedReplayMode` | +0x1e0 `appliedReplayMode` |
| `NumberCameraView` (Krusty3DObjects.h) | Camera | `field_0x194[0x198 - 0x194]` | `roll` | +0x194 `roll` |
| `VisualCueBikeRaceView` (Krusty3DObjects.h) | BikeRace | `field_0x04c[0x50 - 0x4c]` | `raceTerrain` | +0x4c `raceTerrain` |
| `SceneCollisionCharacterView` (SceneManager.h) | CollisionCharacter | `field_0x008[0x0c - 0x08]` | `sltFile` | +0x08 `sltFile` (Character) |
| `InstrumentKrustyBikeView` (TrackOverlay.h) | KrustyBike | `field_0x7a2[0x7a4 - 0x7a2]` | `prevLapCount` | +0x7a2 `prevLapCount` |
| `EcoRenderTargetView` (EcoSystem.h) | RenderTarget | `field_0x04[4]` | `targetDisplay` | +0x04 `targetDisplay` |
| `GuiControlView` (GUIManager.h) | UIControl | `field_0x2c[0x3c - 0x2c]` | `screenArea` | +0x2c `screenArea` |
| `GuiControlView` (GUIManager.h) | UIControl | `field_0x4c[0x5c - 0x4c]` | `visibleArea` | +0x4c `visibleArea` |
| `ArcadeCameraView` (ArcadeObject.h) | Camera | `field_0x0ec[0x16c - 0xec]` | `screenMatrix` | +0xec `screenMatrix` (`screenMatrix[2]`) |
| `QuarryLoadKrustyBikeCameraView` (Group3_QuarryLoad.cpp, QuarryStuntEventLoaderNearMiss.cpp) | KrustyBikeCamera | `field_0x3b8[0x3bc - 0x3b8]` | `raceView` | +0x3b8 `raceView` |
| `QuarryLoadFogOnView` (Group3_QuarryLoad.cpp, QuarryStuntEventLoaderNearMiss.cpp) | FogOn | `field_0x2c[0x30 - 0x2c]` | `fog` | +0x2c `fog` |

### Held-back comment edits

Wave 26's 21 held-back comment lines now use member names: Quadtree.cpp (9:
`quantScale`, `cellSizes`, `extentZ`, `yCenter`/`yHalfExtent`, `rootNode`,
`queryListHead`, `queryStamp`, `sortedStack`/`sortKey`,
`resultArray`/`resultCapacity`/`resultCount`), CollisionObject.cpp (8:
`shapeType`, `contactRecord`, `triangleTree`/`pointTree`, `centerScale`,
`radiusScale`, `endpointScale`), FollowCamera.cpp (`settleChecked`),
BikeRaceNearMisses.cpp (`racers[i]`) and TrackOverlayNearMisses.cpp
(`gridSlotStep`, `node`).

Left for later:

- VehicleWheelAux `field_0x8c`/`field_0x90` (Vehicle.h). Vehicle::UpdateWheelsInContact
  `0x00529a20` reads them through the wheel's Shock pointers (+0x2ac, +0x2b0),
  where Suspension.h names them `prevLength` and `clearedLength`. However, the
  same view is the engine gearbox at Vehicle+0x2a8, and
  tests/fixtures/vehicle_gearbox_boundaries.cpp pins them to EngineGearbox
  `throttleTimeScale`/`gearCount`. Naming them needs the view split into a
  Shock view and a gearbox view. The rename was applied and reverted.
- Members used by functions that were being matched at the same time:
  Vehicle `field_0x4e8` and SoultreePhysicsBaseObjectCore `field_0x28` (both
  in the Vehicle constructor `0x005257a0`), and the UIEditBox union aliases
  used in UIEditBox::OnChar.
- Undecided after this wave's reads:
  - The tag ball's `field_0x10a` (TrackGame.h), probably SoultreePhysicsBaseObject's `asleep`.
  - Terrain `field_0xbe8` (compared with 1 by Bike::UpdateCrashState and two others).
  - Terrain `field_0xc94`/`field_0xc98` (Load stores 5 and 6; never read).
  - QuadTree `field_0x58`, `field_0x860`, `field_0x868` (write-only).
  - ConstraintMethodCollisionModel `field_0xbc`/`field_0xd8`. The pairmatch lead to CollisionPoint's `ownerNode`/`surfacePosition` is spurious: slots 10 and 11 address this+0xc.
  - UIDialog `field_0x180`, passed only as PlayControlSound's ignored argument, and `field_0xd4`, a function pointer set only to 0.
  - GUIUser `field_0x1dc`.
  - RaceSound `field_0x40`, `field_0x48`, `field_0x5c`.
  - KrustyUI `field_0x4a4`, `field_0x4ac`.
  - The TrackGame and OptionProcs option blocks (resets and whole-block profile I/O only).
  - GridTerrain `field_0xc9c`/`field_0xca0` (union arms).
  - FollowCamera `field_0x33c`.
  - HiResMeter `field_0x38`.
  - Cube `field_0x10` (one reader; tier 3 at best).
  - AuralScape `field_0x1a0_bit2`, whose set condition and qsort role contradict.
- Placeholders that cover several members of the layout they view, which need a split:
  - BikeQuadTreeHead and CarProceduralNode `field_0x04` (the two shorts `queryStamp`/`sortKey`).
  - KbNetState `field_0x00`/`field_0x04` and RacerStateMessage `field_0x04`.
  - The DIDEVCAPS tail `field_0x18[5]`.
  - TreeModelVertex `field_0x0c[5]`.
  - The padding arrays of the Wave 25 views that cover more than one member.
- Exact-size view pads whose view agrees with the class at fewer than two other offsets: GroundFogView `visibilityOffset`, VisualCueKrustyBikeView `sceneNode`, RacerD3DIMSoultreeObjectView `lowestLod`.
- Constructor-only, lifecycle-only and write-only members, listed per group
  in work/naming/w27/leftA.txt, leftB1.txt, leftB2.txt and leftC.txt (particle
  emitter `field_0x38`/`field_0x68`/`field_0x6c`, the KrustyBike reset-only
  fields, CarProcedural's Create arguments, PhysicsBody `field_0x180`/`field_0x184`,
  Camera `field_0x1e8..field_0x1fc`, VideoCard, TextureMapManager, UIDialog,
  Game and RaceSound constructor stores, and others).

## Wave 28: the remaining Unknown types, the 0x464e90 hooks and the last view pads

Wave 28 is the leftovers wave. It worked the remaining `UnknownVirtualSlotN`
slots, the `Unknown*` types, functions and methods, the `field_0xNN`
placeholders in samples/ that a canonical src/ view settles, and the Wave 27
leftover list. Names only: no declaration, typedef or member was added,
removed or reordered, and every edited file keeps its line count. No static,
global or type of a static was renamed, and no type whose RTTI descriptor is
emitted.

Counts over `src` and `samples` (`git grep -ohE`, work/naming/w28/counts.py):

| Placeholder | Before (b28e75a1) | After |
| --- | --- | --- |
| `\bUnknownVirtualSlot[0-9]+` | 74 uses, 17 names | 74 uses, 17 names |
| placeholder methods (`UnknownVirtualSlotN`, `UnknownMethodN`, `Method_`, `Fn_`) | 85 uses | 85 uses |
| `\bUnknown[A-Za-z0-9_]*` | 1317 uses, 217 names | 573 uses, 86 names |
| `\bfield_0x[0-9a-fA-F]+` | 1288 uses, 326 spellings | 1274 uses, 326 spellings |
| `field_0x` in samples/ only | 118 uses | 113 uses |

The `Unknown*` count includes the 74 `UnknownVirtualSlotN` uses, which did not
change, and the `UnknownNodeVirtualSlotN` names of CarProceduralNode, which
did. That is why the placeholder-method count stays the same.

| Tier | Renames |
| --- | --- |
| 1 | 0 |
| 2 | 118: 109 types, typedefs and functions, and 9 members |
| 3 | 24: 12 types and functions, the three per-class names of the shared `0x00464e90` stub, and CarProceduralNode slots 0-8 |

Method:

- work/naming/w28/triage.py listed every remaining `Unknown*` name with its
  definitions and uses (work/naming/w28/triage.txt). Three groups worked
  through them:
  - R covered render, soultree, texture, camera, input and the ecosystem
    (work/naming/w28/specR.py, editR.py, leftR.txt).
  - G covered game, UI, race, podium, recorder, network and audio
    (specG.py, editG.py, leftG.txt).
  - F covered the samples field placeholders and the Wave 27 leftovers
    (editF.py, leftF.txt).
- Views follow Wave 25's `<Consumer><Class>View` convention. The class is
  fixed by bound method addresses, offsets that agree with the named class at
  several members, or the same retail pointer. Records, helpers and callbacks
  take role names.
- The type and function renames (121 names, work/naming/w28/map.json) were
  applied tree-wide with work/naming/rename.py. The run skipped
  docs/NEAR_MISS_INDEX.md, docs/REBUILD_GAPS.md,
  docs/DECOMPILATION_PROGRESS.md and config/decompilation_progress.json.
  It also updated the `.bindings.json` mangled keys, tools/run_calibration.py
  and the test text and fixtures that spell those names. Two cases were edited
  separately with count-checked substitutions (work/naming/sub.py):
  - The per-class renames of the shared stub spelling and the CarProceduralNode
    slots (editR.py).
  - The members (editG.py and editF.py).
- The functions being matched at the same time were left alone. No rename
  touches the bodies of:
  - Scene::ReadStaticModels `0x004ecd60`
  - Bike `0x004060c0`
  - PCJoystickDevice `0x004c3790`
  - CubeDraw `0x0043dc60`
  - MatrixUtil `0x004a1a50`
  - BoundingBoxTreeBuild `0x0042c260`
  - SoultreeBounds `0x004fd340`

  The names used there were deferred. These are
  `UnknownSceneLodObject`, `UnknownFunction464e80`, `UnknownFunction4de580`,
  `UnknownFolded4de580`, the Scene emitter and keyword views, and
  `UnknownWreckerCharacter`.

Scope A, the virtual slots: no slot that is not a destructor gained evidence.
None of them is the same retail slot, at the same `object_offset`, as a named
method, and none has a named or bound target:

- UIDialog slot 30 has no override and no UIDialog caller.
- BaseQuarryEvent slot 28 is not an attach step: Create3DObjects appends its
  child itself.
- Slot 34 has no new evidence.
- PhysicsBody slots 31 and 32 have none either. Their `+0x180` and `+0x184`
  are written only by the constructor, at `0x004cbd1f` and `0x004cbd50`.
- Vehicle slots 78 and 79 have no retail caller.
- ConstraintMethodCollisionModel slot 2, KbSink slots 2-5 and
  ConnectionInfoType slot 1 have no new evidence.
- `UnknownVirtualSlot0` (26 uses) is always the destructor slot of a view of
  GameObject, RenderTarget, Game, ArcadeObject, ControlInterface or
  QuadTreeObject. The InputDevice views' slot 1 is also the destructor. Both
  stay placeholders under the policy of Waves 4, 12, 21, 22 and 23.

`0x00464e90` is one shared `RET`. Each class's name for it comes from that
class's own call sites only, so the names are tier 3:

- GUIManager: `OnPaletteChanged`
- DrawableGridNode: `OnMaskCleared`
- ManagedTextureGroup: `OnOutOfPageSpace`

CarProceduralNode slots 0-8 take the QuadTreeObject and SoultreeObject slot
names of the class it views. It has no constructor or virtual definition, so
no vtable is emitted and the calls go by offset.

Two proposed renames were reverted:

- `UnknownEventGraphicsTest` → `GraphicsTest` (tier 1, from RTTI).
  EventManager.cpp already sees the real `GraphicsTest` class through
  src/krusty2/core/GraphicsTest.h, so the local view became a redefinition
  (C2011, COMPILE-FAIL in the proof). Using the real class means deleting the
  local declaration, which is outside a names-only wave.
- `UnknownSoultreeClipper` → `D3DIMSoultreeVisibilityClipperView` (tier 2).
  Its only method, TestBox `0x0052f570`, is bound as `VisibilityClipper::TestBox`,
  and the object is the clipper at `0x00575a98`. The unit was NAME-ONLY. But
  the type is the type of the extern global `g_visibilityClipper`, so the
  global's decorated name changed. test_clipper_global_connections could then
  no longer resolve it to retail (`RelocationError: unresolved symbol`).
  Renaming the type of a global is excluded, so it was reverted.

A check of every old object (work/naming/w28/datasyms.py, rttisyms.py) found
that no data, RTTI, vtable or guard symbol name contains a renamed name.

Result: one proof from b28e75a1 to the renamed tree covered every unit whose
closure reaches a changed file: 361 units. The map was the old→new decorated
spellings (`Old@` → `New@`, 133 entries, work/naming/w28/proofmap.json). The
EventManager.cpp revert and the D3DIMSoulTreeNearMisses.cpp revert were
proved again on their own (work/naming/w28/report-fix.txt). The final verdicts
are in work/naming/w28/report-final.txt:

| Verdict | Units |
| --- | --- |
| IDENTICAL | 286 |
| IDENTICAL-COMDAT-ORDER | 8 (Group4_DlgMain.cpp, Group5_PCBailoutDlg.cpp, Group6_BikeRaceCreate.cpp, DlgProcs.cpp, KrustyUI.cpp, OptionProcs.cpp, ProCircuitProcs.cpp, SelectGamePicProcs.cpp) |
| NAME-ONLY | 65 |
| NAME-ONLY+COMDAT-ORDER | 2 (DlgProcsNearMisses.cpp, SelectGamePicProcsNearMisses.cpp) |
| DIFF, compile failure | 0 |

Negative controls (work/naming/w28/negcontrol.txt):

1. Three NAME-ONLY units were compared again without the map, and each gave
   DIFF:
   - D3DIMSoulTreeNearMisses.cpp: the relocations of
     `?PrepareGeometry@D3DIMSoultreeObject@@UAEHXZ` differ.
   - GUIManagerHooks.cpp: `?UnknownFunction464e90@GUIManager@@QAEXXZ` exists
     only in the old object.
   - EventManagerNearMisses.cpp: the relocations of `CreatePodiumScene` differ.
2. One edit was made in the new tree only. ProjectedShadow::Init was changed to
   store `textureOwner` instead of `target` into the renamed `renderTarget`.
   The proof gave DIFF in the `.text` of
   `?Init@ProjectedShadow@@QAEPAV1@PAVShadowRenderTarget@@PAX@Z`.

Fixture pins: the whole-object digests of 22 NAME-ONLY units changed, which
made 24 test modules fail. They were repinned with
work/naming/w22/repin_all.py, which runs repin.py and then repin2.py:

- 38 digest swaps (30 distinct old/new pairs) in 22 tests/fixtures files.
  The logs are work/naming/w28/repin.log and repin2.log.
- Every repinned unit is NAME-ONLY or NAME-ONLY+COMDAT-ORDER in the report.
- Where a pin records relocation and section counts, they did not change.
- No digest in a test-module tuple changed.

The other edits under tests/ and tools/ change the spelling of a renamed
name. They cover:

- mangled names in tests/test_empty_lifecycle_providers.py and
  tools/run_calibration.py, for the GUIManager hook;
- view and type names in the test texts and the compare fixtures.

130 test modules reach a changed file, a renamed name, a NAME-ONLY unit or a
changed digest (work/naming/w28/pick_tests.py and pinmods.py). All 139 of
their tests pass; the log is work/naming/w28/tests-final.log.
tools/static_check.py passes.

### Types, typedefs and functions

| Old | New | Tier | Evidence |
| --- | --- | --- | --- |
| `UnknownAuralScapeReverbOwner` | `AuralScapeSceneView` | 2 | +0xa4 is Scene::environment |
| `UnknownAuralScapeReverbTable` | `AuralScapeSceneEnvironmentView` | 2 | +0x410 enviroID[4] is SceneEnvironment::enviroID |
| `UnknownAuralScapeTarget` | `AuralScapeRenderTargetView` | 2 | GameObject+0x18 renderContext (RenderTarget); +0x08 currentCamera |
| `UnknownAuralScapeViewer` | `AuralScapeCameraView` | 2 | RenderTarget currentCamera; +0x170 is Camera eyePosition (Camera.h:90) |
| `UnknownBackgroundCamera` | `BackgroundCameraView` | 2 | BackgroundImage.h view of the render target camera; offsets are Camera.h members |
| `UnknownBackgroundRegion` | `BackgroundRegion` | 2 | BackgroundImage.h: one tracked dirty region (0x40 bytes, a DebugMalloc array at BackgroundImage+0x50) |
| `UnknownBikeAIPath` | `BikeAIRacingLine` | 2 | KrustyBike+0x838 aiPath "racing line rebuilt by 0x00414370 each frame" (KrustyBikeCore.h:201) |
| `UnknownBlitSource` | `BlitPCTextureMapView` | 2 | PCRenderTarget::Blit 0x004c53f0 reads +0x70 = PCTextureMap::systemSurface (PCTextureMap.h +0x70); GameUi passes textures as sources |
| `UnknownBltEffects` | `BltFillEffects` | 2 | ManagedTextureGroup.cpp: DDBLTFX layout (dwSize +0, dwFillColor +0x50, 0x64 bytes) for the DDBLT_COLORFILL (0x400) Blt |
| `UnknownBltRect` | `BltDestinationRect` | 2 | ManagedTextureGroup.cpp: RECT layout passed as the destination of DirectDrawSurface::Blt |
| `UnknownCreateOverlayIconService` | `CreateOverlayIconService` | 2 | OverlayIconService.h inline: new OverlayIconService(10) at OverlayIconService.h line 90, inlined by Overlay 0x004b5f50 |
| `UnknownCubeCameraView` | `CubeCameraView` | 2 | CubeDraw.h: +0xac view matrix and +0x170 eye position = Camera.h viewMatrix/eyePosition |
| `UnknownCubeClipper` | `CubeVisibilityClipperView` | 2 | CubeDraw.h methods TransformPoints 0x0052f0a0, CanFaceCamera 0x0052f140, IsQuadOnScreen 0x0052f4d0 are VisibilityClipper TransformVectors/TestDot/CullQuad (tests/fixtures/visibility_connections_objects.json joins) |
| `UnknownCubeFace` | `CubeFace` | 2 | Cube.h: the 0xf8-byte per-face record of DrawableCube |
| `UnknownCubeTextureContext` | `CubeTextureContextView` | 2 | Cube.h {manager, textureGroup}: the TextureContext pair DrawableCube::Load and SkyCube::Create receive |
| `UnknownCueOwner` | `VisualCueRenderTargetView` | 2 | GameObject+0x18 renderContext is a RenderTarget; +0x08 is RenderTarget currentCamera (RenderTarget.h:48) |
| `UnknownCursorAnimation` | `CursorUIAnimView` | 2 | GUIManager.h cursor animation; AdvancePastSounds joined to UIAnim::AdvancePastSounds (tests/test_uianim_connections.py RENAMES) |
| `UnknownCursorPosition` | `CursorPixelPosition` | 2 | GameCursor.h: {x, y} in pixels returned by GameCursor::CursorPosition 0x0043f100 |
| `UnknownDialogImage` | `DialogUIAnimView` | 2 | GameUi.h: "DlgProcs.h's UnknownDialogImage is the same object" as UIAnim; GetCurrentTexture 0x00472f90 joined to UIAnim::GetCurrentTexture (tests/fixtures/uianim_connection_objects.json) |
| `UnknownEcoCoordinates` | `EcoQuantizedPosition` | 2 | EcoCoordinates.h {x,y,z} ushorts: Vegetation::quantizedPosition (unitsPerCoordinate units), the 6-byte record EcoSystem::PlaceStoredObjects reads |
| `UnknownEcoSampleTga` | `SampleEcoProbabilityTga` | 2 | Group4_EcoSystem.cpp static inline: the green channel of a probability TGA at terrain (x,z), scaled 0..1 |
| `UnknownEcoSampleTgaWeighted` | `SampleEcoProbabilityTgaWeighted` | 2 | Group4_EcoSystem.cpp static inline: the same sample times `weight` in retail's multiply order |
| `UnknownEulaProc` | `EulaEntryProc` | 2 | typedef of EBUEula.dll's entry point (TrackGame.cpp) |
| `UnknownFollowCameraPoint` | `FollowCameraPoint` | 2 | FollowCamera.h: element of the points table at FollowCamera+0x2e4 |
| `UnknownFollowCameraProjector` | `FollowCameraVisibilityClipperView` | 2 | FollowCamera.h ProjectPoint 0x0052f340 = VisibilityClipper::ProjectPoint; object at 0x00575a98 |
| `UnknownFollowedRacerSlots` | `FollowRacerKrustyBikeView` | 2 | the racer's own vtable (KrustyBike slot names); FollowRacer 0x004a9e80 calls slot 50 |
| `UnknownFunctionRacerAt` | `RacerAtIndex` | 2 | BikeRace inline: localRacer for index 0 single player, else the racer slot (Group1_BikeRaceCamera.cpp:157) |
| `UnknownFunctionRacerCount` | `CountRaceRacers` | 2 | static inline body: own + AI racers single player (playerCount in mode 4), own + remote multiplayer |
| `UnknownGameUiState` | `UIMultiStateEntry` | 2 | UIMultiState+0x1f4 stateTable entries, 32 bytes each |
| `UnknownGridEntry` | `SelectGameGridEntryView` | 2 | view of TrackGame.h GridEntry at RaceSettings +0x14c (gridOrder), qsorted by points |
| `UnknownGuiRgb` | `FilledTexturePixel` | 2 | GUIManager.cpp: the 24-bit pixel {red, green, blue} GUIManager::CreateFilledTexture 0x00486740 fills |
| `UnknownInlineArchiveUsed` | `IsArchiveInUse` | 2 | ResourceManager.h inline: 1 when any item's archive == `archive` (ResourceManager.cpp:100 deletes the old archive only when unused) |
| `UnknownInlineButton` | `GetButtonPart` | 2 | inline returning UIDropDownList buttonPart |
| `UnknownInlineFrame` | `GetFrameAt` | 2 | inline returning frameList[index] |
| `UnknownInlineIdle` | `IsSubjectUncrashed` | 2 | VehicleCamera.h inline: !vehicleMode \|\| vehicle->crashState == 0, the complement of sibling IsSubjectCrashed |
| `UnknownInlineSetField18` | `StoreRenderContext` | 2 | inline storing renderContext (+0x18) |
| `UnknownIsEnabled` | `IsCueRacerEnabled` | 2 | inline returning GameObject isEnabled for a cue racer |
| `UnknownJoystickBoolView` | `FollowCameraJoystickDeviceView` | 2 | FollowCamera.h: FollowCamera view of the active JoystickDevice at ControlInterface+0x0c (slot 4 read as bool); no ctor, vtable not emitted |
| `UnknownKeyboardBoolView` | `FollowCameraKeyboardDeviceView` | 2 | ControlInterface.h: FollowCamera view of the KeyboardDevice at ControlInterface+0x34 (slot 5 IsButtonDown read as bool); no ctor, vtable not emitted |
| `UnknownKrustyBike` | `KrustyBikeCameraKrustyBikeView` | 2 | KrustyBikeCamera+0x3b4; every member sits at a KrustyBike offset with a matching use (KrustyBikeCamera.h) |
| `UnknownKrustyBikeAxis` | `KrustyBikeCameraBikeWheelView` | 2 | KrustyBike+0x5f4 rearWheel is a BikeWheel; +0x150 penetration |
| `UnknownKrustyUIEntry` | `KrustyUITrackEntry` | 2 | 0x54-byte KrustyUI+0x60 list appended by AddEntry 0x0049ba70 (directory index, scene, series, file time/size, .env name) |
| `UnknownKrustyUIGui` | `KrustyUIGuiManager` | 2 | typedef GUIManager (KrustyUI.h:14; +0x2c built by GUIManager ctor 0x00485190); typedefs are not mangled |
| `UnknownKrustyUIGuiLayer` | `KrustyUIGuiUser` | 2 | typedef GUIUser (KrustyUI.h:15) |
| `UnknownKrustyUIModelObject` | `SelectGameD3DIMSoultreeCharacterView` | 2 | KrustyUI.cpp:388 casts new D3DIMSoultreeCharacter(0) to it; slots carry Character names (SelectGamePicProcs.h:63) |
| `UnknownKrustyUISlot` | `KrustyUIGridEntryView` | 2 | KrustyUI+0x634 and DlgProcs.h header +0x180 are RaceSettings::gridOrder; layout = TrackGame.h GridEntry (playerId, points) |
| `UnknownKrustyUISoultree` | `KrustyUISoultreeObjectView` | 2 | SetPosition 0x004fc660 / SetAxes 0x004fbd10 are SoultreeObject methods (KrustyUI.cpp:54) |
| `UnknownKrustyUIToolTipView` | `KrustyUIToolTipView` | 2 | +0x5c enabled is GUIManager.h ToolTip::enabled; GUIManagerCore.h +0xc0 userToolTip is a ToolTip* |
| `UnknownModelTexture` | `DialogSoultreeObjectView` | 2 | DlgProcs.h: ReplaceMaterialTexture joined to D3DIMSoultreeObject::ReplaceMaterialTexture (tests/fixtures/garage_material_objects.json); the object is the bike/rider model |
| `UnknownNumberOwner` | `NumberRenderTargetView` | 2 | GameObject+0x18 renderContext is a RenderTarget; +0x08 is RenderTarget currentCamera (RenderTarget.h:48) |
| `UnknownOptKrustyUIRecord` | `OptionsBikeRecord` | 2 | KrustyUI+0x50 0x94-byte bike records (KrustyUI.h 'each manufacturer's bikes'); +0x8c engineSize as BikeRace.h BikeRaceBikeRecord |
| `UnknownPickCamera` | `PickerCameraView` | 2 | ObjectPicker.h view; its offsets are Camera.h members (Camera +0xac viewMatrix, +0x170 eyePosition, +0x1a0 viewport) |
| `UnknownPodiumArcade` | `PodiumArcadeObjectView` | 2 | SetPosition 0x004014f0 / SetAxes 0x00401520 are ArcadeObject.h methods; Group1_PodiumViews.cpp casts to ArcadeObject |
| `UnknownPodiumContext` | `PodiumTextureContextView` | 2 | Game+0x1c resource context as the podium characters' slot 11 takes it; laid out like TextureMap.h TextureContext |
| `UnknownPodiumGrid` | `PodiumSceneView` | 2 | reached through BaseQuarryEvent+0x2c eventScene; reads Scene podiumPosition +0x8c / podiumDirection +0x98 / environment +0xa4 (SceneManager.h) |
| `UnknownPodiumModel` | `PodiumSceneEnvironmentView` | 2 | Scene+0xa4 environment; the field read at +0x398 is SceneEnvironment terrainBreadth (SceneManager.h) |
| `UnknownPodiumNode` | `PodiumSoultreeObjectView` | 2 | methods bound to SoultreeObject (0x004fc970 / 0x004fc4f0 / 0x004fc660 / 0x004fbd10); Group1_PodiumViews.cpp casts to SoultreeObject |
| `UnknownPodiumNodeOwner` | `PodiumStartGateView` | 2 | BikeRace+0x64 startGate (BikeRaceCore.h); its model node is read for GetPosition |
| `UnknownPodiumOwner` | `PodiumBaseQuarryEventView` | 2 | FindRaceMode() result; offsets read are BaseQuarryEvent.h members (eventScene +0x2c, raceView +0x34, lightManager +0x7c, texture groups +0x94/+0x98) |
| `UnknownPodiumRacer` | `PodiumKrustyBikeView` | 2 | racer list entries; +0x784 finishPosition (KrustyBikeCore.h:160) and +0x5c4 riderCharacter (BikeCore.h:131) |
| `UnknownPodiumRacerModel` | `PodiumRiderView` | 2 | Bike+0x5c4 riderCharacter (BikeCore.h:131) as the podium reads it |
| `UnknownPodiumSoultree` | `PodiumD3DIMSoultreeObjectView` | 2 | calls 0x004444c0 D3DIMSoultreeObject::SetUseLighting; ArcadeObject+0x2c arcadeModel is a D3DIMSoultreeObject* |
| `UnknownPodiumSound` | `PodiumSoundView` | 2 | ctor 0x004bba10, Load 0x004bc320 and PlayWithOptions 0x004bc6b0 are PCAudio.h Sound methods |
| `UnknownPodiumTextureEntry` | `PodiumSoultreeMaterialView` | 2 | entries of D3DIMSoultreeObject+0x290 materialTable (SoultreeMaterial**, D3DIMSoulTree.h:307); +0x2c texture name = SoultreeMaterial::textureName (first member after GameObject 0x2c) |
| `UnknownPodiumTrackObject` | `PodiumTerrainView` | 2 | BikeRace+0x4c raceTerrain; +0x40 is BikeRaceTerrainView gridCellSize; Group1_PodiumViews.cpp reinterprets it as Terrain |
| `UnknownPodiumViewFields` | `PodiumBikeRaceView` | 2 | BaseQuarryEvent+0x34 raceView; +0x4c raceTerrain and +0x64 startGate match BikeRaceCore.h |
| `UnknownProCircuitResult` | `ProCircuitRacerResultView` | 2 | view of EventManager's 0x50-byte RacerResult entries (eventManager->results) |
| `UnknownProjector` | `OverlayVisibilityClipperView` | 2 | TrackOverlay.h ProjectPoint 0x0052f340 is bound as VisibilityClipper::ProjectPoint; FollowCamera.h says it views the same pointer 0x00575a98 |
| `UnknownRaceSettings` | `SelectGameRaceSettingsView` | 2 | TrackGame+0x2d70 = mode.raceSettings (TrackGame.h RaceSettings, 0x1ec bytes) |
| `UnknownRadarView` | `RadarBikeRaceView` | 2 | TrackOverlay.h radarView is EventBikeRaceView*; +0x48 Track* is BikeRaceCore.h raceTrack |
| `UnknownRecorderCallback` | `RecorderCallbackProc` | 2 | typedef of the VCRInterface+0x80 cdecl callback; BikeRace 0x004230e0 is passed as one (typedef not mangled) |
| `UnknownRecorderOwner` | `RecorderVCRFileView` | 2 | VCRInterface+0xac: the worker calls VCRFile methods on it and +0x23c is VCRFile memory (Recorder.h) |
| `UnknownRecorderThread` | `RecorderWorkerThread` | 2 | 0x004e6f80 is the _beginthreadex start routine of VCRInterface 0x004e7a90 (docs/RECORDER.md worker thread) |
| `UnknownRegionStep` | `CacheRegionWalkStep` | 2 | CacheTexture.cpp: {region, index} step of CacheTexture::ReserveRegions 0x0050f9b0's depth-first walk |
| `UnknownRenderStateEntry` | `CachedRenderState` | 2 | PCRenderTarget.h: {state, value} of the 300-entry SetRenderState cache at +0x264 (PCRenderTarget.cpp:262 compares value) |
| `UnknownRenderStatePair` | `TextureRenderState` | 2 | TextureMap.h: {state, value} pairs at +0x48 applied through RenderTarget::SetRenderState (ManagedTexture.cpp:120, PCTextureMap.cpp:290) |
| `UnknownReplayTapeHeader` | `ReplayTapeHeader` | 2 | "VCRtape.dat" header, 0xeb0 bytes, RaceSettings at +0x34 |
| `UnknownScoreBoard` | `EventKbScoreBoardView` | 2 | EventBikeRaceView+0xbc; ShowValue 0x0048d1e0 is KbScoreBoard::ShowValue (KrustyBikeTypes.h:327) |
| `UnknownScreenX` | `ScreenXFraction` | 2 | returns the cue's screenX fraction (LoadVisualCue's screenXFraction) |
| `UnknownScreenY` | `ScreenYFraction` | 2 | returns the cue's screenY fraction (LoadVisualCue's screenYFraction) |
| `UnknownSkyCubeCamera` | `SkyCubeCameraView` | 2 | SkyCube.cpp: +0x170 eyePosition = Camera.h eyePosition (comment: Camera +0x170 is its position) |
| `UnknownSkyCubeTarget` | `SkyCubeRenderTargetView` | 2 | SkyCube.cpp: renderContext cast, +0x08 currentCamera = RenderTarget +0x08 current Camera |
| `UnknownSoultreeCameraView` | `D3DIMSoultreeCameraView` | 2 | D3DIMSoulTree.h: +0xec viewMatrix, +0x170 position, +0x17c viewDirection, +0x198 imagePlaneDistance, +0x1a0 viewport = Camera.h screenMatrix/eyePosition/viewDirection/focalLength/viewport |
| `UnknownSoultreeMatrix` | `SoultreeD3DMatrix` | 2 | D3DIMSoulTree draw samples: a 4x4 with D3DMATRIX element names |
| `UnknownSoultreeMesh` | `ModifierSoultreeSurfaceView` | 2 | MorphBastardModifier.h: 0x38-byte record whose members mirror SoultreeSurface (D3DIMSoulTree.h:159) offset for offset |
| `UnknownSoultreeNodeView` | `DrawSoultreeObjectView` | 2 | derives SoultreeObject in the D3DIMSoulTree draw samples; GetMatrixIn 0x004fca80, GetPositionIn 0x004fc9a0, WorldToLocalDirection 0x004fd710 are soultree.cpp SoultreeObject helpers |
| `UnknownSoultreeScreenRect` | `SoultreeScreenRect` | 2 | D3DIMSoulTree.h: screen rectangles at +0x298/+0x2a8 |
| `UnknownTexturePalette` | `TextureColorMapperView` | 2 | GetColors/GetIndexFrom555/GetIndexFrom565 0x004de270/80/90 are ColorMapper methods (Quantize.h, RTTI ColorMapper vtable 0x00557658); entries +0x10, entries555 +0x310, entries565 +0x510, indexFrom555 +0x710 match ColorMapper; Quantize.h says TextureMap.h views the same object |
| `UnknownThreadProc` | `RefillThreadProc` | 2 | 0x004bb630, started by StartRefillThread 0x004bb740; refills the half that finished playing |
| `UnknownTrackGameModeOptions6ac` | `TrackGameInterfaceOptions` | 2 | TrackGameMode+0x6ac; ResetInterfaceOptions 0x00522720; members are OptionProcs interface check boxes |
| `UnknownTrackGameModeOptions6ac360` | `GarageCurveChoice` | 2 | the member is garageCurveChoice (+0x360 of the interface block): one chosen curve row per bike class |
| `UnknownTrackGameModeOptionsA20` | `TrackGameSoundOptions` | 2 | TrackGameMode+0xa20; ResetSoundOptions 0x00522780; CD audio, sound, EAX, volumes |
| `UnknownTrackGameModeOptionsA4c` | `TrackGameGraphicsOptions` | 2 | TrackGameMode+0xa4c; ResetGraphicsOptions 0x005227d0; display mode, shadows, particles, sky cube, detail |
| `UnknownTrackGameModeOptionsA68` | `TrackGameControlOptions` | 2 | TrackGameMode+0xa68; ResetControlOptions 0x00522800; response sliders, feedback, joystick, gyros |
| `UnknownTrackGameModeOptionsFd8` | `TrackGameGarageOptions` | 2 | TrackGameMode+0xfd8/+0x1034/+0x1090; ResetGarageOptions 0x00522cb0 / ResetEngineOptions / ResetSuspension / CopyGarageTables |
| `UnknownTrackGameObject56cItem` | `TrackGameUIDialogView` | 2 | returned by 0x00485df0 GUIManager::FindInputDialog (UIDialog*); slot bound 0x0046ff30 UIDialog::EndDialog |
| `UnknownTrackGameSessionList` | `SessionList` | 2 | TrackGameMode+0xa98 sessions: SessionInfoType entries[5] (NetProcs.cpp enumerates sessions into it) |
| `UnknownTrackGameViewEntry` | `TrackGameRaceGateView` | 2 | RaceGateEvent+0x420 gates (BikeRace.h RaceGateObject); SelectLapGates 0x00404df0 toggles each gate +0x58 object +0x44 flag |
| `UnknownTrackGameViewOwnerDc` | `TrackGameTagBallView` | 2 | TrackGameViewOwner+0xdc ball ("FREE BALL!" in StatsOverlay 0x0051aa40); BikeRace.cpp reads the same object as BikeRace.h TagBall |
| `UnknownTrackGameViewPart` | `TrackGameSceneView` | 2 | BaseQuarryEvent+0x2c eventScene as TrackGame.h reads it (Scene members by offset) |
| `UnknownTrackRecordListBox` | `TrackRecordUIListBoxView` | 2 | only slot 66 is called, UIListBox::ReportClick 0x00477d30 (TrackRecordDlg.h) |
| `UnknownUpdateDistances` | `UpdateListenerDistances` | 2 | inline: calls UpdateListenerDistance on every emitter |
| `UnknownWreckerMatrix` | `WreckerD3DMatrix` | 2 | Wrecker.h: 4x4 with D3DMATRIX element names _11.._44 (64 bytes, tests/fixtures/camera_wrecker_math_layout.cpp) |
| `UnknownWreckerMotion` | `WreckerMotionView` | 2 | Wrecker.h: frameCount +0xa8, fps +0xb4 = Motion (Motnctrl.h), asserted by tests/fixtures/wrecker_model_layout.cpp |
| `UnknownWreckerParticle` | `WreckerParticleView` | 2 | Wrecker.h: layout of ParticleManager.h Particle (position, color, frame, age, lifetime, size, growth, flags +0x34, velocity +0x38); sibling WreckerParticleManagerView |
| `UnknownZonePlayerRecord` | `ZonePlayerRecord` | 2 | MSZoneInterfaceBase+0x08 players[8], reported by 0x004aa670 (MSZoneInterface.cpp) |
| `UnknownZoneRaceRecord` | `ZoneRaceRecord` | 2 | one racer's line of the race status; labels from 0x0049c770's debug messages |
| `UnknownZoneRaceStatus` | `ZoneRaceStatus` | 2 | MSZoneInterfaceBase+0x2a4 raceStatus, sent through 0x004aa670 by 0x0049c770 with size 0x88 |
| `UnknownZoneScore` | `ZoneScoreInterface` | 2 | CoCreateInstance(g_ZoneScoreClassGuid, ..., g_ZoneScoreInterfaceGuid, &score) (MSZoneInterface.cpp:179): the COM interface of that IID |
| `UnknownBikeAIPointOwner` | `BikeAIDirectionObstacle` | 3 | CollisionObject::ownerObject for ownerType 0x69 (direction +0x40); roles tier 3 |
| `UnknownBikeAISegmentOwner` | `BikeAISegmentObstacle` | 3 | CollisionObject::ownerObject for ownerType 0x64/0x65 (start +0x0c, direction +0x64); roles tier 3 |
| `UnknownFunction4ea7e0` | `AttachSceneManager` | 3 | 0x004ea7e0: stores the SceneArchiveFinder owner, aural scape and probe mask, adds the SoundGroup child |
| `UnknownGridDraw` | `GridDrawKey` | 3 | a racer's 12-byte random grid key sorted by CompareGridDraws 0x004f20a0 |
| `UnknownPlace` | `PlaceCue` | 3 | the placement block slot 10 repeats; places the cue at its screen position in front of the camera |
| `UnknownPlaceEmitter` | `ApplyNearestListenerVolume` | 3 | static inline: the emitter's volume from its nearest listener (pan always 0) |
| `UnknownRecorderRecord` | `RecorderRecordHeader` | 3 | head of the VCRInterface+0x7c record buffer: slot 10 reads time and count from it |
| `UnknownReset` | `ClearCurveChoices` | 3 | uncalled in-class inline that zeroes the five curve choices |
| `UnknownSceneTextureInfo` | `SceneTextureRecord` | 3 | SceneManager.h: the record CountTexture 0x004f00e0 reads (texture name +0x2c), built on the stack by its two callers |
| `UnknownSoundNotifier` | `SoundRefillNotifier` | 3 | 0x1c-byte helper at Sound+0x3c whose thread refills the buffer halves on DirectSound notifications |
| `UnknownVideoDecoder` | `VfwDecompressor` | 3 | VfwDeco.h: vfwdeco.cpp TU; destructor 0x0052d050 ICClose/ICDecompressExEnd on its HIC; held at PCTextureMap+0x7c (class name unproven) |
| `UnknownWreckerFrame` | `WreckerPoseFrame` | 3 | Wrecker.h: 4x4 read as right/up/forward/position rows; Wrecker::lastPose +0x6c and slot 10's pose local |

### Methods and virtual slots

| Owner | Old | New | Tier | Evidence |
| --- | --- | --- | --- | --- |
| GUIManager | `UnknownFunction464e90` | `OnPaletteChanged` | 3 | GUIManager::Tick 0x00485870 calls it after clearing paletteChanged (+0x1f0); ~UIDialog 0x0046a070 calls it in 8-bit mode unless the palette is locked |
| DrawableGridNode | `UnknownFunction464e90` | `OnMaskCleared` | 3 | Griddraw: all three calls follow childMask becoming 0 (child->childMask = 0, or oldMask != 0 && childMask == 0) |
| ManagedTextureGroup | `UnknownFunction464e90` | `OnOutOfPageSpace` | 3 | placement loop calls it when cachePages.Next() returns no page; TextureMapManager.h already notes "out of page space" |
| CarProceduralNode | `UnknownNodeVirtualSlot0` | `ComputeSortKey` | 3 | QuadTreeObject slot 0 (EcoQuadTreeObject.h) |
| CarProceduralNode | `UnknownNodeVirtualSlot1` | `ComputeQuadTreeCode` | 3 | QuadTreeObject slot 1 (EcoQuadTreeObject.h) |
| CarProceduralNode | `UnknownNodeVirtualSlot2` | `LoadHierarchy` | 3 | SoultreeObject slot 2, 0x004fdc00 |
| CarProceduralNode | `UnknownNodeVirtualSlot3` | `LoadHierarchyFromParameters` | 3 | SoultreeObject slot 3, 0x004fddc0 |
| CarProceduralNode | `UnknownNodeVirtualSlot4` | `CreateNode` | 3 | SoultreeObject slot 4, 0x004fe020 |
| CarProceduralNode | `UnknownNodeVirtualSlot5` | `UpdateLocalBounds` | 3 | SoultreeObject slot 5 (shared empty 0x00464e90) |
| CarProceduralNode | `UnknownNodeVirtualSlot6` | `ResetLocalMatrices` | 3 | SoultreeObject slot 6, 0x004fec70 |
| CarProceduralNode | `UnknownNodeVirtualSlot7` | `CloneFrom` | 3 | SoultreeObject slot 7, 0x004feb10 |
| CarProceduralNode | `UnknownNodeVirtualSlot8` | `CopySubtree` | 3 | SoultreeObject slot 8, 0x004feb30 |

### Fields

| Owner | Old | New | Tier | Evidence |
| --- | --- | --- | --- | --- |
| podium Scene environment view (Group1_PodiumScene.cpp, EventManagerNearMisses.cpp) | `racerCount` | `terrainBreadth` | 2 | Scene+0xa4 environment +0x398 is SceneEnvironment terrainBreadth (SceneManager.h) |
| podium terrain view (Group1_PodiumScene.cpp, EventManagerNearMisses.cpp) | `gridSpacing` | `gridCellSize` | 2 | BikeRace+0x4c raceTerrain +0x40 is Terrain gridCellSize (BikeRace.h BikeRaceTerrainView) |
| PodiumSoultreeMaterialView (was UnknownPodiumTextureEntry) | `field_0x2c` | `textureName` | 2 | The podium materialTable entries are SoultreeMaterial objects, whose +0x2c is char textureName[0x40] (SoultreeMaterial : GameObject); the only uses pass it as ReplaceMaterialTexture's texture name (EventManagerNearMisses.cpp:341, Group1_PodiumScene.cpp:311). |
| PodiumBaseQuarryEventView (was UnknownPodiumOwner) | `field_0x30[0x34 - 0x30]` | `skyCube[0x34 - 0x30] (bytes kept)` | 2 | Exact-size view pad of BaseQuarryEvent +0x30 skyCube; the view (commented TrackGameViewOwner) agrees with BaseQuarryEvent at +0x34 raceView, +0x7c lights and +0x94 opaqueTextureGroup. |
| TrackGameViewOwner | `field_0x30[0x34 - 0x30]` | `skyCube[0x34 - 0x30] (bytes kept)` | 2 | Exact-size view pad of BaseQuarryEvent +0x30 skyCube; the view agrees with BaseQuarryEvent at +0x2c eventScene, +0x34 raceView, +0x3c raceCamera, +0x5c..+0x6c overlays and +0x9c auralScape (its own comments cite them). No TrackGameViewOwner method has a skyCube parameter or local; nothing derives from it. |
| TrackGameViewOwner | `field_0x38[0x3c - 0x38]` | `visualCue[0x3c - 0x38] (bytes kept)` | 2 | Exact-size view pad of BaseQuarryEvent +0x38 visualCue (VisualCue*); same view agreement as skyCube. |
| QuerySweepRecord | `field_0x0c[0x0c]` | `contactNormal[0x0c] (bytes kept)` | 2 | The struct comment names it CollisionShapeTests.h's CollisionSweepQuery (same pointer as g_CollisionBoxResult); that struct has +0x0c contactNormal and agrees at +0x00 offset/contactOffset, +0x18 contact, +0x28 count/contactCount. |
| KbTrackRec | `field_0x00` | `position` | 2 | bajaEnduroEvent[lapGate + 9] is gates[lapGate] at +0xd8 (24-byte records, KrustyBikeTypes.h:219 comment); the gate record is Krusty3DObjects.h CueCourseGate, whose +0x0 is Vector3 position. KbTrackRec has no methods. |
| ProjectedShadow | `field_0x5c` | `renderTarget` | 2 | Write-only. Init 0x004da7b0 stores its ShadowRenderTarget* argument here right after SetRenderContext(target); the ctor clears it; no retail reader. Named like wave 27's Overlay pixelFormat (write-only copy of a named value). Not `target` (Init's parameter); no renderTarget identifier in the shadow units and ProjectedShadow has no derived classes. |

### What remains and why

This was the last planned naming wave. The placeholders it leaves fall into
groups, and the reason for each group decides whether another wave would help.

The 573 remaining `Unknown*` uses (86 names), grouped by work/naming/w28/remain.py:

| Uses | Names | Group | Why it remains | What would clear it |
| --- | --- | --- | --- | --- |
| 197 | 12 | Scene loader: the `Scene::UnknownFunction4de580` key setters, the `Scene::UnknownFunction464e80` message hook, `UnknownFolded4de580` and the SceneManagerNearMisses.cpp views (`UnknownSceneLodObject`, the emitter, physics-body and keyword views) | Their uses are in Scene::ReadStaticModels `0x004ecd60`, which is being matched. `0x004de580` and `0x00464e80` are also shared empty bodies, so their names can be tier 3 at most. | A small follow-up once that function is free. `UnknownSceneLodObject` → `SceneSoultreeObjectView` (tier 2) is ready: SetAxesPtr `0x004fbd70` and SetPosition `0x004fc660` are SoultreeObject methods. That rename must also update config/game_build.json and the view tuple in test_model_render_state_connections. |
| 177 | 25 | Types of statics or globals, and their accessors: the options `UnknownOpt*Settings` statics and their `Unknown*SettingsOf` / `UnknownMessagesOf` accessors, `UnknownStatic65b478` / `UnknownFontTextureManager`, the `UnknownTimer` fixture static, the KrustyUI and soultree static vectors, `UnknownEcoDetailBand`, `UnknownVisibilityStatic`, `UnknownBonusKey`, `UnknownFilterKeys`, `UnknownBikeAIFilter`, `UnknownCubeTextureMapping`, `UnknownSoultreeClipper`, `UnknownSceneKeyword` | A static's decorated name includes its type. That name changes the .bss/.data placement or the retail symbol resolution (see `UnknownSoultreeClipper` above). | A wave whose proof also compares the section layout and the resolved retail addresses, not the object alone. Several names are ready: `UnknownTimer` is a PeakHold view (`0x004cb670/90/b0`), and `UnknownSoultreeClipper` is a VisibilityClipper view. |
| 87 | 30 | `UnknownVirtualSlotN`, `UnknownMethodN` (ZoneScore slots 5 and 7-14), `UnknownRacerVirtualSlot26/78/79` | 34 uses are destructor slots: slot 0 of the views and slot 1 of InputDevice. The rest have no retail caller and no override, or point at shared stubs. No slot was the same retail slot as a named method. | For the destructors, one mechanical pass with a convention name such as `DeletingDestructor`, if the policy is changed (about 34 uses). For the rest, only new retail evidence. |
| 46 | 7 | Typedef aliases of named classes (`UnknownResourceEntry` = ResourceItem, `UnknownSceneResourceManager` = ResourceManager, `UnknownParameterStream` = FileStream, `UnknownPickCallback` = CollisionCallback, `UnknownWreckerSkeleton` = SoultreeObject, `UnknownWreckerCharacter` = D3DIMSoultreeCharacter, `UnknownVehiclePart` = SoultreeObject) | The target is already known. A role name would only add a second name for the same class. | Spell the target at the uses and drop the typedef. That is a declaration edit, not a rename. |
| 33 | 5 | Retired or comment-only names (`UnknownQuarry`, `UnknownBikeRaceOverlay19c`, `UnknownBikeRaceNewRacer`, `UnknownProCircuitSkinned`) and the retail string literal `"Unknown"` (23 uses, not a placeholder) | Some old names are asserted absent by tests, or are stale bindings keys. `UnknownProCircuitSkinned` is an unused declaration. | Clean up the comments and stale keys. Deleting `UnknownProCircuitSkinned` needs its fixture pin moved. |
| 20 | 4 | Class or role not established (`UnknownLightEmitterTarget`, `UnknownTrackGameViewEntryTarget`, `UnknownAxisSettle_4cb6e0`, `UnknownEventPlayerMessage`) | No constructor, vtable, binding or offset join fixes the class. A name would be a guess. | New retail evidence. |
| 11 | 2 | `UnknownEcoQuadTreeObject`, `UnknownSoultreeQuadTreeObject` | Polymorphic bases whose `??_R0` type descriptors and base-class arrays are emitted in .data. | None by renaming. Retail RTTI would name them if they had RTTI of their own. |
| 2 | 1 | `UnknownEventGraphicsTest` | The RTTI name `GraphicsTest` collides with the real class that EventManager.cpp includes. | Delete the local view and use GraphicsTest.h (a declaration edit). |

The 1274 remaining `field_0x` uses fall into these groups
(work/naming/w28/leftF.txt and the Wave 27 left lists):

- **Padding arrays that span several members of the viewed class.** These are
  the podium, Quarry loader, DDSURFACEDESC2 colour-key and EventManager AI
  table pads. They need a split into members, which changes declarations.
- **Members that are only written.** Examples are Vehicle `field_0x4e8`
  (tier-3 candidate `scaledTurnRadius`), SoultreePhysicsBaseObjectCore
  `field_0x28` (candidate `inverseLoadMass`), CacheTexture `field_0x188`,
  Terrain `field_0xc94`/`field_0xc98` and QuadTree `field_0x58`. They stay by
  the write-only precedent, except where a write copies a named value.
- **Int/float union arms.** These are the Griddraw unlit colour arms and the
  UIEditBox, UIScrollBar and UIListBox arms. Wave 24 keeps alias arms as
  placeholders on purpose.
- **The tag ball `field_0x10a`.** The ball's class is not established.
- **Members inside SceneManagerNearMisses.cpp.** That file is deferred with
  the Scene loader.

Is another wave worth it? Probably not a general naming wave. Waves 25-28
took the placeholders that a view, a binding or a shared layout settles. What
is left is blocked by one of four things:

- **Concurrency.** The Scene loader group, about 200 uses, is ready for a
  small follow-up once Scene::ReadStaticModels is matched.
- **The static and global rule.** About 180 uses. These need a proof that also
  checks the layout and the resolved symbols, not a bigger rename.
- **Declaration edits.** The typedef aliases and the padding-array splits.
  This is layout work that needs fixture repins, not renaming.
- **Policy.** The destructor-slot convention and the union arms. One decision
  on each would clear them mechanically.

The rest has no new evidence and should wait for matching work to produce it.

## Wave 29: the Scene loader names

Wave 29 is the Scene loader follow-up that Wave 28 deferred while
Scene::ReadStaticModels `0x004ecd60` was being matched. That function is now
strict exact, so the names used in it and in the rest of the Scene readers
could be taken. Names only: no declaration, typedef or member was added,
removed or reordered, and every edited file keeps its line count and line
endings. No static, global or type of a static was renamed, and no type whose
RTTI descriptor is emitted.

Counts over `src` and `samples` (`git grep -ohE`, work/naming/w28/counts.py):

| Placeholder | Before (78e01ae0) | After |
| --- | --- | --- |
| `\bUnknownVirtualSlot[0-9]+` | 74 uses, 17 names | 74 uses, 17 names |
| placeholder methods (`UnknownVirtualSlotN`, `UnknownMethodN`, `Method_`, `Fn_`) | 85 uses | 85 uses |
| `\bUnknown[A-Za-z0-9_]*` | 573 uses, 86 names | 376 uses, 74 names |
| `\bfield_0x[0-9a-fA-F]+` | 1274 uses, 326 spellings | 1274 uses, 326 spellings |
| `field_0x` in samples/ only | 113 uses | 113 uses |

The 197 cleared uses are the whole Wave 28 "Scene loader" group
(work/naming/w28/remain.py now reports 0 for it). That group also counted the
two `int` forms of `UnknownFunction464e80` (StatsOverlay and
MorphBastardModifier), so they were named here too.

| Tier | Renames |
| --- | --- |
| 1 | 2 types: the two keyword records, named by the retail keys they hold |
| 2 | 7 types: the view of SoultreeObject and the emitter and physics views |
| 3 | 5 methods: the Scene report hooks on the shared empty bodies `0x004de580` and `0x00464e80`, and the two other per-class names of `0x00464e80` |

Method:

- The type renames and the `0x004de580` names (11 names,
  work/naming/w29/map.json) were applied tree-wide with work/naming/rename.py.
  The run skipped docs/NAMING.md, docs/NEAR_MISS_INDEX.md,
  docs/REBUILD_GAPS.md, docs/DECOMPILATION_PROGRESS.md and
  config/decompilation_progress.json. It updated the `.bindings.json`
  mangled keys, the config/game_build.json link aliases (the four view
  constructors and the two `SceneSoultreeObjectView` methods),
  tools/run_calibration.py, the view tuple and alias keys of
  tests/test_model_render_state_connections.py, tests/test_scene_callbacks.py
  and the scene_callbacks.cpp fixture.
- `UnknownFunction464e80` names three different methods, so each class was
  edited separately with count-checked substitutions (work/naming/sub.py,
  work/naming/w29/sub464.py and sub464b.py): Scene's `const char*` form in
  the Scene units and `?...@Scene@@` keys, then the StatsOverlay and
  MorphBastardModifier `int` forms.
- Two comments in SceneManager.h and one sentence in docs/SCENE_CALLBACKS.md
  were reworded to the new names without changing their line counts.
- None of the renamed names is used in the bodies the concurrent matching
  work edits (BikeRace `0x00419970`, BikeAI `0x0040e510`/`0x00415640`,
  EventManager `0x0045d480`, Tire `0x005135f0`, SoultreeMatrix `0x004fc050`,
  TextService `0x0050ba00`).

The four Scene report hooks are tier 3. `0x004de580` (`ret 0xc`) and
`0x00464e80` (`ret 4`) are shared empty bodies, so the name comes from the
Scene call sites alone: every reader passes (section, key, value read) to
`0x004de580`, with a "(Default)" key or a "NOT FOUND" value when the key is
missing, and passes the formatted error text, or "" between sections, to
`0x00464e80`. Scene::Load `0x004efb20` reads the "LogProgress" key
(`0x00573b3c`) and opens "scnmgr.log" (`0x00573b30`) into `logFile` when it
is set; these calls are what such a log would print. The retail bodies write
nothing, so the names say what the calls report, not what they do.

The `field_0x` pads of SceneManagerNearMisses.cpp stay. Each one spans
several members of the class it views, so naming it would split an array:

- the emitter views' `field_0x2c` (DustParticleEmitter +0x2c firstUpdate,
  +0x30 manager and the rest, ParticleEmitters.h);
- SceneSoultreePhysicsBaseObjectView `field_0x0c` and `field_0x134`;
- SceneSoultreePhysicsObjectView `field_0x4f4` (the vtordisp and the
  virtual GameObject);
- SceneCollisionObjectView `field_0x38` and `field_0x64` (CollisionObject.h
  members +0x38..+0x5c, and +0x64 ownerType..+0xb4).

A check of every old object (work/naming/w29/datasyms.py, rttisyms.py) found
that no data, RTTI, vtable or guard symbol name contains a renamed name.

Result: one proof from 78e01ae0 to the renamed tree covered every unit whose
closure reaches a changed file: 56 units. The map was the old→new decorated
spellings (`Old@` → `New@`, plus `UnknownFunction464e80@Scene@`,
`@StatsOverlay@` and `@MorphBastardModifier@`; 14 entries,
work/naming/w29/proofmap.json). The verdicts are in
work/naming/w29/report-final.txt:

| Verdict | Units |
| --- | --- |
| IDENTICAL | 47 |
| IDENTICAL-COMDAT-ORDER | 1 (Group6_BikeRaceCreate.cpp) |
| NAME-ONLY | 8 (SceneManagerNearMisses.cpp, SceneManager.cpp, SceneCallbacks.cpp, scene_callbacks.cpp, QuarryStuntEvent.cpp, StatsOverlayDefaults.cpp, MorphBastardModifierNearMisses.cpp, Group3_MorphBastard.cpp) |
| DIFF, compile failure | 0 |

SceneManagerNearMisses.cpp is NAME-ONLY, so Scene::ReadStaticModels keeps the
bytes that made it strict exact.

Negative controls (work/naming/w29/negcontrol.txt):

1. Three NAME-ONLY units were compared again without the map, and each gave
   DIFF:
   - SceneManagerNearMisses.cpp: the relocations of
     `?ReadStaticModels@Scene@@QAEHPAVLightManager@@HHHP6AXH@ZH@Z` differ.
   - SceneManager.cpp: the relocations of
     `?ReadVector@Scene@@QAEHPAUVector3@@PBD11HPAVParameterBlock@@@Z` differ.
   - StatsOverlayDefaults.cpp: `?UnknownFunction464e80@StatsOverlay@@QAEXH@Z`
     exists only in the old object.
2. One edit was made in the new tree only. The "Sphere" ObjectShape value in
   ReadStaticModels was changed from 1 to 2. The proof gave DIFF in the
   `.text` bytes of `?ReadStaticModels@Scene@@...`.

Pins: the eight NAME-ONLY units change their decorated names, so their
whole-object digests change. Three of them are pinned:

- SceneManager.cpp (relocations 1273, sections 272);
- QuarryStuntEvent.cpp (393, 86);
- SceneManagerNearMisses.cpp (484, 96).

They are pinned 10 times across six fixtures, and those pins made six
modules fail:

- eco_light_objects.json
- race_detail_objects.json
- scene_motion_objects.json
- soultree_attachment_emitters_objects.json
- soultree_position_objects.json
- texture_history_timer_objects.json

work/naming/w22/repin_all.py (work/naming/repin.py) swapped each digest only
after checking that the unit is NAME-ONLY in report-final.txt and that its
relocation and section counts were unchanged. The log is
work/naming/w29/repin.log. Pinned digests:

| Unit | Old | New | Fixtures |
| --- | --- | --- | --- |
| SceneManager.cpp | `b3cd7b3bcecb` | `07aefc358850` | eco_light, texture_history_timer |
| SceneManager.cpp | `cffb89f0c58c` | `d71f442dee04` | scene_motion |
| SceneManager.cpp | `3ba831ee951d` | `f4f2d6e5e6ca` | soultree_position |
| QuarryStuntEvent.cpp | `1395805734a3` | `6978c516b78e` | eco_light, race_detail |
| SceneManagerNearMisses.cpp | `93b93afe7d7a` | `52b885be7378` | eco_light, soultree_attachment_emitters |
| SceneManagerNearMisses.cpp | `7e53a949b61b` | `17b3b60a914b` | scene_motion |
| SceneManagerNearMisses.cpp | `dc538fc9481f` | `26d0bddc6e68` | soultree_position |

The different digests of one unit come from different compile profiles.

Tests: work/naming/w29/pick_tests.py and pinmods.py selected 79 modules,
including:

- test_model_render_state_connections;
- test_scene_callbacks;
- test_stats_overlay_default;
- the six repinned modules.

All of them pass after the repin except one: the
vehicle_simulate_frame case of test_emu_diff. That case fails in a worktree
before and after this wave.
tools/static_check.py passes.

### Types

| Old | New | Tier | Evidence |
| --- | --- | --- | --- |
| `UnknownSceneShapeKeyword` | `SceneObjectShapeKeyword` | 1 | The 0x24-byte {name, value} record of ReadStaticModels' shape table, {"Box", 0} and {"Sphere", 1}, looked up with the model's "ObjectShape" string (`0x005734f0`) |
| `UnknownSceneEmitterKeyword` | `SceneEmitterTypeKeyword` | 1 | The 0x1c-byte record of the "EmitterType%d" table (`0x005734a8`): "DustEmitter", "DirtChunkEmitter", "DirtSprayEmitter", "ExhaustEmitter"; the default case prints "Unknown EmitterType (%s) for [%s]." (`0x00573418`) |
| `UnknownSceneLodObject` | `SceneSoultreeObjectView` | 2 | Its methods SetAxesPtr `0x004fbd70` and SetPosition `0x004fc660` are SoultreeObject methods; config/game_build.json aliases both to `SoultreeObject`. The objects behind it (character +0x1a0, CarProcedural +0x34 carModel) are D3DIMSoultreeObjects, whose primary base is SoultreeObject at +0 (RTTI). Ready since Wave 28. |
| `UnknownScenePhysicsObject` | `SceneSoultreePhysicsObjectView` | 2 | Constructor `0x005037c0` is bound as `SoultreePhysicsObject` (SoulTreePhysics.bindings.json) and aliased to it in config/game_build.json; slot 40 LoadPhysicsModel `0x00503970`; bases SoultreePhysicsBaseObject (+0) and D3DIMSoultreeObject (+0x21c), as RTTI gives them |
| `UnknownScenePhysicsBody` | `SceneSoultreePhysicsBaseObjectView` | 2 | The +0 base of that view: RTTI SoultreePhysicsObject : SoultreePhysicsBaseObject (+0), D3DIMSoultreeObject (mdisp 540 = 0x21c, the view's extent); its 41 slots end with slot 37 AddAttachment and slot 40, the SoultreePhysicsObject primary vtable `0x00557f54` |
| `UnknownSceneDustEmitter` | `SceneDustParticleEmitterView` | 2 | Constructor `0x004b8a00` is DustParticleEmitter's (RTTI, vtable `0x00555ab8`, Bike.bindings.json) and the game_build alias target; created for "DustEmitter" |
| `UnknownSceneChunkEmitter` | `SceneDirtChunkParticleEmitterView` | 2 | Constructor `0x004b8df0` is DirtChunkParticleEmitter's (vtable `0x00555b34`); created for "DirtChunkEmitter" |
| `UnknownSceneSprayEmitter` | `SceneDirtSprayParticleEmitterView` | 2 | Constructor `0x004b9310` is DirtSprayParticleEmitter's (vtable `0x00555ba8`); created for "DirtSprayEmitter" |
| `UnknownSceneParticleEmitter` | `SceneParticleEmitterView` | 2 | The view-only base of the three emitter views: slot 27 Create, which all three call (`0x004ba040`, one ICF-folded body). Retail has no ParticleEmitter class (each RTTI chain is Emitter : GameObject : BaseObject); the name describes the view, not a retail base. |

### Methods

| Owner | Old | New | Tier | Evidence |
| --- | --- | --- | --- | --- |
| Scene | `UnknownFunction4de580` (4 overloads) | `LogParameter` | 3 | Shared empty `ret 0xc` body `0x004de580`; the Scene readers pass (47 calls in the ten readers test_scene_callbacks checks) (section, key, value read) in string, float, int and Vector3* forms (docs/SCENE_CALLBACKS.md); see the paragraph above for "LogProgress" |
| Scene | `UnknownFolded4de580` | `LogDefaultParameter` | 3 | The same empty body under a second declaration; its two calls in ReadEnvironment `0x004ebfc0` pass the literal defaults "dirtpart.tga" and "graynoise.tga" when ParticleTextureMapName or DetailTextureMapName is missing |
| Scene | `UnknownFunction464e80` | `LogMessage` | 3 | Shared empty `ret 4` body `0x00464e80`; the Scene readers (25 calls in those ten) pass the formatted error or notice text ("Cannot find data for [%s] in %s.", "Scene:  No AuralScape, ...") or "" after a section |
| StatsOverlay | `UnknownFunction464e80` | `SetCueIndex` | 3 | The same empty body; its only caller, BaseQuarryEvent::Tick (`0x004de4a3`), passes visualCue->NextCueIndex() right after storing it in radarOverlay->cueIndex |
| MorphBastardModifier | `UnknownFunction464e80` | `OnChannelsUpdated` | 3 | The same empty body; ModifyMesh (`0x004a4ce9`) calls it with the object index i right after UpdateChannels(&morphObjects[i]) |

### What remains

The Wave 28 table still holds for the other groups (376 uses, 74 names, by
work/naming/w28/remain.py):

| Uses | Names | Group |
| --- | --- | --- |
| 177 | 25 | types of statics or globals and their accessors (`UnknownSceneKeyword` among them: `s_lightTypes` is a static array of it) |
| 87 | 30 | `UnknownVirtualSlotN` and `UnknownMethodN` |
| 46 | 7 | typedef aliases of named classes |
| 33 | 5 | retired or comment-only names and the retail string "Unknown" |
| 20 | 4 | class or role not established |
| 11 | 2 | types whose RTTI descriptor is emitted |
| 2 | 1 | `UnknownEventGraphicsTest` |

The Scene loader group is closed. The SceneManagerNearMisses.cpp pads and the
other multi-member pads need a declaration split, not a rename.

## Wave 30: members and parameters named by their readers

Wave 30 names the remaining `field_0x` members and placeholder parameters
wherever a reader, a named parameter, an SDK position or another view of the
same class decides the role. Names only: no declaration was added, removed or
reordered, and every edited file keeps its line count and line endings. No
static, global, type of a static, or type with an emitted RTTI descriptor was
renamed. Four read-only research passes (UI and sound; render and terrain;
vehicle, physics and collision; game, events and views) proposed names with
evidence. Each one was checked before it was applied. The proposals, the
left-over lists and the scripts are in work/naming/w30/:

- proposalA-D.json and leftA-D.txt;
- accepted*.json, apply.py and comments*.py;
- prove.sh and negcontrol.sh.

Counts over `src` and `samples` (`git grep -ohE`, work/naming/w28/counts.py):

| Placeholder | Before (38d2a74e) | After |
| --- | --- | --- |
| `\bUnknownVirtualSlot[0-9]+` | 74 uses, 17 names | 74 uses, 17 names |
| placeholder methods | 85 uses | 85 uses |
| `\bUnknown[A-Za-z0-9_]*` | 376 uses, 74 names | 376 uses, 74 names |
| `\bfield_0x[0-9a-fA-F]+` | 1274 uses, 326 spellings | 1214 uses, 322 spellings |
| `field_0x` in samples/ only | 113 uses | 112 uses |

The spelling count drops by only 4 because most renamed spellings (`field_0x00`,
`field_0x10`, `field_0x14` and so on) remain in unrelated structs.

| Tier | Members | Parameters |
| --- | --- | --- |
| 2 | 23 | 31 |
| 3 | 9 | 0 |
| Total | 32 | 31 |

The 63 renames edit 170 lines. Most of the 31 parameter renames are virtual
overrides that take the name of their base declaration, or view declarations
that take the name of the canonical declaration of the same retail function.

That is well short of the 100 the wave aimed at. The four passes looked at
every code use of a `field_0x` member (306 lines) and at the scalar members
that retail reads. What is left is mostly:

- constructor-only or lifecycle-only stores;
- members written but never read;
- union arms kept by policy;
- multi-member pads, which need a declaration split.

None of these was named to raise the count. The left-over lists give the
reason for each one.

Method:

- Each accepted name was applied with a line-targeted word substitution
  (work/naming/w30/apply.py). Every listed line must hold the old spelling,
  and the line endings are kept at the byte level.
- Comments that name the member were reworded in the same line
  (comments0.py, commentsA.py, commentsB.py, commentsD.py, work/naming/sub.py).
- The single-letter parameters (`a`, `b`) were applied only on their own
  declaration and code lines. Comment lines were edited by hand.
- An exact-size view pad that names one member of the viewed class keeps its
  byte-array type and gets a "(bytes kept)" comment (the Wave 27 rule).
- Every new name was checked against the parameters, locals, members and
  globals visible at each use.
- No renamed member or local is used in the bodies the concurrent matching
  work edits:
  - DlgProcs `0x0044fae0`;
  - ProCircuitProcs `0x004d72c0`, `0x004d8fc0`, `0x004d9860`;
  - SelectGamePicProcs `0x004f8820`;
  - SceneManager `0x004edfe0`;
  - MorphBastardModifier `0x004a33b0`.

  Three view declarations in SelectGamePicProcs.h and ProCircuitProcs.h were
  renamed. Their pads have no code use.
- One `targets.json` note (Vehicle::DeflectVelocity) spelled `field_0x4e8`
  and now says `scaledTurnRadius`. No test or fixture spells a renamed
  member or parameter.

### Decisions requested by the brief

- Vehicle `field_0x4e8` -> `scaledTurnRadius` (tier 3). DeflectVelocity
  (`0x005281cf`) is the only writer apart from the constructor, which stores
  0 at `0x0052556a`. It stores GetTurnRadius(point, dir) * d there, then
  reads the member back for the abs test and into k. Nothing else reads +0x4e8
  on a Vehicle. The name describes the stored value. Its purpose beyond
  DeflectVelocity is not shown, hence tier 3.
- SoultreePhysicsBaseObjectCore `field_0x28` -> `inverseLoadMass` (tier 3).
  Bike::LoadBike stores 32.2f / loadWeight there (or 1.0f when loadWeight is
  0), right after riderMass = loadWeight * 0.0310559f, so the value is
  1 / riderMass. The constructors store 1.0f. It is the load-weight
  counterpart of invMass (+0x24 = 1 / bodyMass, UpdateMass `0x005008d0`).
  The member is write-only, but the value is derived from the named
  loadWeight, which is the Overlay `pixelFormat` rule. Hence tier 3.
- VideoCard `field_0x14[16]` -> `textureKindCost` (tier 3). Its only reader is
  the texture memory report in TextureMapManager (`0x005116ce`,
  `mov ecx,[eax+ebx*4+0x14]`). The report indexes it with the texture kind:
  bits 6/4/2 for the size, bit 0 for mips and bit 3 for 8-bit. It adds the
  value to `cost`. No writer outside the constructor was found. The indexed
  store at `0x004c9703` writes a 36-byte record of a PCVideoCard list, not
  this member.

### Proof

One proof from 38d2a74e to the renamed tree (work/naming/w30/prove.sh all)
covered every unit whose closure reaches a changed file: 471 units. A member,
parameter or local name is not part of any decorated name, so the map is
empty (work/naming/w30/map.json is `{}`).

| Verdict | Units |
| --- | --- |
| IDENTICAL | 458 |
| IDENTICAL-COMDAT-ORDER | 13 |
| NAME-ONLY / NAME-ONLY-LABELS | 0 |
| DIFF | 0 |

The 13 IDENTICAL-COMDAT-ORDER units are:

- BikeRaceNearMisses.cpp, DlgProcsNearMisses.cpp,
  ProCircuitProcsNearMisses.cpp and SelectGamePicProcsNearMisses.cpp;
- Group4_DlgMain.cpp, Group4_DlgPCBunny.cpp and Group5_PCBailoutDlg.cpp;
- VisibilityQuadTree.cpp;
- DlgProcs.cpp, KrustyUI.cpp, OptionProcs.cpp, ProCircuitProcs.cpp and
  SelectGamePicProcs.cpp.

tests/fixtures/bvh_vector_compare.cpp includes a header that its test writes,
so it does not compile on its own in either tree. It was compiled in both
trees with the same stub `bvh_vector_patches.h`, as in Wave 26
(work/naming/w30/bvhstub.sh), and it is IDENTICAL. It is counted among the
458 above.

Negative control (work/naming/w30/negcontrol.sh, negmutate.py): two wrong
edits were made in the new tree only.

- In Vehicle::DeflectVelocity, `scaledTurnRadius = ...` became
  `float scaledTurnRadius = ...`. A local now shadows the renamed member,
  which is what a name collision would do.
- In Bike::LoadBike, the inverseLoadMass store became 32.0f / loadWeight
  instead of 32.2f / loadWeight.

Both units gave DIFF, in the `.text` bytes of
`?DeflectVelocity@Vehicle@@UAEMPAUVec3@@0MM@Z` and of `?LoadBike@Bike@@...`.

Pins: no unit is NAME-ONLY and no object digest changed, so nothing was
repinned (work/naming/w30/pinmods.py: 0 changed digests).

Tests: work/naming/w30/pick_tests.py selected 131 modules. It matches the
changed files' basenames and the renamed spellings. All 131 pass
(work/naming/w30/tests-picked.log). tools/static_check.py passes.

### Members

| Owner | Old | New | Tier | Evidence |
| --- | --- | --- | --- | --- |
| BikeRaceBikeRecord | `field_0x04` | `displayName` | 2 | Both views read the same retail array g_TrackGame->ui->bikes (BikeRace.cpp:665, Group6_BikeRaceCreateRacers.cpp:109 vs DlgProcs.cpp:1096, Group2_MPBikeRiderDlg.cpp:79); KrustyUIBikeRecord (SelectGamePicProcs.h:93) names +0x04 `char displayName[0x44]` ("BikeName_%02d") and agrees at +0x00 manufacturer, +0x8c engineSize, +0x90 fourStroke; the pad is exactly 0x44 bytes. Note pairmatch conflict=1: BikeRaceBikeRecord calls +0x48 `bikeName` where KrustyUIBikeRecord calls it `texture` ("BikeTexture_%02d") - a naming disagreement at the same offset, not a layout shift. Keep `unsigned char[0x48 - 0x04]`; comment `// +0x04 KrustyUI "BikeName_%02d" (bytes kept)`. No code uses. |
| CollisionMeshBody | `field_0x00` | `sceneNode` | 3 | Third view of the type-2 mesh payload. Its +0x04 boxTree and +0x08 bodyTransform agree with CollisionMeshShapeBlock. The +0 role is set by the SyncTransformFromNode reader at 0x00435f75 (GetMatrixIn 0x004fca80 on it). No code use spells it; it is the declaration only. The type stays void*. |
| CollisionMeshShapeBlock | `field_0x00` | `sceneNode` | 3 | The only retail reader is SyncTransformFromNode 0x00435f10 case 2 (shapeType 2 = mesh): `mov ecx,[shape]` at 0x00435f75, then it calls SoultreeObject::GetMatrixIn 0x004fca80 with that value as `this`. The hull payload's +4 and the model payload's +0x10, read the same way in cases 0 and 1, are already named `sceneNode` (CollisionHullShape, CollisionModelShape). MeshNodeView in the same file names this offset `node`. SetMeshShape 0x00432ab0 stores 0 (0x00432ae2), and I found no other writer: one reader, so this is close to tier 3. |
| CollisionMeshShapeData | `field_0x00` | `sceneNode` | 3 | Another view of the same type-2 mesh payload. Its +0x04 boxTree agrees with CollisionMeshShapeBlock and CollisionMeshBody. Same evidence as CollisionMeshShapeBlock::field_0x00: GetMatrixIn 0x004fca80 is called on [shape+0] in SyncTransformFromNode 0x00435f75. Only the declaration spells it. |
| CollisionVegetation | `field_0x0e` | `cellY` | 2 | CollisionVegetation is 0x00438e70's view of a Vegetation. Vegetation's base UnknownEcoQuadTreeObject is 0xc bytes, followed by EcoQuantizedPosition quantizedPosition {x, y, z} at +0x0c/+0x0e/+0x10 (Vegetation.h:35, EcoCoordinates.h). This view already names +0x0c cellX and +0x10 cellZ, which both agree with x and z (both are scaled by the EcoSystem cell size at CollisionObject.cpp:1494-1495). So +0x0e is quantizedPosition.y, read as y in Group4_EcoSystem.cpp:290/745. The name follows the view's cellX/cellZ pattern. Declaration only. |
| Cube | `field_0x10` | `headerFlags` | 2 | Cube::LoadTexture 0x0043d460 reads it (0x0043d5f9 `test byte ptr [esi+0x10],2`), and bit 1 gates SkipToSector(textureStream, alignmentBase). That is the same header-flag bit that Load 0x0043d230 tests (0x0043d3ae) before the same SkipToSector. The member sits in the file-header slot between fileFormat (+0xc) and field_0x12. Retail Load reads the flags into a stack local (0x0043d26a lea [esp+0x20]), so the member keeps Reset's 0 (0x0043d110). The obvious name `flags` would clash with Load's local `flags`. |
| EcoLightEmitterView | `field_0x64` | `lightPosition` | 2 | The view is the LightEmitter LightManager::FindLightOfType 0x004a0190 returns in EcoSystem::LightingChanged 0x0045a9a0 (EcoSystem.cpp:946/956/958). It agrees with LightEmitter at +0x54 colour[4] (= ambientRGBA float[4]) and +0x70 direction (= lightDirection Vector3); LightEmitter +0x64 is `Vector3 lightPosition`, exactly 12 bytes (0x64..0x70). Keep the array; comment `// +0x64 LightEmitter lightPosition (bytes kept)`. No code uses; `lightPosition` elsewhere is LightEmitter's own member and locals in D3DIMSoulTree draw code, none in EcoSystem.h/.cpp. |
| EventKrustyBikeView | `field_0x5c8` | `loadInfo` | 2 | Exact 24-byte pad = Bike +0x5c8 `BikeA5C8 loadInfo` (BikeCore.h:132; sizeof BikeA5C8 = 24, the loader's info block copied by Bike::LoadBike 0x005498b0 path). The view is the racer KrustyBike and agrees with Bike/KrustyBike at riderCharacter +0x5c4, racerName +0x5e0 (= riderName[16]), frontWheel +0x5f0, outOfRace +0x4a0, vehicleModel +0x3bc and further offsets (viewmatch agree=47). Keep `unsigned char[0x5e0 - 0x5c8]`; comment `// +0x5c8 Bike loadInfo (BikeA5C8) (bytes kept)`. No code uses. |
| GridBaseCell | `field_0x7_b0` | `flipDiagonal` | 3 | Retail reads it only in DrawableGridNode::EmitLeafEdgeFan 0x0047fe70 (0x0047ff82, 0x00480122 `test byte ptr [cell*8+7],1`). There the bit (or 1 - bit for the opposite neighbour cell), combined with vertexFlags & 0x80, decides whether the fan's third vertex shifts to the diagonal corner. So it selects which way the cell's split diagonal runs. Tier 3: one function. Terrain::QueryGround uses a checkerboard instead, so collision code does not confirm the bit. |
| GridBlockRecord | `field_0x08` | `ageEntry` | 2 | Grid1.h's Grid1BlockTexture is the same 0x2c-byte record at GridNodeExtra::blockRecords and names +0x08 `AgeEntry ageEntry` (0x14 bytes, matching the +0x08..+0x1c pad). It is registered with textureAgeManager in Grid1NearMisses.cpp:172 and cleared at Grid1.cpp:67. The views agree on texture +0x04, mappingScale/OffsetU/OffsetV +0x1c..+0x24 and detail +0x28. |
| GridNodeExtra | `field_0x14` | `ageEntry` | 2 | Grid1.h's Grid1NodeExtra, a view of the same DrawableGridNode+0x3c block, names +0x14 `AgeEntry ageEntry` (0x14 bytes = exactly the +0x14..+0x28 pad). That view registers the entry with textureAgeManager (Grid1NearMisses.cpp:254), marks it used (Grid1.cpp:135) and reads its size field (retail 0x0047d325 [extra+0x24]). The views agree on byteCount, data, texture, textureReference, builtTexture, mappingScale/OffsetU/OffsetV, detail and blocks. Keep the type; only the identifier changes. Same name as the precedent at Terrain.h:125. |
| GridNodeExtra | `field_0x3c` | `textureFlags` | 2 | The Grid1NodeExtra view of the same block names +0x3c `int textureFlags`. Bit 0x10000 means the node texture is built and bit n means block n is built. Retail 0x0047caa0 reads and writes it, as do Grid1.cpp:55/66 and Grid1NearMisses.cpp:141/160/169/252. The alignment checks out against the other named offsets (see ageEntry above). |
| Group1ParticleCamera | `field_0x194` | `roll` | 2 | Cast directly from Camera*; agrees with Camera at screenMatrix +0xec, eyePosition +0x170, viewDirection +0x17c, upDirection +0x188, focalLength +0x198, nearPlane +0x1bc, farPlane +0x1c0, targetWidth +0x1c4, targetHeight +0x1c8. Camera +0x194 is `roll`; precedent Krusty3DObjects.h:50 NumberCameraView `unsigned char roll[0x198 - 0x194]; // +0x194 Camera roll (bytes kept)`. Keep `char[0x198 - 0x194]`, same comment. No code uses; no `roll` identifier in the file. |
| GuiControlView | `field_0x15c` | `italic` | 2 | Exact-size (4-byte) view pad of UIControl +0x15c `italic` (GameUi.h:308); GuiControlView agrees with UIControl at controlType 0x5c, ownerDialog 0xb8, ownerGui 0xbc, toolTipText 0xf0, fontFace 0x130, bold 0x158 and fontHeight 0x160 (bytes kept, W27/W28 precedent). No code uses. |
| HullNodeView | `field_0x00` | `swept` | 2 | HullNodeView is SyncTransformFromNode's (0x00435f10) view of the type-0 hull payload. It reads only +4 (`node`, 0x00435f2f). The other views of the same payload, CollisionHullShape (CollisionObject.h:53-54) and CollisionHullBody (CollisionShapeTests.h:35-36), name +0x00 `swept` and +0x04 `sceneNode`, and their +4 agrees with this view's node. SetHullShape 0x00432720 stores its `swept` argument there, and HullVsHull 0x00436720 branches on it. |
| KbEffectParams | `field_0x0` | `angle` | 2 | This is the direction array for joystick slot 7, SetConstantForce (PCJoystickDevice.cpp:344). It forwards to SetDirection 0x004c3db0, which copies direction[0..1] into the DIEFFECT rglDirection with cAxes 2 and dwFlags 0x22 (DIEFF_OBJECTOFFSETS\|DIEFF_POLAR). By SDK position, element 0 of a polar direction is the angle in hundredths of a degree. KrustyBike::UpdateForceFeedback passes 0x4650 = 18000 = 180 degrees (KrustyBike.cpp:668). It is used only through the brace initializer, so it is the declaration only. |
| KrustyUI | `field_0x480` | `riderPosition` | 2 | One Vector3 (12 bytes, not a multi-member pad): BuildGarageScene 0x00498cf0 stores kVec3Zero with y += 0.1 there (0x00498ede..0x00498f08) and passes &this+0x480 (0x004995c9 lea [ebp+0x480]) to the "UIRider.mcf" rider model's SoultreeObject::SetPosition(const Vector3* position) 0x004fc660; KrustyUI.cpp's own comment and the KRUSTYUI_RIDER_POSITION macro already call it the rider's position. |
| MovingPart (Suspension.h) | `field_0x00` | `matrix` | 2 | Second declaration of the same MovingPart (sceneNode +0x40, ownerRef +0x44 agree with MovingPart.h); +0x00 is MovingPart.h `Matrix4 matrix` filled by ctor 0x004a23a0 through GetLocalMatrix 0x004fca60 and read back by the shock path (Group3_KrustyBikeAISupport.cpp:182 SetLocalMatrix). Keep `char[0x40]`; same comment. No code uses; no `matrix` identifier in Suspension.h/.cpp. |
| MovingPart (Tire.h) | `field_0x00` | `matrix` | 2 | Same retail class as the canonical src/reconstructed/MovingPart.h, which names +0x00 `Matrix4 matrix` (the node's local matrix): ctor 0x004a23a0 calls node->GetLocalMatrix(&matrix) (SoultreeObject::GetLocalMatrix 0x004fca60) into +0x00; agrees at +0x40 sceneNode and +0x44 ownerRef. The matrix has a reader: Group3_KrustyBikeAISupport.cpp:182 G3ShockView passes its MovingPart localMatrix to sceneNode->SetLocalMatrix. Keep `char[0x40]`; comment `// +0x00 the node's local matrix at construction (MovingPart.h matrix) (bytes kept)`. No code uses; no `matrix` identifier in Tire.h/Tire.cpp (comment-only hits at Tire.h:57, Tire.cpp:472). |
| Particle | `field_0x24` | `screenRect` | 2 | ParticleManager Render slot 14 0x004bac60 stores the projected sprite's top/bottom/left/right floats at +0x24/+0x28/+0x2c/+0x30 (0x004bae2f, 0x004bae4f, 0x004badfa, 0x004bae19) and reads them back for clipping, all through the one 16-byte Group1ParticleRect {top,bottom,left,right} view (Group1_ParticleRender.cpp:100/136). It is a single rect, not a multi-member pad. No local or member named screenRect exists in the particle files; the hits for that name are TestBox parameters in the visibility/shadow code. |
| ProCircuitRacerResultView | `field_0x40` | `name` | 2 | The view is cast from EventManager results, which are RacerResult (samples/ui/ProCircuitProcsNearMisses.cpp:94, src/bringup/Group0_PCLastRaceDlg.cpp:47). It agrees with RacerResult at +0x00 racer (= playerId), +0x04 position, +0x28 points, +0x34/0x38/0x3c repairs/medical/stunts (= repairDamage/injuryDamage/stuntEarnings); RacerResult +0x40 is `char name[16]`, identical type and size. No code uses, so the locked ProCircuitProcs bodies (0x004d72c0/0x004d8fc0/0x004d9860) are untouched. Comment `// +0x40: the racer's name (RacerResult name)`. |
| RacerD3DIMSoultreeObjectView | `field_0x278` | `lowestLod` | 2 | Exact 4-byte pad = D3DIMSoultreeObject +0x278 `int lowestLod` (D3DIMSoulTree.h:301). D3DIMSoultreeObject::SetLowestLod 0x00444d40 writes [ecx+0x278] (0x00444d57) and SetLod 0x00444d00 clamps the requested level from below with it. The view agrees at +0x274 detailLevelCount (= lodCount), +0x27c currentDetailLevel (= currentLod), +0x280 detailDistances (= autoLodDistances), +0x28c levelGroups (= lodTable). Keep the array type; comment `// +0x278 D3DIMSoultreeObject lowestLod (bytes kept)`. No code uses. |
| SelectGameD3DIMSoultreeCharacterView | `field_0x008` | `sltFile` | 2 | The models are `new D3DIMSoultreeCharacter(0)` (KrustyUI.cpp:388). The view agrees with D3DIMSoultreeCharacter at +0x0c motionFinished (= motionStopped), +0x1a0 plateTexture (= modelNode, cast to D3DIMSoultreeObject* in DlgProcs.cpp:1128/1172/1180, SelectGamePicProcs.cpp:2014/2261/2269) and the vfptr/vbptr-to-virtual-GameObject layout. D3DIMSoultreeCharacter.h:121 names +0x008 `ParameterBlock* sltFile` (LoadCharacter 0x00445680 creates it, D3DIMSoultreeMotnctrl.cpp:430). Keep the array; comment `// +0x08 D3DIMSoultreeCharacter sltFile (bytes kept)`. No code uses. |
| SelectGameRaceSettingsView | `field_0x08` | `series` | 2 | The view is cast from &g_TrackGame->mode.raceSettings.raceMode (DlgProcs.cpp:558/826, SelectGamePicProcs.cpp:1499/1574/1755); TrackGame.h RaceSettings names +0x08 `series` (TrackGame+0x2d78, also the `series : 3` bitfield of LobbySettingsMessage at SelectGamePicProcs.h:160). The view agrees at 15 offsets (raceMode, eventType, races, treeCollision, laps, opponents, multiplayerAiRacerCount, gateSeed, trackNumber, trackName, minutes, tagBall, stuntMode, ...). Keep the array; comment `// +0x08: series (+0x2d78) (bytes kept)`. No `settings->field_0x08` use exists, so the locked SelectGamePicProcs 0x004f8820 body is untouched (declaration only). |
| SoultreeObject (D3DIMSoulTree.h view) | `field_0x148` | `prevSibling` | 2 | The canonical src/krusty2/core/SoultreeObject.h:326 names +0x148 `SoultreeObject* prevSibling`, which AppendSibling and RemoveChild link (soultree.cpp:703-726). This view agrees on firstChild +0x140, nextSibling +0x144, hidden +0x14c, boundsFound/localBounds* +0x150..+0x164 and parameterBlock +0x1a0. The 4-byte pad covers exactly one pointer. |
| SoultreePhysicsBaseObjectCore | `field_0x28` | `inverseLoadMass` | 3 | Bike::SetLoadWeight-path (Bike.cpp:620-624) stores 32.2f / loadWeight (1 / riderMass, riderMass = loadWeight * 0.0310559 = loadWeight / 32.2), else 1.0f; ctors store 1.0f; write-only, value derived from the named loadWeight, beside invMass (+0x24 = 1 / bodyMass) |
| StartObjectColliderBase | `field_0x08` | `objectTypeId` | 2 | StartObjectCollider is StartObjectCharacterView +0x210 collisionObject, which krusty2 CollisionCharacter.h:39 types `CollisionObject*` (0xb8 bytes): QuadTreeObject at +0 and GameObject at +0xc, the same split as StartObjectCollider (StartObjectColliderBase, GameObject at +0xc). QuadTreeObject +0x08 is `objectTypeId` (CollisionObject.h:46; ctor 0x00431e70 stores TypeRegistry::FindTypeId, 0x00438e70 compares [obj+8]); the int covers that char plus padding up to the GameObject base, as BikeQuadTreeHead (Bike.cpp:561) and CarProceduralNode already name it (pairmatch agree=13, conflict=0). No code uses. field_0x04 stays (spans queryStamp + sortKey). |
| Terrain | `field_0x84` | `runMeshChecks` | 3 | It has one reader, Terrain::PrepareGeometry (0x0050789e). When nonzero, it gates GridRoot()->TestDroppableVertices() 0x00481580 and CountEdgeDifferences() 0x004813e0, a per-frame mesh consistency pass whose results nothing reads. The constructor (0x0050587b) is the only writer and stores 0, so the checks are off in retail. Wave 27 (leftB2) missed this reader. The name rests on that single gate; tier 3 is also defensible. |
| TrackGameTagBallView | `field_0x10a` | `asleep` | 2 | The tag ball is a SoultreePhysicsBaseObject: retail reads its +0x128 collisionObject at 0x0048dae6, and its SoultreeObject subobject sits at +0x21c, where the base's own part ends. SoultreePhysicsBaseObjectCore.h:211 names +0x10a `asleep`: UpdateRestState 0x00502c40 sets it, and GameObject slot 10 (0x005036f0) skips the update while it is set. SetFollowedRacer 0x004a9e80 clears it to wake the ball. The bring-up body of the same function, src/bringup/Group3_FollowedRacer.cpp:63, already spells it `ball->asleep = 0`. The declaration comment can say: 'SoultreePhysicsBaseObject asleep; SetFollowedRacer 0x004a9e80 clears it (wakes the ball)'. |
| Vehicle | `field_0x4e8` | `scaledTurnRadius` | 3 | DeflectVelocity 0x005281cf stores GetTurnRadius(point, dir) * d there and reads it back for the abs test; ctor 0x0052556a stores 0; no other reader |
| VideoCard | `field_0x14` | `textureKindCost` | 3 | Only reader TextureMapManager 0x00511580 (0x005116ce mov ecx,[eax+ebx*4+0x14]) indexes it with the texture kind (size bits 6/4/2, mip bit 0, 8-bit bit 3) and sums it into the texture cost; no writer found outside the ctor |
| VisualCueKrustyBikeView | `field_0x08` | `sceneNode` | 3 | The objects are the racers BikeRace::NextBike 0x004204e0 returns (VisualCue CollectRacers 0x0048af20), i.e. KrustyBike: vfptr +0, vbptr +4 to virtual GameObject (isEnabled read through it), position Vector3 +0x0c = SoultreePhysicsBaseObject position. SoultreePhysicsBaseObjectCore.h:175 names +0x08 `SoultreeObject* sceneNode`, and the sibling view RaceView.h:102 EventKrustyBikeView already spells this exact pad `sceneNode[0x0c - 0x08] // +0x08 KrustyBike sceneNode (bytes kept)`. Keep the array; same comment. Only one named data offset (+0x0c) agrees directly; the rest is the shared vfptr/vbptr layout and the precedent view. No code uses. |

### Parameters

| Owner | Old | New | Tier | Evidence |
| --- | --- | --- | --- | --- |
| BackgroundImage::GetDc | `a` | `more` | 2 | 0x004049d0 stores 0 then 1 when it returns the copy surface / a further dirty region (samples/render/BackgroundImageNearMisses.cpp:36/43/70) so the caller draws again; UIDialog::BeginControlDraw forwards its `more` out-parameter here. Near miss 108/606; param rename is codegen-neutral. |
| GameObject::OnChar / PCGame::OnChar / BikeRace::OnChar | `value` | `character` | 2 | Same virtual slot as UIControl::OnChar (character, below): GameObject 0x00469c00 passes it to each child's OnChar, PCGame 0x004c0340 to the root object's, BikeRace 0x0041f590 to ChatOverlay::HandleChatKey |
| GameObject::OnKeyDown / PCGame::OnKeyDown | `value` | `key` | 2 | Same virtual slot as UIControl::OnKeyDown (key): GameObject 0x00469c40 passes it to each child's OnKeyDown, PCGame 0x004c0370 to the root object's |
| KrustyUI::Create | `value` | `showIntro` | 2 | Declaration only: the exact definition 0x004988a0 (KrustyUI.cpp:187) already names the parameter showIntro and opens Intro1Dlg only when it is set (KrustyUI.cpp:257). |
| KrustyUID3DIMSoultreeCharacterView::AdvanceMotion | `a` | `mirror` | 2 | View declaration of Character::AdvanceMotion 0x004a6bb0, canonically (float dt, int mirror, int mask) in D3DIMSoultreeCharacter.h:113 / Motnctrl.cpp:508 and (float time, int mirror, int mask) in SceneManager.h:44. Declaration only. |
| KrustyUID3DIMSoultreeCharacterView::AdvanceMotion | `b` | `mask` | 2 | Same as `mirror`: third parameter of 0x004a6bb0, `mask` in D3DIMSoultreeCharacter.h:113 and SceneManager.h:44. Declaration only. |
| KrustyUID3DIMSoultreeCharacterView::LoadCharacter | `a` | `useOptions` | 2 | View declaration of D3DIMSoultreeCharacter slot 11 0x00445680, canonically (int context, const char* path, const SoultreeLoadDesc* desc, int loadInfo, int useOptions, int unused) (D3DIMSoultreeCharacter.h:182; useOptions is forwarded to LoadModel's useOptions, Group5_D3DIMForwarders.cpp:30-32). Declaration only. |
| KrustyUID3DIMSoultreeCharacterView::LoadCharacter | `b` | `unused` | 2 | Sixth parameter of 0x00445680, `unused` in the canonical D3DIMSoultreeCharacter.h:182 declaration. Declaration only; no `unused` identifier in KrustyUI.cpp code. |
| KrustyUISoultreeObjectView::SetAxes | `a` | `zx` | 2 | View declaration of SoultreeObject::SetAxes 0x004fbd10 (?SetAxes@SoultreeObject@@QAEXMMMMMMHH@Z), whose canonical declaration (src/krusty2/core/SoultreeObject.h:229, D3DIMSoulTree.h:80) names the parameters zx, zy, zz, yx, yy, yz, orthogonalize, keepZ. Declaration only (callers use SoultreeObject directly). The view's 7th parameter `g` is typed float but retail's is int `orthogonalize`; left (see leftA). |
| KrustyUISoultreeObjectView::SetAxes | `b` | `zy` | 2 | View declaration of SoultreeObject::SetAxes 0x004fbd10 (?SetAxes@SoultreeObject@@QAEXMMMMMMHH@Z), whose canonical declaration (src/krusty2/core/SoultreeObject.h:229, D3DIMSoulTree.h:80) names the parameters zx, zy, zz, yx, yy, yz, orthogonalize, keepZ. Declaration only (callers use SoultreeObject directly). The view's 7th parameter `g` is typed float but retail's is int `orthogonalize`; left (see leftA). |
| KrustyUISoultreeObjectView::SetAxes | `c` | `zz` | 2 | View declaration of SoultreeObject::SetAxes 0x004fbd10 (?SetAxes@SoultreeObject@@QAEXMMMMMMHH@Z), whose canonical declaration (src/krusty2/core/SoultreeObject.h:229, D3DIMSoulTree.h:80) names the parameters zx, zy, zz, yx, yy, yz, orthogonalize, keepZ. Declaration only (callers use SoultreeObject directly). The view's 7th parameter `g` is typed float but retail's is int `orthogonalize`; left (see leftA). |
| KrustyUISoultreeObjectView::SetAxes | `d` | `yx` | 2 | View declaration of SoultreeObject::SetAxes 0x004fbd10 (?SetAxes@SoultreeObject@@QAEXMMMMMMHH@Z), whose canonical declaration (src/krusty2/core/SoultreeObject.h:229, D3DIMSoulTree.h:80) names the parameters zx, zy, zz, yx, yy, yz, orthogonalize, keepZ. Declaration only (callers use SoultreeObject directly). The view's 7th parameter `g` is typed float but retail's is int `orthogonalize`; left (see leftA). |
| KrustyUISoultreeObjectView::SetAxes | `e` | `yy` | 2 | View declaration of SoultreeObject::SetAxes 0x004fbd10 (?SetAxes@SoultreeObject@@QAEXMMMMMMHH@Z), whose canonical declaration (src/krusty2/core/SoultreeObject.h:229, D3DIMSoulTree.h:80) names the parameters zx, zy, zz, yx, yy, yz, orthogonalize, keepZ. Declaration only (callers use SoultreeObject directly). The view's 7th parameter `g` is typed float but retail's is int `orthogonalize`; left (see leftA). |
| KrustyUISoultreeObjectView::SetAxes | `f` | `yz` | 2 | View declaration of SoultreeObject::SetAxes 0x004fbd10 (?SetAxes@SoultreeObject@@QAEXMMMMMMHH@Z), whose canonical declaration (src/krusty2/core/SoultreeObject.h:229, D3DIMSoulTree.h:80) names the parameters zx, zy, zz, yx, yy, yz, orthogonalize, keepZ. Declaration only (callers use SoultreeObject directly). The view's 7th parameter `g` is typed float but retail's is int `orthogonalize`; left (see leftA). |
| KrustyUISoultreeObjectView::SetAxes | `h` | `keepZ` | 2 | View declaration of SoultreeObject::SetAxes 0x004fbd10 (?SetAxes@SoultreeObject@@QAEXMMMMMMHH@Z), whose canonical declaration (src/krusty2/core/SoultreeObject.h:229, D3DIMSoulTree.h:80) names the parameters zx, zy, zz, yx, yy, yz, orthogonalize, keepZ. Declaration only (callers use SoultreeObject directly). The view's 7th parameter `g` is typed float but retail's is int `orthogonalize`; left (see leftA). |
| Overlay::Attach | `a12` | `textureContext` | 2 | 0x004b5f50 forwards it as TextService::Create's last argument, i.e. FontTexture::Load's textureContext (all callers pass 0: TrackOverlay.cpp:108, 270, 504, 1231, 1522, 1532, 1541). Exact. |
| PCSoundInterface::InitializeSound | `value` | `budget` | 2 | 0x004be5a0 passes it to SoundMemoryManager::StartLoaderThread(int budget) (PCAudio.cpp:1326, 1144), which stores it as byteBudget; callers pass 4000000 (GUIManager.cpp:194, TrackGame.cpp:244). Exact; `budget` is otherwise only StartLoaderThread's parameter. |
| PCVideoCard::SetDisplayMode | `a` | `bufferCount` | 2 | 0x004c9ec0 passes it straight into FindDisplayMode's bufferCount parameter (PCVideoCard.cpp:272). PCGame passes 2 (PCGame.cpp:519). The pure-virtual base declaration VideoCard.h:20 should be renamed with it. The body has no other `bufferCount`. The trailing `c` is unused and stays. |
| RaceSound::PlayScoreSounds | `value` | `score` | 2 | KbRaceHandler::ReportTrickScore(KrustyBike* who, float score) passes its score (src/bringup/Group5_KbRaceHandler.cpp:20); 0x004e57d0 compares it with 10000.0f to play bonus.wav for the listened racer. Exact; no `score` identifier in RaceSound. |
| RaceSound::PlayScoreSounds (bring-up redeclaration) | `value` | `score` | 2 | The same function 0x004e57d0 as RaceSound.h (score) |
| SoultreeQuadTreeRenderer::OnControlPress | `a` | `event` | 2 | This is the slot 23 override (0x005048d0, expect exact in src/krusty2/soultree/targets.json) of GameObject::OnControlPress(ControlEvent* event, InputButtonState* entry) (GameObject.h:72). It passes `a` to TestInputEvent's event parameter and forwards both to GraphicsTest::OnControlPress. Neither SoultreeQuadTreeRenderer nor GraphicsTest/GameObject has a member named event or entry. |
| SoultreeQuadTreeRenderer::OnControlPress | `b` | `entry` | 2 | This is the slot 23 override (0x005048d0, expect exact in src/krusty2/soultree/targets.json) of GameObject::OnControlPress(ControlEvent* event, InputButtonState* entry) (GameObject.h:72). It passes `a` to TestInputEvent's event parameter and forwards both to GraphicsTest::OnControlPress. Neither SoultreeQuadTreeRenderer nor GraphicsTest/GameObject has a member named event or entry. |
| TextService::Create | `a10` | `textureContext` | 2 | 0x0050ae80 passes it unchanged as FontTexture::Load's `textureContext` (FontTexture.h:47-48, named in b0754c51; TerrainLoadTexture casts it to TextureContext*). Wave 24 left it as never interpreted. Exact; no textureContext identifier in TextService/Overlay. |
| UIControl::KillFocus (+ overrides UIEditBox, UIDDLScrollBar, UIDDLStatic, UIDDLButton, UIDDLListBox, UIDropDownList) | `value` | `next` | 2 | GUIUser::MoveFocus 0x00487870 calls focusControl->KillFocus(control) with the control receiving the focus (GUIManager.cpp:1076; 0 at 1159); GuiControlView::KillFocus(GuiControlView* next) names it; the drop-down parts' overrides (0x004796e0, 0x00479bc0, 0x00479ce0, 0x00479e70, 0x0047a7d0) test ownerList->IsPart((UIControl*)value) to close the list only when the focus leaves it. All exact; UIEditBox 0x004740b0 forwards it. |
| UIControl::LeaveHover | `value` | `next` | 2 | GUIUser::SetHoverControl 0x00487800 (GUIManager.cpp:1057, 1074) calls hoverControl->LeaveHover(control) with the control taking the hover; GUIManager.h:170 GuiControlView::LeaveHover(GuiControlView* next) already names this slot-32 parameter `next`. The body 0x00472630 ignores it. Exact; no member/local `next` in UIControl (GameObject has nextSibling; UpdateHoverAt's local `next` is another function). |
| UIControl::OnChar | `value` | `character` | 2 | 0x00472320 receives the WM_CHAR wParam (Main.cpp:236 via g_TrackGame->OnChar -> GameObject::OnChar children), copies it into DialogEvent::key for kind 0xb and forwards it to GameObject::OnChar. Exact (not in NEAR_MISS_INDEX); no `character` identifier in GameUi/UIDialog/GameObject. |
| UIControl::OnKeyDown | `value` | `key` | 2 | 0x004723d0 receives the WM_KEYDOWN wParam (virtual key), copies it to DialogEvent::key and forwards it to GameObject::OnKeyDown; the sibling override UIListBox::OnKeyDown (GameUi.h:492, 0x00478040) already names it `key`. `event.key = key;` stays unambiguous. Exact. |
| UIDialog::BeginControlDraw | `a` | `more` | 2 | 0x0046ed70 clears *a then forwards it to BackgroundImage::GetDc, which sets it when another surface/dirty region remains; every caller passes &more and loops `while (more && BeginControlDraw(...))` (GameUi.cpp:4313/4343, 4712/4740/4860, 5091/5118). Exact; no `more` in UIDialog. |
| UIEditBox::OnChar | `value` | `character` | 2 | 0x00474150 (near miss 1018/1044, body in samples/ui/EditBoxOnChar.cpp) narrows it to `char c` and forwards it to UIControl::OnChar; it is the WM_CHAR character as for UIControl::OnChar. Parameter renames do not change the mangled name or codegen. |
| UIEditBox::OnKeyDown | `value` | `key` | 2 | 0x00474570 switches on it over VK_HOME/VK_END/VK_DELETE/VK_LEFT/VK_RIGHT after UIControl::OnKeyDown(value); same name as UIListBox::OnKeyDown(int key). The `char key` local at GameUi.cpp:4227 is in another function. Exact. |
| VideoCard::FindDisplayMode | `a` | `bufferCount` | 2 | The body of 0x0052d250 compares it with DisplayMode::bufferCount (`!a \|\| mode->bufferCount == a`, VideoCard.cpp:83), the same any-match pattern used for refreshRate. Callers pass 0 (EventManager.cpp:343) and the 2 that PCGame sends through SetDisplayMode (front plus back buffer). VideoCard has no member or local named bufferCount. tests/fixtures/leave_race_menu_compare.cpp:96 defines its own stub with params (a, b) and is unaffected. |

### Left for wave 31

The full lists, with a reason for each line, are work/naming/w30/leftA.txt
(UI, sound), leftB.txt (render, terrain), leftC.txt (vehicle, physics,
collision) and leftD.txt (game, events, views). These open points need a
decision rather than more evidence:

- VehicleWheelAux `field_0x8c`/`field_0x90` (Vehicle.h:283-284). Retail
  `0x00529a20` copies +0x90 into +0x8c on the shock pointers at wheel+0x2b0
  and +0x2ac. Shock names those offsets prevLength and clearedLength. The same
  struct also types the gearbox pointer at wheel+0x2a8, and
  tests/fixtures/vehicle_gearbox_boundaries.cpp equates the two members with
  EngineGearbox throttleTimeScale/gearCount. The fix is a separate shock view
  for those casts, which is not a rename.
- KbEffectParams `field_0x4` (KrustyBikeTypes.h:462). It is the second
  element of the DIEFF_POLAR direction array: SetDirection `0x004c3db0`,
  flags 0x22, two axes. DirectInput requires that element to be 0. `field_0x0`
  became `angle`; the second element is left until a name is chosen.
- KrustyUISoultreeObjectView::SetAxes `g` (KrustyUI.cpp:56). The view types
  it as `float`, but retail SoultreeObject::SetAxes takes an int there
  (orthogonalize). That needs a type fix, or the unused view declaration
  removed, not a rename.
- BikeRaceBikeRecord `bikeName` (+0x48, BikeRace.h:810). KrustyUIBikeRecord
  fills that offset from "BikeTexture_%02d" (`texture`), so the existing name
  looks wrong. Its size (0x44) also differs from `texture` (0x40).
- Terrain `field_0xbe8`. The level file supplies it, and every reader compares
  it with 1. It drives two unrelated tunings: the Bike nose-over limit and
  the FollowCamera rate, 0.25 or 0.3. The level headers are compressed, so the
  values in use are unknown.
- GameObject `field_0x24`. The GameObject constructor stores 0xff and the
  SoultreeObject constructor stores FindTypeId("SoultreeObject"). These are
  constructor stores only (`objectType` was proposed at tier 3 and not
  taken).
- EventManager RacerStanding `field_0x04`/`field_0x08`. These are comparator
  tie-breakers, but the only sort caller, EndRace, never fills them.
- Constructor-only or write-only members, not named:
  - TrackGameMode `field_0x1bdc`: saved and loaded with the profile, never
    read.
  - NameOverlay `field_0x170`.
  - EngineGearbox `field_0x74`/`field_0x80`/`field_0x14c`/`field_0x1d8`, and
    the `arg6` that the last one is stored from.
  - FollowCamera `field_0x33c`.
  - CarProcedural `a11`-`a15`, and KbBikeLoader `a13`/KrustyBike `a16`,
    which only pass through to LoadBike's unused `a19`.
- Union arms kept by policy:
  - UIEditBox, UIScrollBar and UIListBox `field_0x1ec`/`1f4`/`1f8`/`204`/
    `208`/`218`;
  - GridTerrain `field_0xc9c`/`field_0xca0`, the float arms of
    unlitColor/unlitSpecular.
- Not renamed because of the concurrent matching work:
  - KrustyUI `field_0x4ac`: DlgProcs.cpp may set it inside `0x0044fae0`, and
    nothing reads it.
  - The MorphBastard channel `field_0x5c` (-1), and
    KbMorphMod::LoadParameterFile `a` (MorphBastardModifier `0x004a33b0`).
  - The SceneManager `0x004edfe0` body.

Multi-member pads that need a declaration split, not a rename:

- RaceView, BikeRace and racer views of KrustyBike:
  - EventKrustyBikeView `field_0x018`, `field_0x070`, `field_0x094`, and
    `field_0x488` (takeoffPosition + takeoffVelocity);
  - BikeRaceKrustyBikeView `field_0x070`, `field_0x094`, `field_0x0c0`, and
    `field_0x5c8` (loadInfo + riderName);
  - QuarryLoadKrustyBikeView `field_0x5c8` (Group3_QuarryLoad.cpp,
    QuarryStuntEventLoaderNearMiss.cpp);
  - BikeCameraBikeView `field_0x5c8` (loadInfo .. tailContact);
  - InstrumentKrustyBikeView `field_0x0c0`;
  - VehicleCameraVehicleView `field_0x070`, `field_0x094` and `field_0x0c0`;
  - EventManagerNearMisses.cpp:110 and Group1_PodiumScene.cpp:79
    `field_0x5c8[0x784 - 0x5c8]`;
  - the `field_0x10a[2]` racer/tag-ball view at BikeRace.h:410 (asleep and
    +0x10b).
- Camera views:
  - ArcadeCameraView, BonusCameraView and PickerCameraView `field_0x17c`
    (viewDirection, upDirection and roll);
  - D3DIMSoultreeCameraView `field_0x188` and EcoCameraView `field_0x188`
    (upDirection + roll);
  - EcoCameraView `field_0xec` (screenMatrix + 0x12c..0x170);
  - Group1ParticleCamera `field_0x000`, `field_0x12c` and `field_0x19c`;
  - BikeRaceKrustyBikeCameraView and QuarryLoadKrustyBikeCameraView
    `field_0x398` (cachedTargetHeading + cachedTargetPoint);
  - QuarryLoadKrustyBikeCameraView `field_0x02c`.
- SoultreeObject views:
  - StartD3DIMSoultreeObjectView `field_0x278` (lowestLod .. useAutoLod);
  - RacerD3DIMSoultreeObjectView `field_0x284`;
  - D3DIMSoulTree.h `field_0x194` (inQuadtree, quadtreeCell,
    loadedNodeCount);
  - SelectGameD3DIMSoultreeCharacterView `field_0x010`;
  - SceneSoultreePhysicsBaseObjectView and SceneSoultreePhysicsObjectView
    `field_0x0c`.
- Race settings and results:
  - SelectGameRaceSettingsView `field_0x14` and `field_0x14c`;
  - KrustyVCRHeader (DlgProcs.h) `field_0x40`, `field_0x58` and `field_0x61`;
  - KrustyUI `field_0x4b4` and `field_0x674` (a KrustyVCRHeader copy around
    gridOrder);
  - ProCircuitRacerResultView `field_0x08` and `field_0x2c`;
  - EventManager `field_0x444` (the AI racer table);
  - TrackGameRaceGateView `field_0x00`;
  - SceneCarProceduralView `field_0x40`.
- UI:
  - GuiControlView `field_0x60`, `field_0xc0`, `field_0xf4` and
    `field_0x164`;
  - CursorUIAnimView `field_0x00` and `field_0x14`;
  - KrustyUIToolTipView `field_0x00`;
  - the Instrument, Chat, Stats, RadarGateEvent and NameOverlayEvent view
    pads;
  - DDSurfaceDesc2 `field_0x28` (the four DDCOLORKEYs) and `field_0x1c`
    (dwAlphaBitDepth + dwReserved).
- Render and terrain:
  - Particle corner `field_0x0c` (rhw .. v of the FVF 0x1c4 vertex);
  - GridTerrain `field_0x540`, `field_0xc18` and `field_0xc90`;
  - GridManagedTexture `field_0x00`.
- Quad-tree heads:
  - CarProcedural.h `field_0x04`, Bike.cpp BikeQuadTreeHead `field_0x04` and
    StartObjectColliderBase `field_0x04` (queryStamp + sortKey);
  - CarProcedural model `field_0x38`.
- KbNetState `field_0x00` and `field_0x04`.

## Wave 31: multi-member pads split by the class they view

Wave 31 works through the Wave 30 lists "Multi-member pads that need a
declaration split" and "Left for wave 31". A pad is split only where every
new member has evidence of one of these kinds:

- the canonical class names that offset (CodeView layouts,
  work/naming/cv/layouts.json; work/naming/w31/padmap.py lists the canonical
  members inside each pad);
- another view of the same object names it;
- an SDK layout fixes it.

Gaps without evidence stay as smaller `field_0xNN` pads, so every size and
offset is unchanged. A member whose type the view cannot spell keeps a byte
array named after the member, with "(bytes kept)" (the Wave 27 rule). No
static, global, type of a static, or type with an emitted RTTI descriptor was
renamed. No union arm was touched. No real class changed member order or
size. Some splits are in declarations that are not views: SoultreeObject in
D3DIMSoulTree.h, CarProceduralNode, KbNetState and VehicleShock. They keep
every member offset and the size.

The helpers and the edit specs are in work/naming/w31/:

- edit.py (exact replacement that keeps CRLF) and spec/*.py;
- padmap.py;
- prove.sh, try1.sh and kscan.py (the counter scan below);
- negcontrol.sh and negmutate.py.

Counts over `src` and `samples` (`git grep -ohE`, work/naming/w28/counts.py):

| Placeholder | Before (1011ac65) | After |
| --- | --- | --- |
| `\bUnknownVirtualSlot[0-9]+` | 74 uses, 17 names | 74 uses, 17 names |
| placeholder methods | 85 uses | 85 uses |
| `\bUnknown[A-Za-z0-9_]*` | 376 uses, 74 names | 376 uses, 74 names |
| `\bfield_0x[0-9a-fA-F]+` | 1214 uses, 322 spellings | 1167 uses, 316 spellings |
| `field_0x` in samples/ only | 112 uses | 106 uses |
| `field_0x` declarations (`\bfield_0x[0-9a-fA-F]+(\[[^]]*\])?;`) | 815 | 776 |

The wave adds 127 named member declarations from splits and renames five
members:

- `carriedFlagPosition` -> `centerOfMass`;
- `bikeName` -> `texture`;
- VehicleWheelAux `field_0x8c`/`field_0x90` -> `throttleTimeScale`/`gearCount`;
- KbEffectParams `field_0x4` -> `reserved`.

The splits leave four new residual pads: BikeRace.h and TrackOverlay.h
`field_0x0fc`, GUIManager.h `field_0x126`, and DDSurfaceDesc2
`field_0x28[0x40 - 0x28]`. They also shorten two pads: VehicleShock
`pad_0x80` and Group1ParticleCamera `field_0x12c`. The edited files gain 86
lines.

### Splits

Tier 2 throughout, except where the row says otherwise. "SPBO" is
SoultreePhysicsBaseObject, the KrustyBike/Vehicle base that the canonical
layout puts at those offsets.

| Struct (file) | Old pad | New members | Tier | Evidence |
| --- | --- | --- | --- | --- |
| EventKrustyBikeView (RaceView.h) | `field_0x018[0x50 - 0x18]` | centerOfMass, invMass, inverseLoadMass, bodyRoll, bodyPitch, bodyYaw, bodySinRoll, bodyCosRoll, bodySinPitch, bodyCosPitch, savedRoll, savedPitch | 2 | SPBO layout +0x18..+0x4c |
| EventKrustyBikeView | `field_0x070` | acceleration, prevVelocity | 2 | SPBO +0x70, +0x7c |
| EventKrustyBikeView | `field_0x094[0xb8 - 0x94]` | bodyUp, savedForward, savedUp | 2 | SPBO +0x94..+0xac |
| EventKrustyBikeView | `field_0x488` | takeoffPosition, takeoffVelocity | 2 | Vehicle +0x488, +0x494 |
| BikeRaceKrustyBikeView (BikeRace.h) | `field_0x008` | sceneNode (bytes kept), position | 2 | SPBO +0x8, +0xc; RaceView.h names both |
| BikeRaceKrustyBikeView | `field_0x070` | acceleration, prevVelocity | 2 | SPBO |
| BikeRaceKrustyBikeView | `field_0x094` | bodyUp, savedForward, savedUp, prevSpeed | 2 | SPBO +0x94..+0xb8 |
| BikeRaceKrustyBikeView | `field_0x0c0[0x109 - 0xc0]` | angularAcceleration, worldAngularVelocity, angularVelocity, invInertia, shapeInvInertia, `field_0x0fc[0x108 - 0xfc]`, airborne | 2 | SPBO +0xc0..+0x108; +0xfc is a pad in the canonical class too |
| BikeRaceKrustyBikeView | `field_0x10a[2]` | asleep (+0x10b is padding) | 2 | SPBO +0x10a |
| BikeRaceKrustyBikeView | `field_0x5c8[0x5f0 - 0x5c8]` | loadInfo (bytes kept), riderName | 2 | Bike +0x5c8 (BikeA5C8), +0x5e0; EventKrustyBikeView racerName |
| BikeRaceKrustyBikeCameraView | `field_0x398` | cachedTargetHeading, cachedTargetPoint | 2 | VehicleCamera.h; the flags at +0x391/+0x392 guard them |
| StartD3DIMSoultreeObjectView | `field_0x278` | lowestLod, currentLod, autoLodDistances, inverseLodThreshold, useAutoLod | 2 | D3DIMSoultreeObject layout |
| RacerD3DIMSoultreeObjectView | `field_0x284` | inverseLodThreshold, useAutoLod | 2 | D3DIMSoultreeObject layout |
| StartObjectColliderBase | `int field_0x04` | short queryStamp, short sortKey | 2 | QuadTreeObject (CollisionObject.h, D3DIMSoulTree.h) |
| BikeRaceBikeRecord | `char bikeName[0x44]` | char texture[0x40], int engineSizeSelectable | 2 | KrustyUIBikeRecord (SelectGamePicProcs.h): "BikeTexture_%02d" fills +0x48, and DlgProcs/MPBikeRiderDlg read +0x88; CreateRacers resolves it to the AI bike's texture |
| VehicleCameraVehicleView (VehicleCamera.h) | `field_0x070`, `field_0x094` | acceleration, prevVelocity; bodyUp, savedForward, savedUp, prevSpeed | 2 | SPBO |
| BikeCameraBikeView (BikeCamera.h) | `field_0x5c8[0x604 - 0x5c8]` | loadInfo, riderName, frontWheel, rearWheel, leftBarContact, rightBarContact (bytes kept), tailContact | 2 | Bike layout +0x5c8..+0x600 |
| InstrumentKrustyBikeView (TrackOverlay.h) | `field_0x0c0` | the five angular Vector3s, `field_0x0fc` | 2 | SPBO |
| ArcadeCameraView, BonusCameraView, PickerCameraView | `field_0x17c` | viewDirection, upDirection, roll | 2 | Camera layout +0x17c..+0x194 |
| D3DIMSoultreeCameraView (D3DIMSoulTree.h) | `field_0x188` | upDirection, roll | 2 | Camera |
| SoultreeObject (D3DIMSoulTree.h) | `field_0x194` | inQuadtree, quadtreeCell, loadedNodeCount | 2 | canonical SoultreeObject (src/krusty2/core) |
| EcoCameraView (EcoSystem.h) | `field_0xec` | Matrix4 screenMatrix[2], fieldOfView | 2 | Camera; EcoSystem.cpp passes `&camera->screenMatrix` to the clipper, which the old comment called "the clipper's matrix" |
| Group1ParticleCamera (Group1_ParticleRender.cpp) | `field_0x12c[0x170 - 0x12c]` | `field_0x12c[0x16c - 0x12c]` (screenMatrix[1]), fieldOfView | 2 | Camera |
| Group1ParticleCamera | `field_0x19c` | displayAspect, viewport[6], heightWidthRatio | 2 | Camera |
| ParticleCornerVertex (ParticleManager.h) | `field_0x0c` | rhw, color, specular, tu, tv | 2 | FVF 0x1c4 = XYZRHW, DIFFUSE, SPECULAR, TEX1, which is the D3DTLVERTEX layout; Group1CornerColor reads color |
| QuarryLoadKrustyBikeView (Group3_QuarryLoad.cpp, QuarryStuntEventLoaderNearMiss.cpp) | `field_0x5c8` | loadInfo (bytes kept), riderName | 2 | Bike |
| QuarryLoadKrustyBikeCameraView (same two files) | `field_0x398` | cachedTargetHeading, cachedTargetPoint | 2 | VehicleCamera.h |
| CarProceduralNode (CarProcedural.h), BikeQuadTreeHead (Bike.cpp) | `int field_0x04` | short queryStamp, short sortKey | 2 | QuadTreeObject |
| KbNetState (KrustyBikeTypes.h) | `short field_0x00`, `int field_0x04` | char type, unsigned char finishPosition; short lapCount, short gatesPassed | 2 | KbBikeMessage (KrustyBikeRecords.h), whose fields the state is decoded from |
| DDSurfaceDesc2 (RenderInterfaces.h) | `field_0x28[0x48 - 0x28]` | `field_0x28[0x40 - 0x28]`, srcBltColorKey[2] | 2 | DDSURFACEDESC2 +0x40 ddckCKSrcBlt; ControlHitTest.cpp compares the surface with it (it spelled `field_0x28[0x18]`) |
| CursorUIAnimView (GUIManager.h) | `field_0x14` | passesLeft, passCountdown, frameDelay | 2 | UIAnim |
| GuiControlView (GUIManager.h) | `field_0xc0[0xf0 - 0xc0]` | controlText, textColor, dropColor, currentTextColor, controlTextLength, textDrop, textAlign, textExtent[2], textLines, textLeftMargin, textTopMargin | 2 | UIControl layout |
| GuiControlView | `field_0xf4[0x130 - 0xf4]` | controlName[0x32], `field_0x126[2]`, fontHandle, ownsFont | 2 | UIControl layout |
| VehicleShock (Vehicle.h) | `pad_0x80[0x14]` | `pad_0x80[0xc]`, prevLength, clearedLength | 2 | Suspension.h Shock |

### Decisions requested by the brief

- VehicleWheelAux and the shock pointers. VehicleShock gets prevLength
  (+0x8c) and clearedLength (+0x90), the Suspension.h Shock names.
  UpdateWheelsInContact (`0x00529a20`) now casts the wheel's
  secondaryAux/primaryAux pointers to `VehicleShock*` and copies
  clearedLength into prevLength. VehicleWheelAux keeps typing only the
  gearbox pointer at wheel+0x2a8. Its two members take the EngineGearbox
  names throttleTimeScale and gearCount (GearRatios.h:62-63), which
  tests/fixtures/vehicle_gearbox_boundaries.cpp already equated. The float
  copy instead of the old int copy compiles to the same bytes: Vehicle.cpp
  is IDENTICAL. VehicleWheel `secondaryAux`/`primaryAux` stay `int`;
  retyping them is left.
- KbEffectParams `field_0x4` -> `reserved` (tier 2). With DIEFF_POLAR the
  DirectInput documentation ("Effect Direction" and DIEFFECT) says the last
  element of rglDirection is reserved and must be 0. SetDirection
  (`0x004c3db0`) passes flags 0x22 with two axes.
- KrustyUISoultreeObjectView::SetAxes `float g` -> `int orthogonalize`. This
  matches SoultreeObject::SetAxes `0x004fbd10` (D3DIMSoulTree.h:80). The view
  declaration has no caller; the callers cast to SoultreeObject. KrustyUI.cpp
  is NAME-ONLY-LABELS.
- BikeRaceBikeRecord `bikeName[0x44]` -> `texture[0x40]` +
  `engineSizeSelectable`, as in KrustyUIBikeRecord. CreateRacers and
  BikeRaceNearMisses.cpp now pass `.texture` to ResolveBikeFile.
- BikeRaceKrustyBikeView `carriedFlagPosition` -> `centerOfMass`. The
  canonical class has centerOfMass at +0x18, which RaceView.h also names. Its
  only reader (BikeRace.cpp) puts the "you" indicator 4.25 above it.

### Reverted or tuned edits

Proof run "a" gave four DIFFs. All four were the declaration-count lottery
of Wave 18. Every new member adds 1 to the count of each unit that includes
its header. No member's code changed.

work/naming/w31/kscan.py compiles a unit behind k typedefs (k = 0..63) and
lists the k whose function bytes equal the base. That gives the shifts that
restore each function:

- `?MapReference@@` in tests/fixtures/game_tick_compare.cpp. It reaches only
  RenderInterfaces.h. The full DDSurfaceDesc2 split (+4) was a DIFF, and so
  were the four DDCOLORKEYs alone (+3) and alpha + reserved + srcBltColorKey
  (+2). srcBltColorKey alone (+1) is IDENTICAL, and it is the one member
  code reads. Reverted: alphaBitDepth/reserved (+0x1c) and
  destOverlayColorKey, destBltColorKey and srcOverlayColorKey.
- `?Check@@` in tests/fixtures/vegetation_collision_compare.cpp. It reaches
  only EcoSystem.h. The base table holds k = 0, 1, 27, 28, 42 and 52..63.
  The edit was +2, so it gave DIFF. Reverted: EcoCameraView `field_0x188`
  (upDirection + roll), which leaves the screenMatrix split at +1. That is
  IDENTICAL.
- `?CreateRacers@BikeRace@@` (0x00419970) in BikeRaceNearMisses.cpp and
  Group6_BikeRaceCreateRacers.cpp. This is the Wave 18 `xor edi,edi` /
  `mov ebx,1` order at body offset 0x21a. After the two reverts above, the
  scan showed that one more declaration (+1) in their shared closure restores
  both units. The BikeRaceKrustyBikeView `field_0x008` split (sceneNode +
  position, both named in RaceView.h and the canonical class) supplies it.
  Both units are then IDENTICAL or IDENTICAL-COMDAT-ORDER.

### Proof

work/naming/w31/prove.sh final, from 1011ac65 to the final tree, over every
unit whose closure reaches a changed file: 325 units. The map is empty: no
decorated name changed.

| Verdict | Units |
| --- | --- |
| IDENTICAL | 272 |
| IDENTICAL-COMDAT-ORDER | 7 |
| NAME-ONLY-LABELS | 46 |
| NAME-ONLY | 0 |
| DIFF | 0 |

The seven IDENTICAL-COMDAT-ORDER units are:

- BikeRaceNearMisses.cpp, DlgProcsNearMisses.cpp and
  ProCircuitProcsNearMisses.cpp;
- Group4_DlgMain.cpp, Group4_DlgPCBunny.cpp and Group6_BikeRaceCreate.cpp;
- SoulTreePhysics.cpp.

NAME-ONLY-LABELS means that only the goto-label counters (`$label$N`)
moved. tests/fixtures/bvh_vector_compare.cpp is not in the unit list.

Negative control (negcontrol.sh, negmutate.py): two wrong edits were made in
the new tree only.

- UpdateWheelsInContact: the shock store was reversed
  (`clearedLength = prevLength`).
- EcoSystem.cpp: `&camera->screenMatrix` became `&camera->screenMatrix[1]`.

Both units gave DIFF, in the `.text` bytes of
`?UpdateWheelsInContact@Vehicle@@QAEXXZ` and `?PrepareGeometry@EcoSystem@@UAEHXZ`.

Pins: one digest changed. test_terrain_connections pins `?Load@Terrain@@`
with its relocation names. The EH label moved from `$L6434` to `$L6435`, and
the 2117 body bytes and the other 64 relocation records are unchanged. It was
repinned with a comment (work/naming/w31/pinmods.py: 2 digests, 1 module).
No layout fixture counts declarations.
tests/fixtures/vehicle_gearbox_boundaries.cpp now spells VehicleWheelAux's new
names. It is IDENTICAL, and its 48 native cases pass.

Tests: work/naming/w31/pick_tests.py selected 119 modules: the changed files'
basenames, the old spellings and the NAME-ONLY-LABELS units. 118 passed at
first. test_terrain_connections failed on the Terrain::Load pin above and
passes after the repin (work/naming/w31/tests-picked.log). The full suite
(`python -m unittest discover -s tests` with VC6_ROOT and MCM2_EXE) ran 675
tests: 674 pass and 12 are skipped. The one failure is the worktree-only
test_emu_diff case vehicle_simulate_frame (retail_faults 50), which fails the
same way at 1011ac65. tools/static_check.py passes once
docs/DECOMPILATION_PROGRESS.md is regenerated (see below). The EOL check
(work/naming/w31/eolcheck.py) finds every edited file with its original line
endings.

### Left for wave 32

- tools/static_check.py fails only on docs/DECOMPILATION_PROGRESS.md, because
  the splits add 86 canonical source lines (112,913 -> 112,999). The brief
  forbids touching that file. `make progress` in the merge fixes it; it
  changes only that line, and config/decompilation_progress.json is
  unchanged. With the regenerated file, static_check passes.
- Splits reverted for the lottery (evidence-backed; a later wave can include
  them if other edits move the count):
  - DDSurfaceDesc2 `field_0x1c` (alphaBitDepth + reserved) and the first
    three DDCOLORKEYs in `field_0x28`;
  - EcoCameraView `field_0x188` (upDirection + roll).
- The headers this wave edits that are in the include closure of the
  concurrent matching units: RaceView.h, D3DIMSoulTree.h, GUIManager.h,
  RenderInterfaces.h, KrustyBikeTypes.h and Vehicle.h. The lead should
  re-prove those units after merging that work, because each declaration
  moves their counters.
- Not split because the concurrent matching work uses them, or because they
  sit next to declarations it uses:
  - SelectGameRaceSettingsView `field_0x14`/`field_0x14c`;
  - KrustyVCRHeader `field_0x40`/`field_0x58`/`field_0x61`;
  - KrustyUI `field_0x4ac`, `field_0x4b4` and `field_0x674`;
  - ProCircuitRacerResultView `field_0x08`/`field_0x2c`;
  - SelectGameD3DIMSoultreeCharacterView `field_0x010`;
  - the SceneSoultreePhysics views and SceneCarProceduralView `field_0x40`;
  - the MorphBastard channel.
- Not split: too many members or uncertain members.
  - EventKrustyBikeView `field_0x054`/`field_0x0bc`.
  - VehicleCameraVehicleView `field_0x0c0` (764 bytes).
  - QuarryLoadKrustyBikeCameraView `field_0x02c`.
  - GuiControlView `field_0x60` and `field_0x164`.
  - EventManager `field_0x444`.
  - EventManagerNearMisses.cpp and Group1_PodiumScene.cpp
    `field_0x5c8[0x784 - 0x5c8]`.
  - GridManagedTexture `field_0x00` and the GridTerrain pads.
  - Group1ParticleCamera `field_0x000`.
  - BikeRaceKrustyBikeView `field_0x024` (16 SPBO floats) and
    `field_0x5f8`.
- No clear evidence: TrackGameRaceGateView `field_0x00` (the RaceGate
  identity is uncertain) and KrustyUIToolTipView `field_0x00`.
- Not done: CursorUIAnimView `field_0x00` (vfptr + refCount), the
  CarProcedural model `field_0x38`, the MediaControl DDSurfaceDesc
  `field_0x10`, and RacerStateMessage `field_0x02`/`field_0x04`.
- VehicleWheel `secondaryAux`/`primaryAux` (`int`) could be typed as
  `VehicleShock*`, and Bike.h types them `BikeQ*`.

## Wave 32: the wave 31 leftovers split, and the declaration counter

Wave 32 works through the Wave 31 lists "Left for wave 32". The rules are the
same as in Wave 31. A pad is split only where every new member has evidence:
the canonical class layout (work/naming/cv/layouts.json), another view of the
same object, or an SDK layout. Gaps without evidence stay as smaller
`field_0xNN` pads, so every size and offset is unchanged. A member whose type
the view cannot spell is a byte array named after the member, marked "(bytes
kept)". A pointer to a type the view cannot spell is `void*`, and the comment
names the real type. No static, global, type of a static, or type with an
emitted RTTI descriptor was renamed. No union arm was touched, and no real
class changed member order or size.

The helpers are in work/naming/w32/:

- gen.py (emits a split from the canonical layout) and spec/*.py;
- prove.sh, trial.py (a quick re-proof of a few units), kscan.py;
- trim.py and the search scripts pcsearch.py, dsearch.py, sgsearch.py,
  gsearch.py and lowsearch.sh (the counter search below);
- reach.py and byheader.py (which changed headers a unit reaches);
- negcontrol.sh and negmutate.py;
- pick_tests.py, runtests.sh, pinmods.py, eolcheck.py and members.py.

Counts over `src` and `samples` (`git grep -ohE`, work/naming/w28/counts.py):

| Placeholder | Before (18acf8e7) | After |
| --- | --- | --- |
| `\bUnknownVirtualSlot[0-9]+` | 74 uses, 17 names | 74 uses, 17 names |
| placeholder methods | 85 uses | 85 uses |
| `\bUnknown[A-Za-z0-9_]*` | 376 uses, 74 names | 376 uses, 74 names |
| `\bfield_0x[0-9a-fA-F]+` | 1167 uses, 316 spellings | 1165 uses, 313 spellings |
| `field_0x` in samples/ only | 106 uses | 109 uses |
| `field_0x` declarations (`\bfield_0x[0-9a-fA-F]+(\[[^]]*\])?;`) | 781 | 780 |

The `field_0x` counts barely move because most splits leave a residual pad
next to the named members. The wave adds 551 named member declarations
(work/naming/w32/members.py), removes 45 pads and adds 44 smaller ones.
The edited files gain 557 lines.

Two edits are not splits:

- CarProcedural.cpp `ReadPath(pathFile, carModel->field_0x38)` is now
  `carModel->nodeName`.
- DlgProcs.cpp SinglePlayerDlg::HandleEvent (`0x0044cf20`) copied the race
  settings from `(unsigned char*)g_TrackGame->ui + 0x4e8`. It now copies from
  `g_TrackGame->ui->ghostRaceSettings`.

One parameter was renamed. The first parameter of LoadParameterFile was
`void* value` in MorphBastardModifier and `void* a` in KbMorphMod. Both are
now `renderContext`, in MorphBastardModifierCore.h, Group0_KbMorphMod.cpp and
MorphBastardModifierNearMisses.cpp. The body passes it straight to
D3DIMSoultreeModifier::SetRenderContext (`0x004452e0`).

### Splits

Tier 2 throughout. "SPBO" is SoultreePhysicsBaseObject, the KrustyBike and
Vehicle base that the canonical layout puts at those offsets.

| Struct (file) | Old pad | New members | Tier | Evidence |
| --- | --- | --- | --- | --- |
| KrustyVCRHeader (DlgProcs.h) | `field_0x40[0x54 - 0x40]` | races, treeCollision, riderCollision, shortTimeLimit, maxBikeClass | 2 | the header holds a RaceSettings copy at +0x34 (raceMode, eventType, series, laps and randomGates are already named from it); TrackGame.h RaceSettings +0x0c..+0x1c |
| KrustyVCRHeader | `field_0x58[0x60 - 0x58]` | aiRacerCount, multiplayerAiRacerCount | 2 | RaceSettings +0x24, +0x28 |
| KrustyVCRHeader | `field_0x61[0x64 - 0x61]` | gateCount, `field_0x62[2]` | 2 | RaceSettings +0x2d |
| KrustyVCRHeader | `field_0x1c0[0x229 - 0x1c0]` | lobbySlots[0x60], `field_0x220[9]` | 2 | RaceSettings lobbySlots |
| KrustyUI (KrustyUI.h) | `field_0x4b4[0x634 - 0x4b4]` | `field_0x4b4[0x10]`, ghostDescription[0x20], ghostLengthSeconds, ghostRaceSettings (bytes kept) | 2 | SPEventDlg::SyncValues (`0x0044e6c0`) copies the chosen ghost's KrustyVCRHeader here whole; the members sit at header offset + 0x4b4. SinglePlayerDlg::HandleEvent copies +0x4e8 (header +0x34, the RaceSettings) to TrackGame+0x2d70 |
| SelectGameRaceSettingsView (SelectGamePicProcs.h) | `field_0x14[0x20 - 0x14]` | riderCollision, shortTimeLimit, maxBikeClass | 2 | TrackGame.h RaceSettings |
| SelectGameRaceSettingsView | `field_0x14c[0x1ec - 0x14c]` | gridOrder (bytes kept), lobbySlots | 2 | RaceSettings gridOrder +0x14c, lobbySlots +0x18c |
| SelectGameD3DIMSoultreeCharacterView | `field_0x010[0x1a0 - 0x10]` | motionTime .. vutLoaded (21 members, `field_0x038` kept, poseList bytes kept) | 2 | D3DIMSoultreeCharacter layout |
| ProCircuitRacerResultView (ProCircuitProcs.h) | `field_0x08[0x28 - 0x08]` | fastestLap, largestStuntPoints, totalTrickTime, totalRunningTime, bestTagRun, totalTagRun, finished, outOfRace | 2 | RacerResult layout |
| ProCircuitRacerResultView | `field_0x2c[0x34 - 0x2c]` | score, `field_0x30[4]` | 2 | RacerResult score |
| SceneCarProceduralView (SceneManager.h) | `field_0x2c`, `field_0x40`, `field_0x60`, `field_0x198` | drawColor, specularColor; pathVelocity, groundSpeed, steersFrontWheels, started; numWheels .. smoothedNormal; wheelNodes, bodyContactPoint, bodyContactNormal (pads kept at +0x58, +0x68, +0x178, +0x1b8, +0x1e8, +0x20c) | 2 | CarProcedural and SoultreeObject layouts |
| SceneManagerNearMisses.cpp SPBO view | `field_0x0c[0x12c - 0x0c]`, `field_0x134[0x21c - 0x134]` | position .. collisionObject; lastCollisionType .. centerNode | 2 | SPBO layout |
| SceneManagerNearMisses.cpp SceneCollisionObjectView | `field_0x38[0x60 - 0x38]`, `field_0x64[0xb8 - 0x64]` | drawColor .. contactRecord; ownerType .. hitNormal | 2 | CollisionObject layout |
| EventKrustyBikeView (RaceView.h) | `field_0x054[0x64 - 0x54]` | savedSinRoll, savedCosRoll, savedSinPitch, savedCosPitch | 2 | SPBO |
| VehicleCameraVehicleView (VehicleCamera.h) | `field_0x050[0x64 - 0x50]` | savedYaw, savedSinRoll, savedCosRoll, savedSinPitch, savedCosPitch | 2 | SPBO |
| VehicleCameraVehicleView | `field_0x0c0[0x3bc - 0xc0]` | angularAcceleration, worldAngularVelocity, angularVelocity, invInertia, shapeInvInertia, `field_0x0fc`, airborne, respawnPending, asleep, `field_0x10b` | 2 | SPBO (+0xfc is a pad in the canonical class too) |
| QuarryLoadKrustyBikeCameraView (Group3_QuarryLoad.cpp, QuarryStuntEventLoaderNearMiss.cpp) | `field_0x02c[0x274 - 0x2c]`, `field_0x275[0x384 - 0x275]` | worldMatrix .. recordsPerSecond; isPlaced .. orientation (89 members) | 2 | Camera, FollowCamera and KrustyBikeCamera layouts |
| GuiControlView (GUIManager.h) | `field_0x60[0xb8 - 0x60]` | currentState .. transitionType (22 members) | 2 | UIControl layout |
| GuiControlView | `field_0x164[0x210 - 0x164]` | transitionFrame .. textScrollX (UIControl, then UIEditBox textLength, textCapacity, textWidth, textScrollX); transitionStep bytes kept; `field_0x204[0x210 - 0x204]` keeps the UIEditBox sound unions | 2 | UIControl and UIEditBox layouts |
| CursorUIAnimView (GUIManager.h) | `field_0x00[0x08]` | `field_0x00[4]` (vfptr), refCount | 2 | BaseObject |
| PodiumKrustyBikeView (Group1_PodiumScene.cpp, EventManagerNearMisses.cpp) | `field_0x5c8[0x784 - 0x5c8]` | loadInfo (bytes kept), riderName, frontWheel, rearWheel, leftBarContact, rightBarContact, tailContact, wrecker, `field_0x608`, aiKind .. heapBufferB | 2 | Bike and KrustyBike layouts |
| GridManagedTexture (Griddraw.h) | `field_0x00[0x68]` | vfptr pad, refCount, nextTexture .. averageColor, `field_0x40`, statePairCount, statePairs (bytes kept) | 2 | TextureMap layout |
| GridTerrain (Griddraw.h) | `field_0x03c`, `field_0x07c`, `field_0x540`, `field_0xc18`, `field_0xc90` | stream; invDistanceRatioSq, invDistanceRatio, runMeshChecks; ownedObjectCount, ownedObjects, drawDistance, qualityIndex, drawDistanceDirty, profileEval, profileMesh, profileSetup, blockCount (pads kept); surfaceDesc (bytes kept), invGridCellSize; detailAverageColor | 2 | Terrain layout |
| Group1ParticleCamera (Group1_ParticleRender.cpp) | `field_0x000[0xec]` | `field_0x000[0x2c]` (GameObject), worldMatrix, projectionMatrix, viewMatrix (float[16] each) | 2 | Camera layout |
| BikeRaceKrustyBikeView (BikeRace.h) | `field_0x024[0x64 - 0x24]` | invMass .. bodyCosPitch (9 floats), `field_0x048[0x64 - 0x48]` | 2 | SPBO |
| BikeRaceKrustyBikeView | `field_0x5f8[0x604 - 0x5f8]` | leftBarContact, rightBarContact, tailContact | 2 | Bike layout |
| RacerStateMessage (BikeRace.h) | `field_0x02[2]`, `int field_0x04` | short elapsedMs, short lapCount, short gatesPassed | 2 | KbBikeMessage (KrustyBikeRecords.h), which the message is |
| RacerStateMessage | `field_0x20[0x28 - 0x20]` | lastLapTime, `field_0x24[4]` | 2 | KbBikeMessage +0x20 |
| CarProceduralModel (CarProcedural.h; a real class, offsets and size kept) | `char field_0x38[0x2d8 - 0x38]` | nodeName[0x80], `field_0xb8` | 2 | SoultreeObject nodeName at +0x38; the old comment said it starts with the model name (0x0042fa00) |
| DDSurfaceDesc (MediaControl.h) | `field_0x10[0x6c - 0x10]` | pitch, backBufferCount, mipMapCount, alphaBitDepth, reserved, surface, `field_0x28[0x6c - 0x28]` | 2 | DDSURFACEDESC (ddraw.h) |

### Reverted or trimmed edits

Every reverted split below is evidence-backed. A later wave can restore it
if other edits move the counter.

- VehicleWheel `secondaryAux`/`primaryAux` typed as `VehicleShock*`
  (Vehicle.h). This was a real code change, not the counter.
  `?ApplyShockForces@Vehicle@@` gave DIFF, and
  tests/fixtures/wheel_attachment_compare.cpp failed to compile, because it
  assigns an int. Reverted; both stay `int`.
- Proof a1 (253 units) gave 7 DIFFs and that COMPILE-FAIL. The DIFFs were
  CreateRacers in two units, FillFileList in two, and Setup in
  edit_box_draw_compare and multi_state_draw_compare, plus `_main` in
  grid_corner_compare. Reverted for the counter:
  - EventKrustyBikeView `field_0x0bc` (6 SPBO members) and `field_0x109`;
  - GridTerrain `field_0x01c` (GameObject) and `field_0x044` (heightField and
    bounds);
  - DDSurfaceDesc's four DDCOLORKEYs, ddpfPixelFormat and ddsCaps. They stay
    in `field_0x28`, and the comment names them.
- Proof a2 (230 units) gave 5 DIFFs, all counter cases. Trimmed:
  - `?Startup@TrackGame@@` (TrackGame.cpp reaches only KrustyUI.h and
    RaceView.h): the KrustyUI `field_0x674` split was dropped (ghostLobbySlots,
    ghostRiderName, ghostEngineSize, ghostFourStroke). The pad's comment
    names them.
  - `?CreateRacers@BikeRace@@`. That KrustyUI trim broke it, so
    BikeRaceKrustyBikeView keeps savedRoll..savedCosPitch (+0x48..+0x60) as
    `field_0x048`.
  - `?AddCircuitRow@PCStartupDlg@@` in ProCircuitProcs.cpp and
    ProCircuitProcsNearMisses.cpp: RacerResult aiIndex (+0x30) stays in
    `field_0x30`. Every other count of the RacerResult splits (0..8) passes.
    Only the full split (+9) fails.
  - `?SendStartMessage@MultiPlayerDlg@@` in Group5_MultiPlayerDlg.cpp and
    SelectGamePicProcsNearMisses.cpp: GuiControlView backgroundBrush (+0x20c)
    stays in `field_0x204[0x210 - 0x204]`.

The counter is not a plain linear count here. Both SendStartMessage units
reach the same changed headers, yet the shifts that restore them did not
overlap. SelectGamePicProcs.h and DlgProcs.h shifts from 0 to 31 restored
Group5_MultiPlayerDlg.cpp at 0, 2, 3 and 9..14. They restored the samples
copy only at 28..30. Folding the GUIManager.h split restored both at most
fold points (work/naming/w32/gsearch.py).

kscan.py (k typedefs ahead of the unit) also mispredicted one unit.
ProCircuitProcsNearMisses.cpp matched at k = 0 through the probe, but a direct
compile gave DIFF. So this wave searched by direct compiles (trial.py and the
search scripts) rather than by kscan tables.

### Proof

work/naming/w32/prove.sh a3, from 18acf8e7 to the final tree, over every
unit whose closure reaches a changed file: 230 units. The map is empty: no
decorated name changed.

| Verdict | Units |
| --- | --- |
| IDENTICAL | 183 |
| IDENTICAL-COMDAT-ORDER | 7 |
| NAME-ONLY-LABELS | 40 |
| NAME-ONLY | 0 |
| DIFF | 0 |

The seven IDENTICAL-COMDAT-ORDER units are:

- DlgProcsNearMisses.cpp and ProCircuitProcsNearMisses.cpp;
- Group4_DlgMain.cpp and Group5_PCBailoutDlg.cpp;
- BikeRace.cpp, DlgProcs.cpp and ProCircuitProcs.cpp.

Negative control (negcontrol.sh, negmutate.py): two wrong edits were made in
the new tree only.

- DlgProcs.cpp: the ghost settings copy read `ghostRaceSettings + 1`.
- CarProcedural.cpp: ReadPath got `carModel->nodeName + 1`.

Both units gave DIFF, in the `.text` bytes of
`?HandleEvent@SinglePlayerDlg@@` and `?Create@CarProcedural@@`.

Pins: one digest changed. test_terrain_connections pins `?Load@Terrain@@`
with its relocation names. The EH label moved from `$L6435` to `$L6468`, and
the 2117 body bytes and the other 64 relocation records are unchanged
(work/naming/w32/loadpin.py compares the proof objects). It was repinned with
a comment (pinmods.py: 2 digests, 1 module).

Tests: work/naming/w32/pick_tests.py selected 99 modules. It picked them by
the changed files' basenames, the removed spellings and the non-IDENTICAL
proof units. 98 passed at first; test_terrain_connections failed on the pin
above and passes after the repin.

The full suite (`python -m unittest discover -s tests`, with VC6_ROOT and
MCM2_EXE set) ran 675 tests, with 12 skipped. The one failure is the
worktree-only test_emu_diff case vehicle_simulate_frame (retail_faults 50),
which fails the same way at the base. tools/static_check.py passes once
docs/DECOMPILATION_PROGRESS.md is regenerated: canonical source lines went
from 112,999 to 113,222, and config/decompilation_progress.json is
unchanged. The EOL check (work/naming/w32/eolcheck.py, now strict CRLF)
finds every edited file still CRLF.

### Left for wave 33

- Splits reverted or trimmed for the counter (see above):
  - KrustyUI `field_0x674`;
  - EventKrustyBikeView `field_0x0bc` and `field_0x109`;
  - GridTerrain `field_0x01c` and `field_0x044`;
  - DDSurfaceDesc `field_0x28`;
  - BikeRaceKrustyBikeView `field_0x048`;
  - ProCircuitRacerResultView `field_0x30` (aiIndex);
  - GuiControlView backgroundBrush.
- Held from Wave 31 and not retried: DDSurfaceDesc2 `field_0x1c` and the first
  three DDCOLORKEYs, and EcoCameraView `field_0x188`.
- VehicleWheel `secondaryAux`/`primaryAux` stay `int`. The retype changes
  ApplyShockForces and breaks the wheel_attachment fixture.
- Not split, because the evidence is uncertain:
  - KrustyUI `field_0x4ac`;
  - the MorphBastard channel `field_0x5c`;
  - EventManager `field_0x444`.
- src/krusty2/vehicle/KrustyBikeTypes.h:541 still spells the KbMorphMod
  LoadParameterFile parameter `void* a`. That header is in the closure of the
  concurrent KrustyBike matching work.
- The concurrent matching units reach headers this wave edits. The lead
  should re-prove them after merging, because each declaration moves their
  counters:
  - FollowCamera.cpp, NetThread.cpp, PCAudio.cpp, UiInfo.cpp and
    Group3_KrustyBike.cpp reach KrustyUI.h and RaceView.h (+6);
  - KrustyBikeCamera.cpp also reaches VehicleCamera.h (+13);
  - GameUi.cpp also reaches GUIManager.h (+53) and MediaControl.h (+6).
  - GR_BitString.cpp, FileStream.cpp, Motnctrl.cpp and GraphicsTest.cpp reach
    none.
