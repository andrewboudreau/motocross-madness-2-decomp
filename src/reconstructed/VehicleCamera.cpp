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
    targetPointCached = 0;
    targetHeadingCached = 0;
    targetCar = 0;
    targetCaster = 0;
    targetCharacter = 0;
    cachedTargetPoint = kVec3Zero;
    cachedTargetHeading = kVec3Zero;
}

// 0x0052b9d0: an explicit empty destructor (vptr store, then FollowCamera's).
VehicleCamera::~VehicleCamera() {}

// 0x0052b9e0
VehicleCamera* VehicleCamera::SetupVehicleCamera(void* value, float rate294, float rate298, float value228,
                                                    float value2d0, float value2e8, int capacity, int count,
                                                    const int* list) {
    if (!SetupFollowCamera(value, rate294, rate298, value228, value2d0, value2e8, capacity, count, list))
        return 0;
    return this;
}

// 0x0052ba30: the tracked point: the vehicle's in vehicle mode, else the
// first set target, else the global default.
Vector3 VehicleCamera::UnknownVirtualSlot33() {
    if (vehicleMode)
        return vehicle->velocity;
    if (targetCharacter)
        return targetCharacter->nodeVelocity;
    if (targetCar)
        return targetCar->pathVelocity;
    return kVec3Zero;
}

// 0x0052bac0
Vector3 VehicleCamera::CachedTargetHeading() {
    if (vehicleMode) {
        targetHeadingCached = 0;
        return vehicle->takeoffVelocity;
    }
    if (!targetHeadingCached) {
        cachedTargetHeading = UnknownVirtualSlot33();
        targetHeadingCached = 1;
    }
    return cachedTargetHeading;
}

// 0x0052bb60: part 0 of the vehicle's +0x3bc in vehicle mode; else the first
// target's model (its +0x140 child when set), the second target's +0x34
// translation, the +0x38c translation, or the global default.
Vector3 VehicleCamera::TargetPoint() {
    Vector3 position;
    if (vehicleMode) {
        vehicle->modelNode->GetPositionIn(0, &position);
        return position;
    }
    if (targetCharacter) {
        UnknownVehiclePart* model = targetCharacter->modelNode;
        if (model->firstChild)
            model->firstChild->GetPositionIn(0, &position);
        else
            model->GetPositionIn(0, &position);
        return position;
    }
    if (targetCar) {
        targetCar->carModel->GetPosition(&position);
        return position;
    }
    if (targetCaster) {
        targetCaster->GetPosition(&position);
        return position;
    }
    return kVec3Zero;
}

// 0x0052bc50
Vector3 VehicleCamera::CachedTargetPoint() {
    if (vehicleMode) {
        targetPointCached = 0;
        return vehicle->takeoffPosition;
    }
    if (!targetPointCached) {
        cachedTargetPoint = TargetPoint();
        targetPointCached = 1;
    }
    return cachedTargetPoint;
}

