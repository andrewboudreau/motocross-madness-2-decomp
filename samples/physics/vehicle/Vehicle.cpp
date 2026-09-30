// Vehicle.cpp - reconstruction of the retail Vehicle translation unit
// (__FILE__ xrefs near 0x00525e98). See Vehicle.h for the evidence/provisional notes.
#include "Vehicle.h"
#include <math.h>

// ---- small vector helpers (stand-ins for common/Math3D.h) ----

// Cross product as a member of a Vec3 view: VC6 honours source multiplicand order here
// (free-function form canonicalises it).
struct VehV3 : VehVec3
{
    VehVec3 Cross(const VehVec3& b) const
    {
        VehVec3 r;
        r.x = y * b.z - z * b.y;
        r.y = z * b.x - x * b.z;
        r.z = x * b.y - b.x * y;
        return r;
    }
};

void Vehicle::UnknownVirtualSlot34()
{
    VehV3& a = (VehV3&)field_0x88;
    field_0x3bc->Method_004FC540(0, &a, &field_0x94);
    field_0x4cc = a.Cross(field_0x94);
}

void Vehicle::UnknownVirtualSlot35(int a, int b)
{
    VehV3& v = (VehV3&)field_0x88;
    field_0x3bc->Method_004FC050(0, &v, &field_0x94, b, a);
    field_0x4cc = v.Cross(field_0x94);
}

float Vehicle::UnknownVirtualSlot45() { return 3.0f; }
int Vehicle::UnknownVirtualSlot51() { return field_0x4f0; }
int Vehicle::UnknownVirtualSlot52() { return field_0x444 == 0; }

int Vehicle::UnknownVirtualSlot68(int* out)
{
    *out = 15;
    return 1;
}

void Vehicle::UnknownVirtualSlot69() { field_0x454 = field_0x458; }

int Vehicle::UnknownVirtualSlot23()
{
    return !field_0x108 && field_0x434 >= 0.2f;
}

int Vehicle::UnknownVirtualSlot24()
{
    return field_0x5b0 < *field_0x480 && field_0x444 == 0 && field_0xbc < 22.0f;
}

int Vehicle::UnknownVirtualSlot25()
{
    return field_0x5b0 > *field_0x480 && field_0x444 == 0 && field_0xbc < 22.0f;
}

void Vehicle::UnknownVirtualSlot92(int, int) {}

void Vehicle::UnknownVirtualSlot0(float dt)
{
    SoultreePhysicsCharacter::UnknownVirtualSlot0(dt);
    field_0x4dc = (field_0x4d8 * field_0x150 + dt) * 0.0310558993f;   // 1/32.2
}

int Vehicle::UnknownVirtualSlot42()
{
    return field_0x444 == 0 && field_0x433 >= 0 && field_0x430;
}

void Vehicle::UnknownVirtualSlot50(int a, int b, int c)
{
    field_0x4f0 = a;
    field_0x4f4 = b;
}

float Vehicle::UnknownVirtualSlot53()
{
    field_0x47c->Method_00504EC0(field_0x4bc * field_0x13c, field_0x42c);
    return 1.0f;
}

void Vehicle::UnknownVirtualSlot55(int arg, VehVec3* out)
{
    *out = g_VehZeroVec3;
    *out = field_0xa0;
}

float Vehicle::UnknownVirtualSlot57() { return field_0x4ac; }
float Vehicle::UnknownVirtualSlot59() { return field_0x47c->field_0x08; }
float Vehicle::UnknownVirtualSlot61(int arg) { return 0.0f; }

int Vehicle::UnknownVirtualSlot62(int arg)
{
    return field_0x5a4 && field_0x444 == 0;
}

int Vehicle::UnknownVirtualSlot89(int arg)
{
    return UnknownVirtualSlot66();
}

void Vehicle::UnknownVirtualSlot93() { field_0x51c = 0.5f; }

void Vehicle::UnknownVirtualSlot96()
{
    field_0x524[0] = 1.0f;
    field_0x524[1] = 1.0f;
    field_0x524[2] = 1.0f;
    field_0x524[3] = 1.0f;
    field_0x524[4] = 1.0f;
    field_0x524[5] = 1.0f;
}

void Vehicle::UnknownVirtualSlot21()
{
    SoultreePhysicsCharacter::UnknownVirtualSlot21();
    field_0x5b0 = *field_0x480;
}

int Vehicle::UnknownVirtualSlot82()
{
    return field_0x468->UnknownVirtualSlot3(0xe, 0, 0x3f, 0) != 0;
}

int Vehicle::UnknownVirtualSlot81()
{
    return field_0x468->UnknownVirtualSlot3(0x1d, 0, 0x3f, 0) != 0;
}

void Vehicle::UnknownVirtualSlot84(int a, int b)
{
    field_0x468->UnknownVirtualSlot2(a, b);
}

// Provisional semantics: true when the accumulated travel (0xbc - 0xb8) is less than arg * 0x450.
int Vehicle::UnknownVirtualSlot70(float arg)
{
    return field_0xbc - field_0xb8 < arg * field_0x450;
}

// Provisional: field_0x4ec / sin(steer angle), zero when the angle is zero.
float Vehicle::UnknownVirtualSlot73(int a, int b)
{
    VehicleSteerState* s = field_0x47c;
    float m = s->field_0x04;
    if (m < 0.0f)
        m = -m;
    if (m == 0.0f)
        return 0.0f;
    return field_0x4ec / sinf(s->field_0x04);
}

