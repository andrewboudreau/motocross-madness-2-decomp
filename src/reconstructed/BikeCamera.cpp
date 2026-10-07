#include "BikeCamera.h"

// The unit's per-file vectors (Math3D.h's four constants); their initializers
// 0x00417350..0x0041746b sit after slot 0x00417340 and build 0x00578e20,
// 0x00578e30, 0x00578e40 and 0x00578e10. No code in this unit reads them.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// 0x00416e20
BikeCamera::BikeCamera(int flags) : VehicleCamera(flags) {
    bike = 0;
}

// 0x00416e70: an explicit empty destructor.
BikeCamera::~BikeCamera() {}

// 0x00416e80
BikeCamera* BikeCamera::UnknownFunction416e80(void* value, float rate294, float rate298, float value228,
                                              float value2d0, float value2e8, int capacity, int count,
                                              const int* list) {
    if (!UnknownFunction52b9e0(value, rate294, rate298, value228, value2d0, value2e8, capacity, count, list))
        return 0;
    return this;
}

// 0x00416ed0: table entries 0 (rider's head) and 1 (target point) with two
// values that depend on bike state 6, then slot 38.
void BikeCamera::UnknownVirtualSlot40(float a) {
    bool special = bike->field_0x460 == 6;
    float value10;
    float value14;
    if (special) {
        value10 = 2.5f;
        value14 = 4.0f;
    } else {
        value10 = 5.0f;
        value14 = 5.0f;
    }
    UnknownVehiclePart* headPart = bike->field_0x5c4->field_0x1a0->UnknownFunction4fdae0("Head");
    Vector3 head;
    headPart->UnknownFunction4fc9a0(0, &head);
    UnknownFunction463450(0, &head, 0, &value10, &value14, 0, 0);
    UnknownFunction463450(1, &targetPoint, 0, &value10, &value14, 0, 0);
    UnknownVirtualSlot38(a, bike->field_0x604->field_0x44 != 1, special);
}

// 0x00416fb0: 38% of the way from the rider's head to the target point.
Vector3 BikeCamera::UnknownVirtualSlot37() {
    UnknownVehiclePart* headPart = bike->field_0x5c4->field_0x1a0->UnknownFunction4fdae0("Head");
    Vector3 head;
    headPart->UnknownFunction4fc9a0(0, &head);
    Vector3 result;
    result.x = (targetPoint.x - head.x) * 0.38f + head.x;
    result.y = (targetPoint.y - head.y) * 0.38f + head.y;
    result.z = (targetPoint.z - head.z) * 0.38f + head.z;
    return result;
}

// 0x00417290: the rider's head position.
Vector3 BikeCamera::UnknownVirtualSlot50() {
    UnknownVehiclePart* headPart = bike->field_0x5c4->field_0x1a0->UnknownFunction4fdae0("Head");
    Vector3 head;
    headPart->UnknownFunction4fc9a0(0, &head);
    return head;
}

// 0x004172e0
void BikeCamera::UnknownVirtualSlot53() {
    bike->field_0x3bc->UnknownFunction4444e0();
    bike->field_0x5c4->field_0x1a0->UnknownFunction4444e0();
}

// 0x00417310
void BikeCamera::UnknownVirtualSlot54() {
    bike->field_0x3bc->UnknownFunction4fdb50();
    bike->field_0x5c4->field_0x1a0->UnknownFunction4fdb50();
}

// 0x00417340
float BikeCamera::UnknownVirtualSlot51() {
    return bike->field_0x58 * bike->field_0x48;
}

// 0x004174b0
bool BikeCamera::UnknownVirtualSlot75() {
    return FollowCamera::UnknownVirtualSlot75() || !vehicleMode || !bike->field_0x444 ||
           (bike->field_0x460 == 6 && bike->field_0x604->field_0x44 != 3);
}
