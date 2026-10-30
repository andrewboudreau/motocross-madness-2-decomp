// Group2_VehicleWheel.cpp -- bring-up definitions of the wheel class the vehicle
// code calls VehicleWheel (retail RTTI name: Tire, ctor 0x00512f10, size 0x2c0).
//
// Status: best-guess-unreviewed bring-up port, not a matching candidate.
// The bodies are ported from samples/physics/tire/Tire.cpp (most of them strict
// exact under the class name Tire there; UpdateContactPatch 1630/1664 and
// UpdateSuspensionProbe 2566/4258 are near misses) with the callees renamed to
// the providers that exist in src/. Helpers that have no provider in src/ are
// file-local copies (static) so this unit adds no new external dependency:
//   * 0x0040ae30 dot, 0x00515600 cross, 0x005087b0 normalise, 0x0042a510 /
//     0x0042a580 point transforms (decoded; both add the translation row);
//   * 0x00515880 frame builder and 0x00515c90 normalised cross (Tire.cpp);
//   * InlineShock::SolveContact 0x004fa400 / RotatingShock::SolveContact
//     0x004fac60 with their ClampAndStep 0x004fa090 / 0x004fa8b0
//     (samples/physics/suspension/Shock.cpp, strict exact there) on a local
//     view of the Shock layout (src/reconstructed/Suspension.h offsets).
//
// Layout (Tire.h, tier 1/2): CollisionObject @0 (0xb8), CollisionPoint @0xb8
// (0xc4), MovingPart @0x17c (0x4c), own data 0x1c8..0x2c0. It agrees with the
// offsets of the VehicleWheel view in src/krusty2/vehicle/Vehicle.h, which is
// not included here because that view is a flat padded struct.
#include <stddef.h>
#include <math.h>
#include <float.h>

#include "math/Math3D.h"                 // Vec3, Matrix4, SoultreeObject, FastSqrt/FastInvSqrt
#include "motion/SteeringControl.h"
#include "collision/CollisionObject.h"
#include "collision/CollisionPoint.h"

class SoultreeSlot1f0;
class Vehicle;
struct SoultreeProbe;

// d3drm.dll's D3DRMVectorRotate (D3DRM.LIB import).
extern "C" CollisionVec3* __stdcall D3DRMVectorRotate(CollisionVec3* result, const CollisionVec3* vector,
                                                       const CollisionVec3* axis, float theta);

// The terrain ground query 0x00507c10 (src/krusty2/broadphase/Terrain.h). The
// probe the vehicle passes (SoultreeProbe, its terrain) is that object.
struct TerrainVec3;
class Terrain {
public:
    void QueryGround(TerrainVec3* pos, TerrainVec3* outNormal, int flags, unsigned char* outSurface);
};

// src/reconstructed/MovingPart.h declares the same class (ctor 0x004a23a0); that
// header carries its own SoultreeObject stand-in, so the class is restated here
// with the shared node declaration. Same mangled constructor name.
class MovingPart {
public:
    MovingPart(const char* name, SoultreeObject* root, int field_0x44_);

    Matrix4 matrix;          // +0x00 the node's local matrix at construction
    SoultreeObject* node;    // +0x40
    int field_0x44;          // +0x44 constructor argument (the wheel's SteeringControl or 0)
    int field_0x48;          // +0x48 0
};

// ---------------------------------------------------------------------------------------
// Shock views (src/reconstructed/Suspension.h offsets; vptr at +0, MovingPart at +4).
struct WheelShock {
    void* vptr;                      // +0x00
    Matrix4 restMatrix;              // +0x04 MovingPart::matrix
    SoultreeObject* sceneNode;       // +0x44 MovingPart::node
    int ownerRef;                    // +0x48
    int movingPartField48;           // +0x4c
    int atLimit;                     // +0x50
    int extending;                   // +0x54
    CollisionVec3 spring;            // +0x58
    CollisionVec3 damper;            // +0x64
    float ratio;                     // +0x70
    CollisionVec3 displacement;      // +0x74
    CollisionVec3 stepDisplacement;  // +0x80
    float prevLength;                // +0x8c
    float clearedLength;             // +0x90
    float impulseScale;              // +0x94
    float length;                    // +0x98
    float stiffnessMin;              // +0x9c
    float stiffnessMax;              // +0xa0
    float dampingRatio;              // +0xa4
    float speedThreshold;            // +0xa8
    float retractScale;              // +0xac
    CollisionVec3 velocity;          // +0xb0
    float speed;                     // +0xbc
};
struct WheelInlineShock : WheelShock {
    CollisionVec3 axis;              // +0xc0
    float minLength;                 // +0xcc
    SoultreeObject* anchorNode;      // +0xd0
    float axisLength;                // +0xd4
};                                   // 0xd8 bytes
struct WheelRotatingShock : WheelShock {
    float maxAngle;                  // +0xc0
    float chord;                     // +0xc4
    float armLength;                 // +0xc8
    float armLengthSq;               // +0xcc
    float angle;                     // +0xd0
    CollisionVec3 rotationAxis;      // +0xd4
    int axisId;                      // +0xe0
};                                   // 0xe4 bytes
typedef char wheel_assert_inline_shock[(sizeof(WheelInlineShock) == 0xd8) ? 1 : -1];
typedef char wheel_assert_rotating_shock[(sizeof(WheelRotatingShock) == 0xe4) ? 1 : -1];

// The vehicle passed to 0x00513f90: only the members read there (Tire.h's TireVehicle view).
class VehicleWheel;
class WheelVehicleView {
public:
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5();
    virtual void UnknownVirtualSlot6();
    virtual void UnknownVirtualSlot7();
    virtual void UnknownVirtualSlot8();
    virtual void UnknownVirtualSlot9();
    virtual void UnknownVirtualSlot10();
    virtual void UnknownVirtualSlot11();
    virtual void UnknownVirtualSlot12();
    virtual void UnknownVirtualSlot13();
    virtual void UnknownVirtualSlot14();
    virtual void UnknownVirtualSlot15();
    virtual void UnknownVirtualSlot16();
    virtual void UnknownVirtualSlot17();
    virtual void UnknownVirtualSlot18();
    virtual void UnknownVirtualSlot19();
    virtual void UnknownVirtualSlot20();
    virtual void UnknownVirtualSlot21();
    virtual void UnknownVirtualSlot22();
    virtual void UnknownVirtualSlot23();
    virtual void UnknownVirtualSlot24();
    virtual void UnknownVirtualSlot25();
    virtual void UnknownVirtualSlot26();
    virtual void UnknownVirtualSlot27();
    virtual void UnknownVirtualSlot28();
    virtual void UnknownVirtualSlot29();
    virtual void UnknownVirtualSlot30();
    virtual void UnknownVirtualSlot31();
    virtual void UnknownVirtualSlot32();
    virtual void UnknownVirtualSlot33();
    virtual void UnknownVirtualSlot34();
    virtual void UnknownVirtualSlot35();
    virtual void UnknownVirtualSlot36();
    virtual void UnknownVirtualSlot37();
    virtual void UnknownVirtualSlot38();
    virtual void UnknownVirtualSlot39();
    virtual void UnknownVirtualSlot40();
    virtual void UnknownVirtualSlot41();
    virtual void UnknownVirtualSlot42();
    virtual void UnknownVirtualSlot43();
    virtual void UnknownVirtualSlot44();
    virtual void UnknownVirtualSlot45();
    virtual void UnknownVirtualSlot46();
    // best guess: VehicleCore.h declares slot 47 as float(float); retail 0x00513f90
    // pushes the wheel pointer and reads st(0), so the argument is the wheel.
    virtual float UnknownVirtualSlot47(VehicleWheel* wheel);   // grip factor for this wheel

