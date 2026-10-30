// Group2_CarProcedural.cpp -- link bring-up: CarProcedural::PositionAtTime (0x00430b10)
// and CarProcedural::Tick (0x0042fd80, slot 10).
//
// Bodies taken unchanged from samples/race/CarProceduralNearMisses.cpp (near misses:
// PositionAtTime has retail's control flow and sum order but VC6 out-of-lines some
// operators; Tick has retail's 781-instruction stream at 88% alignment, the frame layout
// differs). See that file for the matching notes. best guess: behaviour follows the
// samples, which were compared instruction by instruction but never differentially tested.

#include <math.h>

#include "../reconstructed/CarProcedural.h"
#include "../krusty2/core/SoultreeObject.h"

// The ground provider is the terrain (0x00507c10, Terrain::QueryGround in src); the
// scene-node calls are SoultreeObject's (soultree.cpp). CarProcedural.h's own views of
// them (CarProceduralTerrain, CarProceduralNode) have no definitions, so this unit calls
// the defined names through file-local adapters.
struct TerrainVec3;
class Terrain {
public:
    void QueryGround(TerrainVec3* position, TerrainVec3* normal, int a, unsigned char* surface);
};

static inline SoultreeObject* CarNode(CarProceduralNode* node)
{
    return reinterpret_cast<SoultreeObject*>(node);
}
static inline Vector3 CarToVector3(const Vec3& v)
{
    Vector3 r;
    r.x = v.x;
    r.y = v.y;
    r.z = v.z;
    return r;
}
static inline Vec3 CarToVec3(const Vector3& v)
{
    Vec3 r;
    r.x = v.x;
    r.y = v.y;
    r.z = v.z;
    return r;
}
static inline Vector3 CarWorldToLocal(CarProceduralNode* node, const Vector3& v)
{
    return CarToVector3(CarNode(node)->WorldToLocalDirection(CarToVec3(v)));
}
static inline Vector3 CarLocalToWorld(CarProceduralNode* node, const Vector3& v)
{
    return CarToVector3(CarNode(node)->LocalToWorldDirection(CarToVec3(v)));
}

static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);

// Constructor-free helpers: with CarProcedural.cpp's constructor forms even
// the per-component chord leaves the function over budget.
static inline Vector3 operator+(const Vector3& a, const Vector3& b)
{
    Vector3 r;
    r.x = a.x + b.x;
    r.y = a.y + b.y;
    r.z = a.z + b.z;
    return r;
}
static inline Vector3 operator-(const Vector3& a, const Vector3& b)
{
    Vector3 r;
    r.x = a.x - b.x;
    r.y = a.y - b.y;
    r.z = a.z - b.z;
    return r;
}

// 0x00430b10: the path position at `time` (wrapped to the path's period):
// a cubic Hermite segment between the keys around it, whose tangents are
// the neighbouring chords scaled by the key spacing.
Vector3 CarProcedural::PositionAtTime(float time)
{
    unsigned int count = pathKeyCount;
    CarProceduralKey* keys = pathKeys;
    float period = keys[count - 1].time * inversePathSpeed;
    float t = fmod(time, period);
    if (t < 0.0)
        t += period;
    if (count > 0) {
        t *= pathSpeed;
        unsigned int next = currentPathKey + 1;
        while (next < count && t > keys[next].time) {
            currentPathKey++;
            next++;
        }
        while (currentPathKey > 0 && t < keys[currentPathKey].time) {
            currentPathKey--;
            next--;
        }
        // The fourth weight is written over `time`.
        float h0, h1, h2;
        HermiteWeights((t - keys[currentPathKey].time) / (keys[next].time - keys[currentPathKey].time),
                       &h0, &h1, &h2, &time);

        CarProceduralKey* fromKey = &pathKeys[currentPathKey];
        CarProceduralKey* toKey = &pathKeys[next];
        Vector3 chord;
        chord.x = toKey->position.x - fromKey->position.x;
        chord.y = toKey->position.y - fromKey->position.y;
        chord.z = toKey->position.z - fromKey->position.z;
        Vector3 t0;
        if (currentPathKey == 0) {
            t0 = chord;
        } else {
            float spacing = (toKey->time - fromKey->time) / (toKey->time - fromKey[-1].time);
            t0 = ((fromKey->position - fromKey[-1].position) + chord) * spacing;
        }
        Vector3 t1;
        if (next == pathKeyCount - 1) {
            t1 = chord;
        } else {
            float spacing = (toKey->time - fromKey->time) / (toKey[1].time - fromKey->time);
            t1 = ((toKey[1].position - toKey->position) + chord) * spacing;
        }
        return h0 * fromKey->position + h1 * toKey->position + t0 * h2 + t1 * time;
    }
    return kVec3Zero;
}


