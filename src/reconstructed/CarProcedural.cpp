// CarProcedural.cpp -- reconstruction of part of
// D:\aardvark\VC\krusty2\CarProcedural.cpp. See CarProcedural.h.

#include "CarProcedural.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "DebugAlloc.h"
#include "MemTag.h"
#include "TextureMap.h"
#include "ResourceEntry.h"
#include "D3DIMSoulTree.h"
#include "../krusty2/broadphase/Terrain.h"

// Per-TU vector constants (the shape documented in docs/LZW.md): four
// dynamic initializers (0x00430eb0..0x00430feb, .CRT$XCU 0x005660d8..
// 0x005660e4) build (0,0,0), (1,0,0), (0,1,0) and (0,0,1) into 0x00579700,
// 0x00579710, 0x00579720 and 0x005796f0.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// Inline vector helpers (tier 2: the inlined shapes in 0x004308e0; the
// constructor form of operator- and the (x*x + y*y) grouping are what
// reproduce retail's instruction order).
static inline Vector3 operator-(const Vector3& a, const Vector3& b)
{
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}
// The sum without the constructor: in the constructor form (as operator-
// above) PositionAtTime calls the Vector3 constructor out of line.
static inline Vector3 operator+(const Vector3& a, const Vector3& b)
{
    Vector3 r;
    r.x = a.x + b.x;
    r.y = a.y + b.y;
    r.z = a.z + b.z;
    return r;
}
// |v|, exact for unit vectors.
static inline float Length(Vector3 v)
{
    float squared = v.z * v.z + (v.x * v.x + v.y * v.y);
    if (squared == 1.0f)
        return 1.0f;
    return FastSqrt(squared);
}

// 0x0042f390
CarProcedural::CarProcedural(int flags) : GraphicsTest(flags)
{
    started = 0;
    steersFrontWheels = 1;
    field_0x58 = 0;
    numWheels = 0;
    currentPathKey = 0;
    field_0x68 = 0;
    pathKeyCount = 0;
    pathSpeed = 120.0f;
    field_0x178 = 0.0f;
    field_0x17c = 0.0f;
    field_0x180 = 0.0f;
    lookAheadTime = 0.1f;
    velocityScale = 10.0f;
    backBlend = 0.8f;
    normalBlend = 0.5f;
    wheelSpinAngle = 0.0f;
    steeringAngle = 0.0f;
    frontWheelZ = 0.0f;
    trailDistance = 0.0f;
    trailTimeOffset = 0.0f;
    groundSpeed = 0.0f;
    pathTime = 1e30f;
    pathKeys = 0;
    field_0x1b8 = kVec3Zero;
    pathPosition = kVec3Zero;
    pathOffset = kVec3Zero;
    bodyOffset = kVec3Zero;
    smoothedBack = kVec3Zero;
    field_0x1e8 = kVec3Zero;
    smoothedNormal = kVec3YAxis;
    pathVelocity = kVec3Zero;
    groundProvider = 0;
    carModel = 0;
    wheelNodes = 0;
    sensorCollider = 0;
    bodyCollider = 0;
    skipSmoothing = 1;
}

// 0x0042f590 (scalar deleting wrapper 0x0042f570)
CarProcedural::~CarProcedural()
{
    if (wheelNodes)
        delete wheelNodes;
    if (pathKeys)
        delete pathKeys;
}

