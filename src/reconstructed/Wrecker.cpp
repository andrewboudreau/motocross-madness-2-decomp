// Wrecker.cpp -- reconstruction of part of D:\aardvark\VC\krusty2\wrecker.cpp.
// See Wrecker.h and docs/WRECKER.md.

#include "Wrecker.h"

#include <stdlib.h>

#include "DebugAlloc.h"

// Per-TU vector constants (tier 2, the shape documented in docs/LZW.md and
// src/krusty2/math/Math3D.h): wrecker.cpp's four dynamic initializers
// (.CRT$XCU 0x00566574..0x00566580, code 0x00532f00..0x0053303b) build
// (0,0,0), (1,0,0), (0,1,0) and (0,0,1) into 0x0068ad00, 0x0068ad10,
// 0x0068ad20 and 0x0068acf0. Slot 10 (0x00530739) reads the zero vector.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// A random value in [0, 1). A macro: as an inline function, VC6 moves the
// constant factor behind the caller's multiply (retail keeps rand * 1/32768
// first, then the scale).
#define RANDOM_UNIT() (rand() * (1.0f / 32768))

Wrecker::Wrecker(int flags) : GraphicsTest(flags) {
    field_0x48 = 0;
    field_0x4c = 0;
    field_0x50 = 0;
    field_0x54 = 0;
    field_0x58 = 0;
    field_0x5c = 0;
    field_0x60 = 0;
    field_0x64 = 0;
    field_0x44 = 0;
    field_0xac = 0;
    field_0xb4 = 0;
    field_0xd4 = 0;
    field_0xd8 = 0;
    field_0x104 = 0;
    field_0x108 = 0;
    field_0xdc = Vector3(0.0f, 0.0f, 0.0f);
    field_0x10c = Vector3(0.0f, 0.0f, 0.0f);
    field_0x118 = 5.0f;
    field_0x120 = 0;
    field_0x124 = 0x1d;
    field_0x11c = 0;
    for (int i = 0; i < 256; i++)
        field_0x128[i] = RANDOM_UNIT();
    field_0x528 = 0;
    field_0x52c = 0;
    field_0x530 = 0;
    field_0x534 = 0;
    field_0x538 = 0;
}

Wrecker::~Wrecker() {
    if (field_0x104) DebugFree(field_0x104, __FILE__, 0x3e);
    if (field_0x108) DebugFree(field_0x108, __FILE__, 0x40);
    if (field_0x58) DebugFree(field_0x58, __FILE__, 0x43);
    if (field_0x50) DebugFree(field_0x50, __FILE__, 0x45);
    if (field_0x48) DebugFree(field_0x48, __FILE__, 0x47);
}

Wrecker* Wrecker::UnknownFunction530190(void* owner, UnknownWreckerCharacter* character, const char* name) {
    GameObject::UnknownVirtualSlot8(owner);
    UnknownFunction5301c0(character, name);
    return this;
}

