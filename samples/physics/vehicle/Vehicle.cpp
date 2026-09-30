// Vehicle.cpp - reconstruction of the retail Vehicle translation unit
// (__FILE__ xrefs near 0x00525e98). See Vehicle.h for the evidence/provisional notes.
#include "Vehicle.h"
#include <math.h>
#include <float.h>

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
    VehVec3* p = &field_0x94;
    VehVec3* q = &field_0x88;
    ((VehicleXform*)d3d_field_0x1a0)->Method_004FC540(0, q, p);
    VehVec3 r;
    r.x = (q->y * p->z) - (q->z * p->y);
    r.y = (p->x * q->z) - (q->x * p->z);
    r.z = (q->x * p->y) - (p->x * q->y);
    field_0x4cc = r;
}

void Vehicle::UnknownVirtualSlot35(int a, int b)
{
    VehVec3* p = &field_0x94;
    VehVec3* q = &field_0x88;
    ((VehicleXform*)d3d_field_0x1a0)->Method_004FC050(0, q, p, a, b);
    VehVec3 r;
    r.x = (q->y * p->z) - (q->z * p->y);
    r.y = (p->x * q->z) - (q->x * p->z);
    r.z = (q->x * p->y) - (p->x * q->y);
    field_0x4cc = r;
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
    return field_0x5b0 < field_0x480->field_0x00 && field_0x444 == 0 && field_0xbc < 22.0f;
}

int Vehicle::UnknownVirtualSlot25()
{
    return field_0x5b0 > field_0x480->field_0x00 && field_0x444 == 0 && field_0xbc < 22.0f;
}

void Vehicle::UnknownVirtualSlot92(const VehVec3*, const VehVec3*) {}

void Vehicle::UnknownVirtualSlot0(float dt)
{
    SoultreePhysicsCharacter::UnknownVirtualSlot0(dt);
    field_0x4dc = (field_0x4d8 * field_0x150 + dt) * 0.0310558993f;   // 1/32.2
}

int Vehicle::UnknownVirtualSlot42()
{
    return field_0x444 == 0 && field_0x433 >= 0 && field_0x430;
}

void Vehicle::UnknownVirtualSlot50(int a, float b, int c)
{
    field_0x4f0 = a;
    field_0x4f4 = b;
}

float Vehicle::UnknownVirtualSlot53()
{
    field_0x47c->Method_00504EC0(field_0x4bc * field_0x13c, field_0x42c);
    return 1.0f;
}

VehVec3* Vehicle::UnknownVirtualSlot55(VehVec3* unused, VehVec3* out)
{
    out->x = g_VehZeroVec3.x;
    out->y = g_VehZeroVec3.y;
    out->z = g_VehZeroVec3.z;
    *out = field_0xa0;
    return out;
}

float Vehicle::UnknownVirtualSlot57() { return field_0x4ac; }
float Vehicle::UnknownVirtualSlot59() { return field_0x47c->field_0x08; }
float Vehicle::UnknownVirtualSlot61(float arg) { return 0.0f; }

int Vehicle::UnknownVirtualSlot62(float arg)
{
    return field_0x5a4 && field_0x444 == 0;
}

int Vehicle::UnknownVirtualSlot89(float arg)
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
    field_0x5b0 = field_0x480->field_0x00;
}

int Vehicle::UnknownVirtualSlot82()
{
    return field_0x468->UnknownVirtualSlot3(0xe, 0, 0x3f, 0) != 0;
}

int Vehicle::UnknownVirtualSlot81()
{
    return field_0x468->UnknownVirtualSlot3(0x1d, 0, 0x3f, 0) != 0;
}

int Vehicle::UnknownVirtualSlot84(int a, int b)
{
    return field_0x468->UnknownVirtualSlot2(a, b);
}

// Provisional semantics: true when the accumulated travel (0xbc - 0xb8) is less than arg * 0x450.
int Vehicle::UnknownVirtualSlot70(float arg)
{
    return field_0xbc - field_0xb8 < arg * field_0x450;
}

// Provisional: field_0x4ec / sin(steer angle), zero when the angle is zero.
float Vehicle::UnknownVirtualSlot73(const VehVec3* a, const VehVec3* b)
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
        VehicleContact* c = ((VehicleContact**)field_0x12c)[i];
        if (c->field_0x04 != 0)
            c->field_0xa4 = 0;
    }
    field_0x444 = 0;
    field_0x454 = 0.0f;
    field_0x5b4 = 0;
}

// Reads a float through the input map's value source (written back into the argument slot).
// Tier 2: retail tests the argument as an int (callers pass a bool/flag) and then reuses
// that same stack slot as the float out-buffer.  Retyping it as float (tried) changes the
// test to an FPU compare and the callers' pushes, and loses the exact match of slots
// 77-83, so it stays int with the (float*)&arg pun.
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

void Vehicle::UnknownVirtualSlot64(float arg)
{
    if (field_0x444 == 0) {
        UnknownVirtualSlot63(arg);
        return;
    }
    field_0x504 = g_VehZeroVec3;
    field_0x474 = 0.0f;
    field_0x470 = 0.0f;
}

// Provisional semantics: a cross product returned through out (a x b).
VehVec3 Vehicle::UnknownVirtualSlot76(const VehVec3* a, const VehVec3* b)
{
    VehVec3 r;
    const VehVec3& p = *b; const VehVec3& q = *a;
    r.x = p.z * q.y - p.y * q.z;
    r.y = p.x * q.z - p.z * q.x;
    r.z = p.y * q.x - p.x * q.y;
    return r;
}

