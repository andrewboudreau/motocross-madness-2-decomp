// Near-miss Krusty3DObjects.cpp candidates, kept out of src/reconstructed
// until they match. See docs/KRUSTY3DOBJECTS.md. Check one with
//   tools/compile.py --compiler vc6 samples/race/Krusty3DObjectsNearMisses.cpp -o X.obj
// and compare the symbol against retail with
// samples/race/Krusty3DObjectsNearMisses.bindings.json.
//
// BonusObjectManager::UnknownVirtualSlot10 (0x0048ccc0, 1308 bytes): steps
// and places the bonus frame. 1295 of 1308 bytes match. The difference is
// the size factor's denominator `field_0x38->field_0x198 * field_0x1e4`:
// retail loads field_0x38 before storing the length and multiplies
// `fld [camera+0x198]; fmul [this+0x1e4]`; VC6 here loads the camera after
// the numerator and emits `fld [this+0x1e4]; fmul [camera+0x198]`. Both
// operand orders, a separate length local, a chained assignment, a ternary
// length and a denominator local were tried.
//
// VisualCue::UnknownVirtualSlot10 (0x0048b100, 2796 bytes, jump table at
// 0x0048bbec): places the cue in front of the camera by camera mode
// (TrackGame +0x2d74) and turns it toward the followed racer, the next
// gate, the selected racer or the free camera's target. The data flow below
// is decoded from retail; the codegen is far off. Retail inlines the
// placement block in each case with the camera's third matrix column copied
// by integer moves and the first two through the FPU, and never calls the
// out-of-line Vector3 constructor (0x00404e60) that VC6 here emits for the
// inlined helper.

#include <math.h>

#include "../../src/reconstructed/Krusty3DObjects.h"