// ---------------------------------------------------------------------------
// Slot 10, 0x0042fd80 (2907 bytes): the per-frame update along the path.
// ---------------------------------------------------------------------------

static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// 0x004b5d00 (cdecl, returns 1, 0 for a null output): the look and up vectors of
// (heading, pitch, roll) -- (0,0,1) and (0,1,0) rotated about z by roll, about x by pitch,
// then about y by heading. Nothing in src defines it yet (other units declare it with
// different parameter types), so this unit keeps a file-local copy decoded from retail.
static int CarOrientationAxesFromAngles(Vector3* look, Vector3* up, float heading, float pitch,
                                        float roll)
{
    if (!up || !look)
        return 0;
    float c = (float)cos(roll);
    up->x = 0.0f;
    up->y = 1.0f;
    up->z = 0.0f;
    look->x = 0.0f;
    look->y = 0.0f;
    look->z = 1.0f;
    float s = (float)sin(roll);
    float y = c * up->y - s * up->x;
    up->x = s * up->y + c * up->x;
    up->y = y;

    c = (float)cos(pitch);
    s = (float)sin(pitch);
    y = s * up->z + c * up->y;
    up->z = c * up->z - s * up->y;
    up->y = y;
    y = c * look->y + s * look->z;
    look->z = c * look->z - s * look->y;
    look->y = y;

    c = (float)cos(heading);
    s = (float)sin(heading);
    float z = c * up->z - s * up->x;
    up->x = c * up->x + s * up->z;
    up->z = z;
    z = c * look->z - s * look->x;
    look->x = c * look->x + s * look->z;
    look->z = z;
    return 1;
}

// Out-of-line call views (docs/VC6_INLINE_BUDGET.md): the orthonormalisation
// of the collider frame calls the COMDAT cross product and normalisation.
Vec3 CrossProductCall(const Vec3& a, const Vec3& b);                      // 0x00515600
Vec3 Vec3Normalize(const Vec3& v);                                        // 0x005087b0
static inline Vector3 CarCrossCall(const Vector3& a, const Vector3& b)
{
    return CarToVector3(CrossProductCall(CarToVec3(a), CarToVec3(b)));
}
static inline Vector3 CarNormalizeCall(const Vector3& v)
{
    return CarToVector3(Vec3Normalize(CarToVec3(v)));
}

static inline Vector3 CarCross(const Vector3& a, const Vector3& b)
{
    Vector3 r;
    r.x = a.y * b.z - a.z * b.y;
    r.y = a.z * b.x - a.x * b.z;
    r.z = a.x * b.y - a.y * b.x;
    return r;
}
static inline float CarDot(const Vector3& a, const Vector3& b)
{
    return a.z * b.z + (a.x * b.x + a.y * b.y);
}
// |v| as 0x005087b0 reads its components (y, x, z), exact for unit vectors.
static inline float CarLength(const Vector3& v)
{
    float y = v.y, x = v.x, z = v.z;
    float squared = z * z + (x * x + y * y);
    if (squared == 1.0f)
        return 1.0f;
    return FastSqrt(squared);
}
// v / |v| in place; the zero vector stays zero.
static inline void CarNormalizeInPlace(Vector3& v)
{
    float squared = v.z * v.z + (v.y * v.y + v.x * v.x);
    if (squared == 0.0f) {
        v = kVec3Zero;
    } else {
        float inv = FastInvSqrt(squared);
        v.x *= inv;
        v.y *= inv;
        v.z *= inv;
    }
}
static inline Vector3& operator-=(Vector3& a, const Vector3& b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}