void Vehicle::UnknownVirtualSlot90(int* a, float b)
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
        field_0x4bc = -(a * b) - field_0x47c->field_0x04;
        float m = field_0x4bc;
        if (m < 0.0f) m = -m;
        if (m < 0.00001f) {
            field_0x4bc = 0.0f;
        } else {
            if (field_0x1e0 > 0.0001f)
                field_0x4bc = field_0x4bc / ((float)c * field_0x13c + field_0x1e0);
            else
                field_0x4bc = field_0x4bc / ((float)c * field_0x13c);
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
            if (w->field_0x2a8) {
                float sp = field_0xbc;
                float dt = field_0x4a4;
                w->field_0x2a8->Method_004D31B0(sp, &w->field_0x230, dt, field_0x47a, 100.0f,
                                                &w->field_0x248, &w->field_0x280);
            }
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

extern float FastInvSqrt(float v);   // retail 0x00460c00 (float in, ~1/sqrt out)

// Provisional: snapshot position (0x10c) and a horizontal heading vector (0x118 = field_0xa0
// with y forced to 0), normalised with FastInvSqrt; zero vector if degenerate.
static inline float VehLen2(const VehVec3& v) { return (v.x * v.x + v.y * v.y) + v.z * v.z; }
void Vehicle::UnknownVirtualSlot43()
{
    field_0x10c = field_0x0c;
    field_0x118 = field_0xa0;
    field_0x118.y = 0.0f;
    float len2 = VehLen2(field_0x118);
    if (len2 == 0.0f) {
        field_0x118 = g_VehZeroVec3;
        return;
    }
    float inv = FastInvSqrt(len2);
    field_0x118.x *= inv;
    field_0x118.y *= inv;
    field_0x118.z *= inv;
}

// ---- heavy dynamics ----

// Distributes the vector *arg (a force/impulse, tier 3) over the active contacts (or wheels).
// One active contact takes all of it; several share it by inverse-distance blending:
// weight = 1 - dist_i / sum(dist), so the nearest contact takes the biggest share.
// Wheel mode: field_0x430 gives every wheel its stored weight (+0x294); otherwise the same
// blending as for contacts using the wheels' contact points, or a single lead wheel.
static inline VehVec3 VehScale(const VehVec3& v, float k)
{
    VehVec3 r;
    r.x = v.x * k;
    r.y = v.y * k;
    r.z = v.z * k;
    return r;
}

static inline float VehDistance(const VehVec3& a, const VehVec3& b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    float d2 = dx * dx + dy * dy + dz * dz;
    if (d2 == 1.0f)
        return 1.0f;
    return VehFastSqrt(d2);
}

void Vehicle::UnknownVirtualSlot7(const VehVec3* arg)
{
    VehVec3 share;
    float total;
    float dist[128];
    int i;
    int active;

    field_0x570 = field_0x1cc + field_0x4a8;
    if (field_0x1d0) {
        active = 0;
        total = 0.0f;
        int last = 0;
        for (i = 0; i < field_0x130; i++) {
            VehicleContact* c = ((VehicleContact**)field_0x12c)[i];
            if (c->field_0xa4) {
                dist[i] = VehDistance(field_0x18, c->field_0x14);
                total += dist[i];
                active++;
                last = i;
            } else {
                c->field_0xa0 = 0.0f;
                c->field_0x44 = g_VehZeroVec3;
            }
        }
        if (active == 1) {
            ((VehicleContact**)field_0x12c)[last]->field_0xa0 = 1.0f;
            ((VehicleContact**)field_0x12c)[last]->field_0x44 = *arg;
            return;
        }
        for (i = 0; active > 0; i++) {
            VehicleContact* c = ((VehicleContact**)field_0x12c)[i];
            if (c->field_0xa4) {
                float w = 1.0f - dist[i] / total;
                active--;
                c->field_0xa0 = w;
                share.x = arg->x * w;
                share.y = arg->y * w;
                share.z = arg->z * w;
                c->field_0x44 = share;
            }
        }
        return;
    }
    if (field_0x430) {
        for (i = 0; i < field_0x544; i++) {
            field_0x53c[i]->field_0x158 = field_0x53c[i]->field_0x294;
            float w = field_0x53c[i]->field_0x158;
            share.x = arg->x * w;
            share.y = arg->y * w;
            share.z = arg->z * w;
            field_0x53c[i]->field_0xfc = share;
        }
        return;
    }
    if (field_0x4a8 > 1) {
        total = 0.0f;
        for (i = 0; i < field_0x544; i++) {
            VehicleWheel* w = field_0x53c[i];
            if (w->field_0x260) {
                dist[i] = VehDistance(field_0x18, w->field_0xcc);
                total += dist[i];
            } else {
                w->field_0x158 = 0.0f;
                w->field_0xfc = g_VehZeroVec3;
            }
        }
        for (i = 0; i < field_0x544; i++) {
            VehicleWheel* w = field_0x53c[i];
            if (w->field_0x260) {
                float k = 1.0f - dist[i] / total;
                w->field_0x158 = k;
                share.x = arg->x * k;
                share.y = arg->y * k;
                share.z = arg->z * k;
                w->field_0xfc = share;
            }
        }
    } else if (field_0x4a8 == 1) {
        for (i = 0; i < field_0x544; i++) {
            field_0x53c[i]->field_0x158 = 0.0f;
            field_0x53c[i]->field_0xfc = g_VehZeroVec3;
        }
        field_0x548->field_0x158 = 1.0f;
        field_0x548->field_0xfc = *arg;
    } else {
        for (i = 0; i < field_0x544; i++) {
            field_0x53c[i]->field_0x158 = 0.0f;
            field_0x53c[i]->field_0xfc = g_VehZeroVec3;
        }
    }
}

static inline float VehAbs(float x)
{
    if (x < 0.0f)
        x = -x;
    return x;
}

// Tilt/lean angle of the body about the given axis (tier 3 names). The reference normal
// (arg c, or the average wheel contact normal) is projected perpendicular to the axis and
// normalised; the angle is asin(|axis-perp-normal x right-vector-side|) signed by the
// handedness, stored in field_0x4ac; its cosine goes to field_0x4b0 when b is set.
static inline float VehDotI(const VehVec3* a, const VehVec3* b)
{
    return a->y * b->y + a->x * b->x + a->z * b->z;
}
static inline VehVec3 VehNormalizedD(const VehVec3& v)
{
    float len2 = VehDot(&v, &v);
    if (len2 == 1.0f)
        return v;
    float inv = VehFastInvSqrt(len2);
    VehVec3 r;
    r.x = inv * v.x;
    r.y = inv * v.y;
    r.z = inv * v.z;
    return r;
}
void Vehicle::UnknownVirtualSlot56(VehVec3* a, int b, VehVec3* c)
{
    if (field_0x544 == 0) {
        field_0x4ac = field_0x48;
        field_0x4b0 = field_0x3c;
        return;
    }
    VehVec3 n;
    VehVec3 t;
    if (!c)
        n = *Method_00528400(&t);
    else
        n = *c;
    VehVec3 proj = *VehScaleVec(&t, a, VehDot(&n, a));
    VehVec3* perp = &field_0x1b8;
    *perp = *VehSubVec(&t, &n, &proj);
    if (!(perp->x == 0.0f && perp->y == 0.0f && perp->z == 0.0f) && perp) {
        *perp = *VehNormalize(&t, perp);
        field_0x1ac = ((VehV3&)field_0x4cc).Cross(*a);
        VehVec3 side = VehNormalizedD(field_0x1ac);
        field_0x1ac = ((VehV3&)field_0x1b8).Cross(side);
        float sign = -1.0f;
        if (VehDotI(&field_0x1ac, a) < 0.0f)
            sign = 1.0f;
        float len2 = VehDotI(&field_0x1ac, &field_0x1ac);
        float len = 1.0f;
        if (len2 != 1.0f) {
            len = (float)sqrt(len2);
            if (!(len < 1.0f))
                len = 1.0f;
        }
        double angle = asin(len * sign);
        field_0x4ac = (float)angle;
        if (b)
            field_0x4b0 = (float)cos(angle);
        return;
    }
    field_0x4ac = field_0x48;
    field_0x4b0 = field_0x3c;
}

// Snaps the body pose to the wheel contact state (tier 3): with one wheel only the height
// (field_0x0c.y) follows that wheel; with several the body basis is rebuilt from the wheel
// normal (or the supplied direction a), the highest wheel sets the height, and a two-wheel
// bike additionally re-aims the frame by the lean angle (field_0x4ac). Finally the 7-float
// orientation block is rebuilt from the basis vectors and the previous copy is refreshed.
static inline float VehAbsT(float x) { return x < 0.0f ? -x : x; }

void Vehicle::UnknownVirtualSlot58(VehVec3* a, int b)
{
    if (field_0x544 < 1)
        return;
    Method_00525C60();
    if (field_0x544 == 1) {
        if (b == 0) {
            float y = field_0x548->field_0xd8.y;
            field_0x0c.y = y;
            ((VehicleXform*)d3d_field_0x1a0)->Method_004FC630(field_0x0c.x, y, field_0x0c.z);
        }
        field_0x548->field_0x150 = 0.0f;
    } else {
        VehicleWheel* top = field_0x53c[0];
        VehVec3 dir;
        VehVec3 t;
        if (b) {
            dir = *a;
        } else {
            VehicleWheel* second = field_0x53c[1];
            for (int i = 1; i < field_0x544; i++) {
                VehicleWheel* w = field_0x53c[i];
                if (w->field_0xd8.y > top->field_0xd8.y) {
                    second = top;
                    top = w;
                } else if (w->field_0xd8.y > second->field_0xd8.y) {
                    second = w;
                }
            }
            dir = *Method_00528400(&t);
        }
        ((VehicleXform*)d3d_field_0x1a0)->Method_004FC050(0, &field_0x88, &dir, 1, 0);
        UnknownVirtualSlot34();
        ((VehicleXform*)d3d_field_0x1a0)->Method_004FC9A0(0, &field_0x0c);
        if (b == 0) {
            float y = top->field_0x228 - top->field_0xcc.y + top->field_0xd8.y;
            field_0x0c.y = y;
            ((VehicleXform*)d3d_field_0x1a0)->Method_004FC630(field_0x0c.x, y, field_0x0c.z);
        }
        if (field_0x544 == 2) {
            VehVec3 v0;
            VehVec3 v2;
            t = *UnknownVirtualSlot55(&v0, &field_0x194);
            UnknownVirtualSlot56(&t, 0, &dir);
            v0 = *((VehicleXform*)d3d_field_0x1a0)->Method_004FD710(&v2, &t);
            float lean = field_0x4ac;
            if (VehAbsT(lean) > 0.001f) {
                if (VehAbsT(v0.z) > 0.001f || VehAbsT(v0.y) > 0.001f || VehAbsT(v0.x) > 0.001f) {
                    ((VehicleXform*)d3d_field_0x1a0)->Method_004FD1F0(field_0x194, v0, lean);
                    UnknownVirtualSlot34();
                    ((VehicleXform*)d3d_field_0x1a0)->Method_004FC9A0(0, &field_0x0c);
                }
            }
        }
    }
    field_0x47c->field_0x00->Method_004FC050(0, &field_0x88, &field_0x94, 0, 1);
    field_0x108 = false;
    VehBasisToBlock(field_0x88, field_0x94, &field_0x34, &field_0x30, &field_0x2c,
                    &field_0x38, &field_0x3c, &field_0x44, &field_0x40);
    field_0xa0 = field_0x88;
    field_0xac = field_0x94;
    field_0x50 = field_0x34;
    field_0x4c = field_0x30;
    field_0x48 = field_0x2c;
    field_0x54 = field_0x38;
    field_0x58 = field_0x3c;
    field_0x60 = field_0x44;
    field_0x5c = field_0x40;
}

static inline VehVec3 VehNormalized(const VehVec3& v)
{
    float len2 = v.y * v.y + v.x * v.x + v.z * v.z;
    if (len2 == 1.0f)
        return v;
    float inv = VehFastInvSqrt(len2);
    VehVec3 r;
    r.x = inv * v.x;
    r.y = inv * v.y;
    r.z = inv * v.z;
    return r;
}

// Tier 3 reading: an impulse-style velocity deflection.  k is an effective-mass term along dir,
// r = -m^2/k * dt is the impulse magnitude, and the deflected velocity is v' = v + min(1.5c,1) * r * dir,
// rescaled to keep the old speed (a pure turn); the returned value is the signed angle between
// v and v' (acos of the normalised dot product).
// Deflects the velocity (field_0x64) by an impulse applied at `point` along `dir` (tier 3).
// k = slot73(effective mass/stiffness) * d; the change is dir * (-(m*m)/k * dt) scaled by
// min(1.5*c, 1); the new velocity is renormalised to the old speed (field_0xbc) and the signed
// deflection angle is returned (0 when the change is negligible or not finite).
float Vehicle::UnknownVirtualSlot74(VehVec3* point, VehVec3* dir, float c, float d)
{
    float k = UnknownVirtualSlot73(point, dir) * d;
    field_0x4e8 = k;
    if (VehAbs(k) <= 0.0f)
        return 0.0f;
    float m = field_0x434;
    float r = (m * m) / -k * field_0x13c;
    VehVec3 t;
    t.x = r * dir->x;
    t.y = r * dir->y;
    t.z = r * dir->z;
    float s = c * 1.5f;
    if (!(s < 1.0f))
        s = 1.0f;
    VehVec3 u;
    u.x = s * t.x;
    u.y = t.y * s;
    u.z = t.z * s;
    VehVec3 sumv;
    sumv.x = u.x + field_0x64.x;
    sumv.y = u.y + field_0x64.y;
    sumv.z = u.z + field_0x64.z;
    field_0x1ac = sumv;
    VehVec3* nv = &field_0x1ac;
    float len2 = nv->y * nv->y + nv->x * nv->x + nv->z * nv->z;
    float len;
    if (len2 == 1.0f)
        len = 1.0f;
    else
        len = (float)sqrt(len2);
    if (VehAbs(len) < 0.01f)
        return 0.0f;
    float sign = -1.0f;
    if (!(k < 0.0f))
        sign = 1.0f;
    float dot = nv->y * field_0x64.y + nv->x * field_0x64.x + nv->z * field_0x64.z;
    sign = VehAbs((float)acos(dot / (len * field_0xbc))) * sign;
    if (!_finite(sign))
        return 0.0f;
    float scale = field_0xbc / len;
    field_0x64.x = scale * nv->x;
    field_0x64.y = scale * nv->y;
    field_0x64.z = scale * nv->z;
    if (field_0x440)
        return sign;
    return -sign;
}

// Tier 3 reading: accumulates per-wheel ground normals and suspension loads into one
// support normal and a lean/steer response (via slot 74).  Wheels are averaged, so more than
// one contact normalises the sum.
// Sums the contact normals (and, for wheels with field_0x1c0 set, their +0x230 vectors) of the
// wheels and pushes the vehicle along the resulting cross direction (tier 3). *out receives the
// summed normal (normalised when more than one contact); field_0x4b8/0x43c receive the result
// of slot 74 (deflection angle and angle*dt) using the last flagged wheel's point (+0x23c).
void Vehicle::UnknownVirtualSlot72(VehVec3* out, VehicleWheel* wheel)
{
    if (field_0x444 != 0)
        return;
    VehVec3 sum = g_VehZeroVec3;
    *out = g_VehZeroVec3;
    float load = 0.0f;
    int mixed = 0;
    int flagged = 0;
    int plain = 0;
    for (int i = 0; i < field_0x544; i++) {
        VehicleWheel* w = field_0x53c[i];
        if (w->field_0x260 || wheel) {
            if (w->field_0x1c0) {
                wheel = w;
                flagged++;
                sum.x += w->field_0x230.x;
                sum.y += w->field_0x230.y;
                sum.z += w->field_0x230.z;
                out->x += w->field_0xe4.x;
                out->y += w->field_0xe4.y;
                out->z += w->field_0xe4.z;
                load += w->field_0x144;
                if (plain > 0)
                    mixed = 1;
            } else {
                plain++;
                out->x += w->field_0xe4.x;
                out->y += w->field_0xe4.y;
                out->z += w->field_0xe4.z;
                load += field_0x53c[i]->field_0x144;
                if (flagged > 0)
                    mixed = 1;
            }
        }
    }
    if (field_0x4a8 > 1) {
        float len2 = out->y * out->y + out->x * out->x + out->z * out->z;
        if (len2 == 0.0f) {
            *out = g_VehZeroVec3;
        } else {
            float inv = VehFastInvSqrt(len2);
            out->x = inv * out->x;
            out->y = inv * out->y;
            out->z = inv * out->z;
        }
        load = load / field_0x4a8;
    }
    if (mixed == 0 && wheel == 0) {
        field_0x4b8 = 0.0f;
        field_0x43c = 0.0f;
        if (field_0x4a8 < 1)
            *out = field_0x53c[0]->field_0xe4;
        return;
    }
    VehVec3 c = ((VehV3&)sum).Cross(*out);
    VehVec3 axis = VehNormalized(c);
    float mag = load;
    if (!(mag < 1.0f))
        mag = 1.0f;
    field_0x4b8 = UnknownVirtualSlot74(&wheel->field_0x23c, &axis, mag, 1.0f);
    field_0x43c = field_0x4b8 * field_0x140;
}

// ---- impact handlers (slots 18..20) ----
// Commit an impact to a sink: latch the current vector into the previous one, mark it dirty.
static inline void VehCommitImpact(VehicleImpactEvent* ev, VehicleImpactSink* sink)
{
    if (ev->field_0x24)
        sink->field_0x50 = sink->field_0x44;
    sink->field_0x60 = 1;
    ev->field_0x24 = 0;
}

// Posts a one-shot impact for the first wheel (aux-driven first, then any) or contact whose
// surface material is flagged in the material table and which has not yet reported one.
void Vehicle::UnknownVirtualSlot18(SoultreeAttachment* arg)
{
    VehicleImpactEvent* ev = (VehicleImpactEvent*)arg;
    int i;
    for (i = 0; i < field_0x544; i++) {
        VehicleWheel* w = field_0x53c[i];
        if (w->field_0x2a8 && w->field_0x260 && !w->field_0x160 &&
            (!field_0x1f0 || ((VehicleMaterialSet*)field_0x1f0)->field_0xa4[0x400 + w->field_0x174])) {
            w->field_0x160 = 1;
            float v = w->field_0x2bc * 20.0f;
            if (!(v > 1.0f))
                v = 1.0f;
            ev->field_0x04->Method_004B8D90(w->field_0xd8, v);
            VehCommitImpact(ev, ev->field_0x04);
            return;
        }
    }
    for (i = 0; i < field_0x544; i++) {
        VehicleWheel* w = field_0x53c[i];
        if (w->field_0x260 && !w->field_0x160 &&
            (!field_0x1f0 || ((VehicleMaterialSet*)field_0x1f0)->field_0xa4[0x400 + w->field_0x174])) {
            w->field_0x160 = 1;
            ev->field_0x04->Method_004B8D90(w->field_0xd8, 0.0f);
            VehCommitImpact(ev, ev->field_0x04);
            return;
        }
    }
    for (i = 0; i < field_0x130; i++) {
        VehicleContact* c = ((VehicleContact**)field_0x12c)[i];
        if (c->field_0xa4 && !c->field_0xa8 &&
            (!field_0x1f0 || ((VehicleMaterialSet*)field_0x1f0)->field_0xa4[0x400 + c->field_0xbc])) {
            c->field_0xa8 = 1;
            ev->field_0x04->Method_004B8D90(c->field_0x20, 0.0f);
            VehCommitImpact(ev, ev->field_0x04);
            return;
        }
    }
}

// Slide/scrape impact: like slot 18 but posts a clamped scrape vector (wheel normal-ish frame
// scaled by the wheel's 0x290 gain and dt) into the sink's +0x74 vector.
void Vehicle::UnknownVirtualSlot19(SoultreeAttachment* arg)
{
    VehicleImpactEvent* ev = (VehicleImpactEvent*)arg;
    VehicleWheel* w = 0;
    int i;
    for (i = 0; i < field_0x544; i++) {
        w = field_0x53c[i];
        if (w->field_0x2a8 && w->field_0x260 && !w->field_0x164 &&
            (!field_0x1f0 || ((VehicleMaterialSet*)field_0x1f0)->field_0xa4[0x408 + w->field_0x174]))
            break;
    }
    if (i < field_0x544) {
        if (!(w->field_0x2b8 < 0.95f))
            return;
        w->field_0x164 = 1;
        ev->field_0x08->Method_004B9DC0(w->field_0xd8);
        VehCommitImpact(ev, ev->field_0x08);
        field_0x1b8.x = (field_0x4b8 < 0.0f ? -1.0f : 1.0f) * w->field_0x280.z;
        field_0x1b8.y = 0.0f;
        field_0x1b8.z = -w->field_0x28c;
        VehVec3 t;
        field_0x1ac = *((VehicleXform*)d3d_field_0x1a0)->Method_004FD5C0(&t, &field_0x1b8);
        field_0x1ac.y = w->field_0x2bc * 3.0f;
        float s = w->field_0x290 * field_0x140;
        field_0x1ac.x *= s;
        field_0x1ac.y *= s;
        field_0x1ac.z *= s;
        if (!(field_0x1ac.x < 15.0f))
            field_0x1ac.x = 15.0f;
        if (!(field_0x1ac.y < 18.0f))
            field_0x1ac.y = 18.0f;
        if (!(field_0x1ac.z < 15.0f))
            field_0x1ac.z = 15.0f;
        ev->field_0x08->field_0x74 = field_0x1ac;
        return;
    }
    for (i = 0; i < field_0x544; i++) {
        w = field_0x53c[i];
        if (w->field_0x260 && !w->field_0x164 &&
            (!field_0x1f0 || ((VehicleMaterialSet*)field_0x1f0)->field_0xa4[0x408 + w->field_0x174]))
            break;
    }
    if (i < field_0x544) {
        w->field_0x164 = 1;
        if (w->field_0x280.z > 0.2f) {
            VehCommitImpact(ev, ev->field_0x08);
        }
    }
}

// Second scrape channel: like slot 19 (sink at event+0x0c, no per-material table gate) but posts a clamped scrape vector (wheel normal-ish frame
// scaled by the wheel's 0x290 gain and dt) into the sink's +0x74 vector.
void Vehicle::UnknownVirtualSlot20(SoultreeAttachment* arg)
{
    VehicleImpactEvent* ev = (VehicleImpactEvent*)arg;
    VehicleWheel* w = 0;
    int i;
    for (i = 0; i < field_0x544; i++) {
        w = field_0x53c[i];
        if (w->field_0x2a8 && w->field_0x260 && !w->field_0x168)
            break;
    }
    if (i < field_0x544) {
        if (!(w->field_0x2b8 < 0.95f))
            return;
        w->field_0x168 = 1;
        ev->field_0x0c->Method_004B9DC0(w->field_0xd8);
        VehCommitImpact(ev, ev->field_0x0c);
        field_0x1b8.x = (field_0x4b8 < 0.0f ? -1.0f : 1.0f) * w->field_0x280.z;
        field_0x1b8.y = 0.0f;
        field_0x1b8.z = -w->field_0x28c;
        VehVec3 t;
        field_0x1ac = *((VehicleXform*)d3d_field_0x1a0)->Method_004FD5C0(&t, &field_0x1b8);
        field_0x1ac.y = w->field_0x2bc * 3.0f;
        float s = w->field_0x290 * field_0x140;
        field_0x1ac.x *= s;
        field_0x1ac.y *= s;
        field_0x1ac.z *= s;
        if (!(field_0x1ac.x < 15.0f))
            field_0x1ac.x = 15.0f;
        if (!(field_0x1ac.y < 18.0f))
            field_0x1ac.y = 18.0f;
        if (!(field_0x1ac.z < 15.0f))
            field_0x1ac.z = 15.0f;
        ev->field_0x0c->field_0x74 = field_0x1ac;
        return;
    }
    for (i = 0; i < field_0x544; i++) {
        w = field_0x53c[i];
        if (w->field_0x260 && !w->field_0x168)
            break;
    }
    if (i < field_0x544) {
        w->field_0x168 = 1;
        if (w->field_0x280.z > 0.2f) {
            VehCommitImpact(ev, ev->field_0x0c);
        }
    }
}

static inline VehVec3 VehCrossA(const VehVec3& a, const VehVec3& b)
{
    VehVec3 r;
    r.x = b.z * a.y - b.y * a.z;
    r.y = b.x * a.z - b.z * a.x;
    r.z = b.y * a.x - b.x * a.y;
    return r;
}
static inline VehVec3 VehCrossB(const VehVec3& a, const VehVec3& b)
{
    VehVec3 r;
    r.x = a.y * b.z - a.z * b.y;
    r.y = a.z * b.x - a.x * b.z;
    r.z = a.x * b.y - a.y * b.x;
    return r;
}
// ---- slot 4 / slot 46 ----
// Slot 4: builds two lever-arm offsets (a1 + a2 x a3, a7 + a8 x a9), each scaled by a14, and hands
// them with the frame data to helper 0x5004a0. Afterwards it caches |*a1| in field_0xbc and
// field_0xcc = the frame transform of field_0xd8 (tier 3).
void Vehicle::UnknownVirtualSlot4(const VehVec3* a0, VehVec3* a1, const VehVec3* a2,
                                  const VehVec3* a3, int a4, float a5, int a6, const VehVec3* a7,
                                  const VehVec3* a8, const VehVec3* a9, VehVec3* a10, VehVec3* a11,
                                  int a12, float* a13, float a14)
{
    field_0x1ac = VehCrossA(*a2, *a3);
    VehVec3 p;
    p.x = (a1->x + field_0x1ac.x) * a14;
    p.y = (field_0x1ac.y + a1->y) * a14;
    p.z = (field_0x1ac.z + a1->z) * a14;
    field_0x1ac = VehCrossB(*a8, *a9);
    VehVec3 q;
    q.x = (field_0x1ac.x + a7->x) * a14;
    q.y = (field_0x1ac.y + a7->y) * a14;
    q.z = (field_0x1ac.z + a7->z) * a14;
    VehSlot4Helper(field_0x14c, a0, field_0x24, ((VehicleXform*)d3d_field_0x1a0), &p, a3,
                   field_0x444 ? &field_0xf0 : &field_0xe4, &field_0xd8, a1, a5, a6,
                   &q, a9, a10, a11, a12, a13);
    float len2 = a1->x * a1->x + a1->y * a1->y + a1->z * a1->z;
    field_0xbc = (len2 == 1.0f) ? 1.0f : (float)sqrt(len2);
    VehVec3 t;
    field_0xcc = *((VehicleXform*)d3d_field_0x1a0)->Method_004FD5C0(&t, &field_0xd8);
}

static inline VehVec3 VehOffset(const VehVec3& a, const VehVec3& b)
{
    VehVec3 r;
    r.x = a.x - b.x;
    r.y = a.y - b.y;
    r.z = a.z - b.z;
    return r;
}

// Slot 46: the point the follow/aim logic looks at, relative to the vehicle position: toward the
// lead wheel (or the midpoint of two wheels when only the aux one is not yet fully settled),
// rotated by the frame transform; falls back to field_0x194 when disabled or > 2 wheels.
VehVec3* Vehicle::UnknownVirtualSlot46(VehVec3* out, float arg)
{
    VehVec3* src = &field_0x194;
    VehVec3 t;
    if (!field_0x108 && !field_0x1d0 && field_0x4a8 <= 2) {
        if (field_0x4a8 == 2) {
            VehicleWheel* a = field_0x54c;
            VehicleWheel* b = field_0x548;
            if (!a->field_0x2a8) {
                a = field_0x548;
                if (!a->field_0x2a8) {
                    *out = *src;
                    return out;
                }
                b = field_0x54c;
            } else {
                b = field_0x548;
            }
            // a has the aux object, b is the other wheel
            if (a->field_0x2b8 < 0.9f) {
                field_0x1ac = VehOffset(b->field_0xcc, field_0x0c);
            } else {
                VehVec3 d = VehOffset(a->field_0xcc, b->field_0xcc);
                VehVec3 mid;
                mid.x = d.x * 0.5f;
                mid.y = d.y * 0.5f;
                mid.z = d.z * 0.5f;
                mid.x += b->field_0xcc.x;
                mid.y += b->field_0xcc.y;
                mid.z += b->field_0xcc.z;
                field_0x1ac = VehOffset(mid, field_0x0c);
            }
        } else {
            VehVec3* p = &field_0x548->field_0xcc;
            field_0x1ac = VehOffset(*p, field_0x0c);
        }
        src = ((VehicleXform*)d3d_field_0x1a0)->Method_004FD710(&t, &field_0x1ac);
    }
    *out = *src;
    return out;
}

// ---- slot 49: per-frame vehicle step ----
// Slot 49 is the per-frame step of the vehicle (tier 3 phase names, from the slot calls):
//   A. setup      - slot 9 fixed-timestep accumulator gives the substep count; slots 30/64/60/65
//                   refresh per-frame inputs; an initial contact query (VehContactsA) when more
//                   than one substep is pending.
//   B. per substep, in order:
//        1. input/state  - decay the speed-state timer, poll slots 80-82 (input flags), and
//                          the field_0x4f4 countdown (slot 50 clears it);
//        2. ground query - contact query, then slots 8/6/7 (position sample, drag, weighting
//                          of contact reactions) and slots 71/72/86 (support normal, wheel loads);
//        3. dynamics     - slots 13/14 (force/torque accumulation), Methods 00527A20/005293E0/
//                          00529450/00529C20 (wheel forces), VehSmooth (exponential smoothers);
//        4. integrate    - slots 26/48/85/87-95 (integration, lean/pose), write the position
//                          back into the transform node, slot 28 advances the substep counter;
//        5. bookkeeping  - slot 34, basis block refresh, previous velocity, slot 29.
//   C. epilogue   - slot 21 attachments refresh, final smoothers.
// Runs the substep loop of the vehicle simulation (tier 3 names). Each pass: pulls the substep
// count, updates the smoothed speed/lean state, gathers contact reactions, integrates the
// frame transform from the wheels/contacts, and refreshes the cached basis block.
// Exponential smoother (tier 3 reading): x += (target - x) * a with a = min(dt, tau) / tau,
// where tau is field_0x04, field_0x08 is the blend factor a, and field_0x00 the smoothed value.
static inline void VehSmooth(VehicleSmoother* s, float dt, float target)
{
    float step = dt;
    if (!(step < s->field_0x04))
        step = s->field_0x04;
    s->field_0x08 = step / s->field_0x04;
    s->field_0x00 = (target - s->field_0x00) * s->field_0x08 + s->field_0x00;
}

void Vehicle::UnknownVirtualSlot49(float frame)
{
    int steps;
    VehVec3 up;          // frame reference vector (from field_0x188)
    VehVec3 tmp;
    float speed;
    int hit;
    int res;

    field_0x20d = 0;
    UnknownVirtualSlot9(frame, &steps);
    UnknownVirtualSlot30();
    field_0x510 = field_0x504;
    UnknownVirtualSlot64(frame);
    UnknownVirtualSlot60(field_0x504.x, UnknownVirtualSlot59(), steps);
    UnknownVirtualSlot65(frame);
    field_0x1d0 = 0;
    hit = UnknownVirtualSlot39(frame);
    if (!hit && !field_0x444 && steps != 1) {
        VehContactsA(UnknownVirtualSlot5(0), &field_0x1cc, field_0x130, ((VehicleContact**)field_0x12c), field_0x1f4,
                     &field_0x18, field_0x1c4, field_0x160);
        field_0x1d0 = field_0x1cc > 0;
    }
    field_0x484 = 0;
    while (steps > 0) {
        VehicleSpeedState* ss = field_0x480;
        if (ss->field_0x0c != 0.0f) {
            ss->field_0x0c -= field_0x13c;
            if (ss->field_0x0c < 0.0f)
                ss->field_0x0c = 0.0f;
        }
        VehVec3 zero = g_VehZeroVec3;
        if (field_0x444 == 0) {
            if (Method_00529280())
                field_0x480->field_0x08 = 1;
        } else if (field_0x154 != field_0x150) {
            if (UnknownVirtualSlot10())
                UnknownVirtualSlot0(0.0f);
        }
        field_0x47a = UnknownVirtualSlot82() != 0;
        field_0x479 = field_0x444 == 0 && (field_0x47a || UnknownVirtualSlot81());
        field_0x478 = field_0x444 == 0 && UnknownVirtualSlot80();
        field_0x480->Method_004D2F50(field_0x13c, field_0x478, field_0x479);
        field_0x480->Method_004D3030(field_0x478, field_0xbc);
        if (hit) {
            int dummy = 0;
            UnknownVirtualSlot11(hit, &up, &zero, &tmp, &dummy);
            UnknownVirtualSlot33(&up, &tmp, &field_0x94, &zero, dummy, UnknownVirtualSlot32());
            steps = 0;
            field_0x1e0 = 0.0f;
            break;
        }
        field_0xb8 = field_0xbc;
        if (field_0xbc < 0.001f || !_finite(field_0xbc)) {
            field_0xb8 = 0.0f;
            field_0x64 = g_VehZeroVec3;
            field_0xbc = 0.0f;
        }
        ((VehicleXform*)field_0x218)->Method_004FC9A0(0, &field_0x18);
        float t53 = UnknownVirtualSlot53();
        Method_00528EB0();
        if (field_0x4f4 > 0.0f) {
            field_0x4f4 -= field_0x13c;
            if (field_0x4f4 <= 0.0f)
                UnknownVirtualSlot50(0, 0, 0);
        }
        field_0x5a8 = field_0x4a8 == field_0x544;
        field_0x5a4 = field_0x4a8 != 0;
        speed = field_0xb8;
        up = field_0x188;
        VehVec3 a, b;
        VehVec3 basis = *UnknownVirtualSlot55(&a, &b);
        UnknownVirtualSlot56(&basis, 1, 0);
        res = UnknownVirtualSlot5(0);
        res = VehContactsA(field_0x444 || (steps == 1 && res), &field_0x1cc, field_0x130, ((VehicleContact**)field_0x12c),
                           field_0x1f4, &field_0x18, field_0x1c4, field_0x160);
        if (res)
            field_0x1d0 = field_0x1cc > 0;
        Method_00529A20();
        UnknownVirtualSlot8();
        UnknownVirtualSlot6(&up, &speed);
        UnknownVirtualSlot7(&up);
        field_0x4b4 = UnknownVirtualSlot75();
        bool settled = !field_0x5a4 && !field_0x1d0;
        UnknownVirtualSlot71(settled);
        field_0x108 = settled;
        if (settled)
            field_0x59c = 0;
        VehVec3 lift;
        if (field_0x570 > 0) {
            if (field_0x5a4) {
                UnknownVirtualSlot72(&lift, 0);
                UnknownVirtualSlot86();
            } else {
                field_0x5a0 = 0;
                field_0x4b8 = 0.0f;
                field_0x43c = 0.0f;
            }
            if (field_0x1d0) {
                if (!res)
                    VehContactsC(field_0x130, ((VehicleContact**)field_0x12c));
                UnknownVirtualSlot31();
            }
        } else {
            field_0x5a0 = 0;
            field_0x4b8 = 0.0f;
            field_0x43c = 0.0f;
        }
        if (field_0x5a0 || (field_0x1d0 && (!field_0x5a4 || field_0x444))) {
            VehVec3 scale;
            if (field_0x444) {
                scale.x = 1.0f;
                scale.y = 1.0f;
                scale.z = 1.0f;
            } else {
                scale.x = 0.3f;
                scale.y = 0.1f;
                scale.z = 0.3f;
            }
            VehVec3 o1, o2, o3;
            VehContactsB(field_0x130, ((VehicleContact**)field_0x12c), &scale, &field_0xcc, &field_0x64, &field_0x18,
                         &field_0x0c, &o3, &o2, &o1);
            UnknownVirtualSlot3(&o2, &o1, &o3, &scale, 0x67, 0, &tmp.x);
            if (field_0xb8 * 1.3f < field_0xbc && field_0xbc > 5.0f) {
                float r = field_0xb8 / field_0xbc;
                field_0x64.x *= r;
                field_0x64.y *= r;
                field_0x64.z *= r;
                field_0xbc = field_0xb8;
            }
            if (field_0xb8 < 0.001f && field_0xbc < 0.1f) {
                field_0x64 = g_VehZeroVec3;
                field_0xd8.y = 0.0f;
                field_0xbc = 0.0f;
            }
        }
        float aq = field_0x4b8;
        if (aq < 0.0f)
            aq = -aq;
        VehVec3 zeroB = g_VehZeroVec3;
        UnknownVirtualSlot13(&up, &zeroB, t53);
        if (field_0x5a4) {
            Method_00527A20(&speed, &zero, &up);
            if (field_0x444 == 0 || !field_0x1d0) {
                Method_005293E0(&speed);
                Method_00529450(&up, &zeroB);
            }
            Method_00529C20(&up, &zeroB, t53);
        }
        UnknownVirtualSlot14(&up, &zeroB, &zero);
        VehSmooth(field_0x58c, field_0x13c, (field_0x64.y - field_0x7c.y) * field_0x140);
        field_0x588 = field_0x58c->field_0x00;
        float k = UnknownVirtualSlot61(aq);
        VehVec3 aim = *UnknownVirtualSlot46(&tmp, k);
        int mode = UnknownVirtualSlot62(k);
        UnknownVirtualSlot26();
        UnknownVirtualSlot48();
        UnknownVirtualSlot85();
        ((VehicleXform*)d3d_field_0x1a0)->Method_004FC660(&field_0x0c);
        UnknownVirtualSlot95();
        if (field_0x444) {
            UnknownVirtualSlot87();
        } else {
            if (field_0x108) {
                UnknownVirtualSlot91();
            } else if (mode) {
                if (mode == 1 || mode == 2) {
                    bool doit = true;
                    if (aq < 0.0001f) {
                        if (VehAbs(k) > 0.0001f) {
                            field_0x1ac = *UnknownVirtualSlot54(&tmp);
                            field_0x4c0 = *((VehicleXform*)d3d_field_0x1a0)->Method_004FD710(&a, &field_0x1ac);
                        } else {
                            doit = false;
                        }
                    } else {
                        field_0x4c0 = *((VehicleXform*)d3d_field_0x1a0)->Method_004FD710(&b, &lift);
                    }
                    if (doit) {
                        if (k != 0.0f && aq < 0.003f)
                            ((VehicleXform*)d3d_field_0x1a0)->Method_004FD1F0(aim, field_0x4c0, k);
                        else
                            ((VehicleXform*)d3d_field_0x1a0)->Method_004FD1F0(aim, field_0x4c0, k + field_0x4b8);
                        field_0xd8.x *= 0.85f;
                        field_0xd8.y *= 0.85f;
                        field_0xd8.z *= 0.85f;
                    }
                }
                if (mode == 1 || mode == 3)
                    UnknownVirtualSlot92(&up, &tmp);
            }
            UnknownVirtualSlot90(&steps, frame);
        }
        ((VehicleXform*)d3d_field_0x1a0)->Method_004FC970(&field_0x0c);
        ((VehicleXform*)field_0x218)->Method_004FC9A0(0, &field_0x18);
        field_0xcc = *((VehicleXform*)d3d_field_0x1a0)->Method_004FD5C0(&tmp, &field_0xd8);
        steps = UnknownVirtualSlot28(steps) - 1;
        field_0x1e8 = field_0x13c;
        if (steps == 0 && field_0x1e0 > 0.0001f) {
            steps = 1;
            field_0x13c = field_0x1e0;
            field_0x140 = 1.0f / field_0x13c;
            field_0x1e0 = 0.0f;
        }
        UnknownVirtualSlot34();
        VehBasisToBlock(field_0x88, field_0x94, &field_0x34, &field_0x30, &field_0x2c,
                        &field_0x38, &field_0x3c, &field_0x44, &field_0x40);
        field_0x7c = field_0x64;
        field_0x5b4 = field_0x444;
        UnknownVirtualSlot29(steps == 0);
    }
    UnknownVirtualSlot21();
    field_0x1ac = *((VehicleXform*)d3d_field_0x1a0)->Method_004FD710(&tmp, &field_0x64);
    VehSmooth(field_0x598, field_0x144, (field_0x1ac.z - field_0x594) / field_0x144);
    field_0x590 = field_0x598->field_0x00;
    field_0x594 = field_0x1ac.z;
}

// ---- slot 38: collision response dispatch (tier 3 names) ----
// Called with an event code and the other party of a collision. Builds the relative contact
// geometry (contact point minus our position, other body's velocity/normal) and hands it to the
// impulse solver in slot 4 (other body present) or slot 3 (static/no body). Afterwards kills
// residual creep velocity and, if requested, runs the post-collision update.
static inline VehVec3 VehOnes()
{
    VehVec3 r;
    r.x = 1.0f;
    r.y = 1.0f;
    r.z = 1.0f;
    return r;
}
struct VehicleContactSet {
    char pad_0x00[0xA0];
    VehVec3 field_0xa0;            // contact point
    VehVec3 field_0xac;            // contact normal
};
struct VehicleCollisionEvent {     // 'c' argument (provisional)
    char pad_0x00[0x60];
    Vehicle* field_0x60;           // other body
};

void Vehicle::UnknownVirtualSlot38(int a, int b, void* c)
{
    float l10;                     // the other body's field_0x24 (float) or 0
    int l14;
    VehVec3 s;                     // scale/mask vector (1,1,1)
    VehVec3 p;                     // lever vector
    VehVec3 v30;
    VehVec3 v3c;
    VehVec3 v48;
    int ctx;
    int hasBody;
    Vehicle* other = 0;
    VehVec3* otherVel = 0;
    VehVec3 rel;

    s = VehOnes();
    switch (b) {
    case 0x66:
    case 0x6a:
        s = VehOnes();
        hasBody = 0;
        if (field_0x124 && (field_0x124->field_0x25 & 1)) {
            VehVec3 pos = ((VehicleContactSet*)field_0x128)->field_0xa0;
            ((VehicleImpactSink*)field_0x5ac)->Method_004B9DC0(pos);
            p.x = 0.0f; p.y = 12.0f; p.z = 0.0f;
            ((VehicleImpactSink*)field_0x5ac)->field_0x74 = p;
            ((VehicleImpactSink*)field_0x5ac)->field_0x60 = 1;
        }
        break;
    case 0x3e9:
        s = VehOnes();
        hasBody = 0;
        break;
    case 0:
    case 1:
        hasBody = 1;
        other = ((VehicleCollisionEvent*)c)->field_0x60;
        l10 = other->field_0x24;
        otherVel = &other->field_0x64;
        ctx = (int)other->d3d_field_0x1a0;
        v3c = VehOffset(((VehicleContactSet*)field_0x128)->field_0xa0, other->field_0x18);
        v48 = other->field_0xcc;
        c = &other->field_0xd8;
        v30 = other->field_0xe4;
        break;
    case 0x69:
        hasBody = 1;
        ctx = *(int*)((char*)((VehicleCollisionEvent*)c)->field_0x60 + 0x34);
        v30 = g_VehZeroVec3; v3c = g_VehZeroVec3; v48 = g_VehZeroVec3;
        l10 = 0;
        p.x = 1.0f; p.y = 1.0f; p.z = 1.0f;
        s = p;
        otherVel = (VehVec3*)(ctx + 0x40);
        c = 0;
        break;
    case 0x2711:
        hasBody = 1;
        ctx = *(int*)((char*)((VehicleCollisionEvent*)c)->field_0x60 + 0x1a0);
        v30 = g_VehZeroVec3; v3c = g_VehZeroVec3; v48 = g_VehZeroVec3;
        l10 = 0;
        p.x = 1.0f; p.y = 1.0f; p.z = 1.0f;
        s = p;
        otherVel = (VehVec3*)(ctx + 0x224);
        c = 0;
        break;
    default:
        return;
    }

    rel = VehOffset(((VehicleContactSet*)field_0x128)->field_0xa0, field_0x18);
    if (hasBody) {
        l14 = (b == 0x2711 || b == 0x69) ? 0 : (int)otherVel;
        UnknownVirtualSlot4(&((VehicleContactSet*)field_0x128)->field_0xac, &field_0x64, &field_0xcc, &rel, b, l10, ctx,
                            otherVel, (VehVec3*)c, &v48, &v3c, &v30, l14, &l10, 1.0f);
        if (other) {
            *((char*)other + 0x10a) = 0;
            float len2 = otherVel->x * otherVel->x + otherVel->y * otherVel->y + otherVel->z * otherVel->z;
            field_0xbc = (len2 == 1.0f) ? 1.0f : (float)sqrt(len2);
            field_0xcc = *((VehicleXform*)other->d3d_field_0x1a0)->Method_004FD5C0(&v48, (VehVec3*)c);
        }
    } else {
        VehVec3 t;
        t.x = field_0xcc.x * s.x; t.y = field_0xcc.y * s.y; t.z = field_0xcc.z * s.z;
        field_0x1ac.x = rel.z * t.y - rel.y * t.z;
        field_0x1ac.y = rel.x * t.z - rel.z * t.x;
        field_0x1ac.z = rel.y * t.x - rel.x * t.y;
        p.x = field_0x64.x + field_0x1ac.x;
        p.y = field_0x64.y + field_0x1ac.y;
        p.z = field_0x64.z + field_0x1ac.z;
        UnknownVirtualSlot3(&((VehicleContactSet*)field_0x128)->field_0xac, &p, &rel, &s, b, 0, &l10);
    }
    if (field_0xb8 < 0.001f && field_0xbc < 0.1f) {
        field_0x64 = g_VehZeroVec3;
        field_0xd8.y = 0.0f;
        field_0xbc = 0;
    }
    if (a)
        Method_00526830();
}
