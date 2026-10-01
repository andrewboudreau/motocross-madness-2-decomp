#include "VehicleCamera.h"

// 0x0052b920
VehicleCamera::VehicleCamera(int flags) : FollowCamera(flags) {
    field_0x394 = 0;
    field_0x390 = true;
    field_0x391 = 0;
    field_0x392 = 0;
    field_0x388 = 0;
    field_0x38c = 0;
    field_0x384 = 0;
    field_0x3a4 = g_UnknownVector68a728;
    field_0x398 = g_UnknownVector68a728;
}

// 0x0052b9d0: an explicit empty destructor (vptr store, then FollowCamera's).
VehicleCamera::~VehicleCamera() {}

// 0x0052bfa0: preset; distance 17 in vehicle mode, else 60.
void VehicleCamera::UnknownVirtualSlot67() {
    field_0x22c = 0.75f;
    field_0x234 = 3.1415927f;
    field_0x220 = field_0x390 ? g_UnknownFloat558d60 : 60.0f;
    field_0x258 = 85.0f;
}

// 0x0052bff0: holds while the vehicle reports +0x444; otherwise cycles the
// state list in vehicle mode or resets to state 0.
void VehicleCamera::UnknownVirtualSlot72() {
    if (field_0x390 && field_0x394 && field_0x394->field_0x444)
        return;
    if (!field_0x390) {
        field_0x244 = 0;
        UnknownVirtualSlot71(0);
    } else {
        FollowCamera::UnknownVirtualSlot72();
    }
}

// 0x0052cb80: the vehicle's position 0, raised by 5.
Vector3 VehicleCamera::UnknownVirtualSlot50() {
    Vector3 position;
    field_0x394->field_0x3bc->UnknownFunction4fc9a0(0, &position);
    position.y += 5.0f;
    return position;
}

// 0x0052cbd0: the vehicle's +0x45c; also turns +0x23c by pi when the vehicle
// flags +0x464.
float VehicleCamera::UnknownVirtualSlot39() {
    float value = field_0x394->field_0x45c;
    if (field_0x394->field_0x464)
        field_0x23c += 3.1415927f;
    return value;
}

// 0x0052cec0
float VehicleCamera::UnknownVirtualSlot51() {
    return field_0x394->field_0x48;
}

// 0x0052d010
int VehicleCamera::UnknownVirtualSlot75() {
    return FollowCamera::UnknownVirtualSlot75() || UnknownInlineIdle();
}