    char pad_0x04[0x24 - 0x04];
    float field_0x24;
    char pad_0x28[0x60 - 0x28];
    float field_0x60;
    char pad_0x64[0x70 - 0x64];
    CollisionVec3 field_0x70;              // velocity-like; its length drives the share
    char pad_0x7c[0xa4 - 0x7c];
    float field_0xa4;
    char pad_0xa8[0x444 - 0xa8];
    int field_0x444;
};

// Object at +0x2a8 (Vehicle.h's VehicleWheelAux); only its first float is read here.
struct WheelAux {
    float value;
};

class VehicleWheel : public CollisionObject, public MovingPart, public CollisionPoint {
public:
    VehicleWheel(const char* name, SoultreeObject* root, float rollFriction_, float sideFriction_,
                 int field_0x298_, SteeringControl* steer, float field_0x2b4_,
                 float rollFrictionScale_, float sideFrictionScale_, float field_0x2a0_,
                 float field_0x1d8_, float field_0x1dc_, float field_0x1e0_, float blendWeight_,
                 SoultreeSlot1f0* track);
    virtual ~VehicleWheel();                                   // 0x005133a0 / core 0x005133d0
    virtual GameObject* SetRenderContext(void* context);      // 0x00513490 (GameObject slot 8)
    virtual void CollisionPointVirtualSlot1();                 // 0x00515b50 (CollisionPoint slot 1)

    void HandleContact(int unused, int tag, CollisionObject* other);   // 0x00512e80
    void UpdateRoll(float dt, float distanceScale, int locked, int driven, int crashState,
                    float driveScale);                                  // 0x005143d0
    void UpdateDriveShare(Vehicle* owner);                              // 0x00513f90
    void UpdateShock(float dt, int pointsTouching, int crashed, float unused,
                     Vec3* velocity, Vec3* axisB);                      // 0x00513c70
    void UpdateContactPatch(Vec3* pos, Vec3 axis, Vec3 ref, float minLength,
                            float* outValue, int* outSign);             // 0x005135f0
    void UpdateSuspensionProbe(SoultreeProbe* world, const Vec3* velocity, float angle,
                               float radiusScale, float yaw, const Vec3* forward,
                               const Vec3* up, SoultreeObject* node);   // 0x00514550
    void ApplyDrive(float share, float stepTime, int forward, float mass, float* speed,
                    Vec3* torque, Vec3* force);                         // 0x00514170

    // 0x00513560 / 0x005135b0 (inline here; retail has them out of line).
    void SetRollDistance(float distance);
    void ApplyRollAngle();

    float invWheelRadius;            // +0x1c8
    float sideFriction;              // +0x1cc
    float rollFrictionScale;         // +0x1d0
    float sideFrictionScale;         // +0x1d4
    float field_0x1d8;               // +0x1d8 ApplyDrive torque scale
    float field_0x1dc;
    float field_0x1e0;
    int hasContactObjectVelocity;    // +0x1e4
    CollisionVec3 slipVector;        // +0x1e8
    CollisionVec3 contactObjectVelocity;  // +0x1f4
    CollisionVec3 wheelCenter;       // +0x200
    CollisionVec3 wheelUpAxis;       // +0x20c
    CollisionVec3 normalLeverCross;  // +0x218
    CollisionVec3 wheelVelocity;     // +0x224
    CollisionVec3 rollDirection;     // +0x230
    CollisionVec3 sideAxis;          // +0x23c
    CollisionVec3 field_0x248;       // +0x248 drive force (Vehicle.h driveForce)
    CollisionVec3 field_0x254;
    int inContact;                   // +0x260
    int reportContactOutputs;        // +0x264
    int field_0x268;                 // +0x268 shockEngaged
    int field_0x26c;                 // +0x26c shockExtending
    float rollFriction;              // +0x270
    float wheelRadius;               // +0x274
    float rollAngle;                 // +0x278
    float slipSpeed;                 // +0x27c
    float field_0x280;               // +0x280 drive magnitude
    float surfaceScaleB;             // +0x284
    float tangentSin;                // +0x288
    float tangentCos;                // +0x28c
    float field_0x290;               // +0x290 drive distance
    float field_0x294;               // +0x294 rest load weight
    int field_0x298;
    float rampLevel;                 // +0x29c
    float field_0x2a0;               // +0x2a0 drive strength
    float field_0x2a4;
    WheelAux* field_0x2a8;           // +0x2a8
    WheelRotatingShock* field_0x2ac; // +0x2ac (Vehicle.h secondaryAux)
    WheelInlineShock* field_0x2b0;   // +0x2b0 (Vehicle.h primaryAux)
    float field_0x2b4;               // +0x2b4 slip threshold
    float field_0x2b8;               // +0x2b8 drive share
    float field_0x2bc;               // +0x2bc 1 - drive share
};

typedef char wheel_assert_first_field[(offsetof(VehicleWheel, invWheelRadius) == 0x1c8) ? 1 : -1];
typedef char wheel_assert_in_contact[(offsetof(VehicleWheel, inContact) == 0x260) ? 1 : -1];
typedef char wheel_assert_last_field[(offsetof(VehicleWheel, field_0x2bc) == 0x2bc) ? 1 : -1];
typedef char wheel_assert_sizeof[(sizeof(VehicleWheel) == 0x2c0) ? 1 : -1];

// ---------------------------------------------------------------------------------------
// Unit constants (retail: the Tire unit's own vector set 0x0068a3c0..0x0068a3f0).
static inline CollisionVec3 WheelVec3(float x, float y, float z)
{
    CollisionVec3 v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}
static const CollisionVec3 kWheelZero = WheelVec3(0.0f, 0.0f, 0.0f);
static const CollisionVec3 kWheelYAxis = WheelVec3(0.0f, 1.0f, 0.0f);
static const CollisionVec3 kWheelZAxis = WheelVec3(0.0f, 0.0f, 1.0f);

#define WHEEL_PI 3.14159265358979f

// ---- local copies of out-of-line vector helpers ----------------------------------------

// 0x0040ae30: a.b.
static float WheelDot(const CollisionVec3* a, const CollisionVec3* b)
{
    return a->z * b->z + (a->x * b->x + a->y * b->y);
}
static inline float WheelDotInline(const CollisionVec3& a, const CollisionVec3& b)
{
    return a.z * b.z + (a.x * b.x + a.y * b.y);
}
// 0x00515600: a x b.
static CollisionVec3 WheelCross(const CollisionVec3& a, const CollisionVec3& b)
{
    CollisionVec3 r;
    r.x = a.y * b.z - a.z * b.y;
    r.y = a.z * b.x - a.x * b.z;
    r.z = a.x * b.y - a.y * b.x;
    return r;
}
// 0x005087b0: src unchanged when |src|^2 == 1, else src * FastInvSqrt(|src|^2) (decoded:
// no zero test).
static CollisionVec3* WheelNormalize(CollisionVec3* out, const CollisionVec3* src)
{
    float lenSq = src->z * src->z + (src->x * src->x + src->y * src->y);
    if (lenSq == 1.0f) {
        *out = *src;
        return out;
    }
    float inv = FastInvSqrt(lenSq);
    out->x = src->x * inv;
    out->y = src->y * inv;
    out->z = src->z * inv;
    return out;
}
// 0x0042a510 (by value) and 0x0042a580 (by pointer): row vector times matrix, plus the
// translation row (both decoded with the translation add).
static void WheelTransformPoint(CollisionVec3* out, const CollisionVec3* v, const CollisionMatrix4* m)
{
    const float* r = m->m;
    float x = v->x, y = v->y, z = v->z;
    out->x = z * r[8] + y * r[4] + x * r[0] + r[12];
    out->y = z * r[9] + y * r[5] + x * r[1] + r[13];
    out->z = z * r[10] + y * r[6] + x * r[2] + r[14];
}

