// Near-miss BikeCamera candidates, kept out of src/reconstructed until they
// match. See docs/FOLLOW_CAMERA.md.
//
// BikeCamera slot 34 (0x00417050, 573 bytes; vtable 0x00550bc0 entry 34):
// the followed point, KrustyBikeCamera slot 34 (0x00498340) without the
// race-view test. Strict exact here (573/573) as a helper of a derived
// view, so it is not registered: its argument is the frame time as a float
// (FollowCamera slot 48 passes slot 10's dt through), while FollowCamera.h
// declares slot 34 (and slot 48) with an int, and the registered
// FollowCamera/KrustyBikeCamera symbols carry that signature.
// Source form that mattered: retail loads the returned vector's x before
// the member's (`fld [eax]; fmul [esi+0x29c]`) but the member's y and z
// first. Only a by-value read of the returned x (`VectorX(a)`) gives that;
// every term order, grouping, per-term operand order, operator[] access,
// pointer or by-value parameters and a reference local leave the x product
// in the y/z order (scratch sweep of 384 forms).
#include "../../src/reconstructed/BikeCamera.h"

static inline float VectorX(const Vector3& v) { return v.x; }

// The inline dot product; the grouping gives retail's y, x, z order.
static inline float BikeCameraDot(const Vector3& a, const Vector3& b) {
    return a.z * b.z + (VectorX(a) * b.x + a.y * b.y);
}

struct BikeCameraSlot34View : public BikeCamera {
    BikeCameraSlot34View() : BikeCamera(0) {}
    Vector3 UnknownVirtualSlot34Float(float t);
};

// 0x00417050: in camera mode +0x276, the rider's part position led by the
// bike's +0x604 -> +0x40 -> +0x154 vector times `t` (scaled by
// 4 - 0.006 * height below height 500); otherwise the target point plus the
// +0x29c direction times a distance built from the dot product with slot
// 33, the fov/zoom ratio and vehicle +0x438. Raised by 3.
Vector3 BikeCameraSlot34View::UnknownVirtualSlot34Float(float t) {
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
    } else {
        float dot = BikeCameraDot(UnknownVirtualSlot33(), field_0x29c);
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
