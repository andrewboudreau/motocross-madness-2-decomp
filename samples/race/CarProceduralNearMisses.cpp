// Near-miss CarProcedural.cpp candidates, kept out of src/reconstructed until
// they match. They use src/reconstructed/CarProcedural.h; the bindings are in
// CarProceduralNearMisses.bindings.json. Scores are matching bytes under
// vc6_o2_mt with relocations resolved.
//
// CarProcedural::UnknownFunction430b10 (0x00430b10, 842 bytes; 299 of 842):
// the path evaluation. The key search loops, the Hermite weights call (its
// fourth weight written over the `time` parameter, as retail's
// `lea ecx,[esp+0x60]` shows), the tangent temporaries and the final
// h0*a + h1*b + t0*h2 + t1*h3 sum order match. Retail inlines all fourteen
// vector operators; written with operators throughout, VC6 runs out of
// inline budget (docs/PHYSICS_VALIDATION.md) and calls the Vector3
// constructor and operator+ out of line. With the chord written per
// component (below) everything inlines, but VC6 then keeps `this` in edi and
// the key count in esi (retail: esi, edi) and addresses the second key
// through its base instead of retail's `lea ecx,[key + 4]` (the inlined
// operator-'s reference). Named intermediate vectors barely change the
// budget. The budget follows the caller's front-end tree size
// (docs/VC6_INLINE_BUDGET.md): about 30 extra trivial statements (dead
// code counts, empty statements do not) make VC6 expand all 14 operators
// and match retail's first 0x19b bytes; the tangent temporaries' slots
// then differ. No padded source is kept.
//
// Not reconstructed: slot 10 (0x0042fd80, 2907 bytes), the per-frame
// update along the path (wheel spin and steering, the collision objects'
// transform and the body velocity).

#include <math.h>

#include "../../src/reconstructed/CarProcedural.h"

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
Vector3 CarProcedural::UnknownFunction430b10(float time)
{
    unsigned int count = field_0x6c;
    CarProceduralKey* keys = field_0x1b4;
    float period = keys[count - 1].time * field_0x174;
    float t = fmod(time, period);
    if (t < 0.0)
        t += period;
    if (count > 0) {
        t *= field_0x170;
        unsigned int next = field_0x64 + 1;
        while (next < count && t > keys[next].time) {
            field_0x64++;
            next++;
        }
        while (field_0x64 > 0 && t < keys[field_0x64].time) {
            field_0x64--;
            next--;
        }
        // The fourth weight is written over `time`.
        float h0, h1, h2;
        UnknownFunction430e60((t - keys[field_0x64].time) / (keys[next].time - keys[field_0x64].time),
                              &h0, &h1, &h2, &time);

        CarProceduralKey* a = &field_0x1b4[field_0x64];
        CarProceduralKey* b = &field_0x1b4[next];
        Vector3 chord;
        chord.x = b->position.x - a->position.x;
        chord.y = b->position.y - a->position.y;
        chord.z = b->position.z - a->position.z;
        Vector3 t0;
        if (field_0x64 == 0) {
            t0 = chord;
        } else {
            float s = (b->time - a->time) / (b->time - a[-1].time);
            t0 = ((a->position - a[-1].position) + chord) * s;
        }
        Vector3 t1;
        if (next == field_0x6c - 1) {
            t1 = chord;
        } else {
            float s = (b->time - a->time) / (b[1].time - a->time);
            t1 = ((b[1].position - b->position) + chord) * s;
        }
        return h0 * a->position + h1 * b->position + t0 * h2 + t1 * time;
    }
    return kVec3Zero;
}


// ---------------------------------------------------------------------------
// Slot 10, 0x0042fd80 (2907 bytes): the per-frame update along the path.
// ---------------------------------------------------------------------------