static inline CollisionVec3 WheelNormalizeY(const CollisionVec3& v)
{
    float lenSq = WheelDotInline(v, v);
    if (lenSq == 1.0f)
        return v;
    float invLen = FastInvSqrt(lenSq);
    return WheelVec3(v.x * invLen, v.y * invLen, v.z * invLen);
}

// 0x00515880 (Tire.cpp TireBuildFrame): rotation frame, row 1 = y, row 2 = z,
// row 0 = y x z, row 3 = pos; optionally orthogonalised keeping z or y.
static void WheelBuildFrame(CollisionMatrix4* out, const CollisionVec3* pos, const CollisionVec3* zAxis,
                            const CollisionVec3* yAxis, int orthogonalize, int keepZ)
{
    CollisionVec3 z = *zAxis;
    CollisionVec3 y = *yAxis;

    if (orthogonalize) {
        CollisionVec3 c;
        c.x = y.y * z.z - y.z * z.y;
        c.y = y.z * z.x - y.x * z.z;
        c.z = y.x * z.y - y.y * z.x;
        CollisionVec3 perpendicular;
        if (keepZ) {
            perpendicular.x = c.z * z.y - c.y * z.z;
            perpendicular.y = c.x * z.z - c.z * z.x;
            perpendicular.z = c.y * z.x - c.x * z.y;
            y = perpendicular;
        } else {
            perpendicular.x = c.y * y.z - c.z * y.y;
            perpendicular.y = c.z * y.x - c.x * y.z;
            perpendicular.z = c.x * y.y - c.y * y.x;
            z = perpendicular;
        }
    }

    y = WheelNormalizeY(y);
    CollisionVec3 unitZ = WheelNormalizeY(z);

    CollisionVec3 xAxis;
    xAxis.x = y.y * unitZ.z - y.z * unitZ.y;
    xAxis.y = unitZ.x * y.z - y.x * unitZ.z;
    xAxis.z = y.x * unitZ.y - unitZ.x * y.y;
    out->m[0] = xAxis.x;
    out->m[1] = xAxis.y;
    out->m[2] = xAxis.z;
    out->m[3] = 0.0f;
    out->m[4] = y.x;
    out->m[5] = y.y;
    out->m[6] = y.z;
    out->m[7] = 0.0f;
    out->m[8] = unitZ.x;
    out->m[9] = unitZ.y;
    out->m[10] = unitZ.z;
    out->m[11] = 0.0f;
    out->m[12] = pos->x;
    out->m[13] = pos->y;
    out->m[14] = pos->z;
    out->m[15] = 1.0f;
}

// 0x00515c90: normalised a x b, the fallback when the cross is exactly zero.
static CollisionVec3* WheelUnitCross(CollisionVec3* out, const CollisionVec3* a, const CollisionVec3* b,
                                     const CollisionVec3* fallback)
{
    CollisionVec3 n;
    n.x = a->y * b->z - a->z * b->y;
    n.y = a->z * b->x - a->x * b->z;
    n.z = a->x * b->y - a->y * b->x;
    if (n.x == 0.0f && n.y == 0.0f && n.z == 0.0f) {
        n = *fallback;
    } else {
        float lenSq = WheelDotInline(n, n);
        if (lenSq == 0.0f) {
            n = kWheelZero;
        } else {
            float inv = FastInvSqrt(lenSq);
            n.x *= inv;
            n.y *= inv;
            n.z *= inv;
        }
    }
    *out = n;
    return out;
}

static inline Vec3* AsVec3(CollisionVec3* v) { return (Vec3*)v; }
static inline const Vec3& AsVec3(const CollisionVec3& v) { return *(const Vec3*)&v; }
static inline CollisionVec3 FromVec3(const Vec3& v) { return WheelVec3(v.x, v.y, v.z); }

static inline void QueryGround(SoultreeProbe* world, CollisionVec3* pos, CollisionVec3* normal,
                               unsigned char* surface)
{
    ((Terrain*)world)->QueryGround((TerrainVec3*)pos, (TerrainVec3*)normal, 0, surface);
}

// ---- shock solves (samples/physics/suspension/Shock.cpp, strict exact there) -----------

static inline float ShockLengthOf(const CollisionVec3& v)
{
    float lenSq = WheelDotInline(v, v);
    return (lenSq == 1.0f) ? 1.0f : FastSqrt(lenSq);
}

// Projection of v onto `onto` (zero when `onto` is zero): 0x004fb110.
static inline CollisionVec3 ShockProjectOnto(const CollisionVec3& v, const CollisionVec3& onto)
{
    if (onto.x == kWheelZero.x && onto.y == kWheelZero.y && onto.z == kWheelZero.z)
        return kWheelZero;
    float scale = WheelDot(&v, &onto) / WheelDot(&onto, &onto);
    return onto * scale;
}

// Shared tail of both ClampAndStep bodies.
static void ShockStepVelocity(WheelShock* s, float maxDelta, float dt)
{
    float delta = s->prevLength - s->length;
    if (s->length != 0.0f) {
        float clamped;
        if (delta >= 0.0f)
            clamped = (delta < maxDelta) ? delta : maxDelta;
        else
            clamped = (delta > -maxDelta) ? delta : -maxDelta;
        s->velocity = s->displacement * (clamped / (dt * s->length));
        s->stepDisplacement = s->displacement * -(delta / s->length);
    } else {
        s->velocity = kWheelZero;
        s->stepDisplacement = kWheelZero;
    }
    s->extending = (s->length > s->prevLength);
    s->prevLength = s->length;
}

// 0x004fa090 InlineShock::ClampAndStep.
static void InlineShockClampAndStep(WheelInlineShock* s, float maxDelta, float dt, CollisionVec3& pos)
{
    if (s->length >= s->minLength) {
        s->atLimit = 1;
        s->ratio = 1.0f;
        pos = s->axis * -s->minLength;
        s->displacement = s->displacement * (s->minLength / s->length);
        s->length = s->minLength;
    } else {
        s->atLimit = 0;
        s->ratio = s->length / s->minLength;
    }
    s->sceneNode->SetPosition(AsVec3(pos));   // 0x004fc660
    ShockStepVelocity(s, maxDelta, dt);
}

