// Near-miss VehicleCamera candidates, kept out of src/reconstructed until they
// match. See docs/FOLLOW_CAMERA.md.
//
// VehicleCamera::UnknownVirtualSlot36 (0x0052cc80) is exact in src/reconstructed/VehicleCamera.cpp.
//
// VehicleCamera::UnknownVirtualSlot41 (0x0052c030) is exact in src/reconstructed/VehicleCamera.cpp.
//
// VehicleCamera::UnknownVirtualSlot35 (0x0052c510, 1267 bytes, 817/1261):
// frame (0x48) and slot assignment match retail. The squared length loads
// x*x first as in retail when written `(x*x) + y*y + z*z` (all 144
// bracketings tried). Left: retail duplicates the
// x scaling into both range branches (`fld x; fmul st(1)` after
// distance * field_0x228, `fld st(0); fmul x` after the -0.25 range) where
// VC6 here shares one block. Tried without effect: the four groupings of
// Length's terms and of `along`, `direction * range`, `range * direction`
// and per-component scaling in the range branch. Writing the scaling out in
// the body (either branch, both, or one shared range variable) as slot 41's
// normalisations are is worse (798..824).
// `a` is tested as a byte (FollowCamera.h declares it unsigned char).
#include "../../src/reconstructed/VehicleCamera.h"

static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

static inline Vector3 Add(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
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
    float squared = (v.x * v.x) + v.y * v.y + v.z * v.z;
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
