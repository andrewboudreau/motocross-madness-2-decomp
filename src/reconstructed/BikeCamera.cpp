#include "BikeCamera.h"

// 0x00416e20
BikeCamera::BikeCamera(int flags) : VehicleCamera(flags) {
    field_0x3b0 = 0;
}

// 0x00416e70: an explicit empty destructor.
BikeCamera::~BikeCamera() {}

// 0x00416fb0: 38% of the way from the rider's head to the target point.
Vector3 BikeCamera::UnknownVirtualSlot37() {
    UnknownVehiclePart* headPart = field_0x3b0->field_0x5c4->field_0x1a0->UnknownFunction4fdae0("Head");
    Vector3 head;
    headPart->UnknownFunction4fc9a0(0, &head);
    Vector3 result;
    result.x = (field_0x2b4.x - head.x) * 0.38f + head.x;
    result.y = (field_0x2b4.y - head.y) * 0.38f + head.y;
    result.z = (field_0x2b4.z - head.z) * 0.38f + head.z;
    return result;
}

// 0x00417290: the rider's head position.
Vector3 BikeCamera::UnknownVirtualSlot50() {
    UnknownVehiclePart* headPart = field_0x3b0->field_0x5c4->field_0x1a0->UnknownFunction4fdae0("Head");
    Vector3 head;
    headPart->UnknownFunction4fc9a0(0, &head);
    return head;
}

// 0x004172e0
void BikeCamera::UnknownVirtualSlot53() {
    field_0x3b0->field_0x3bc->UnknownFunction4444e0();
    field_0x3b0->field_0x5c4->field_0x1a0->UnknownFunction4444e0();
}

// 0x00417310
void BikeCamera::UnknownVirtualSlot54() {
    field_0x3b0->field_0x3bc->UnknownFunction4fdb50();
    field_0x3b0->field_0x5c4->field_0x1a0->UnknownFunction4fdb50();
}

// 0x00417340
float BikeCamera::UnknownVirtualSlot51() {
    return field_0x3b0->field_0x58 * field_0x3b0->field_0x48;
}

// 0x004174b0
int BikeCamera::UnknownVirtualSlot75() {
    if (FollowCamera::UnknownVirtualSlot75() || !field_0x390 || !field_0x3b0->field_0x444)
        return 1;
    if (field_0x3b0->field_0x460 == 6 && field_0x3b0->field_0x604->field_0x44 != 3)
        return 1;
    return 0;
}
