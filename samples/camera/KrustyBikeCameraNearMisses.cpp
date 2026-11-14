// Near-miss KrustyBikeCamera candidates, kept out of src/reconstructed until
// they match. See docs/FOLLOW_CAMERA.md.
//
// KrustyBikeCamera::Tick (0x00497e20, 352 bytes + 6-entry jump
// table): everything up to the second 0x004210f0 call matches. From there
// to the end (that call, the slot 47 block and the restore tail) VC6 rotates
// the scratch registers: retail loads the global into edx and builds the
// arguments in ecx/eax/edx exactly like the first call (and uses edx, ecx, eax
// in the tail); VC6 uses ecx/eax/edx/ecx (eax, eax, edx in the tail).
// Declaring the locals at the top, in the block, in either order or as an
// array, an else form, a named bool for the +0x18e test, a local for
// +0x560 and a named int switch value all leave it unchanged.
// Also unchanged: 0..15 dead locals, 0..12 file-scope declarations (prototypes,
// structs, statics, inline functions) before the function, a Vector3 pointer for
// slot 47, a float local for the height, `2.0f + y`, explicit this-> calls, a
// local for cameraState, `== false`, and the tail nested in an else. A view
// local for the second call and setting the flag last are worse.
// Session 14 (all 337): `= 1` for the flag, casts on the slot 46/47 vector
// arguments, `!= 0` and `== 0` tests, `== false`, an empty-then-else form,
// a `starting` bool local, and dead statements (a discarded load, an unused
// int, a scoped pointer) ahead of the second call. The rotation also
// reaches the restore tail in the other branch.
// Session 25 (all 337): pointer, reference and up-axis locals for the
// vector arguments, event/view/game locals, float and reference locals in
// the tail, a comma return, `BikeCamera::` on the three direct calls,
// inline wrappers for them and a local for the static flag. Clue: with the
// 0x004210f0 call made virtual (380 bytes, so not retail's direct call)
// every later register matches and only that call and the last Tick's
// `mov ecx, [frameTime] ; push ecx ; mov ecx, esi` differ, so retail has
// exactly one more live temporary at that direct call.
// The vector passed to slot 46 (0x0067c3e8) is the unit's kVec3YAxis
// (src/reconstructed/KrustyBikeCamera.cpp); it is file-static there, so this
// sample keeps its own static.
// Tick, argument-order sweep of 2026-11 (all 337): the event, racer,
// position, height and pending-view reads through inline accessors (by
// reference and by value), inline wrappers around WriteRelativePositions,
// an int-returning view of 0x004210f0 with the result in a dead local or
// tested in an empty if, every order of the flag store, the first call and
// BuildOrientationBasis, the PlayRecordedFrame result in a bool local,
// through this->, as a comma return or tested, and an `int result = 1`
// local. Probe only: a virtual view of the FIRST 0x004210f0 call fixes
// every later register (14 instruction diffs, sizes shift); virtual at the
// second call does not (19).
// Wave 26 (33 renderings, all 313 of 376 with the table, the same code):
// the static flag copied into a bool or int local (declared with the copy
// or assigned inside the test) and that known-zero local passed as the
// first call's last argument, stored back negated, or selected through a
// ternary. The call ahead of the flag store is far worse (150).
#include "../../src/reconstructed/KrustyBikeCamera.h"

// .bss state used by slot 10. There is no initialisation guard, so these are
// file scope.
static bool s_UnknownActive67c3f4;
static Vector3 s_UnknownVector67c3e8;

// 0x00497e20: picks the view by the global mode on first use. While the view
// has its network start pending it drives FollowCamera slots 46 (once) and 47 from it;
// otherwise it leaves that mode, restoring the state, and runs FollowCamera
// slot 10.
int KrustyBikeCamera::Tick(float frameTime) {
    if (!raceView) {
        switch (g_TrackGame->mode.raceSettings.eventType) {
            case 2:
                raceView = g_TrackGame->nationalEvent->raceView;
                break;
            case 3:
                raceView = g_TrackGame->supercrossEvent->raceView;
                break;
            case 0:
                raceView = g_TrackGame->quarryEvent->raceView;
                break;
            case 1:
            case 5:
                raceView = g_TrackGame->bajaEnduroEvent->raceView;
                break;
            case 4:
                raceView = g_TrackGame->tagEvent->raceView;
                break;
        }
    }
    if (raceView->networkStartPending) {
        Vector3 first;
        Vector3 second;
        if (!s_UnknownActive67c3f4) {
            s_UnknownActive67c3f4 = true;
            raceView->WriteRelativePositions(&second, &first, g_TrackGame->bajaEnduroEvent, 0);
            FollowCamera::BuildOrientationBasis(&first, &s_UnknownVector67c3e8);
        }
        raceView->WriteRelativePositions(&first, &second, g_TrackGame->bajaEnduroEvent, 0);
        first.y = raceView->localRacer->position.y + 2.0f;
        FollowCamera::PlayRecordedFrame(frameTime, &first);
        return 1;
    }
    if (s_UnknownActive67c3f4) {
        s_UnknownActive67c3f4 = false;
        RestoreCameraState();
        SetCameraState(cameraState);
    }
    FollowCamera::Tick(frameTime);
    return 1;
}