// 0x004fa400 InlineShock::SolveContact.
static void InlineShockSolveContact(WheelInlineShock* s, float dt, const CollisionVec3* offset,
                                    const CollisionVec3* base, const CollisionVec3* point,
                                    const CollisionVec3* normal, float limit, float* outLoad,
                                    int* outActive)
{
    int miss;
    float contactRatio;
    CollisionVec3 anchor;
    s->anchorNode->GetPositionIn(0, AsVec3(&anchor));
    CollisionVec3 toPoint = *point - anchor;
    float dist = WheelDotInline(toPoint, *normal);
    if (dist >= 0.0f)
        dist = WheelDotInline(toPoint, WheelVec3(-normal->x, normal->y, -normal->z));
    if (dist < 0.0f) {
        contactRatio = limit / dist;
        miss = 0;
    } else {
        contactRatio = 0.0f;
        miss = 1;
    }
    float len = WheelDotInline(toPoint, toPoint);
    len = (len == 1.0f) ? 1.0f : (float)sqrt(len);
    float over = s->axisLength - len;
    if (!(over > 0.0f))
        over = 0.0f;
    s->length = over - len * contactRatio;
    s->displacement = toPoint * -(s->length / len);
    CollisionVec3 pos = s->axis * -s->length;
    CollisionVec3 pointVelocity = *base + *offset;
    CollisionVec3 proj = ShockProjectOnto(pointVelocity, s->displacement);
    s->speed = ShockLengthOf(proj);
    InlineShockClampAndStep(s, s->speed * dt, dt, pos);
    if (s->atLimit == 0 && miss == 0) {
        *outLoad = 0.0f;
        *outActive = 0;
        return;
    }
    *outLoad = limit - WheelDotInline(s->stepDisplacement, *normal);
    *outActive = 1;
}

// 0x004fa8b0 RotatingShock::ClampAndStep.
static void RotatingShockClampAndStep(WheelRotatingShock* s, float maxDelta, float dt)
{
    if (s->length >= s->chord) {
        float limitScale = s->chord / s->length;
        s->atLimit = 1;
        s->ratio = 1.0f;
        s->angle = s->maxAngle;
        s->displacement = s->displacement * limitScale;
        s->length = s->chord;
    } else {
        s->angle = s->length / s->armLength;
        s->atLimit = 0;
        s->ratio = s->length / s->chord;
    }
    if (_finite(s->angle)) {
        s->sceneNode->SetLocalMatrix(&s->restMatrix);                  // 0x004fca30
        s->sceneNode->Rotate(AsVec3(s->rotationAxis), s->angle);       // 0x004fceb0
    }
    ShockStepVelocity(s, maxDelta, dt);
}

// 0x004fac60 RotatingShock::SolveContact.
static void RotatingShockSolveContact(WheelRotatingShock* s, float dt, const CollisionVec3* axisA,
                                      const CollisionVec3* axisB, const CollisionVec3* point,
                                      const CollisionVec3* normal, const CollisionVec3* offsetA,
                                      const CollisionVec3* offsetB, float limit, float* outLoad,
                                      int* outActive)
{
    int miss = 0;
    float contactRatio;
    CollisionVec3 anchor;
    s->sceneNode->GetPositionIn(0, AsVec3(&anchor));
    CollisionVec3 toPoint = *point - anchor;
    CollisionVec3 swing = kWheelZero;   // best guess: retail leaves it unset for axisId > 3
    switch (s->axisId) {
    case 0: swing = WheelCross(*axisA, toPoint); break;
    case 1: swing = WheelCross(toPoint, *axisA); break;
    case 2: swing = WheelCross(*axisB, toPoint); break;
    case 3: swing = WheelCross(toPoint, *axisB); break;
    }
    float dist = (swing.x * normal->x + swing.y * normal->y) + swing.z * normal->z;
    if (dist >= 0.0f) {
        if (dist == 0.0f) {
            *outLoad = limit;
            float side = (offsetA->y * normal->y + normal->x * offsetA->x) + offsetA->z * normal->z;
            *outActive = (side < -1.0f) ? 1 : 0;
            s->speed = 0.0f;
            return;
        }
        dist = WheelDotInline(swing, WheelVec3(-normal->x, normal->y, -normal->z));
    }
    if (dist < 0.0f) {
        contactRatio = limit / dist;
    } else {
        contactRatio = 0.0f;
        miss = 1;
    }
    s->length = s->clearedLength - contactRatio * s->armLength;
    s->displacement = swing * ((-s->length) / s->armLength);
    s->speed = ShockLengthOf(ShockProjectOnto(*offsetA + *offsetB, s->displacement));
    RotatingShockClampAndStep(s, s->speed * dt, dt);
    if (s->atLimit == 0 && miss == 0) {
        *outLoad = 0.0f;
        *outActive = 0;
        return;
    }
    *outLoad = limit - WheelDot(normal, &s->stepDisplacement);
    *outActive = 1;
}

// ---------------------------------------------------------------------------------------
// Collision callback stored by SetRenderContext (0x00512e30).
static void WheelCollisionCallback(CollisionObject* self, CollisionObject* other)
{
    VehicleWheel* wheel = (VehicleWheel*)self->ownerObject;
    int tag = other->ownerType;
    if (tag == 0x64) {
        // virtual call through vtable offset 0xcc (slot 51) on the other object's owner
        void* owner = other->ownerObject;
        typedef int (__fastcall* OwnerSlot51)(void* self, int edx);
        OwnerSlot51 slot51 = (OwnerSlot51)(*(void***)owner)[0xcc / 4];
        if (slot51(owner, 0))
            return;
    } else if (tag == 0x65) {
        return;
    }
    wheel->HandleContact(1, tag, other);
}

struct WheelContactOwner {
    char pad0[0x40];
    CollisionVec3 field_0x40;
    char pad1[0x64 - 0x4c];
    CollisionVec3 field_0x64;
    char pad2[0x224 - 0x70];
    CollisionVec3 field_0x224;
};

// 0x00512e80: records a velocity of the touched object's owner, chosen by its tag.
void VehicleWheel::HandleContact(int unused, int tag, CollisionObject* other)
{
    switch (tag) {
    case 0x64:
    case 0x68:
        contactObjectVelocity = ((WheelContactOwner*)other->ownerObject)->field_0x64;
        break;
    case 0x2711:
        contactObjectVelocity = ((WheelContactOwner*)other->ownerObject)->field_0x224;
        break;
    case 0x69:
        contactObjectVelocity = ((WheelContactOwner*)other->ownerObject)->field_0x40;
        break;
    default:
        hasContactObjectVelocity = 0;
        return;
    }
    hasContactObjectVelocity = 1;
}