// 0x0042f600: slot 27 sets the car up from its parameters: the model and
// its wheels, the two collision objects and the path.
CarProcedural* CarProcedural::Create(void* renderContext, const char* name, const char* collisionFile,
                                     const LightManager* lights, const TextureContext* textures, void* ground, const char* pathFile,
                                     const Vector3* offset, float speed, int steerFront, float trail,
                                     float a11, unsigned int wheelCount, float a13,
                                     float a14, float a15)
{
    GameObject::SetRenderContext(renderContext);
    int n = strlen(name);
    int length = n > 0xff ? 0xff : n;
    strncpy(carName, name, length);
    carName[length] = 0;

    carModel = new (__FILE__, 99) D3DIMSoultreeObject(1);
    carModel->LoadModel(renderContext, name, lights, textures, 1);
    AppendChild(carModel, -1);
    pathOffset = *offset;
    trailDistance = trail;
    pathTime = 0.0f;
    smoothedBack = kVec3Zero;
    steersFrontWheels = steerFront;
    wheelSpinAngle = 0.0f;
    SoultreeObject* body = carModel->FindByName("Body");
    body->GetPosition(reinterpret_cast<Vec3*>(&bodyOffset));

    numWheels = wheelCount;
    if (numWheels > 0) {
        wheelNodes = new (__FILE__, 126) SoultreeObject*[numWheels];
        if (numWheels > 0)
            wheelNodes[0] = carModel->FindByName("TireFrontL");
        if (numWheels > 1)
            wheelNodes[1] = carModel->FindByName("TireFrontR");
        if (numWheels > 2)
            wheelNodes[2] = carModel->FindByName("TireBackL");
        if (numWheels > 3)
            wheelNodes[3] = carModel->FindByName("TireBackR");
        if (numWheels > 4)
            wheelNodes[4] = carModel->FindByName("TireMidL");
        if (numWheels > 5)
            wheelNodes[5] = carModel->FindByName("TireMidR");
        Vector3 center;
        Vector3 halfExtent;
        wheelNodes[0]->GetLocalBounds(
            reinterpret_cast<Vec3*>(&center), reinterpret_cast<Vec3*>(&halfExtent));
        inverseWheelRadius = 1.0f / halfExtent.y;
    }
    groundProvider = ground;

    sensorCollider = new (__FILE__, 149) CarProceduralCollision(1);
    sensorCollider->Configure(renderContext, 1, 1, 0);
    sensorCollider->ignoreVegetation = 1;
    AppendChild(sensorCollider, -1);
    sensorHalfHeight = 5.0f;

    int tag = g_MemTagStack->Push("Collision");
    bodyCollider = new (__FILE__, 162) CarProceduralCollision(1);
    bodyCollider->Configure(renderContext, 1, 1, 1);
    bodyCollider->ignoreVegetation = 1;
    AppendChild(bodyCollider, -1);
    if (collisionFile && *collisionFile)
        bodyCollider->LoadShape(carModel, collisionFile);
    else
        bodyCollider->SetHullShape(carModel, 0, 1, 0, 0);
    ((CarProceduralBody*)bodyCollider->shape)->swept = 1;
    bodyCollider->SnapPlacement();
    sensorCollider->AddIgnoredOwner(bodyCollider);
    g_MemTagStack->Pop(tag);

    ReadPath(pathFile, carModel->name);
    pathSpeed = speed;
    if (speed > 0.0)
        inversePathSpeed = 1.0f / speed;
    field_0x178 = a13;
    field_0x17c = a14;
    field_0x180 = a15;
    return this;
}

// 0x0042fa00: reads the path from `pathFile`: one key per "FRAME <n>" line,
// whose position is the translation of the "transform" line naming the
// model (in quotes). The file's y and z are swapped.
void CarProcedural::ReadPath(const char* pathFile, const char* name)
{
    char quoted[0x100];
    char line[0x400];

    sprintf(quoted, "\"%s\"", name);
    FileStream* stream = new (__FILE__, 217) FileStream(g_ResourceManager);
    if (stream->Open(pathFile, "r", 0)) {
        pathKeyCount = 0;
        while (!stream->AtEnd()) {
            stream->ReadLine(line, 0x400);
            if (strncmp(line, "FRAME", strlen("FRAME")) == 0)
                pathKeyCount++;
        }
        stream->Seek(stream->sliceOffset, 0, 1);
        pathKeys = new (__FILE__, 241) CarProceduralKey[pathKeyCount];
        int i = -1;
        while (!stream->AtEnd()) {
            stream->ReadLine(line, 0x400);
            if (strncmp(line, "FRAME", strlen("FRAME")) == 0) {
                i++;
                char tag[0x400];
                unsigned long frame;
                sscanf(line, "%s %ld", tag, &frame);
                pathKeys[i].time = (float)frame;
            } else if (strncmp(line, "transform", strlen("transform")) == 0) {
                char key[0x100];
                char object[0x100];
                float m[9];
                Vector3 p;
                sscanf(line, "%s %s %f %f %f %f %f %f %f %f %f %f %f %f", key, object, &m[0], &m[1],
                       &m[2], &m[3], &m[4], &m[5], &m[6], &m[7], &m[8], &p.x, &p.z, &p.y);
                if (strcmp(object, quoted) == 0)
                    pathKeys[i].position = p;
            }
        }
    }
    delete stream;
}

// Slot 10's helpers and views (0x0042fd80 below).

// 0x004b5d00 (cdecl, OrientationAxes.cpp): the look and up vectors of a
// heading, pitch and roll.
int OrientationAxesFromAngles(Vector3* look, Vector3* up, float heading, float pitch, float roll);

// Out-of-line call views (docs/VC6_INLINE_BUDGET.md): the orthonormalisation
// of the collider frame calls the COMDAT cross product and normalisation.
Vector3 CarCrossCall(const Vector3& a, const Vector3& b);                 // 0x00515600
Vector3 CarNormalizeCall(const Vector3& v);                               // 0x005087b0

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
    float fraction;   // +0x00 CollisionBoxResult fraction
    float point;      // +0x04 CollisionBoxResult point
    Vector3 normal;
};

// v limited to [lo, hi].
static inline float CarClamp(float v, float lo, float hi)
{
    return v > lo ? (v < hi ? v : hi) : lo;
}