static inline Vector3 Add(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

// 0x0052bcf0: places the camera beside and behind the heading, leads it by
// the heading's climb (y / 32.2 * 1.5), keeps it above the base point and
// derives the field of view (+0x258, 10..70) from the horizontal distance to
// the raised target point, then aims at that point (Camera slot 29). Retail
// multiplies by 1/32.2 and 1.5 separately (VC6 folds the two constants of a
// plain product, hence the float cast), and the 0.19 lift reuses the y of
// heading * lead.
void VehicleCamera::UnknownVirtualSlot68() {
    Vector3 heading = CachedTargetHeading();
    Vector3 base = CachedTargetPoint();
    float lead = (float)(heading.y * 0.0310559f) * 1.5f;
    Vector3 side(heading.z, 0.0f, -heading.x);
    eyePosition = Add(Add(Add(base, heading * lead), heading * -0.2f), side * 0.45f);
    subject->QueryGround(&eyePosition, 0, 0, 0);
    if (heading.y > 10.0f)
        base.y += (heading * lead).y * 0.19f;
    if (vehicleMode) {
        float low = eyePosition.y + 7.0f;
        float high = base.y - 1.0f;
        eyePosition.y = high > low ? high : low;
    } else {
        float low = eyePosition.y + 21.0f;
        float high = base.y - 1.0f;
        eyePosition.y = high > low ? high : low;
    }
    Vector3 target = targetPoint;
    target.y += 3.0f;
    float dx = FollowCameraAbs(eyePosition.x - target.x);
    float dz = FollowCameraAbs(eyePosition.z - target.z);
    float closeness = (200.0f - FastSqrt(dx * dx + dz * dz)) * 0.0055555557f;
    targetFieldOfView = closeness * 60.0f + 10.0f;
    if (targetFieldOfView < 10.0f)
        targetFieldOfView = 10.0f;
    if (targetFieldOfView > 70.0f)
        targetFieldOfView = 70.0f;
    LookAt(target);
}

// 0x0052ca10: 0x0052bb60's position, led by the tracked point (slot 33)
// scaled by dt (at most 0.2) while the camera is free to follow.
Vector3 VehicleCamera::UnknownVirtualSlot57(float dt) {
    Vector3 result = TargetPoint();
    if (cameraState != 7 && !g_TrackGame->paused) {
        if (vehicleMode) {
            if (!vehicle->crashState) {
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
    followPitch = 0.75f;
    followYaw = 3.1415927f;
    followDistance = vehicleMode ? g_UnknownFloat558d60 : 60.0f;
    targetFieldOfView = 85.0f;
}

// 0x0052bff0: holds while the vehicle reports +0x444; otherwise cycles the
// state list in vehicle mode or resets to state 0.
void VehicleCamera::UnknownVirtualSlot72() {
    if (vehicleMode && vehicle && vehicle->crashState)
        return;
    if (!vehicleMode) {
        cameraState = 0;
        UnknownVirtualSlot71(0);
    } else {
        FollowCamera::UnknownVirtualSlot72();
    }
}

float FastInvSqrt(float value); // 0x00460c00

static inline Vector3 Sub(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline float LengthSquared(const Vector3& v) {
    return v.z * v.z + ((v.x * v.x) + (v.y * v.y));
}

// 0x0052c030: the heading the camera follows (slot 10 stores it at +0x29c),
// flattened to the ground plane and normalised. The three normalisations are
// written out with the squared length through LengthSquared: retail's slow
// branch scales x as `fld x; fmul st(1)`, which an inline normalising helper
// (or a squared length written in the body) turns into `fld st(0); fmul x`.
// The lead weight is `/ 10.0f` (the reciprocal constant 0x005511cc, not the
// literal 0.1f at 0x005507d0).
Vector3 VehicleCamera::UnknownVirtualSlot41() {
    Vector3 heading;
    if (vehicleMode) {
        if (FollowCameraAbs(vehicle->forwardSpeed) < 10.0f) {
            Vector3 velocity = UnknownVirtualSlot33();
            if (!vehicle->movingForward)
                velocity = Vector3(-velocity.x, -velocity.y, -velocity.z);
            velocity.y = 0.0f;
            if (velocity.x == 0.0f && velocity.z == 0.0f)
                velocity = vehicle->bodyForward;
            float scale = vehicle->linearSpeed;
            if (scale <= 0.01f)
                scale = 0.01f;
            Vector3 facing = vehicle->bodyForward * scale;
            facing.y = 0.0f;
            if (facing.x == 0.0f && facing.z == 0.0f)
                facing = velocity;
            heading = Add(Sub(velocity, facing) * (FollowCameraAbs(vehicle->forwardSpeed) / 10.0f), facing);
            if (!vehicle->movingForward)
                heading.y = heading.y * 0.5f;
            else
                heading.y = 0.0f;
            float lengthSquared = LengthSquared(heading);
            if (lengthSquared == 0.0f) {
                heading = kVec3Zero;
            } else {
                float inverseLength = FastInvSqrt(lengthSquared);
                heading.x *= inverseLength;
                heading.y *= inverseLength;
                heading.z *= inverseLength;
            }
            heading.x = followHeading.x * 0.82f + heading.x * 0.18f;
            heading.y = followHeading.y * 0.82f + heading.y * 0.18f;
            heading.z = followHeading.z * 0.82f + heading.z * 0.18f;
        } else {
            heading = UnknownVirtualSlot33();
            if (!vehicle->movingForward) {
                heading = Vector3(-heading.x, -heading.y, -heading.z);
                heading.y = (float)(heading.y * 0.5f) * 0.18f + followHeading.y * 0.82f;
            } else {
                heading.y = 0.0f;
            }
            float lengthSquared = LengthSquared(heading);
            if (lengthSquared == 0.0f) {
                heading = kVec3Zero;
            } else {
                float inverseLength = FastInvSqrt(lengthSquared);
                heading.x *= inverseLength;
                heading.y *= inverseLength;
                heading.z *= inverseLength;
            }
        }
    } else {
        Vector3 other;
        if (targetCharacter) {
            UnknownVehiclePart* model = targetCharacter->modelNode;
            if (model->firstChild)
                model->firstChild->GetAxes(&heading, &other);
            else
                model->GetAxes(&heading, &other);
        } else if (targetCar) {
            targetCar->carModel->GetAxes(&heading, &other);
        } else {
            heading = kVec3ZAxis;
        }
        heading.y = 0.0f;
        float lengthSquared = LengthSquared(heading);
        if (lengthSquared == 0.0f) {
            heading = kVec3Zero;
        } else {
            float inverseLength = FastInvSqrt(lengthSquared);
            heading.x *= inverseLength;
            heading.y *= inverseLength;
            heading.z *= inverseLength;
        }
    }
    return heading;
}

// 0x0052cb80: the vehicle's position 0, raised by 5.
Vector3 VehicleCamera::UnknownVirtualSlot50() {
    Vector3 position;
    vehicle->modelNode->GetPositionIn(0, &position);
    position.y += 5.0f;
    return position;
}

// 0x0052cbd0: the vehicle's +0x45c; also turns +0x23c by pi when the vehicle
// flags +0x464.
float VehicleCamera::UnknownVirtualSlot39() {
    float value = vehicle->crashYaw;
    if (vehicle->crashedUpsideDown)
        worldYaw += 3.1415927f;
    return value;
}

// 0x0052cc00: +0x308 from the fov/zoom ratio and the vehicle's +0x43c.
void VehicleCamera::UnknownVirtualSlot42(bool flag) {
    float ratio = fieldOfView / maxFieldOfView;
    if (cameraState != 3) {
        if (!flag && vehicleMode)
            turnYaw = ratio * vehicle->turnRate * 0.3f;
        else
            turnYaw = 0.0f;
    } else {
        turnYaw = ratio * vehicle->turnRate * 0.4f;
    }
}

// 0x0052cc80: FollowCamera slot 36 with vehicle tolerances: while the
// vehicle's +0x434 is above 0.1 a point within 10 of the current values is
// accepted, otherwise within 0.01. The two `result` joins are retail's
// shapes: a constant `return` gets an immediate +0x277 store and shares the
// final `return false` tail, where retail materialises the 0 in al
// (`xor al, al; mov [esi+0x277], al`) and keeps a separate final tail.
bool VehicleCamera::UnknownVirtualSlot36(const Vector3& point, bool enable, bool force) {
    if (cameraState == 6)
        return true;
    if (enable) {
        if (vehicleMode) {
            if (vehicle->forwardSpeed > 0.1f) {
                if (!force && isSettled &&
                    FollowCameraAbs(positionEaseX->value - point.x) < 10.0f &&
                    FollowCameraAbs(positionEaseZ->value - point.z) < 10.0f &&
                    FollowCameraAbs(positionEaseY->value - point.y) < 10.0f) {
                    UnknownVirtualSlot44(point);
                    UnknownVirtualSlot43(cachedTarget);
                }
            } else {
                bool result;
                if (force)
                    result = false;
                else {
                    if (!isSettled &&
                        FollowCameraAbs(positionEaseX->value - point.x) < 0.01f &&
                        FollowCameraAbs(positionEaseZ->value - point.z) < 0.01f &&
                        FollowCameraAbs(positionEaseY->value - point.y) < 0.01f) {
                        UnknownVirtualSlot44(point);
                        UnknownVirtualSlot43(cachedTarget);
                        settleChecked = true;
                        return true;
                    }
                    result = isSettled;
                }
                settleChecked = true;
                return result;
            }
        } else {
            UnknownVirtualSlot44(point);
        }
        settleChecked = true;
        return true;
    }
    if (vehicleMode && vehicle->forwardSpeed > 0.1f && settleChecked) {
        bool result;
        if (force)
            result = true;
        else {
            result = false;
            settleChecked = result;
        }
        return result;
    }
    return false;
}

// 0x0052cec0
float VehicleCamera::UnknownVirtualSlot51() {
    return vehicle->savedRoll;
}

// 0x0052d010
bool VehicleCamera::UnknownVirtualSlot75() {
    return FollowCamera::UnknownVirtualSlot75() || UnknownInlineIdle();
}