// ---------------------------------------------------------------------------------------
// Constructor 0x00512f10. CollisionPoint's constructor is inlined in retail (only
// declared in CollisionPoint.h); its stores are written out here.
VehicleWheel::VehicleWheel(const char* name, SoultreeObject* root, float rollFriction_, float sideFriction_,
                           int field_0x298_, SteeringControl* steer, float field_0x2b4_,
                           float rollFrictionScale_, float sideFrictionScale_, float field_0x2a0_,
                           float field_0x1d8_, float field_0x1dc_, float field_0x1e0_, float blendWeight_,
                           SoultreeSlot1f0* track)
    : CollisionObject(1), MovingPart(name, root, (int)steer), CollisionPoint(sideFriction_, (int)track)
{
    CollisionVec3 boundsCenter;
    CollisionVec3 halfExtent;
    MovingPart::node->GetLocalBounds(AsVec3(&boundsCenter), AsVec3(&halfExtent));
    wheelRadius = halfExtent.y;
    invWheelRadius = 1.0f / halfExtent.y;
    rollAngle = 0.0f;
    MovingPart::node->GetPositionIn(0, AsVec3(&wheelCenter));
    slipVector = kWheelZero;
    inContact = 0;
    wheelVelocity = kWheelZero;
    rollDirection = kWheelZero;
    sideAxis = kWheelZero;
    field_0x248 = kWheelZero;
    field_0x280 = 0;
    field_0x254 = kWheelZero;
    tangentCos = 0;
    rollFriction = rollFriction_;
    sideFriction = sideFriction_;
    field_0x294 = blendWeight_;
    CollisionPoint::blendWeight = blendWeight_;
    field_0x298 = field_0x298_;
    field_0x2b4 = field_0x2b4_;
    rollFrictionScale = rollFrictionScale_;
    sideFrictionScale = sideFrictionScale_;
    field_0x2a0 = field_0x2a0_;
    field_0x1d8 = field_0x1d8_;
    tangentSin = 1.0f;
    field_0x2a8 = 0;
    field_0x2ac = 0;
    field_0x2b0 = 0;
    field_0x2b8 = 1.0f;
    field_0x2bc = 0;
    reportContactOutputs = 0;
    field_0x268 = 0;
    rampLevel = 0;
    field_0x1dc = field_0x1dc_;
    field_0x1e0 = field_0x1e0_;
    field_0x290 = 0;
    wheelUpAxis = kWheelZAxis;
    surfaceScaleB = 1.0f;
    field_0x26c = 0;
    contactObjectVelocity = kWheelZero;
    hasContactObjectVelocity = 0;
    field_0x2a4 = 1.0f;
    normalLeverCross = kWheelZero;
}

// CollisionPoint's inline constructor body (Tire.cpp; the unit's own zero/+Y vectors).
inline CollisionPoint::CollisionPoint(float force, int owner)
{
    normalForce = force;
    surfaceOwner = owner;
    localPosition = kWheelZero;
    ownerNode = 0;
    worldPosition = kWheelZero;
    surfacePosition = kWheelZero;
    surfaceNormal = kWheelYAxis;
    relativePosition = kWheelZero;
    appliedShare = kWheelZero;
    frictionDirection = kWheelZero;
    field_0x5c = kWheelZero;
    tangentSpeed = 0;
    spinSpeed = 0;
    penetration = -999.0f;
    penetrationThreshold = 0;
    inContact = 0;
    field_0xa8 = 0;
    field_0xac = 0;
    field_0xb0 = 0;
    field_0xb4 = 0;
    blendWeight = 0.5f;
    field_0xb8 = 1.0f;
    surfaceType = 0;
    surfaceGrip = 1.0f;
    frictionMagnitude = 0;
    frictionForce = kWheelZero;
    frictionCoefficient = 0;
    field_0x68 = kWheelZero;
}

// 0x005133d0: releases the attached shock (+0x2b0 first, else +0x2ac). Both shock
// destructors (0x004f9f80, 0x004fa8a0) only run the empty MovingPart destructor, so the
// storage is returned with operator delete.
VehicleWheel::~VehicleWheel()
{
    if (field_0x2b0)
        operator delete(field_0x2b0);
    else if (field_0x2ac)
        operator delete(field_0x2ac);
}

// 0x00513490 (GameObject slot 8): sets up the wheel's collision shape, a half-circle
// polyline of 8 rim points of radius wheelRadius in the y/z plane, and the hit callback.
GameObject* VehicleWheel::SetRenderContext(void* context)
{
    struct WheelPoint {
        CollisionVec3 origin;
        CollisionVec3 rim;
    };
    WheelPoint pts[8];

    Configure(context, 1, 1, 1);
    for (int i = 0; i < 8; i++) {
        float t = i * 0.142857149f;
        float angle = t * WHEEL_PI + 1.57079637f;
        pts[i].origin = kWheelZero;
        pts[i].rim.x = 0.0f;
        pts[i].rim.y = -(float)cos(angle) * wheelRadius;
        pts[i].rim.z = (float)sin(angle) * wheelRadius;
    }
    SetMeshShape(8, pts);
    CollisionObject::onHitCallback = WheelCollisionCallback;
    CollisionObject::onHitByCallback = 0;
    ownerObject = this;
    return this;
}

// 0x00515b50 (CollisionPoint slot 1): anisotropic friction. The tangent's angle to the
// rolling direction blends the roll and side friction coefficients.
void VehicleWheel::CollisionPointVirtualSlot1()
{
    if (!(CollisionPoint::tangentSpeed > 0.001f) && !(CollisionPoint::spinSpeed > 0.001f)) {
        CollisionPoint::frictionMagnitude = 0.0f;
        CollisionPoint::frictionForce = kWheelZero;
    } else {
        float rollCos = WheelDotInline(CollisionPoint::frictionDirection, rollDirection);
        if (rollCos < 0.0f)
            rollCos = -rollCos;
        if (rollCos >= 1.0f)
            rollCos = 1.0f;
        tangentCos = rollCos;
        tangentSin = (float)sqrt(1.0f - rollCos * rollCos);
        float rollTerm = (-rollFriction) * rollFrictionScale * tangentCos * CollisionPoint::frictionCoefficient;
        CollisionPoint::frictionMagnitude = (rollTerm + (-sideFriction) * sideFrictionScale * tangentSin
                                             * CollisionPoint::frictionCoefficient)
                                            * CollisionPoint::surfaceGrip;
        CollisionPoint::frictionForce = CollisionPoint::frictionDirection * CollisionPoint::frictionMagnitude;
        if (CollisionPoint::frictionMagnitude < 0.0f)
            CollisionPoint::frictionMagnitude = -CollisionPoint::frictionMagnitude;
    }
}

// 0x00513560
void VehicleWheel::SetRollDistance(float distance)
{
    rollAngle = distance * invWheelRadius;
    if (_finite(rollAngle))
        MovingPart::node->RotateAbout(1.0f, 0.0f, 0.0f, rollAngle);
}

// 0x005135b0
void VehicleWheel::ApplyRollAngle()
{
    if (_finite(rollAngle))
        MovingPart::node->RotateAbout(1.0f, 0.0f, 0.0f, rollAngle);
}

// ---------------------------------------------------------------------------------------
static inline float WheelLengthFromSq(float lenSq)
{
    if (lenSq == 1.0f)
        return 1.0f;
    return FastSqrt(lenSq);
}

// Shared tail of UpdateContactPatch when there is no usable tangent: the slip is the
// rotational lever term alone.
static void ContactPatchLeverOnly(VehicleWheel* w, float* outValue, int* outSign)
{
    w->CollisionPoint::tangentSpeed = 0.0f;
    w->slipVector = w->normalLeverCross;
    float len = WheelLengthFromSq(WheelDot(&w->slipVector, &w->slipVector));
    w->CollisionPoint::spinSpeed = len;
    if (w->reportContactOutputs)
        *outValue = len;
    if (w->CollisionPoint::spinSpeed > 0.001f) {
        float inv = 1.0f / w->CollisionPoint::spinSpeed;
        w->CollisionPoint::frictionDirection = w->slipVector * inv;
    } else {
        w->CollisionPoint::frictionDirection = kWheelZero;
        w->CollisionPoint::spinSpeed = 0.0f;
    }
    float rollCos = WheelDotInline(w->CollisionPoint::frictionDirection, w->rollDirection);
    if (rollCos >= 0.0f) {
        w->slipSpeed = rollCos * w->CollisionPoint::spinSpeed;
        if (w->reportContactOutputs)
            *outSign = 1;
    } else {
        w->slipSpeed = (-rollCos) * w->CollisionPoint::spinSpeed;
        if (w->reportContactOutputs)
            *outSign = 0;
    }
}