// 0x0042fd80: slot 10, the per-frame update along the path.
// Not exact (604 of 2909 bytes in place, 2880 compiled: the frame layout
// differs; docs/CARPROCEDURAL.md).
int CarProcedural::Tick(float frameTime)
{
    Vector3 axisA;
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
            wheelNodes[i]->GetPosition(reinterpret_cast<Vec3*>(&position));
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
    Vec3 local = carModel->WorldToLocalDirection(*reinterpret_cast<const Vec3*>(&forward));
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
        Vec3 steer = carModel->WorldToLocalDirection(*reinterpret_cast<const Vec3*>(&side));
        steeringAngle = (float)atan2(steer.x, steer.z);
        steeringAngle = CarClamp(steeringAngle, -0.52359873f, 0.52359873f);
    }
    Vector3 position;
    carModel->GetPositionIn(0, reinterpret_cast<Vec3*>(&position));
    Vector3 ground = position;
    Vector3 normal;
    ((Terrain*)groundProvider)->QueryGround(reinterpret_cast<TerrainVec3*>(&ground),
                                           reinterpret_cast<TerrainVec3*>(&normal), 0, 0);
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
            normal = ((CarProceduralSensorContact*)contact)->normal;
        }
    }
    if (skipSmoothing)
        smoothedNormal = normal;
    else
        smoothedNormal = smoothedNormal * normalBlend + normal * (1.0f - normalBlend);
    Vector3 flat = smoothedBack - smoothedNormal * CarDot(smoothedBack, smoothedNormal);
    carModel->SetAxesPtr(reinterpret_cast<const Vec3*>(&flat), reinterpret_cast<const Vec3*>(&smoothedNormal), 0, 1);
    Vector3 target(pathOffset.x + pathPosition.x, ground.y, pathOffset.z + pathPosition.z);
    Vec3 offset = carModel->LocalToWorldDirection(*reinterpret_cast<const Vec3*>(&bodyOffset));
    Vector3 placement = target - *reinterpret_cast<const Vector3*>(&offset);
    carModel->SetPositionInFrame(0, reinterpret_cast<const Vec3*>(&placement));
    for (unsigned int i = 0; i < numWheels; i++) {
        Vector3 axisB;
        if (steersFrontWheels && i < 2) {
            OrientationAxesFromAngles(&axisA, &axisB, steeringAngle, -wheelSpinAngle, 0.0f);
        } else {
            axisA.x = 0.0f;
            axisA.y = (float)sin(-wheelSpinAngle);
            axisA.z = (float)cos(-wheelSpinAngle);
            axisB.x = 0.0f;
            axisB.y = axisA.z;
            axisB.z = -axisA.y;
        }
        wheelNodes[i]->SetAxesPtr(reinterpret_cast<const Vec3*>(&axisA), reinterpret_cast<const Vec3*>(&axisB), 0, 1);
    }
    bodyCollider->UpdatePlacement();
    if (bodyCollider->QueryCollisions()) {
        CarProceduralContact* contact = (CarProceduralContact*)bodyCollider->contactRecord;
        bodyContactPoint = contact->contactPoint;
        bodyContactNormal = contact->contactNormal;
        CarProceduralBody* body = (CarProceduralBody*)bodyCollider->shape;
        body->position.x -= contact->contactOffset.x * 1.005f;
        body->position.y -= contact->contactOffset.y * 1.005f;
        body->position.z -= contact->contactOffset.z * 1.005f;
    }
    return result;
}

// 0x004308e0: searches, by bisection on the path parameter, for the offset
// before `time` at which the path is `distance` long (at most 200 steps).
float CarProcedural::FindOffsetAtDistance(float time, float distance)
{
    unsigned int count = 0;
    float result = 0.0f;
    float travelled = 0.0f;
    float step = 2.0f;
    int forward = 0;
    if (!(distance < 0.0))
        forward = 1;
    Vector3 previous = PositionAtTime(time);
    for (; count < 200; count++) {
        if (fabs(travelled - distance) <= 0.1f)
            break;
        Vector3 position;
        if (travelled < distance) {
            if (!forward) {
                step *= 0.5f;
                forward = 1;
            }
            result += step;
            position = PositionAtTime(time - result);
            travelled += Length(position - previous);
            previous = position;
        } else {
            if (forward) {
                step *= 0.5f;
                forward = 0;
            }
            result -= step;
            position = PositionAtTime(time - result);
            travelled -= Length(position - previous);
            previous = position;
        }
    }
    return result;
}

// 0x00430b10: the path position at `time` (wrapped to the path's period):
// a cubic Hermite segment between the keys around it, whose tangents are
// the neighbouring chords scaled by the key spacing.
// Not exact (299 of 843 bytes in place: `this` in edi rather than esi and
// the slots differ; docs/CARPROCEDURAL.md).
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

// 0x00430e60
void CarProcedural::HermiteWeights(float t, float* h0, float* h1, float* h2, float* h3)
{
    float t2 = t * t;
    float t3 = t2 * t;
    *h0 = 1.0f - (3.0f * t2 - t3 - t3);
    *h1 = 3.0f * t2 - t3 - t3;
    *h2 = t3 - t2 - t2 + t;
    *h3 = t3 - t2;
}