// The orthonormal frame of a forward and an up direction at a position.
static inline void CarFrame(Matrix4* m, Vector3 forwardAxis, Vector3 upAxis, const Vector3& position)
{
    Vector3 rightAxis = CarCross(upAxis, forwardAxis);
    upAxis = CarCrossCall(forwardAxis, rightAxis);
    upAxis = CarNormalizeCall(upAxis);
    forwardAxis = CarNormalizeCall(forwardAxis);
    rightAxis = CarCrossCall(upAxis, forwardAxis);
    (*m)(0, 0) = rightAxis.x;
    (*m)(0, 1) = rightAxis.y;
    (*m)(0, 2) = rightAxis.z;
    (*m)(0, 3) = 0.0f;
    (*m)(1, 0) = upAxis.x;
    (*m)(1, 1) = upAxis.y;
    (*m)(1, 2) = upAxis.z;
    (*m)(1, 3) = 0.0f;
    (*m)(2, 0) = forwardAxis.x;
    (*m)(2, 1) = forwardAxis.y;
    (*m)(2, 2) = forwardAxis.z;
    (*m)(2, 3) = 0.0f;
    (*m)(3, 0) = position.x;
    (*m)(3, 1) = position.y;
    (*m)(3, 2) = position.z;
    (*m)(3, 3) = 1.0f;
}

// The sensor collider's contact record: the penetration depth at +0 and
// the surface normal at +0x08 (tier 3 roles).
struct CarProceduralSensorContact {
    float field_0x00;
    float field_0x04;
    Vector3 field_0x08;
};

// v limited to [lo, hi].
static inline float CarClamp(float v, float lo, float hi)
{
    return v > lo ? (v < hi ? v : hi) : lo;
}