void Wrecker::UnknownFunction5301c0(UnknownWreckerCharacter* character, const char* name) {
    field_0x34 = character;
    character->field_0x1a0->UnknownFunction4fc690(0, Vector3(0.0f, 120.0f, 0.0f));
    field_0x3c = new (__FILE__, 0x5b) UnknownWreckerGravityModel(1);
    field_0x3c->UnknownVirtualSlot8(field_0x18);
    AppendChild(field_0x3c, -1);
    field_0x40 = new (__FILE__, 0x5f) UnknownWreckerRigidBody(1);
    field_0x40->UnknownVirtualSlot8(field_0x18);
    AppendChild(field_0x40, -1);
    field_0x40->UnknownVirtualSlot28(field_0x34->field_0x1a0);
    field_0x3c->UnknownVirtualSlot27(field_0x40);
    field_0x38 = new (__FILE__, 0x68) UnknownWreckerCollisionModel(1);
    field_0x38->UnknownVirtualSlot8(field_0x18);
    field_0x38->UnknownFunction43b9e0(field_0x40, 1, name);
    AppendChild(field_0x38, -1);
    field_0x38->field_0xec = 0;
    field_0x38->field_0x6c = 0;
    field_0x38->field_0x70 = 0;
    field_0x38->field_0x108 = 0;
    field_0x38->field_0xf0 = 0.99f;
    field_0x38->field_0xf4 = 0.95f;
    field_0x38->field_0xf8 = 2000.0f;
    g_UnknownGlobal68aba4->UnknownFunction4dcf20(field_0x38, field_0x38->field_0x84);
    field_0x40->UnknownVirtualSlot29(200.0f);
    field_0x68 = field_0x40->field_0x178 * (1.0f / 12);
    field_0x40->field_0x190 = ZeroMatrix();
    field_0x40->field_0x1d0 = ZeroMatrix();
    field_0x40->field_0x25_bit0 = 0;
    field_0x40->field_0x148 = Vector3(0.0f, 0.0f, 0.0f);
    field_0x40->field_0x13c = Vector3(0.0f, 0.0f, 0.0f);
    field_0x34->field_0x1a0->field_0x18c = 1;
    field_0x34->UnknownFunction4a8b10("Fall02");
    UnknownFunction532150();
    field_0x40->UnknownVirtualSlot34(&Vector3(0.0f, 0.0f, 0.0f));
    field_0x40->UnknownVirtualSlot33(&Vector3(0.0f, 0.0f, 0.0f));
    field_0x40->UnknownVirtualSlot44();
    field_0xb8 = field_0x34->field_0x1a0->field_0x140;
    field_0x38->field_0xe8 = 1;
    field_0x40->field_0x2a4 = 1;
    field_0x3c->field_0x38 = 1;
    field_0xe8 = Vector3(0.0f, 0.0f, 0.0f);
    field_0xf4 = Vector3(0.0f, 0.0f, 0.0f);
    field_0x100 = 0;
}

void Wrecker::UnknownFunction5305b0(int value) {
    field_0x58 = (int*)DebugRealloc(field_0x58, field_0x5c * 4 + 4, __FILE__, 0x9d);
    field_0x58[field_0x5c] = value;
    field_0x5c++;
}

void Wrecker::UnknownFunction5305f0(int value) {
    field_0x50 = (int*)DebugRealloc(field_0x50, field_0x54 * 4 + 4, __FILE__, 0xa4);
    field_0x50[field_0x54] = value;
    field_0x54++;
}

int Wrecker::UnknownFunction530630(int value) {
    field_0x48 = (int*)DebugRealloc(field_0x48, field_0x4c * 4 + 4, __FILE__, 0xac);
    field_0x48[field_0x4c] = value;
    field_0x4c++;
    return field_0x4c - 1;
}

void Wrecker::UnknownFunction530680(UnknownWreckerCollisionObject* object) {
    field_0x60 = (UnknownWreckerCollisionObject**)DebugRealloc(field_0x60, field_0x64 * 4 + 4, __FILE__, 0xb4);
    field_0x60[field_0x64] = object;
    field_0x64++;
    field_0x38->UnknownFunction439410(object);
    object->UnknownFunction439410(field_0x38);
}

// Vector helpers in the d3dvec.inl style (MatrixUtil.h's Vector3 has only
// the scale and += operators).
inline Vector3 operator+(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}
inline Vector3 operator-(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}
// ---------------------------------------------------------------------------
// Out-of-line call views (docs/VC6_INLINE_BUDGET.md): the sites where retail
// calls the COMDAT copy of a header inline instead of expanding it. Declared
// only; the bindings map them to the retail copies.
// ---------------------------------------------------------------------------
struct WreckerVec3Call : Vector3 {
    WreckerVec3Call(float x_, float y_, float z_);                 // 0x00404e60
};
Vector3 WreckerSubtractValue(const Vector3& a, const Vector3& b);  // 0x00421d00
Vector3 WreckerNormalizeCall(const Vector3& v);                    // 0x005087b0
Vector3 WreckerCrossCall(const Vector3& a, const Vector3& b);      // 0x00515600

// Copies a vector member by member into a returned local: retail copies
// the old position (and forward row) through a stack temporary this way.
static inline Vector3 WreckerCopy(const Vector3& v) {
    Vector3 r;
    r.x = v.x;
    r.y = v.y;
    r.z = v.z;
    return r;
}


static inline Vector3& operator*=(Vector3& v, float s) {
    v.x *= s;
    v.y *= s;
    v.z *= s;
    return v;
}