static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// 0x004b5d00 (cdecl): the look and up vectors of a heading and pitch
// (samples/track/SceneManagerNearMisses.cpp declares the same function).
void UnknownFunction4b5d00(Vector3* look, Vector3* up, float heading, float pitch, int a);

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
    return UnknownFunction460b50(squared);
}
// v / |v| in place; the zero vector stays zero.
static inline void CarNormalizeInPlace(Vector3& v)
{
    float squared = v.z * v.z + (v.y * v.y + v.x * v.x);
    if (squared == 0.0f) {
        v = kVec3Zero;
    } else {
        float inv = UnknownFunction460c00(squared);
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

int CarProcedural::UnknownVirtualSlot10(float frameTime)
{
    int result = GameObject::UnknownVirtualSlot10(frameTime);
    if (frameTime <= 0.0)
        return result;
    if (!field_0x54) {
        // First update: a two-point mesh for the sensor collider, and the
        // wheels' height from the first wheel above the origin.
        Vector3 box[2];
        box[0] = kVec3Zero;
        box[0].y += field_0x188;
        box[1] = kVec3Zero;
        box[1].y -= field_0x188;
        field_0x38->UnknownFunction432ab0(1, box);
        for (unsigned int i = 0; i < field_0x60; i++) {
            Vector3 position;
            field_0x210[i]->UnknownFunction4fc970(&position);
            if (position.z > 0.0)
                field_0x198 = position.z;
        }
        field_0x54 = 1;
    }
    field_0x194 = (float)fmod(frameTime + field_0x194, field_0x1b4[field_0x6c - 1].time * field_0x174);
    float t;
    if (field_0x19c > 0.0) {
        field_0x1a0 = UnknownFunction4308e0(field_0x194, field_0x19c);
        t = field_0x194 - field_0x1a0;
        field_0x1c4 = UnknownFunction430b10(t);
    } else {
        field_0x1a0 = 0.0f;
        t = field_0x194;
        field_0x1c4 = UnknownFunction430b10(t);
    }
    Vector3 ahead = UnknownFunction430b10(t + field_0x1a4);
    Vector3 forward = ahead - field_0x1c4;
    forward.y = 0.0f;
    field_0x40 = forward * field_0x1a8;
    field_0x4c = CarLength(field_0x40);
    Vector3 local = field_0x34->UnknownFunction4fd710(forward);
    if (field_0x60 > 0)
        field_0x18c = (float)fmod(field_0x184 * field_0x1a8 * local.z * frameTime + field_0x18c, 6.2831855f);
    ahead = UnknownFunction430b10(t - UnknownFunction4308e0(t, -1.0f));
    Vector3 back = ahead - field_0x1c4;
    CarNormalizeInPlace(back);
    if (field_0x5c)
        field_0x1f4 = back;
    else
        field_0x1f4 = field_0x1f4 * field_0x1ac + back * (1.0f - field_0x1ac);
    if (field_0x50 && field_0x60 > 0) {
        ahead = UnknownFunction430b10(t - UnknownFunction4308e0(t, -field_0x198));
        Vector3 side = ahead - field_0x1c4;
        Vector3 steer = field_0x34->UnknownFunction4fd710(side);
        float angle = (float)atan2(steer.x, steer.z);
        field_0x190 = angle;
        if (angle < -0.52359873f)
            field_0x190 = -0.52359873f;
        else if (angle > 0.52359873f)
            field_0x190 = 0.52359873f;
        else
            field_0x190 = angle;
    }
    Vector3 position;
    field_0x34->UnknownFunction4fc9a0(0, &position);
    Vector3 ground = position;
    Vector3 normal;
    ((CarProceduralTerrain*)field_0x20c)->UnknownFunction507c10(&ground, &normal, 0, 0);
    // The sensor collider's frame: the world axes orthonormalised.
    Vector3 forwardAxis = kVec3ZAxis;
    Vector3 upAxis = kVec3YAxis;
    Vector3 rightAxis = CarCross(upAxis, forwardAxis);
    upAxis = CarCrossCall(forwardAxis, rightAxis);
    upAxis = CarNormalizeCall(upAxis);
    forwardAxis = CarNormalizeCall(forwardAxis);
    rightAxis = CarCrossCall(upAxis, forwardAxis);
    Matrix4 transform;
    transform(0, 0) = rightAxis.x;
    transform(0, 1) = rightAxis.y;
    transform(0, 2) = rightAxis.z;
    transform(0, 3) = 0.0f;
    transform(1, 0) = upAxis.x;
    transform(1, 1) = upAxis.y;
    transform(1, 2) = upAxis.z;
    transform(1, 3) = 0.0f;
    transform(2, 0) = forwardAxis.x;
    transform(2, 1) = forwardAxis.y;
    transform(2, 2) = forwardAxis.z;
    transform(2, 3) = 0.0f;
    transform(3, 0) = position.x;
    transform(3, 1) = position.y;
    transform(3, 2) = position.z;
    transform(3, 3) = 1.0f;
    field_0x38->UnknownFunction435830(&transform);
    field_0x38->UnknownFunction438e70();
    if (field_0x38->field_0x58) {
        CarProceduralContact* contact = field_0x38->field_0x5c;
        Vector3 lifted = kVec3YAxis * ((1.0f - contact->field_0x00.x) * field_0x188 * 2.0f - field_0x188) + position;
        if (lifted.y > ground.y) {
            ground = lifted;
            normal = contact->field_0x0c;
        }
    }
    if (field_0x5c)
        field_0x200 = normal;
    else
        field_0x200 = field_0x200 * field_0x1b0 + normal * (1.0f - field_0x1b0);
    Vector3 flat = field_0x1f4 - field_0x200 * CarDot(field_0x1f4, field_0x200);
    field_0x34->UnknownFunction4fbd70(&flat, &field_0x200, 0, 1);
    Vector3 target = field_0x1d0 + field_0x1c4;
    target.y = ground.y;
    Vector3 placement = target - field_0x34->UnknownFunction4fd5c0(field_0x1dc);
    field_0x34->UnknownFunction4fc740(0, &placement);
    for (unsigned int i = 0; i < field_0x60; i++) {
        Vector3 axisA;
        Vector3 axisB;
        if (field_0x50 && i < 2) {
            UnknownFunction4b5d00(&axisA, &axisB, field_0x190, -field_0x18c, 0);
        } else {
            axisA.x = 0.0f;
            axisA.y = (float)sin(-field_0x18c);
            axisA.z = (float)cos(-field_0x18c);
            axisB.x = 0.0f;
            axisB.y = axisA.z;
            axisB.z = -axisA.y;
        }
        field_0x210[i]->UnknownFunction4fbd70(&axisA, &axisB, 0, 1);
    }
    field_0x3c->UnknownFunction435fb0();
    if (field_0x3c->UnknownFunction438e70()) {
        CarProceduralContact* contact = field_0x3c->field_0x5c;
        field_0x214 = contact->field_0x18;
        field_0x220 = contact->field_0x0c;
        field_0x3c->field_0x54->field_0xf8 -= contact->field_0x00 * 1.005f;
    }
    return result;
}
