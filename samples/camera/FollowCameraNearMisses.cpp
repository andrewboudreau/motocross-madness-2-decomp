// Near-miss FollowCamera candidates, kept out of src/reconstructed until they
// match. See docs/FOLLOW_CAMERA.md.
//
// FollowCamera::FollowCamera (0x00462ee0, 564 bytes): every store value,
// offset and the static slot-71 call are right, but VC6 schedules two zero
// stores (+0x274, +0x2dc) into the load-delay slot of the
// +0x29c global-vector copy, where retail placed +0x2e4/+0x276. About 141
// bytes differ, all instruction order; moving those two statements anywhere
// earlier does not help.
//
// FollowCamera::UnknownVirtualSlot36 (0x00465000, 209 bytes): 12 bytes differ.
// Retail's disabled exit is `xor al, al; mov [esi+0x277], al` (one zero for
// the store and the return); every source form tried so far emits an
// immediate store followed by `xor al, al`.
#include "../../src/reconstructed/FollowCamera.h"

// 0x00462ee0. Virtual calls in a constructor bind statically, so slot 71 is a
// direct call here.
FollowCamera::FollowCamera(int flags) : PCCamera(flags) {
    field_0x275 = 0;
    field_0x2b4 = g_UnknownVector65b448;
    field_0x2a8 = g_UnknownVector65b448;
    field_0x23c = 0;
    field_0x2c0 = 0;
    field_0x29c = g_UnknownVector65b438;
    field_0x2e4 = 0;
    field_0x276 = 0;
    field_0x2d8 = 0;
    field_0x228 = 27.0f;
    field_0x258 = field_0x16c;
    field_0x2f0 = field_0x16c;
    field_0x294 = 0;
    field_0x240 = 0;
    field_0x27c = 0;
    field_0x280 = 0;
    field_0x284 = 0;
    field_0x288 = 0;
    field_0x28c = 0;
    field_0x290 = 0;
    field_0x298 = 0;
    field_0x1d4 = 0;
    field_0x268 = 0;
    field_0x26c = 0;
    field_0x274 = 0;
    field_0x2dc = 0;
    field_0x2e0 = 0;
    field_0x2d4 = 7.0f;
    field_0x2ec = 3.0f;
    field_0x244 = 0;
    UnknownVirtualSlot71(0);
    field_0x25c = field_0x170;
    field_0x224 = field_0x220;
    field_0x248 = field_0x244;
    field_0x250 = field_0x244;
    field_0x24c = field_0x16c;
    field_0x230 = field_0x22c;
    field_0x238 = field_0x234;
    field_0x254 = field_0x16c;
    field_0x277 = 0;
    field_0x278 = 0;
    field_0x310 = 0;
    field_0x30c = 0;
    field_0x304 = 0;
    field_0x308 = 0;
    field_0x279 = 0;
    field_0x270 = 15.0f;
    field_0x340 = 0;
    field_0x2f4 = 10.0f;
    field_0x2f8 = 280.0f;
    field_0x2fc = 70.0f;
    field_0x300 = 20.0f;
}

// 0x00465000: when enabled and not forced, a point the +0x288..+0x290 values
// already sit on (within 0.01) is fed through slot 44 and reported as
// settled; +0x277 records that the check ran.
bool FollowCamera::UnknownVirtualSlot36(const Vector3& point, bool enable, bool force) {
    if (enable) {
        if (!force && !field_0x274 &&
            FollowCameraAbs(field_0x288->value - point.x) < 0.01f &&
            FollowCameraAbs(field_0x290->value - point.z) < 0.01f &&
            FollowCameraAbs(field_0x28c->value - point.y) < 0.01f) {
            UnknownVirtualSlot44(point);
            field_0x277 = true;
            return true;
        }
        field_0x277 = true;
        return false;
    }
    field_0x277 = false;
    return false;
}