// Per-step update of a GameObject (slot 11) through the float view. The
// named view pointer (one more local symbol in each of the two expansions)
// is what gives the first carry-over product retail's term order in
// relative._23 (a._23 * b._22 before a._33 * b._23): with the cast written
// inline that element sums the other way (4174/4178). VC6 SP3 orders equal
// candidates there by the count of referenced local symbols created before
// the product, modulo 8: two to seven more give retail's order, one more
// is worse, eight more is back to the old order (dead initialised ints
// probe the same window; the uninitialised ones are not counted).
static inline void WreckerStep(GameObject* object, float step) {
    UnknownWreckerSteppable* steppable = (UnknownWreckerSteppable*)object;
    steppable->UnknownVirtualSlot11(step);
}

// In-place inverse of a rigid transform (the InvertRigid helper of
// src/krusty2/bvh/BoundingBoxTreeQuery.h on a Matrix4).
static inline void WreckerInvertRigid(UnknownWreckerMatrix* m) {
    float swap;
    swap = m->_12; m->_12 = m->_21; m->_21 = swap;
    swap = m->_13; m->_13 = m->_31; m->_31 = swap;
    swap = m->_23; m->_23 = m->_32; m->_32 = swap;
    Vector3 t;
    t.x = -(m->_41 * m->_11 + m->_42 * m->_21 + m->_43 * m->_31);
    t.y = -(m->_41 * m->_12 + m->_42 * m->_22 + m->_43 * m->_32);
    t.z = -(m->_41 * m->_13 + m->_42 * m->_23 + m->_43 * m->_33);
    m->_41 = t.x;
    m->_42 = t.y;
    m->_43 = t.z;
}

// Rebuilds the rotation rows of a pose from its up and forward rows.
static inline void WreckerOrthonormalize(UnknownWreckerMatrix* m) {
    Vector3 up = WreckerVec3Call(m->_21, m->_22, m->_23);
    Vector3 forward = WreckerVec3Call(m->_31, m->_32, m->_33);
    forward = WreckerNormalizeCall(forward);
    Vector3 right = WreckerNormalizeCall(WreckerCrossCall(up, forward));
    up = WreckerCrossCall(forward, right);
    m->_11 = right.x;
    m->_12 = right.y;
    m->_13 = right.z;
    m->_14 = 0.0f;
    m->_21 = up.x;
    m->_22 = up.y;
    m->_23 = up.z;
    m->_24 = 0.0f;
    m->_31 = forward.x;
    m->_32 = forward.y;
    m->_33 = forward.z;
    m->_34 = 0.0f;
}

// The 4x4 product expanded inline: MatrixMultiply (0x0042a1a0,
// src/krusty2/math/Math3D.h) with both operands by value, *out = b * a in
// the row-vector convention. Inlined, VC6 copies only the operands that
// can change under it: the member pose (a, rep movsd) in the first product
// of slot 10 and the output itself (b) in the second; an unmodified local
// is read in place.
static inline void WreckerMatrixProduct(UnknownWreckerMatrix* out, UnknownWreckerMatrix a, UnknownWreckerMatrix b) {
    out->_11 = a._11 * b._11 + a._21 * b._12 + a._31 * b._13 + a._41 * b._14;
    out->_12 = a._12 * b._11 + a._22 * b._12 + a._32 * b._13 + a._42 * b._14;
    out->_13 = a._13 * b._11 + a._23 * b._12 + a._33 * b._13 + a._43 * b._14;
    out->_14 = a._14 * b._11 + a._24 * b._12 + a._34 * b._13 + a._44 * b._14;
    out->_21 = a._11 * b._21 + a._21 * b._22 + a._31 * b._23 + a._41 * b._24;
    out->_22 = a._12 * b._21 + a._22 * b._22 + a._32 * b._23 + a._42 * b._24;
    out->_23 = a._13 * b._21 + a._23 * b._22 + a._33 * b._23 + a._43 * b._24;
    out->_24 = a._14 * b._21 + a._24 * b._22 + a._34 * b._23 + a._44 * b._24;
    out->_31 = a._11 * b._31 + a._21 * b._32 + a._31 * b._33 + a._41 * b._34;
    out->_32 = a._12 * b._31 + a._22 * b._32 + a._32 * b._33 + a._42 * b._34;
    out->_33 = a._13 * b._31 + a._23 * b._32 + a._33 * b._33 + a._43 * b._34;
    out->_34 = a._14 * b._31 + a._24 * b._32 + a._34 * b._33 + a._44 * b._34;
    out->_41 = a._11 * b._41 + a._21 * b._42 + a._31 * b._43 + a._41 * b._44;
    out->_42 = a._12 * b._41 + a._22 * b._42 + a._32 * b._43 + a._42 * b._44;
    out->_43 = a._13 * b._41 + a._23 * b._42 + a._33 * b._43 + a._43 * b._44;
    out->_44 = a._14 * b._41 + a._24 * b._42 + a._34 * b._43 + a._44 * b._44;
}