// 0x005135f0: contact-patch update. relativePosition = contact - *pos,
// field_0x5c = axis x relativePosition, normalLeverCross = (n (axis.n)) x relativePosition;
// with a usable tangential reference the slip is tangentSpeed * tangent + lever, else the
// lever alone. frictionDirection is the normalised slip, slipSpeed its roll component.
void VehicleWheel::UpdateContactPatch(Vec3* pos, Vec3 axisIn, Vec3 refIn, float minLength,
                                      float* outValue, int* outSign)
{
    CollisionVec3 axis = FromVec3(axisIn);
    CollisionVec3 ref = FromVec3(refIn);

    CollisionVec3 rel;
    rel.x = CollisionPoint::worldPosition.x - pos->x;
    rel.y = CollisionPoint::worldPosition.y - pos->y;
    rel.z = CollisionPoint::worldPosition.z - pos->z;
    CollisionPoint::relativePosition = rel;

    CollisionPoint::field_0x5c = WheelCross(axis, CollisionPoint::relativePosition);

    float k = WheelDotInline(axis, CollisionPoint::surfaceNormal);
    CollisionVec3 projected = CollisionPoint::surfaceNormal * k;
    normalLeverCross = WheelCross(projected, CollisionPoint::relativePosition);

    if (minLength > 0.001f) {
        CollisionVec3 tangent;
        if (CollisionRejectFrom(&tangent, &ref, &this->CollisionPoint::surfaceNormal)) {
            CollisionVec3 normalized;
            tangent = *WheelNormalize(&normalized, &tangent);
        }
        float d = ref.z * tangent.z + ref.x * tangent.x + ref.y * tangent.y;
        CollisionPoint::tangentSpeed = d;
        if (d > 0.001f) {
            if (reportContactOutputs)
                *outValue = d;
            slipVector = tangent * CollisionPoint::tangentSpeed + normalLeverCross;
            float lenSq = WheelDot(&slipVector, &slipVector);
            float len;
            if (lenSq == 0.0f)
                len = 0.0f;
            else if (lenSq == 1.0f)
                len = 1.0f;
            else
                len = 1.0f / FastInvSqrt(lenSq);
            float inv = 1.0f / len;
            CollisionPoint::frictionDirection = slipVector * inv;
            CollisionPoint::spinSpeed = WheelLengthFromSq(WheelDot(&normalLeverCross, &normalLeverCross));
            float rollCos = WheelDotInline(CollisionPoint::frictionDirection, rollDirection);
            if (rollCos >= 0.0f) {
                slipSpeed = WheelLengthFromSq(WheelDot(&slipVector, &slipVector)) * rollCos;
                if (reportContactOutputs)
                    *outSign = 1;
            } else {
                slipSpeed = (-rollCos) * WheelLengthFromSq(WheelDot(&slipVector, &slipVector));
                if (reportContactOutputs)
                    *outSign = 0;
            }
            return;
        }
    }
    ContactPatchLeverOnly(this, outValue, outSign);
}

// ---------------------------------------------------------------------------------------
// 0x00514550 support.
struct WheelSurfaceTables {
    char pad[0x3a0];
    float gripScale[16];       // +0x3a0
    float secondScale[16];     // +0x3e0
};
struct WheelSurfaceRef {
    char pad[0xa4];
    WheelSurfaceTables* tables;
};
struct WheelBox {
    float min[3];
    float max[3];
};
struct WheelBoundsRef {        // CollisionObject::contactRecord
    float scale;
    WheelBox* box;
    CollisionVec3 normal;
};

static inline unsigned char* WheelSurfaceByte(VehicleWheel* w)
{
    return (unsigned char*)&w->CollisionPoint::surfaceType;
}

static inline void WheelNormalizeInPlace(CollisionVec3& v)
{
    float lenSq = v.z * v.z + (v.x * v.x + v.y * v.y);
    if (lenSq == 0.0f) {
        v = kWheelZero;
    } else {
        float inv = FastInvSqrt(lenSq);
        v.x *= inv;
        v.y *= inv;
        v.z *= inv;
    }
}

static inline CollisionVec3 WheelNormalizeOr(CollisionVec3 v, const CollisionVec3& fallback)
{
    if (v.x == 0.0f && v.y == 0.0f && v.z == 0.0f) {
        v = fallback;
    } else {
        float lenSq = WheelDot(&v, &v);
        if (lenSq == 0.0f) {
            v = kWheelZero;
        } else {
            float inv = FastInvSqrt(lenSq);
            v.x *= inv;
            v.y *= inv;
            v.z *= inv;
        }
    }
    return v;
}

static inline int WheelSameVec(const CollisionVec3& a, const CollisionVec3& b)
{
    return a.y == b.y && a.x == b.x && a.z == b.z;
}

static inline CollisionVec3 WheelLocalToWorldDirection(SoultreeObject* node, const CollisionVec3& v)
{
    return FromVec3(node->LocalToWorldDirection(AsVec3(v)));
}