// Clears the per-contact flag at +0xa4 for contacts whose +4 field is set, then resets state.
void Vehicle::UnknownVirtualSlot67()
{
    for (int i = 0; i < field_0x130; i++) {
        VehicleContact* c = field_0x12c[i];
        if (c->field_0x04 != 0)
            c->field_0xa4 = 0;
    }
    field_0x444 = 0;
    field_0x454 = 0;
    field_0x5b4 = 0;
}

// Reads a float through the input map's value source (written back into the argument slot).
int Vehicle::UnknownVirtualSlot77(int arg)
{
    if (arg && field_0x468->field_0x0c
        && field_0x468->field_0x0c->UnknownVirtualSlot3(0, (float*)&arg)
        && *(float*)&arg != -1.0f)
        return 1;
    return 0;
}

int Vehicle::UnknownVirtualSlot78()
{
    if (field_0x468->UnknownVirtualSlot3(3, 2, 0x3f, 0) && !UnknownVirtualSlot77(1))
        return 1;
    return field_0x468->UnknownVirtualSlot3(0x1f, 0, 0x3f, 0) != 0;
}

int Vehicle::UnknownVirtualSlot79()
{
    if (field_0x468->UnknownVirtualSlot3(4, 2, 0x3f, 0) && !UnknownVirtualSlot77(1))
        return 1;
    return field_0x468->UnknownVirtualSlot3(0x2d, 0, 0x3f, 0) != 0;
}

int Vehicle::UnknownVirtualSlot64(int arg)
{
    if (field_0x444 == 0)
        return UnknownVirtualSlot63(arg);
    field_0x504 = g_VehZeroVec3;
    field_0x474 = 0.0f;
    field_0x470 = 0.0f;
    return 0;
}

// Provisional semantics: a cross product returned through out (a x b).
void Vehicle::UnknownVirtualSlot76(VehVec3* out, VehVec3* a, VehVec3* b)
{
    *out = ((VehV3*)a)->Cross(*b);
}

void Vehicle::UnknownVirtualSlot90(int* a, int b)
{
    if (Method_00526830()) {
        *a = 1;
        field_0x1e0 = 0.0f;
        return;
    }
    if (*a == 1 && field_0x1e0 <= 0.0001f)
        UnknownVirtualSlot89(b);
}

// Provisional: steering-error style term, t = -(a*b) - angle; result is t / (c*mass + damping)
// unless |t| is tiny or the vehicle is in state 0x444.
void Vehicle::UnknownVirtualSlot60(float a, float b, int c)
{
    if (field_0x444 == 0) {
        float t = -(a * b) - field_0x47c->field_0x04;
        field_0x4bc = t;
        float m = t < 0.0f ? -t : t;
        if (m < 0.00001f) {
            field_0x4bc = 0.0f;
        } else {
            if (field_0x1e0 > 0.0001f)
                field_0x4bc = t / ((float)c * field_0x13c + field_0x1e0);
            else
                field_0x4bc = t / ((float)c * field_0x13c);
        }
    } else {
        field_0x4bc = 0.0f;
    }
}

void Vehicle::UnknownVirtualSlot85()
{
    float v = field_0x440 ? field_0x13c : -field_0x13c;
    int flag = field_0x4a8 == 0;
    for (int i = 0; i < field_0x544; i++)
        field_0x53c[i]->Method_005143D0(field_0x13c, v, flag, field_0x5a8, field_0x444, field_0x24);
}

void Vehicle::UnknownVirtualSlot86()
{
    for (int i = 0; i < field_0x544; i++) {
        VehicleWheel* w = field_0x53c[i];
        if (w->field_0x260) {
            if (w->field_0x2a8)
                w->field_0x2a8->Method_004D31B0(field_0xbc, &w->field_0x230, field_0x4a4,
                                                field_0x47a, 100.0f, &w->field_0x248, &w->field_0x280);
            w->Method_00513F90(this);
        }
    }
}

// Vec3 operators as a view over VehVec3 (stand-in for common/Math3D.h).
struct VehVecOps : VehVec3
{
    VehVec3 operator*(float s) const { VehVec3 r; r.x = x * s; r.y = y * s; r.z = z * s; return r; }
    VehVec3 operator-(const VehVec3& o) const { VehVec3 r; r.x = x - o.x; r.y = y - o.y; r.z = z - o.z; return r; }
};

// Provisional: position += dt * (3*v - v_prev) / 2   (2-step Adams-Bashforth style advance:
// field_0x64 = velocity, field_0x7c = previous velocity, field_0x0c = position).
void Vehicle::UnknownVirtualSlot48()
{
    VehVec3 a = ((VehVecOps&)field_0x64) * 3.0f;
    float dt = field_0x13c;
    VehVec3 d = ((VehVecOps&)a) - field_0x7c;
    VehVec3 e = ((VehVecOps&)d) * 0.5f;
    VehVec3 f = ((VehVecOps&)e) * dt;
    field_0x0c.x += f.x;
    field_0x0c.y += f.y;
    field_0x0c.z += f.z;
}

void Vehicle::UnknownVirtualSlot71(int arg)
{
    if (field_0x444) {
        field_0x59c = 0;
        if (field_0x108 && !arg)
            field_0x484 = 1;
    } else if (field_0x108 && !arg && !field_0x59c) {
        field_0x59c = 1;
        field_0x484 = 1;
    }
    if (!field_0x108 && arg) {
        field_0x494 = field_0x64;
        field_0x488 = field_0x0c;
    }
}
