// Near-miss KrustyBikeCamera candidates, kept out of src/reconstructed until
// they match. See docs/FOLLOW_CAMERA.md.
//
// KrustyBikeCamera::UnknownVirtualSlot10 (0x00497e20, 352 bytes + 6-entry jump
// table): everything up to the second 0x004210f0 call matches. From there
// to the end (that call, the slot 47 block and the restore tail) VC6 rotates
// the scratch registers: retail loads the global into edx and builds the
// arguments in ecx/eax/edx exactly like the first call (and uses edx, ecx, eax
// in the tail); VC6 uses ecx/eax/edx/ecx (eax, eax, edx in the tail).
// Declaring the locals at the top, in the block, in either order or as an
// array, an else form, a named bool for the +0x18e test, a local for
// +0x560 and a named int switch value all leave it unchanged.
// The vector passed to slot 46 (0x0067c3e8) is the unit's kVec3YAxis
// (src/reconstructed/KrustyBikeCamera.cpp); it is file-static there, so this
// sample keeps its own static.
#include "../../src/reconstructed/KrustyBikeCamera.h"

// .bss state used by slot 10. There is no initialisation guard, so these are
// file scope.
static bool s_UnknownActive67c3f4;
static Vector3 s_UnknownVector67c3e8;

// 0x00497e20: picks the view by the global mode on first use. While the view
// is available it drives FollowCamera slots 46 (once) and 47 from it;
// otherwise it leaves that mode, restoring the state, and runs FollowCamera
// slot 10.
int KrustyBikeCamera::UnknownVirtualSlot10(float frameTime) {
    if (!raceView) {
        switch (g_TrackGame->mode.field_0x27f8.field_0x04) {
            case 2:
                raceView = g_TrackGame->field_0x564->field_0x34;
                break;
            case 3:
                raceView = g_TrackGame->field_0x558->field_0x34;
                break;
            case 0:
                raceView = g_TrackGame->field_0x55c->field_0x34;
                break;
            case 1:
            case 5:
                raceView = g_TrackGame->field_0x560->field_0x34;
                break;
            case 4:
                raceView = g_TrackGame->field_0x568->field_0x34;
                break;
        }
    }
    if (raceView->field_0x18e) {
        Vector3 first;
        Vector3 second;
        if (!s_UnknownActive67c3f4) {
            s_UnknownActive67c3f4 = true;
            raceView->UnknownFunction4210f0(&second, &first, g_TrackGame->field_0x560, 0);
            FollowCamera::UnknownVirtualSlot46(&first, &s_UnknownVector67c3e8);
        }
        raceView->UnknownFunction4210f0(&first, &second, g_TrackGame->field_0x560, 0);
        first.y = raceView->field_0x38->field_0x00c.y + 2.0f;
        FollowCamera::UnknownVirtualSlot47(frameTime, &first);
        return 1;
    }
    if (s_UnknownActive67c3f4) {
        s_UnknownActive67c3f4 = false;
        UnknownVirtualSlot62();
        UnknownVirtualSlot71(cameraState);
    }
    FollowCamera::UnknownVirtualSlot10(frameTime);
    return 1;
}
