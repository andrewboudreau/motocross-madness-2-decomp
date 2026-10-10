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
//
// VehicleCamera::UnknownVirtualSlot41 (0x0052c030, 1240 bytes, 1223/1240):
// same length, frame, slots and blocks. Two differences: the slow-vehicle
// branch's in-place normalisation multiplies x as `fld x; fmul st(1)` where
// VC6 emits `fld st(0); fmul x` (the shared tail of the other branches has
// VC6's form in retail too), and the no-vehicle branch stores y = 0 between
// the x*x and z*z products where VC6 stores it after both. `*=`, `v = v * s`
// and `v = s * v` forms and four squared-length associations do not move
// either. The 0.18/0.82 weights are literals (their __real constants sit
// after the vtable, first used by this unit) and `y * 0.5 * 0.18` needs the
// cast to stay two multiplies.
//
// VehicleCamera::UnknownVirtualSlot35 (0x0052c510, 1267 bytes, 813/1261):
// frame (0x48) and slot assignment match retail. Left: the squared length
// loads x*x before y*y in retail (here y*y first), and retail duplicates the
// x scaling into both range branches (`fld x; fmul st(1)` after
// distance * field_0x228, `fld st(0); fmul x` after the -0.25 range) where
// VC6 here shares one block. Tried without effect: the four groupings of
// Length's terms and of `along`, `direction * range`, `range * direction`
// and per-component scaling in the range branch.
// `a` is tested as a byte (FollowCamera.h declares it unsigned char).
#include "../../src/reconstructed/VehicleCamera.h"

static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

float FastInvSqrt(float value); // 0x00460c00

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

