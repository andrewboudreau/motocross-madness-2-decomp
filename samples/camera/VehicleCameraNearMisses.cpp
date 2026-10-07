// Near-miss VehicleCamera candidates, kept out of src/reconstructed until they
// match. See docs/FOLLOW_CAMERA.md.
//
// VehicleCamera::UnknownVirtualSlot36 (0x0052cc80, 573 bytes): VC6 emits 568
// bytes, identical up to the disabled exit at +0x22b. That exit differs as in
// FollowCamera slot 36 (samples/camera/FollowCameraNearMisses.cpp): retail
// emits `xor al, al; mov [esi+0x277], al` and a separate `xor al, al` return;
// VC6 stores the immediate and merges the two `return false` tails. A named
// bool local, `return field_0x277 = false`, storing `force`/`enable` and an
// early-return ordering all give the same code.
#include "../../src/reconstructed/VehicleCamera.h"

// 0x0052cc80: FollowCamera slot 36 with vehicle tolerances: while the
// vehicle's +0x434 is above 0.1 a point within 10 of the current values is
// accepted, otherwise within 0.01.
bool VehicleCamera::UnknownVirtualSlot36(const Vector3& point, bool enable, bool force) {
    if (cameraState == 6)
        return true;
    if (enable) {
        if (vehicleMode) {
            if (vehicle->field_0x434 > 0.1f) {
                if (!force && field_0x274 &&
                    FollowCameraAbs(field_0x288->value - point.x) < 10.0f &&
                    FollowCameraAbs(field_0x290->value - point.z) < 10.0f &&
                    FollowCameraAbs(field_0x28c->value - point.y) < 10.0f) {
                    UnknownVirtualSlot44(point);
                    UnknownVirtualSlot43(cachedTarget);
                }
            } else {
                if (force) {
                    field_0x277 = true;
                    return false;
                }
                if (!field_0x274 &&
                    FollowCameraAbs(field_0x288->value - point.x) < 0.01f &&
                    FollowCameraAbs(field_0x290->value - point.z) < 0.01f &&
                    FollowCameraAbs(field_0x28c->value - point.y) < 0.01f) {
                    UnknownVirtualSlot44(point);
                    UnknownVirtualSlot43(cachedTarget);
                    field_0x277 = true;
                    return true;
                }
                field_0x277 = true;
                return field_0x274;
            }
        } else {
            UnknownVirtualSlot44(point);
        }
        field_0x277 = true;
        return true;
    }
    if (vehicleMode && vehicle->field_0x434 > 0.1f && field_0x277) {
        if (force)
            return true;
        field_0x277 = false;
        return false;
    }
    return false;
}