// Slot 10, 0x005306e0 (4178 bytes): the per-frame update. Retail's frame
// (0x128) and order come from the rider carry-over written with inline
// helpers: the inverse's translation, the by-value product operands and the
// axis rebuild's locals live in sibling inline scopes that share slots (the
// inverse's t with the rebuild's forward); the old position is copied
// through WreckerCopy from a pointer to the member pose (the dead temporary
// at frame 0, the y/z loads through the rep movsd source); the subtract is
// called by value (its result a temporary). Retail rebuilds up as
// forward x right. See WreckerStep for the last term order.
int Wrecker::UnknownVirtualSlot10(float frameTime) {
    GameObject::UnknownVirtualSlot10(frameTime);
    if (field_0x44 == 0) {
        field_0x38->UnknownFunctionStep(frameTime);
        if (field_0x52c)
            field_0x52c = 0;
        else
            UnknownFunction532020(frameTime);
        field_0xd8 = 0;
        UnknownFunction5329e0(frameTime);
        field_0xdc = kVec3Zero;
        float touching = 0.0f;
        for (int i = 0; i < field_0x38->field_0x100; i++) {
            if (field_0x38->field_0x104[i].field_0x10) {
                field_0x108[i] = 1;
                field_0xd8 = 1;
                field_0xdc += field_0x104[i];
                touching += 1.0f;
            } else {
                field_0x108[i] = 0;
            }
        }
        if (field_0xd8)
            field_0xdc *= 1.0f / touching;
        UnknownFunction532580();
        field_0xbc = field_0xbc * (1.0f / frameTime);
        field_0xc8 = field_0xc8 * (1.0f / frameTime);
        field_0xb8->UnknownFunction4fca80(0, &field_0x6c);
    } else {
        if (field_0xd4 && !field_0x34->field_0x34) {
            field_0x34->UnknownFunction4a8b80(field_0xd4, 0.5f);
            field_0xd4 = 0;
            if (field_0x538 == 0 && field_0x530 == 0) {
                field_0x530 = 1;
            } else {
                field_0xb4 = 1;
                field_0x538 = 0;
                field_0x530 = 0;
            }
        }
        float remaining = frameTime;
        field_0xb0 = 0;
        float step = 0.02f;
        if (frameTime > 0.06f)
            step = frameTime * (1.0f / 3);
        while (remaining > 0.0f) {
            if (remaining < step)
                step = remaining;
            if (field_0x44 != 1) {
                WreckerStep(field_0x3c, step);
                WreckerStep(field_0x40, step);
            }
            if (field_0x52c)
                field_0x52c = 0;
            else
                UnknownFunction532020(frameTime);
            for (int i = 0; i < field_0x38->field_0x100; i++)
                field_0x108[i] = field_0x38->field_0x104[i].field_0x10;
            UnknownFunction5329e0(step);
            if (field_0x44 != 1) {
                Vector3 position;
                field_0x40->field_0x234->UnknownFunction4fc9a0(0, &position);
                position += field_0x10c;
                field_0x40->field_0x234->UnknownFunction4fc740(0, &position);
            }
            field_0x38->UnknownFunctionStep(step);
            remaining -= step;
        }
        if (field_0x38->field_0x58) {
            field_0xb0 = 1;
            field_0xac = 1;
            if (field_0x44 == 2) {
                field_0x44 = 3;
                field_0xd4 = field_0x58[(int)(RANDOM_UNIT() * field_0x5c)];
            }
        }
        if (field_0x44) {
            if (field_0x44 == 1) {
                UnknownWreckerRaceState* state = field_0x34->field_0x18;
                if ((float)(state->field_0xa8 - *field_0x34->field_0x1c) / state->field_0xb4 < 0.5f) {
                    UnknownFunction532490();
                    UnknownFunction532580();
                    field_0xbc = field_0xbc * (1.0f / frameTime);
                    field_0xc8 = field_0xc8 * (1.0f / frameTime);
                    field_0x40->UnknownVirtualSlot44();
                    field_0x40->field_0x154 = field_0xbc;
                    field_0x40->field_0x160 = field_0xbc;
                    field_0x40->field_0x16c = field_0x40->field_0x234->UnknownFunction4fd710(field_0xc8);
                }
            }
            if (field_0x530 && !field_0x34->field_0x34) {
                field_0xb4 = 1;
                field_0x530 = 0;
            }
            if (field_0xb4 && field_0x534) {
                UnknownFunction532580();
                field_0xbc = field_0xbc * (1.0f / frameTime);
                field_0xc8 = field_0xc8 * (1.0f / frameTime);
                field_0x534 = 0;
            }
            field_0xb8->UnknownFunction4fca80(0, &field_0x6c);
            field_0x34->UnknownVirtualSlot7(frameTime, 0, 0);
        }
    }
    if (field_0xb4) {
        // Carry the rider's root over by the motion of the wreck node since
        // the pose field_0x6c was taken, then re-orthonormalise its axes.
        UnknownWreckerMatrix pose;
        UnknownWreckerMatrix relative;
        field_0xb8->UnknownFunction4fca80(0, &pose);
        UnknownWreckerFrame* frame = &field_0x6c;
        Vector3 oldPosition = WreckerCopy(frame->position);
        WreckerInvertRigid(&pose);
        WreckerMatrixProduct(&relative, *(UnknownWreckerMatrix*)frame, pose);
        field_0x34->field_0x1a0->UnknownFunction4fca80(0, &pose);
        WreckerMatrixProduct(&pose, relative, pose);
        WreckerOrthonormalize(&pose);
        field_0x34->field_0x1a0->UnknownFunction4fb8c0(0, &pose);
        Vector3 position;
        field_0xb8->UnknownFunction4fc9a0(0, &position);
        Vector3 delta = WreckerSubtractValue(oldPosition, position);
        field_0x34->field_0x1a0->UnknownFunction4fc890(0, &delta);
        field_0xb4 = 0;
    }
    UnknownFunction532150();
    return 1;
}