// 0x00514550: places the wheel on the ground. Recomputes the wheel axes, probes the
// terrain under the wheel centre, on the rim in the travel direction and at a refined
// rim point, keeps the deepest contact, then checks the collision object's own contact
// and finally derives inContact and the surface grip factors.
// Parameter roles (Vehicle.h): world probe, body velocity, pitch, radius scale (sin roll),
// yaw, forward and up axes, pose node.
void VehicleWheel::UpdateSuspensionProbe(SoultreeProbe* world, const Vec3* velocity, float angle,
                                         float radiusScale, float yaw, const Vec3* forwardIn,
                                         const Vec3* upIn, SoultreeObject* poseNode)
{
    const CollisionVec3* a6 = (const CollisionVec3*)forwardIn;
    const CollisionVec3* a7 = (const CollisionVec3*)upIn;
    CollisionVec3& origin = wheelCenter;

    MovingPart::node->GetPositionIn(0, AsVec3(&origin));
    CollisionPoint::surfacePosition = origin;
    QueryGround(world, &this->CollisionPoint::surfacePosition, &this->CollisionPoint::surfaceNormal, WheelSurfaceByte(this));

    if (MovingPart::field_0x44) {
        SoultreeObject* steerNode = ((SteeringControl*)MovingPart::field_0x44)->node;
        wheelUpAxis = WheelLocalToWorldDirection(steerNode, kWheelZAxis);
        CollisionVec3 nodeY = WheelLocalToWorldDirection(steerNode, kWheelYAxis);
        sideAxis = WheelCross(wheelUpAxis, nodeY);
        WheelNormalizeInPlace(sideAxis);
    } else {
        wheelUpAxis = *a6;
        if (a7 == 0) {
            CollisionVec3 nodeY = WheelLocalToWorldDirection(poseNode, kWheelYAxis);
            sideAxis = WheelCross(*a6, nodeY);
        } else {
            sideAxis = WheelCross(*a6, *a7);
        }
        WheelNormalizeInPlace(sideAxis);
    }

    rollDirection = WheelNormalizeOr(WheelCross(CollisionPoint::surfaceNormal, sideAxis), wheelUpAxis);

    CollisionVec3 lateral = WheelCross(rollDirection, sideAxis);
    if (WheelDotInline(lateral, CollisionPoint::surfaceNormal) > 0.0f) {
        lateral = lateral * -1.0f;
        sideAxis = sideAxis * -1.0f;
    }

    // First probe: the wheel centre pushed out along the lateral axis by the radius.
    CollisionVec3 firstProbe = lateral * wheelRadius + origin;
    CollisionPoint::worldPosition = firstProbe;
    CollisionPoint::surfacePosition = firstProbe;
    CollisionVec3 n1;
    QueryGround(world, &this->CollisionPoint::surfacePosition, &n1, WheelSurfaceByte(this));
    CollisionPoint::penetration = (CollisionPoint::surfacePosition.y - CollisionPoint::worldPosition.y) * n1.y;

    // Second probe position pt: on the wheel rim, in the direction selected by `angle`.
    CollisionVec3 pt;
    float mag = angle < 0.0f ? -angle : angle;
    if (mag != 1.5707964f) {
        CollisionVec3 base;
        CollisionVec3 rimDirection;
        if (MovingPart::field_0x44) {
            base = rollDirection * wheelRadius + origin;
            rimDirection = WheelCross(rollDirection, kWheelYAxis);
        } else {
            CollisionVec3 rotated;
            D3DRMVectorRotate(&rotated, &kWheelZAxis, &kWheelYAxis, yaw);
            base = rotated * -wheelRadius + origin;
            rimDirection = WheelCross(rotated, kWheelYAxis);
        }
        float cLenSq = WheelDotInline(rimDirection, rimDirection);
        if (cLenSq == 0.0f) {
            rimDirection = kWheelZero;
        } else {
            float inv = FastInvSqrt(cLenSq);
            rimDirection = rimDirection * inv;
        }
        float reach = radiusScale * wheelRadius;
        pt = rimDirection * reach + base;
    } else {
        pt = *a7 * -wheelRadius + origin;
    }

    // Second probe at pt, then a refinement using the axes found there.
    CollisionVec3 n2;
    unsigned char kind;
    QueryGround(world, &pt, &n2, &kind);
    CollisionVec3 probeRoll = WheelNormalizeOr(WheelCross(n2, sideAxis), wheelUpAxis);
    CollisionVec3 lateral2 = WheelCross(probeRoll, sideAxis);
    if (WheelDotInline(lateral2, n2) > 0.0f)
        lateral2 = lateral2 * -1.0f;

    CollisionVec3 rimPoint = lateral2 * wheelRadius + origin;
    pt = rimPoint;
    CollisionVec3 n3;
    QueryGround(world, &pt, &n3, &kind);
    float delta = pt.y - rimPoint.y;
    float score = delta * n3.y;
    if (score > CollisionPoint::penetration) {
        lateral = lateral2;
        CollisionPoint::worldPosition = rimPoint;
        CollisionPoint::penetration = score;
        CollisionPoint::surfacePosition = pt;
        *WheelSurfaceByte(this) = (unsigned char)(kind & 7);
        if (!WheelSameVec(n2, n3)) {
            CollisionPoint::surfaceNormal = n3;
            rollDirection = WheelNormalizeOr(WheelCross(n3, sideAxis), wheelUpAxis);
        } else {
            CollisionPoint::surfaceNormal = n2;
            CollisionPoint::penetration = delta * n2.y;
            rollDirection = probeRoll;
        }
    } else {
        *WheelSurfaceByte(this) = (unsigned char)(*WheelSurfaceByte(this) & 7);
        if (!WheelSameVec(CollisionPoint::surfaceNormal, n1)) {
            CollisionPoint::surfaceNormal = n1;
            rollDirection = WheelNormalizeOr(WheelCross(n1, sideAxis), wheelUpAxis);
        } else {
            CollisionPoint::penetration = (CollisionPoint::surfacePosition.y - CollisionPoint::worldPosition.y)
                                          * CollisionPoint::surfaceNormal.y;
        }
    }

    // The wheel's own collision shape against other objects.
    CollisionMatrix4 frame;
    WheelBuildFrame(&frame, &origin, &wheelUpAxis, &lateral, 1, 1);
    SetTransform((const Matrix4*)&frame);   // CollisionObject 0x00435830
    QueryCollisions();

    if (CollisionObject::hasContact) {
        WheelBoundsRef* bounds = (WheelBoundsRef*)CollisionObject::contactRecord;
        float boxDepth = (1.0f - bounds->scale) * wheelRadius;
        if (boxDepth > CollisionPoint::penetration) {
            CollisionPoint::penetration = boxDepth;
            CollisionVec3 ext = WheelVec3(bounds->box->max[0] - bounds->box->min[0],
                                          bounds->box->max[1] - bounds->box->min[1],
                                          bounds->box->max[2] - bounds->box->min[2]);
            CollisionVec3 e2 = ext * bounds->scale;
            CollisionVec3 e3 = WheelVec3(e2.x + bounds->box->min[0], e2.y + bounds->box->min[1],
                                         e2.z + bounds->box->min[2]);
            WheelTransformPoint(&this->CollisionPoint::surfacePosition, &e3, &frame);       // 0x0042a510
            CollisionPoint::surfaceNormal = bounds->normal;
            *WheelSurfaceByte(this) = 7;
            WheelTransformPoint(&this->CollisionPoint::worldPosition, (const CollisionVec3*)bounds->box->max,
                                &frame);                                              // 0x0042a580
            CollisionVec3 rolled;
            rollDirection = *WheelUnitCross(&rolled, &this->CollisionPoint::surfaceNormal, &sideAxis, &wheelUpAxis);
        }
    }

    if (CollisionPoint::penetration < 0.0f && CollisionPoint::penetration > -0.01f)
        CollisionPoint::penetration = 0.0f;
    inContact = (CollisionPoint::penetration >= CollisionPoint::penetrationThreshold) ? 1 : 0;
    wheelVelocity = *(const CollisionVec3*)velocity;
    if (CollisionPoint::surfaceOwner) {
        WheelSurfaceRef* ref = (WheelSurfaceRef*)CollisionPoint::surfaceOwner;
        CollisionPoint::surfaceGrip = ref->tables->gripScale[*WheelSurfaceByte(this)];
        surfaceScaleB = ref->tables->secondScale[*WheelSurfaceByte(this)];
    } else {
        CollisionPoint::surfaceGrip = 1.0f;
        surfaceScaleB = 1.0f;
    }
    reportContactOutputs = 0;
    field_0x268 = 0;
    CollisionPoint::inContact = 0;
    CollisionPoint::tangentSpeed = 0.0f;
    CollisionPoint::spinSpeed = 0.0f;
}

