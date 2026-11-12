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
