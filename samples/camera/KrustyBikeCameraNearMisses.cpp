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
//
// KrustyBikeCamera slot 34 (0x00498340, 612 bytes): 605 of 612 bytes match.
// The argument is the frame time as a float (FollowCamera slot 10 passes dt
// through slot 48; FollowCamera.h still declares slot 34 with an int), so the
// body is kept here as a helper of a derived view. Only the second product of
// the inline dot product differs: retail loads the returned vector's x first
// (`fld [eax]; fmul [esi+0x29c]`), VC6 the member's. Every term order and
// grouping, a member Dot, by-value parameters and a reference local were tried;
// `z + (x + y)` grouping with the returned vector first is the closest.
#include "../../src/reconstructed/KrustyBikeCamera.h"

// The inline dot product slot 34 uses; the grouping gives retail's y, x, z order.
static inline float KrustyCameraDot(const Vector3& a, const Vector3& b) {
    return a.z * b.z + (a.x * b.x + a.y * b.y);
}

// KrustyBikeCamera slot 34 (0x00498340): the followed point, led by the bike's
// velocity-like vector or by the camera direction, raised by 3.
struct KrustyBikeCameraSlot34View : public KrustyBikeCamera {
    Vector3 UnknownVirtualSlot34Float(float t);
};

Vector3 KrustyBikeCameraSlot34View::UnknownVirtualSlot34Float(float t) {
    Vector3 result;
    if (field_0x276) {
        bike->field_0x5c4->field_0x1a0->UnknownFunction4fc9a0(0, &result);
        if (field_0x2c0 < 500.0f) {
            Vector3 lead = bike->field_0x604->field_0x40->field_0x154 * t;
            lead = lead * (4.0f - field_0x2c0 * 0.006f);
            result += lead;
        } else {
            result += bike->field_0x604->field_0x40->field_0x154 * t;
        }
    } else if (raceView->field_0x3f8) {
        result = targetPoint;
    } else {
        float dot = KrustyCameraDot(UnknownVirtualSlot33(), field_0x29c);
        float height = field_0x2c0 * 0.009f + 1.0f;
        float ratio = field_0x16c / field_0x1dc;
        float speed = vehicleMode ? vehicle->field_0x438 : 180.0f;
        float lag = (1.0f - field_0x2c0 * 0.00052631577f) * ratio;
        float distance = lag * (height * height * dot / speed) * 35.0f;
        Vector3 offset = field_0x29c * distance;
        result = Vector3(offset.x + targetPoint.x, offset.y + targetPoint.y, offset.z + targetPoint.z);
    }
    result.y += 3.0f;
    return result;
}

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
        switch (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04) {
            case 2:
                raceView = g_UnknownGlobal56e26c->field_0x564->field_0x34;
                break;
            case 3:
                raceView = g_UnknownGlobal56e26c->field_0x558->field_0x34;
                break;
            case 0:
                raceView = g_UnknownGlobal56e26c->field_0x55c->field_0x34;
                break;
            case 1:
            case 5:
                raceView = g_UnknownGlobal56e26c->field_0x560->field_0x34;
                break;
            case 4:
                raceView = g_UnknownGlobal56e26c->field_0x568->field_0x34;
                break;
        }
    }
    if (raceView->field_0x18e) {
        Vector3 first;
        Vector3 second;
        if (!s_UnknownActive67c3f4) {
            s_UnknownActive67c3f4 = true;
            raceView->UnknownFunction4210f0(&second, &first, g_UnknownGlobal56e26c->field_0x560, 0);
            FollowCamera::UnknownVirtualSlot46(&first, &s_UnknownVector67c3e8);
        }
        raceView->UnknownFunction4210f0(&first, &second, g_UnknownGlobal56e26c->field_0x560, 0);
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