// ---------------------------------------------------------------------------------------
// 0x005143d0: advances the wheel's roll angle for a step (crashState is unused).
void VehicleWheel::UpdateRoll(float dt, float distanceScale, int locked, int driven, int crashState,
                              float driveScale)
{
    if (locked) {
        if (rampLevel != 0.0f) {
            rollAngle = (1.0f - rampLevel) * rollAngle;
            ApplyRollAngle();
            return;
        }
        if (field_0x2a8 && field_0x2a8->value < 1.0f && field_0x2a8->value != 0.0f)
            rollAngle = field_0x2a8->value * 6.0f;
        ApplyRollAngle();
    } else if (driven) {
        if (field_0x2a8) {
            float drive = field_0x2bc * field_0x280 * dt * driveScale;
            field_0x290 = drive;
            if (slipSpeed > 0.1f)
                SetRollDistance(distanceScale * slipSpeed + drive);
        } else if (slipSpeed > 0.1f) {
            SetRollDistance(distanceScale * slipSpeed);
        }
    } else if (inContact) {
        if (field_0x2a8) {
            float drive = field_0x2bc * field_0x280 * dt * driveScale;
            field_0x290 = drive;
            SetRollDistance(distanceScale * slipSpeed + drive);
        } else {
            SetRollDistance(distanceScale * slipSpeed);
        }
    } else {
        if (field_0x2a8 && field_0x2a8->value < 1.0f && field_0x2a8->value != 0.0f)
            rollAngle = field_0x2a8->value * 6.0f;
        ApplyRollAngle();
    }
}

// 0x00513f90: drive share +0x2b8 (and 1 - it at +0x2bc) from the vehicle speed while the
// wheel slips less than the threshold +0x2b4; scales the drive force +0x248 by it.
void VehicleWheel::UpdateDriveShare(Vehicle* owner)
{
    WheelVehicleView* vehicle = (WheelVehicleView*)owner;
    if (!field_0x2a8)
        return;
    if (slipSpeed < field_0x2b4 && !vehicle->field_0x444) {
        float boost = 1.0f;
        if (vehicle->field_0xa4 > 0.0f && vehicle->field_0x60 * 1.25f > 1.0f)
            boost = vehicle->field_0x60 * 1.25f;
        float grip = vehicle->UnknownVirtualSlot47(this);
        float excess = field_0x280 * vehicle->field_0x24
                       - WheelLengthFromSq(WheelDotInline(vehicle->field_0x70, vehicle->field_0x70));
        float share;
        if (excess < 1.01f) {
            share = 1.0f;
        } else {
            if (!(excess < 320.0f))
                excess = 320.0f;
            share = (400.0f - excess) * 0.0025f;
        }
        float minimum;
        if (slipSpeed > field_0x2b4)
            minimum = 1.0f;
        else
            minimum = 1.0f - (field_0x2b4 - slipSpeed) / field_0x2b4;
        share = share * grip * boost;
        if (!(share < 1.0f))
            share = 1.0f;
        if (!(share > minimum))
            share = minimum;
        field_0x2b8 = share * surfaceScaleB;
        field_0x248 = field_0x248 * field_0x2b8;
        field_0x2bc = 1.0f - field_0x2b8;
    } else {
        field_0x2b8 = 1.0f;
        field_0x2bc = 0.0f;
    }
}

// 0x00514170: pushes along the roll direction with the drive strength, limited by what
// the body can take this step; the push goes to *force (7th argument) and its moment
// about the contact, times +0x1d8, to *torque (6th argument).
void VehicleWheel::ApplyDrive(float share, float stepTime, int forward, float mass, float* speed,
                              Vec3* torque, Vec3* force)
{
    if (!inContact || field_0x2a0 == 0.0f || rampLevel == 0.0f || slipSpeed == 0.0f)
        return;
    float grip = CollisionPoint::surfaceGrip * field_0x2a0 * rampLevel;
    CollisionVec3 push = rollDirection * -grip;
    CollisionVec3 dir = push;
    float amount = share * slipSpeed;
    if (!forward)
        dir = push * -1.0f;
    amount = *speed < amount ? *speed : amount;
    amount = amount > 0.0f ? amount : 0.0f;
    float rate = mass / stepTime;
    float needed = rate * amount;
    if (grip > needed) {
        push = dir * (needed / grip);
        *speed -= amount;
    } else {
        *speed -= grip / rate;
        push = dir;
    }
    force->x += push.x;
    force->y += push.y;
    force->z += push.z;
    CollisionVec3 moment = WheelCross(CollisionPoint::relativePosition, push);
    float momentScale = field_0x1d8;
    torque->x += moment.x * momentScale;
    torque->y += moment.y * momentScale;
    torque->z += moment.z * momentScale;
}

// 0x00513c70: runs the attached shock (+0x2b0 inline, else +0x2ac rotating) against the
// contact while the wheel touches; the solve's load and active flag become the contact's
// penetration and inContact. pointsTouching and the fourth argument are unused.
void VehicleWheel::UpdateShock(float dt, int pointsTouching, int crashed, float unused,
                               Vec3* velocityIn, Vec3* axisBIn)
{
    const CollisionVec3* velocity = (const CollisionVec3*)velocityIn;
    const CollisionVec3* axisB = (const CollisionVec3*)axisBIn;
    int active;
    float load;
    if (WheelInlineShock* slider = field_0x2b0) {
        if (inContact) {
            float depth = CollisionPoint::penetration;
            if (depth != 0.0f || slider->ratio != 0.0f) {
                InlineShockSolveContact(slider, dt, velocity, &this->CollisionPoint::field_0x5c, &wheelCenter,
                                        &this->CollisionPoint::surfaceNormal, depth, &load, &active);
                field_0x26c = slider->extending;
                if (active || crashed)
                    CollisionPoint::inContact = 1;
                CollisionPoint::penetration = load;
                CollisionPoint::worldPosition = CollisionPoint::worldPosition + slider->stepDisplacement;
                if (active)
                    CollisionPoint::surfacePosition = CollisionPoint::surfaceNormal * load
                                                      + CollisionPoint::worldPosition;
                else
                    CollisionPoint::surfacePosition = CollisionPoint::worldPosition;
                field_0x268 = 1;
                return;
            }
        }
        field_0x26c = 0;
        CollisionPoint::inContact = 0;
    } else if (WheelRotatingShock* hinge = field_0x2ac) {
        if (inContact) {
            float depth = CollisionPoint::penetration;
            if (depth != 0.0f || hinge->ratio != 0.0f) {
                RotatingShockSolveContact(hinge, dt, &sideAxis, axisB, &wheelCenter,
                                          &this->CollisionPoint::surfaceNormal, velocity,
                                          &this->CollisionPoint::field_0x5c, depth, &load, &active);
                field_0x26c = hinge->extending;
                if (active || crashed)
                    CollisionPoint::inContact = 1;
                CollisionPoint::penetration = load;
                CollisionPoint::worldPosition = CollisionPoint::worldPosition + hinge->stepDisplacement;
                if (active)
                    CollisionPoint::surfacePosition = CollisionPoint::surfaceNormal * load
                                                      + CollisionPoint::worldPosition;
                else
                    CollisionPoint::surfacePosition = CollisionPoint::worldPosition;
                field_0x268 = 1;
                return;
            }
        }
        field_0x26c = 0;
        CollisionPoint::inContact = 0;
    } else {
        CollisionPoint::inContact = 1;
    }
}

// ---------------------------------------------------------------------------------------
// KrustyBike's view of the same wheel (src/krusty2/vehicle/KrustyBikeTypes.h BikeWheel)
// calls 0x00513f90 with the bike as an untyped pointer.
struct BikeWheel {
    void UpdateDriveShare(void* vehicle);
};

void BikeWheel::UpdateDriveShare(void* vehicle)
{
    ((VehicleWheel*)this)->UpdateDriveShare((Vehicle*)vehicle);
}
