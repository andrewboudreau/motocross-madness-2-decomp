#include "BikeCamera.h"

// The unit's per-file vectors (Math3D.h's four constants); their initializers
// 0x00417350..0x0041746b sit after slot 0x00417340 and build 0x00578e20,
// 0x00578e30, 0x00578e40 and 0x00578e10. No code in this unit reads them.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// By-value read of a vector's x. Slot 34's dot product needs it: retail loads
// the returned vector's x before the member's but the member's y and z first,
// and only this form gives that order (every term order, grouping, operand
// order, operator[], pointer/by-value parameters and a reference local keep
// the x product in the y/z order).
static inline float VectorX(const Vector3& v) { return v.x; }

// The inline dot product slot 34 uses; the grouping gives retail's y, x, z order.
static inline float BikeCameraDot(const Vector3& a, const Vector3& b) {
    return a.z * b.z + (VectorX(a) * b.x + a.y * b.y);
}

// 0x00416e20
BikeCamera::BikeCamera(int flags) : VehicleCamera(flags) {
    bike = 0;
}

// 0x00416e70: an explicit empty destructor.
BikeCamera::~BikeCamera() {}

// 0x00416e80
BikeCamera* BikeCamera::SetupBikeCamera(void* value, float rate294, float rate298, float value228,
                                              float value2d0, float value2e8, int capacity, int count,
                                              const int* list) {
    if (!SetupVehicleCamera(value, rate294, rate298, value228, value2d0, value2e8, capacity, count, list))
        return 0;
    return this;
}

// 0x00416ed0: table entries 0 (rider's head) and 1 (target point) with two
// values that depend on bike state 6, then slot 38.
void BikeCamera::UnknownVirtualSlot40(float a) {
    bool special = bike->crashReason == 6;
    float value10;
    float value14;
    if (special) {
        value10 = 2.5f;
        value14 = 4.0f;
    } else {
        value10 = 5.0f;
        value14 = 5.0f;
    }
    UnknownVehiclePart* headPart = bike->riderCharacter->modelNode->FindByName("Head");
    Vector3 head;
    headPart->GetPositionIn(0, &head);
    SetTableEntry(0, &head, 0, &value10, &value14, 0, 0);
    SetTableEntry(1, &targetPoint, 0, &value10, &value14, 0, 0);
    UnknownVirtualSlot38(a, bike->wrecker->wreckState != 1, special);
}

// 0x00416fb0: 38% of the way from the rider's head to the target point.
Vector3 BikeCamera::UnknownVirtualSlot37() {
    UnknownVehiclePart* headPart = bike->riderCharacter->modelNode->FindByName("Head");
    Vector3 head;
    headPart->GetPositionIn(0, &head);
    Vector3 result;
    result.x = (targetPoint.x - head.x) * 0.38f + head.x;
    result.y = (targetPoint.y - head.y) * 0.38f + head.y;
    result.z = (targetPoint.z - head.z) * 0.38f + head.z;
    return result;
}

// 0x00417050: in camera mode +0x276, the rider's part position led by the
// bike's +0x604 -> +0x40 -> +0x154 vector times dt (scaled by
// 4 - 0.006 * height below height 500); otherwise the target point plus the
// +0x29c direction times a distance built from the dot product with slot
// 33, the fov/zoom ratio and vehicle +0x438. Raised by 3.
Vector3 BikeCamera::UnknownVirtualSlot34(float dt) {
    Vector3 result;
    if (field_0x276) {
        bike->riderCharacter->modelNode->GetPositionIn(0, &result);
        if (field_0x2c0 < 500.0f) {
            Vector3 lead = bike->wrecker->rigidBody->velocity * dt;
            lead = lead * (4.0f - field_0x2c0 * 0.006f);
            result += lead;
        } else {
            result += bike->wrecker->rigidBody->velocity * dt;
        }
    } else {
        float dot = BikeCameraDot(UnknownVirtualSlot33(), field_0x29c);
        float height = field_0x2c0 * 0.009f + 1.0f;
        float ratio = fieldOfView / maxFieldOfView;
        float speed = vehicleMode ? vehicle->field_0x438 : 180.0f;
        float lag = (1.0f - field_0x2c0 * 0.00052631577f) * ratio;
        float distance = lag * (height * height * dot / speed) * 35.0f;
        Vector3 offset = field_0x29c * distance;
        result = Vector3(offset.x + targetPoint.x, offset.y + targetPoint.y, offset.z + targetPoint.z);
    }
    result.y += 3.0f;
    return result;
}

// 0x00417290: the rider's head position.
Vector3 BikeCamera::UnknownVirtualSlot50() {
    UnknownVehiclePart* headPart = bike->riderCharacter->modelNode->FindByName("Head");
    Vector3 head;
    headPart->GetPositionIn(0, &head);
    return head;
}

// 0x004172e0
void BikeCamera::UnknownVirtualSlot53() {
    bike->modelNode->UnknownFunction4444e0();
    bike->riderCharacter->modelNode->UnknownFunction4444e0();
}

// 0x00417310
void BikeCamera::UnknownVirtualSlot54() {
    bike->modelNode->ClearFlag14c();
    bike->riderCharacter->modelNode->ClearFlag14c();
}

// 0x00417340
float BikeCamera::UnknownVirtualSlot51() {
    return bike->savedCosRoll * bike->savedRoll;
}

// 0x004174b0
bool BikeCamera::UnknownVirtualSlot75() {
    return FollowCamera::UnknownVirtualSlot75() || !vehicleMode || !bike->crashState ||
           (bike->crashReason == 6 && bike->wrecker->wreckState != 3);
}