// Division by a scalar through the reciprocal. Inlined, it gives retail's
// `fld st(0); fmul [delta.x]` for the first component of the step in
// 0x00531da0; the same reciprocal product written in the caller
// (`delta * (1.0f / count)`, or a `scale` local) loads delta.x first.
inline Vector3 operator/(const Vector3& v, float s) {
    return v * (1.0f / s);
}

// 0x00531da0: emits a trail of particles along a probe's movement. Retail
// needs the squared length written `(y*y + x*x) + z*z` (or
// `z*z + (x*x + y*y)`); `x*x + y*y + z*z` reuses delta.z from the FPU stack.
void Wrecker::UnknownFunction531da0(Vector3 from, Vector3 to, float frameTime) {
    Vector3 delta = to - from;
    float lengthSq = (delta.y * delta.y + delta.x * delta.x) + delta.z * delta.z;
    float length = lengthSq == 1.0f ? 1.0f : UnknownFunction460b50(lengthSq);
    float speed = length / frameTime;
    if (speed < 0.5f)
        return;
    speed *= 0.013333f;
    if (field_0x44) {
        if (speed < 1.5f)
            speed = 1.5f;
        else if (speed > 2.0f)
            speed = 2.0f;
    } else {
        if (speed < 0.25f)
            speed = 0.25f;
        else if (speed > 2.0f)
            speed = 2.0f;
    }
    float lifetime = speed * 2000.0f;
    if (lifetime < 500.0f)
        lifetime = 500.0f;
    else if (lifetime > 1000.0f)
        lifetime = 1000.0f;
    int count = (int)(length / (speed * 0.6f));
    if (!count)
        count = 1;
    float age = lifetime * 0.25f;
    Vector3 step = delta / (float)count;
    for (int i = 0; i < count; i++) {
        Vector3 position = from + step * (float)i;
        field_0x11c->UnknownFunction4baa50(age, &position, field_0x120, speed, lifetime, speed + speed, 10, 0xffffff);
        if (++field_0x120 >= 13)
            field_0x120 = 0;
    }
}