static inline Vector3 Add(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline Vector3 Sub(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

// Normalizes `v` in place; the zero vector stays zero.
static inline void NormalizeVector(Vector3* v) {
    float lengthSquared = v->z * v->z + (v->x * v->x + v->y * v->y);
    if (lengthSquared == 0.0f) {
        *v = kVec3Zero;
    } else {
        float scale = FastInvSqrt(lengthSquared);
        v->x *= scale;
        v->y *= scale;
        v->z *= scale;
    }
}

// 0x0052c030: the heading the camera follows (slot 10 stores it at +0x29c),
// flattened to the ground plane and normalised.
Vector3 VehicleCamera::UnknownVirtualSlot41() {
    Vector3 heading;
    if (vehicleMode) {
        if (FollowCameraAbs(vehicle->field_0x434) < 10.0f) {
            Vector3 velocity = UnknownVirtualSlot33();
            if (!vehicle->field_0x440)
                velocity = Vector3(-velocity.x, -velocity.y, -velocity.z);
            velocity.y = 0.0f;
            if (velocity.x == 0.0f && velocity.z == 0.0f)
                velocity = vehicle->field_0x88;
            float scale = vehicle->field_0xbc;
            if (scale <= 0.01f)
                scale = 0.01f;
            Vector3 facing = vehicle->field_0x88 * scale;
            facing.y = 0.0f;
            if (facing.x == 0.0f && facing.z == 0.0f)
                facing = velocity;
            heading = Add(Sub(velocity, facing) * (FollowCameraAbs(vehicle->field_0x434) * 0.1f), facing);
            if (!vehicle->field_0x440)
                heading.y = heading.y * 0.5f;
            else
                heading.y = 0.0f;
            NormalizeVector(&heading);
            heading.x = field_0x29c.x * 0.82f + heading.x * 0.18f;
            heading.y = field_0x29c.y * 0.82f + heading.y * 0.18f;
            heading.z = field_0x29c.z * 0.82f + heading.z * 0.18f;
        } else {
            heading = UnknownVirtualSlot33();
            if (!vehicle->field_0x440) {
                heading = Vector3(-heading.x, -heading.y, -heading.z);
                heading.y = (float)(heading.y * 0.5f) * 0.18f + field_0x29c.y * 0.82f;
            } else {
                heading.y = 0.0f;
            }
            NormalizeVector(&heading);
        }
    } else {
        Vector3 other;
        if (field_0x384) {
            UnknownVehiclePart* model = field_0x384->field_0x1a0;
            if (model->field_0x140)
                model->field_0x140->UnknownFunction4fc4f0(&heading, &other);
            else
                model->UnknownFunction4fc4f0(&heading, &other);
        } else if (field_0x388) {
            field_0x388->field_0x34->UnknownFunction4fc4f0(&heading, &other);
        } else {
            heading = kVec3ZAxis;
        }
        heading.y = 0.0f;
        NormalizeVector(&heading);
    }
    return heading;
}

static inline void ScaleInPlace(Vector3* v, float s) {
    v->x *= s;
    v->y *= s;
    v->z *= s;
}

// 1 - |angle| / (pi / 2), at least 0.
static inline float AngleWeight(float angle) {
    float weight = 1.0f - FollowCameraAbs(angle) * 0.63661975f;
    if (0.0f > weight)
        weight = 0.0f;
    return weight;
}

// |v|, exact when it is 1.
static inline float Length(const Vector3& v) {
    float squared = v.x * v.x + v.y * v.y + v.z * v.z;
    if (squared == 1.0f)
        return 1.0f;
    return UnknownFunction460b50(squared);
}

// `value` scaled down while climbing or diving faster than 5.
static inline float ClimbScale(float y, float length, float value) {
    if (length != 0.0f) {
        if (y < -5.0f)
            return (y / length + 1.0f) * value;
        if (y > 5.0f)
            return (1.0f - y / length) * value;
    }
    return value;
}

// 0x0052c510: the chase position: behind the vehicle along the heading
// (+0x29c) turned by +0x308 (and, in state 3, pitched by the vehicle's
// +0x4c), at a distance scaled by the preset angles, zoom and speed, raised
// by a share of the climb and eased up over the rider when the subject
// lifts the point.
Vector3 VehicleCamera::UnknownVirtualSlot35(unsigned char a, float dt) {
    float distance;
    float offset;
    if (cameraState != 3) {
        Vector3 velocity = UnknownVirtualSlot33();
        float along = velocity.z * field_0x29c.z + (velocity.x * field_0x29c.x + velocity.y * field_0x29c.y);
        distance = AngleWeight(field_0x234) * AngleWeight(field_0x22c);
        float zoom = field_0x16c / field_0x1dc;
        float scale = (float)(field_0x220 * 0.0111f) * zoom + 1.0f;
        if (a) {
            offset = 0.0f;
        } else {
            float speed = vehicleMode ? vehicle->field_0x438 : 180.0f;
            offset = (float)((1.0f - field_0x220 * 0.0016666667f) * 1.8f) * (scale * scale * along / speed);
        }
        float length = vehicleMode ? vehicle->field_0xbc : Length(velocity);
        distance = zoom * distance + ClimbScale(velocity.y, length, offset);
        offset = (1.0f - zoom) * (velocity.y * 0.25f);
    } else {
        distance = 1.0f;
        offset = 0.0f;
    }
    if (field_0x276)
        distance *= 0.8f;
    Vector3 direction;
    if (cameraState == 3) {
        Vector3 side;
        side.x = field_0x29c.z;
        side.y = 0.0f;
        side.z = -field_0x29c.x;
        Vector3 pitched;
        D3DRMVectorRotate(&pitched, &field_0x29c, &side, -vehicle->field_0x4c);
        D3DRMVectorRotate(&direction, &pitched, (Vector3*)&kVec3YAxis, field_0x308 * 0.7f);
    } else {
        D3DRMVectorRotate(&direction, &field_0x29c, (Vector3*)&kVec3YAxis, field_0x308 * 0.7f);
    }
    if (cameraState != 6) {
        ScaleInPlace(&direction, distance * field_0x228);
    } else {
        float range = -field_0x228;
        range *= 0.25f;
        ScaleInPlace(&direction, range);
    }
    Vector3 position = Add(direction, targetPoint);
    Vector3 result = position;
    result.y = position.y + offset;
    if (vehicle->field_0x544 > 0) {
        if (vehicle->UnknownInlineRider()->field_0x150 > -0.5f) {
            float reach = (float)(FollowCameraAbs(field_0x22c) * 0.63661975f) * 1.2f + 0.3f;
            Vector3 ahead = Add(direction * reach, targetPoint);
            Vector3 ground = ahead;
            field_0x240->UnknownFunction507c10(&ground, 0, 0, 0);
            if (ground.y > ahead.y)
                field_0x2d8 = (ground.y - ahead.y) * 0.6f;
            else
                field_0x2d8 *= 0.01f;
        } else {
            field_0x2d8 *= 0.01f;
        }
        result.y += field_0x2d8;
    }
    result.y += 3.0f;
    return result;
}
