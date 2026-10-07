#include "VehicleCamera.h"
#include "TrackGame.h"

// The four Math3D vector constants (.CRT$XCU 336-339, initialisers
// 0x0052ced0..0x0052d00b): 0x0068a728, 0x0068a738, 0x0068a748 and
// 0x0068a718. The constructor and 0x0052bb60 read the zero vector.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// 0x0052b920
VehicleCamera::VehicleCamera(int flags) : FollowCamera(flags) {
    vehicle = 0;
    vehicleMode = true;
    field_0x391 = 0;
    field_0x392 = 0;
    field_0x388 = 0;
    field_0x38c = 0;
    field_0x384 = 0;
    field_0x3a4 = kVec3Zero;
    field_0x398 = kVec3Zero;
}

// 0x0052b9d0: an explicit empty destructor (vptr store, then FollowCamera's).
VehicleCamera::~VehicleCamera() {}

// 0x0052b9e0
VehicleCamera* VehicleCamera::UnknownFunction52b9e0(void* value, float rate294, float rate298, float value228,
                                                    float value2d0, float value2e8, int capacity, int count,
                                                    const int* list) {
    if (!UnknownFunction463140(value, rate294, rate298, value228, value2d0, value2e8, capacity, count, list))
        return 0;
    return this;
}

// 0x0052ba30: the tracked point: the vehicle's in vehicle mode, else the
// first set target, else the global default.
Vector3 VehicleCamera::UnknownVirtualSlot33() {
    if (vehicleMode)
        return vehicle->field_0x64;
    if (field_0x384)
        return field_0x384->field_0x224;
    if (field_0x388)
        return field_0x388->field_0x40;
    return kVec3Zero;
}

// 0x0052bac0
Vector3 VehicleCamera::UnknownFunction52bac0() {
    if (vehicleMode) {
        field_0x392 = 0;
        return vehicle->field_0x494;
    }
    if (!field_0x392) {
        field_0x398 = UnknownVirtualSlot33();
        field_0x392 = 1;
    }
    return field_0x398;
}

// 0x0052bb60: part 0 of the vehicle's +0x3bc in vehicle mode; else the first
// target's model (its +0x140 child when set), the second target's +0x34
// translation, the +0x38c translation, or the global default.
Vector3 VehicleCamera::UnknownFunction52bb60() {
    Vector3 position;
    if (vehicleMode) {
        vehicle->field_0x3bc->UnknownFunction4fc9a0(0, &position);
        return position;
    }
    if (field_0x384) {
        UnknownVehiclePart* model = field_0x384->field_0x1a0;
        if (model->field_0x140)
            model->field_0x140->UnknownFunction4fc9a0(0, &position);
        else
            model->UnknownFunction4fc9a0(0, &position);
        return position;
    }
    if (field_0x388) {
        field_0x388->field_0x34->UnknownFunction4fc970(&position);
        return position;
    }
    if (field_0x38c) {
        field_0x38c->UnknownFunction4fc970(&position);
        return position;
    }
    return kVec3Zero;
}

// 0x0052bc50
Vector3 VehicleCamera::UnknownFunction52bc50() {
    if (vehicleMode) {
        field_0x391 = 0;
        return vehicle->field_0x488;
    }
    if (!field_0x391) {
        field_0x3a4 = UnknownFunction52bb60();
        field_0x391 = 1;
    }
    return field_0x3a4;
}

// 0x0052ca10: 0x0052bb60's position, led by the tracked point (slot 33)
// scaled by dt (at most 0.2) while the camera is free to follow.
Vector3 VehicleCamera::UnknownVirtualSlot57(float dt) {
    Vector3 result = UnknownFunction52bb60();
    if (cameraState != 7 && !g_UnknownGlobal56e26c->field_0x1c4) {
        if (vehicleMode) {
            if (!vehicle->field_0x444) {
                float lead = 0.2f < dt ? 0.2f : dt;
                result += UnknownVirtualSlot33() * lead;
            }
        } else {
            float lead = 0.2f < dt ? 0.2f : dt;
            result += UnknownVirtualSlot33() * lead;
        }
    }
    return result;
}

// 0x0052bfa0: preset; distance 17 in vehicle mode, else 60.
void VehicleCamera::UnknownVirtualSlot67() {
    field_0x22c = 0.75f;
    field_0x234 = 3.1415927f;
    field_0x220 = vehicleMode ? g_UnknownFloat558d60 : 60.0f;
    field_0x258 = 85.0f;
}

// 0x0052bff0: holds while the vehicle reports +0x444; otherwise cycles the
// state list in vehicle mode or resets to state 0.
void VehicleCamera::UnknownVirtualSlot72() {
    if (vehicleMode && vehicle && vehicle->field_0x444)
        return;
    if (!vehicleMode) {
        cameraState = 0;
        UnknownVirtualSlot71(0);
    } else {
        FollowCamera::UnknownVirtualSlot72();
    }
}

// 0x0052cb80: the vehicle's position 0, raised by 5.
Vector3 VehicleCamera::UnknownVirtualSlot50() {
    Vector3 position;
    vehicle->field_0x3bc->UnknownFunction4fc9a0(0, &position);
    position.y += 5.0f;
    return position;
}

// 0x0052cbd0: the vehicle's +0x45c; also turns +0x23c by pi when the vehicle
// flags +0x464.
float VehicleCamera::UnknownVirtualSlot39() {
    float value = vehicle->field_0x45c;
    if (vehicle->field_0x464)
        field_0x23c += 3.1415927f;
    return value;
}

// 0x0052cc00: +0x308 from the fov/zoom ratio and the vehicle's +0x43c.
void VehicleCamera::UnknownVirtualSlot42(bool flag) {
    float ratio = field_0x16c / field_0x1dc;
    if (cameraState != 3) {
        if (!flag && vehicleMode)
            field_0x308 = ratio * vehicle->field_0x43c * 0.3f;
        else
            field_0x308 = 0.0f;
    } else {
        field_0x308 = ratio * vehicle->field_0x43c * 0.4f;
    }
}

// 0x0052cec0
float VehicleCamera::UnknownVirtualSlot51() {
    return vehicle->field_0x48;
}

// 0x0052d010
bool VehicleCamera::UnknownVirtualSlot75() {
    return FollowCamera::UnknownVirtualSlot75() || UnknownInlineIdle();
}