void Wrecker::UnknownFunction532020(float frameTime) {
    for (int i = 0; i < field_0x38->field_0x100; i++) {
        if (field_0x38->field_0x104[i].field_0x10) {
            Vector3 point = field_0x38->field_0x104[i].field_0x0c->UnknownFunction4fd660(field_0x38->field_0x104[i].field_0x00);
            UnknownFunction531da0(field_0x104[i], point, frameTime);
            UnknownFunction531740(field_0x104[i], point, field_0x38->field_0x104[i].field_0x14, frameTime);
        }
    }
}

void Wrecker::UnknownFunction532150() {
    Vector3 size;
    field_0x34->field_0x1a0->UnknownFunction4fe850(&field_0x40->field_0x228, &size);
    field_0x40->field_0x190(0, 0) = (size.y * size.y + size.z * size.z) * field_0x68;
    field_0x40->field_0x190(1, 1) = (size.x * size.x + size.z * size.z) * field_0x68;
    field_0x40->field_0x190(2, 2) = (size.x * size.x + size.y * size.y) * field_0x68;
    field_0x40->field_0x1d0(0, 0) = 1.0f / field_0x40->field_0x190(0, 0);
    field_0x40->field_0x1d0(1, 1) = 1.0f / field_0x40->field_0x190(1, 1);
    field_0x40->field_0x1d0(2, 2) = 1.0f / field_0x40->field_0x190(2, 2);
}

void Wrecker::UnknownFunction532220(int index) {
    field_0x38->field_0xec = 1;
    field_0x38->UnknownFunction435fe0();
    field_0x38->UnknownVirtualSlot5();
    field_0x38->field_0x6c = 1;
    field_0x38->field_0x70 = 1;
    field_0x38->field_0x108 = 1;
    field_0x34->UnknownFunction4a8b40(field_0x48[index]);
    field_0x44 = 1;
    field_0x40->field_0x25_bit0 = 0;
    field_0x40->UnknownVirtualSlot44();
    field_0x40->field_0x16c = Vector3(0.0f, 0.0f, 0.0f);
    field_0x40->field_0x154 = Vector3(0.0f, 0.0f, 0.0f);
    field_0xac = 0;
}

void Wrecker::UnknownFunction532310() {
    field_0x38->field_0xec = 1;
    field_0x38->UnknownFunction435fe0();
    field_0x38->UnknownVirtualSlot5();
    field_0x38->field_0x6c = 1;
    field_0x38->field_0x70 = 1;
    field_0x38->field_0x108 = 1;
    field_0x44 = 2;
    field_0x34->UnknownFunction4a8b80(field_0x50[(int)(RANDOM_UNIT() * field_0x54)], 0.5f);
    field_0x530 = 1;
    field_0x534 = 1;
    field_0x40->field_0x25_bit0 = 1;
    field_0x40->UnknownVirtualSlot44();
    field_0x40->field_0x154 = field_0xbc;
    field_0x40->field_0x160 = field_0xbc;
    field_0x40->field_0x16c = field_0x40->field_0x234->UnknownFunction4fd710(field_0xc8);
    for (int i = 0; i < field_0x38->field_0x100; i++)
        field_0x104[i] = Vector3(0.0f, 0.0f, 0.0f);
    field_0xac = 0;
    field_0x538 = 1;
}

void Wrecker::UnknownFunction532490() {
    field_0x44 = 2;
    field_0xd4 = field_0x50[(int)(RANDOM_UNIT() * field_0x54)];
    if (!field_0x34->field_0x34) {
        field_0x34->UnknownFunction4a8b80(field_0xd4, 0.5f);
        field_0xd4 = 0;
        if (field_0x530) {
            field_0xb4 = 1;
            field_0x538 = 0;
            field_0x530 = 0;
        } else {
            field_0x530 = 1;
        }
    }
    field_0x40->field_0x25_bit0 = 1;
    for (int i = 0; i < field_0x38->field_0x100; i++)
        field_0x104[i] = Vector3(0.0f, 0.0f, 0.0f);
    field_0x534 = 1;
}

