// Near-miss VehicleCamera candidates, kept out of src/reconstructed until they
// match. See docs/FOLLOW_CAMERA.md.
//
// VehicleCamera::UnknownVirtualSlot36 (0x0052cc80) is exact in src/reconstructed/VehicleCamera.cpp.
//
// VehicleCamera::UnknownVirtualSlot41 (0x0052c030) is exact in src/reconstructed/VehicleCamera.cpp.
//
// VehicleCamera::UnknownVirtualSlot35 (0x0052c510, 1267 bytes): frame
// (0x48), slot assignment and size match retail; 2 instructions differ
// (rank_near_misses). The squared length loads x*x first as in retail when
// written `(x*x) + y*y + z*z` (all 144 bracketings tried). Retail scales x
// inside each range branch and cross-jumps the shared y/z tail; scaling x
// in the branch and y/z through one ScaleYZ helper per branch reproduces
// that (a whole ScaleInPlace per branch merges all three components, and
// y/z written out per branch do not merge, 14..22 diffs). Left: the
// direction * reach x component is `fld st(0); fmul x` where retail has
// `fld x; fmul st(1)`. The written order of either branch's x product, the
// reach expression's spelling (operator*, friend operator*, component-wise,
// a named product) and inline vs named scale/range temporaries do not fix it.
// Mechanism (VC6_PATTERN_CORPUS section 6, rule 6): the direct direction.x
// stores in the range branches make x a memory operand at reach. Scaling x
// through a pointer helper in both branches gives retail's reach, but the
// second branch (retail's dup form) then loads x too and merges with the
// first (4 diffs). Pointer in the first branch only: 6; in the second only: 2,
// reach still dup.
// `a` is tested as a byte (FollowCamera.h declares it unsigned char).
#include "../../src/reconstructed/VehicleCamera.h"

static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

static inline Vector3 Add(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline void ScaleYZ(Vector3* v, float s) {
    v->y *= s;
    v->z *= s;
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
    return FastSqrt(squared);
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
        float along = velocity.z * followHeading.z + (velocity.x * followHeading.x + velocity.y * followHeading.y);
        distance = AngleWeight(followYaw) * AngleWeight(followPitch);
        float zoom = fieldOfView / maxFieldOfView;
        float scale = (float)(followDistance * 0.0111f) * zoom + 1.0f;
        if (a) {
            offset = 0.0f;
        } else {
            float speed = vehicleMode ? vehicle->referenceSpeed : 180.0f;
            offset = (float)((1.0f - followDistance * 0.0016666667f) * 1.8f) * (scale * scale * along / speed);
        }
        float length = vehicleMode ? vehicle->linearSpeed : Length(velocity);
        distance = zoom * distance + ClimbScale(velocity.y, length, offset);
        offset = (1.0f - zoom) * (velocity.y * 0.25f);
    } else {
        distance = 1.0f;
        offset = 0.0f;
    }
    if (isCrashView)
        distance *= 0.8f;
    Vector3 direction;
    if (cameraState == 3) {
        Vector3 side;
        side.x = followHeading.z;
        side.y = 0.0f;
        side.z = -followHeading.x;
        Vector3 pitched;
        D3DRMVectorRotate(&pitched, &followHeading, &side, -vehicle->savedPitch);
        D3DRMVectorRotate(&direction, &pitched, (Vector3*)&kVec3YAxis, turnYaw * 0.7f);
    } else {
        D3DRMVectorRotate(&direction, &followHeading, (Vector3*)&kVec3YAxis, turnYaw * 0.7f);
    }
    if (cameraState != 6) {
        float scale = distance * lookAheadDistance;
        direction.x *= scale;
        ScaleYZ(&direction, scale);
    } else {
        float range = -lookAheadDistance;
        range *= 0.25f;
        direction.x *= range;
        ScaleYZ(&direction, range);
    }
    Vector3 position = Add(direction, targetPoint);
    Vector3 result = position;
    result.y = position.y + offset;
    if (vehicle->riderCount > 0) {
        if (vehicle->UnknownInlineRider()->penetration > -0.5f) {
            float reach = (float)(FollowCameraAbs(followPitch) * 0.63661975f) * 1.2f + 0.3f;
            Vector3 ahead = Add(direction * reach, targetPoint);
            Vector3 ground = ahead;
            subject->QueryGround(&ground, 0, 0, 0);
            if (ground.y > ahead.y)
                lift = (ground.y - ahead.y) * 0.6f;
            else
                lift *= 0.01f;
        } else {
            lift *= 0.01f;
        }
        result.y += lift;
    }
    result.y += 3.0f;
    return result;
}