static inline Vector3 operator-(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline Vector3 operator+(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline Vector3& operator*=(Vector3& v, float s) {
    v.x *= s;
    v.y *= s;
    v.z *= s;
    return v;
}

// |v|, exact for unit vectors.
static inline float Length(Vector3 v) {
    float squared = v.z * v.z + (v.y * v.y + v.x * v.x);
    if (squared == 1.0f)
        return 1.0f;
    return UnknownFunction460b50(squared);
}

static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);

static inline float UnknownDegreesToRadians(float degrees) {
    return degrees * 0.01745329f;
}


static const UnknownBonusKey g_UnknownBonusKeys[7] = {
    {0, 0.0f, {0.0275f, 0.0275f, 0.0275f}, {0.0f, 8.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0.0f},
    {6, 0.4f, {2.78f, 2.78f, 2.78f}, {0.0f, 8.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0.0f},
    {8, 0.5333f, {2.09f, 2.09f, 2.09f}, {0.0f, 8.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0.0f},
    {25, 1.6667f, {2.09f, 2.09f, 2.09f}, {0.0f, 8.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0.0f},
    {33, 2.2f, {1.99f, 1.99f, 1.99f}, {0.0f, 18.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 3.1415923f},
    {40, 2.6667f, {1.18f, 1.18f, 1.18f}, {0.0f, 29.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 4.607669f},
    {90, 6.0f, {0.05f, 0.05f, 0.05f}, {0.0f, 117.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 15.079643f},
};


// 0x0048ccc0
int BonusObjectManager::UnknownVirtualSlot10(float frameTime) {
    int digit;
    int place;

    if (field_0x30 == -1 || g_UnknownKrustyGame56e26c->field_0x3430)
        return 1;
    if (field_0x2c->field_0x38->field_0x444 && field_0x34 > 2.0f) {
        UnknownFunction48d540();
        return 1;
    }
    field_0x34 += frameTime;
    if (field_0x34 < g_UnknownBonusKeys[6].field_0x04) {
        while (field_0x34 > g_UnknownBonusKeys[field_0x30 + 1].field_0x04)
            UnknownFunction48cad0();
        float t = (field_0x34 - g_UnknownBonusKeys[field_0x30].field_0x04) / field_0x17c;
        Vector3 position;
        field_0x2c->field_0x38->field_0x3bc->UnknownFunction4fc970(&position);
        position += t * field_0x1a0 + field_0x194;
        field_0x3c->UnknownFunction4fc690(0, position);
        if (field_0x38 != g_UnknownKrustyGame56e26c->field_0x10->field_0x08 || field_0x38->field_0x16c != field_0x1dc ||
            field_0x1f0) {
            field_0x38 = g_UnknownKrustyGame56e26c->field_0x10->field_0x08;
            field_0x1dc = field_0x38->field_0x16c;
            Vector3 delta = position - field_0x38->field_0x170;
            field_0x1f0 = 0;
            field_0x1e8 = (field_0x1ec = Length(delta)) * field_0x1e0 / (field_0x38->field_0x198 * field_0x1e4);
        }
        position += field_0x2c->field_0x38->field_0x5f4->field_0x230 * 5.0f;
        if (field_0x184 == 0.0f && field_0x180 == 0.0f) {
            field_0x188 = position - field_0x38->field_0x170;
            Vector3 forward = field_0x188;
            forward.y = 0.0f;
            field_0x3c->UnknownFunction4fbd70(&forward, &kVec3YAxis, 1, 1);
        } else {
            Vector3 axis = t * field_0x1d0 + field_0x1c4;
            field_0x3c->UnknownFunction4fd090(axis.x, axis.y, axis.z, t * field_0x184 + field_0x180);
        }
        Vector3 scale = t * field_0x1b8 + field_0x1ac;
        scale *= field_0x1e8;
        field_0x3c->UnknownFunction4fd340(scale.x, scale.y, scale.z);
        field_0x40[0][0]->field_0x18c = !field_0x40[0][0]->field_0x14c;
        field_0x40[0][1]->field_0x18c = !field_0x40[0][0]->field_0x14c;
        for (digit = 0; digit < 10; digit++)
            for (place = 2; place < 5; place++)
                field_0x40[digit][place]->field_0x18c = !field_0x40[digit][place]->field_0x14c;
        for (digit = 0; digit < 10; digit++)
            for (place = 0; place < 2; place++)
                field_0x124[digit][place]->field_0x18c = !field_0x124[digit][place]->field_0x14c;
        field_0x11c->field_0x18c = !field_0x11c->field_0x14c;
        field_0x120->field_0x18c = !field_0x120->field_0x14c;
    } else {
        UnknownFunction48d540();
    }
    GameObject::UnknownVirtualSlot10(frameTime);
    return 1;
}


// The owner a VisualCue compares its camera with (its +0x18).
struct UnknownCueOwner {
    unsigned char field_0x00[0x08];
    UnknownArcadeView* field_0x08;
};

// Places the cue at its screen position in front of the camera, rebuilding
// the screen offsets when the field of view changed.
inline void VisualCue::UnknownPlace() {
    Vector3 forward = Vector3(field_0x40->field_0x0ac[0][2], field_0x40->field_0x0ac[1][2], field_0x40->field_0x0ac[2][2]);
    Vector3 up = Vector3(field_0x40->field_0x0ac[0][1], field_0x40->field_0x0ac[1][1], field_0x40->field_0x0ac[2][1]);
    Vector3 right = Vector3(field_0x40->field_0x0ac[0][0], field_0x40->field_0x0ac[1][0], field_0x40->field_0x0ac[2][0]);
    if (field_0x40->field_0x16c != field_0xb0) {
        field_0xb0 = field_0x40->field_0x16c;
        float half = (float)tan(UnknownDegreesToRadians(field_0xb0 * 0.5f)) * field_0x44;
        field_0xb4 = (UnknownScreenX() - 0.5f) * half * 2.0f;
        field_0xb8 = (0.5f - UnknownScreenY()) * field_0x40->field_0x1b8 * half * 2.0f;
        field_0x34 = field_0x38 * field_0x44 / (field_0x40->field_0x198 * field_0x3c);
    }
    Vector3 position = field_0x40->field_0x170;
    position += forward * field_0x44;
    position += right * field_0xb4;
    position += up * field_0xb8;
    UnknownFunction4014f0(&position);
}

// 0x0048b100
int VisualCue::UnknownVirtualSlot10(float frameTime) {
    float scale;

    if (!ArcadeObject::UnknownVirtualSlot10(frameTime))
        return 1;
    if (!field_0xbc) {
        if (((UnknownCueOwner*)field_0x18)->field_0x08 != field_0x40 ||
            field_0xa8->field_0x50->field_0x3b0 != field_0xa8->field_0x38 || !field_0xa8->field_0x38 ||
            field_0xa8->field_0x38->field_0x7a4) {
            field_0x30 = 0;
            return 1;
        }
    }
    switch (g_UnknownKrustyGame56e26c->field_0x2d74) {
    case 2:
    case 3: {
        if (!field_0xa8->field_0x48)
            goto hide;
        if (field_0xa8->field_0x38->field_0x78c) {
            field_0x30 = 0;
            return 1;
        }
        field_0x30 = 1;
        UnknownPlace();
        Vector3 target;
        if (field_0xa8->field_0x48)
            field_0xa8->field_0x48->UnknownFunction518080(field_0xa8->field_0x38->field_0x744->field_0x44, &target);
        else
            target = field_0xa8->field_0x38->field_0x10c;
        field_0xac->UnknownFunction507c10(&target, 0, 0, 0);
        Vector3 direction = target - field_0x40->field_0x170;
        UnknownFunction401520(&direction, &kVec3YAxis, 1, 1);
        scale = field_0x34;
        field_0x2c->UnknownFunction4fd340(scale, scale, scale);
        return 1;
    }
    case 1:
    case 5: {
        UnknownPlace();
        UnknownKrustyRacerRef* ref = g_UnknownKrustyGame56e26c->field_0x560->field_0x34->field_0x50->field_0x3b4;
        if (!ref)
            return 1;
        Vector3 direction =
            g_UnknownKrustyGame56e26c->field_0x560->field_0xd8[ref->field_0x7b8].field_0x00 - field_0x40->field_0x170;
        UnknownFunction401520(&direction, &kVec3YAxis, 1, 1);
        scale = field_0x34;
        break;
    }
    case 0: {
        if (!field_0xbc || field_0xf8 <= 1)
            goto hide;
        field_0x30 = 1;
        UnknownPlace();
        Vector3 target = kVec3Zero;
        for (int i = 0; i < field_0xf8; i++) {
            if (field_0xc0 == i) {
                target = field_0xcc[i]->field_0x0c;
                break;
            }
        }
        Vector3 direction = target - field_0x40->field_0x170;
        UnknownFunction401520(&direction, &kVec3YAxis, 1, 1);
        scale = field_0x34;
        field_0x2c->UnknownFunction4fd340(scale, scale, scale);
        return 1;
    }
    case 4: {
        UnknownPlace();
        if (g_UnknownKrustyGame56e26c->field_0x2eb4) {
            Vector3 target;
            g_UnknownKrustyGame56e26c->field_0x568->field_0xdc->field_0x21c.UnknownFunction4fc970(&target);
            Vector3 direction = target - field_0x40->field_0x170;
            UnknownFunction401520(&direction, &kVec3YAxis, 1, 1);
            scale = field_0x34;
        } else {
            UnknownVisualCueRacer* followed = 0;
            if (g_UnknownKrustyGame56e26c->field_0x568->field_0x3c->field_0x25 & 1)
                followed = g_UnknownKrustyGame56e26c->field_0x568->field_0x3c->field_0x3b4;
            if (!g_UnknownKrustyGame56e26c->field_0x568->field_0xa8)
                return 1;
            if (followed == g_UnknownKrustyGame56e26c->field_0x568->field_0xa8)
                goto hide;
            Vector3 direction = g_UnknownKrustyGame56e26c->field_0x568->field_0xa8->field_0x0c - field_0x40->field_0x170;
            UnknownFunction401520(&direction, &kVec3YAxis, 1, 1);
            scale = field_0x34;
        }
        break;
    }
    default:
        return 1;
    }
    field_0x2c->UnknownFunction4fd340(scale, scale, scale);
    field_0x30 = 1;
    return 1;
hide:
    field_0x30 = 0;
    return 1;
}