// The parenthesised first product puts each fsubp between the load and the
// store of the previous component's copy, as retail does
// (docs/VC6_OPERAND_ORDER.md section 3).
inline Vector3 WreckerCross(const Vector3& a, const Vector3& b) {
    Vector3 r;
    r.x = (a.y * b.z) - (a.z * b.y);
    r.y = (a.z * b.x) - a.x * b.z;
    r.z = (a.x * b.y) - a.y * b.x;
    return r;
}

// 0x00532580: rebuilds the wreck pose's linear (field_0xbc) and angular
// (field_0xc8) step from the node's frame. Two references fix the leaf ages
// (docs/VC6_OPERAND_ORDER.md section 1.2): `target` for the new forward row
// in the equality tests makes its components newer than the copied old
// forward, so the cross product loads them first as retail does (511 -> 523
// of 562; through the reference only in the cross product it stays 511), and
// `up` for the old up row gives retail's schedule of the step's last store
// and the forward copy around the up-row loads (a pointer, a reference to
// field_0xbc for the subtract or the reference for up.x alone do the same).
void Wrecker::UnknownFunction532580() {
    UnknownWreckerFrame pose;
    field_0xbc = WreckerCopy(field_0x6c.position);
    field_0xb8->UnknownFunction4fca80(0, &pose);
    field_0xbc = pose.position - field_0xbc;
    Vector3 forward = WreckerCopy(field_0x6c.forward);
    const Vector3& target = pose.forward;
    // The old up row stays on the FPU stack: float locals.
    const Vector3& up = field_0x6c.up;
    float upX = up.x;
    float upY = up.y;
    float upZ = up.z;
    if (upX == pose.up.x && upY == pose.up.y && upZ == pose.up.z) {
        if (forward.x == target.x && forward.y == target.y && forward.z == target.z)
            field_0xc8 = Vector3(0.0f, 0.0f, 0.0f);
        else
            field_0xc8 = WreckerCross(forward, pose.forward);
    } else {
        field_0xc8 = WreckerCross(Vector3(upX, upY, upZ), pose.up);
    }
    field_0xc8 = UnknownFunction5015b0(field_0xc8, 1.0f);
}

void Wrecker::UnknownFunction5327c0() {
    if (field_0x44) {
        for (int i = 0; i < field_0x64; i++) {
            field_0x38->UnknownFunction439410(field_0x60[i]);
            field_0x60[i]->UnknownFunction439410(field_0x38);
        }
        field_0x44 = 0;
        field_0x40->field_0x25_bit0 = 0;
        field_0x38->field_0xec = 0;
        field_0x38->field_0x6c = 0;
        field_0x38->field_0x70 = 0;
        field_0x38->field_0x108 = 0;
        field_0x38->field_0x58 = 0;
        g_UnknownGlobal68aba4->UnknownFunction4dcf20(field_0x38, field_0x38->field_0x84);
        field_0xd4 = 0;
        field_0x52c = 1;
        field_0x34->field_0x1a0->UnknownFunction4fcc70();
        field_0x34->UnknownFunction4a8ac0(&field_0x34->field_0x190, 0);
        UnknownFunction532150();
    }
}

int Wrecker::UnknownVirtualSlot14() {
    return GameObject::UnknownVirtualSlot14();
}

int Wrecker::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    return GameObject::UnknownVirtualSlot23(event, entry);
}

void Wrecker::UnknownFunction5328b0(UnknownWreckerSkeleton* frame, Vector3 center, Vector3 halfExtents) {
    field_0xe8 = center;
    field_0xf4 = halfExtents;
    field_0x100 = frame;
}

void Wrecker::UnknownFunction532900(Vector3 offset, int bone) {
    field_0x38->UnknownFunction43c7f0(offset, bone);
    field_0x104 = (Vector3*)DebugRealloc(field_0x104, field_0x38->field_0x100 * sizeof(Vector3), __FILE__, 0x308);
    field_0x104[field_0x38->field_0x100 - 1] = Vector3(0.0f, 0.0f, 0.0f);
    field_0x108 = (int*)DebugRealloc(field_0x108, field_0x38->field_0x100 * 4, __FILE__, 0x30b);
    field_0x108[field_0x38->field_0x100 - 1] = 0;
}
