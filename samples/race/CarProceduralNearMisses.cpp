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
// operator-'s reference). Named intermediate vectors do not change the budget.
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