int CarProcedural::Tick(float frameTime)
{
    int result = GameObject::Tick(frameTime);
    if (frameTime <= 0.0)
        return result;
    if (!started) {
        // First update: a two-point mesh for the sensor collider, and the
        // wheels' height from the first wheel above the origin.
        Vector3 box[2];
        box[0] = kVec3Zero;
        box[0].y += sensorHalfHeight;
        box[1] = kVec3Zero;
        box[1].y -= sensorHalfHeight;
        sensorCollider->SetMeshShape(1, box);
        for (unsigned int i = 0; i < numWheels; i++) {
            Vector3 position;
            CarNode(wheelNodes[i])->GetPosition(reinterpret_cast<Vec3*>(&position));
            if (position.z > 0.0)
                frontWheelZ = position.z;
        }
        started = 1;
    }
    pathTime = (float)fmod(frameTime + pathTime, pathKeys[pathKeyCount - 1].time * inversePathSpeed);
    float t;
    if (trailDistance > 0.0) {
        trailTimeOffset = FindOffsetAtDistance(pathTime, trailDistance);
        t = pathTime - trailTimeOffset;
        pathPosition = PositionAtTime(t);
    } else {
        trailTimeOffset = 0.0f;
        t = pathTime;
        pathPosition = PositionAtTime(t);
    }
    Vector3 ahead = PositionAtTime(t + lookAheadTime);
    Vector3 forward = ahead - pathPosition;
    forward.y = 0.0f;
    pathVelocity = forward * velocityScale;
    groundSpeed = CarLength(pathVelocity);
    Vector3 local = CarWorldToLocal(carModel, forward);
    if (numWheels > 0)
        wheelSpinAngle = (float)fmod(inverseWheelRadius * velocityScale * local.z * frameTime + wheelSpinAngle, 6.2831855f);
    float behind = FindOffsetAtDistance(t, -1.0f);
    Vector3 back = PositionAtTime(t - behind) - pathPosition;
    CarNormalizeInPlace(back);
    if (skipSmoothing)
        smoothedBack = back;
    else
        smoothedBack = smoothedBack * backBlend + back * (1.0f - backBlend);
    if (steersFrontWheels && numWheels > 0) {
        float wheelbase = FindOffsetAtDistance(t, -frontWheelZ);
        Vector3 side = PositionAtTime(t - wheelbase) - pathPosition;
        Vector3 steer = CarWorldToLocal(carModel, side);
        steeringAngle = (float)atan2(steer.x, steer.z);
        steeringAngle = CarClamp(steeringAngle, -0.52359873f, 0.52359873f);
    }
    Vector3 position;
    CarNode(carModel)->GetPositionIn(0, reinterpret_cast<Vec3*>(&position));
    Vector3 ground = position;
    Vector3 normal;
    ((Terrain*)groundProvider)->QueryGround((TerrainVec3*)&ground, (TerrainVec3*)&normal, 0, 0);
    // The sensor collider's frame: the world axes orthonormalised.
    Matrix4 transform;
    CarFrame(&transform, kVec3ZAxis, kVec3YAxis, position);
    sensorCollider->SetTransform(&transform);
    sensorCollider->QueryCollisions();
    if (sensorCollider->hasContact) {
        CarProceduralContact* contact = (CarProceduralContact*)sensorCollider->contactRecord;
        Vector3 lifted = kVec3YAxis * ((1.0f - contact->contactOffset.x) * sensorHalfHeight * 2.0f - sensorHalfHeight) + position;
        if (lifted.y > ground.y) {
            ground = lifted;
            normal = ((CarProceduralSensorContact*)contact)->field_0x08;
        }
    }
    if (skipSmoothing)
        smoothedNormal = normal;
    else
        smoothedNormal = smoothedNormal * normalBlend + normal * (1.0f - normalBlend);
    Vector3 flat = smoothedBack - smoothedNormal * CarDot(smoothedBack, smoothedNormal);
    CarNode(carModel)->SetAxesPtr((const Vec3*)&flat, (const Vec3*)&smoothedNormal, 0, 1);
    Vector3 target(pathOffset.x + pathPosition.x, ground.y, pathOffset.z + pathPosition.z);
    Vector3 offset = CarLocalToWorld(carModel, bodyOffset);
    Vector3 placement = target - offset;
    CarNode(carModel)->SetPositionInFrame(0, (const Vec3*)&placement);
    for (unsigned int i = 0; i < numWheels; i++) {
        Vector3 axisA;
        Vector3 axisB;
        if (steersFrontWheels && i < 2) {
            CarOrientationAxesFromAngles(&axisA, &axisB, steeringAngle, -wheelSpinAngle, 0.0f);
        } else {
            axisA.x = 0.0f;
            axisA.y = (float)sin(-wheelSpinAngle);
            axisA.z = (float)cos(-wheelSpinAngle);
            axisB.x = 0.0f;
            axisB.y = axisA.z;
            axisB.z = -axisA.y;
        }
        CarNode(wheelNodes[i])->SetAxesPtr((const Vec3*)&axisA, (const Vec3*)&axisB, 0, 1);
    }
    bodyCollider->UpdatePlacement();
    if (bodyCollider->QueryCollisions()) {
        CarProceduralContact* contact = (CarProceduralContact*)bodyCollider->contactRecord;
        bodyContactPoint = contact->contactPoint;
        bodyContactNormal = contact->contactNormal;
        Vector3& shapePosition = ((CarProceduralBody*)bodyCollider->shape)->position;
        shapePosition.x -= contact->contactOffset.x * 1.005f;
        shapePosition.y -= contact->contactOffset.y * 1.005f;
        shapePosition.z -= contact->contactOffset.z * 1.005f;
    }
    return result;
}
