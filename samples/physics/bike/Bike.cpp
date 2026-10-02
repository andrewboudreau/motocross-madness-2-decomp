// Bike.cpp: reconstruction of the motorcycle physics layer (see Bike.h).
#include <float.h>
#include <math.h>
#include "Bike.h"

static inline float BikeMin(float a, float b) { return a < b ? a : b; }
static inline float BikeMaxF(float a, float b) { return a > b ? a : b; }

float Bike::UnknownVirtualSlot32()
{
    return field_0x15c;
}

float Bike::UnknownVirtualSlot47(float)
{
    return 1.05f - field_0x504.y * 0.25f;
}

int Bike::UnknownVirtualSlot98()
{
    if (field_0x444 && field_0x604->a_0x44)
        return 1;
    return 0;
}

int Bike::UnknownVirtualSlot42()
{
    if (field_0x444 == 0 && field_0x433 >= 0 && field_0x430 && field_0x5c4->c_0xc)
        return 1;
    return 0;
}

int Bike::UnknownVirtualSlot66()
{
    if (field_0x444 || field_0x430)
        return 1;
    return 0;
}

static inline float BikeDot(const Vec3* a, const Vec3* b)
{
    float d = a->y * b->y + a->x * b->x;
    d += a->z * b->z;
    return d;
}

float Bike::UnknownVirtualSlot75()
{
    float d = BikeDot(&field_0xa0, &field_0x5f4->w_0x230);
    if (d < 0.0f)
        d = -d;
    return d;
}

float Bike::UnknownVirtualSlot53()
{
    if (field_0x444 == 0) {
        float f = field_0x4bc * field_0x13c;
        field_0x47c->Method_00504EC0(f, field_0x42c);
        return 1.0f;
    }
    return 1.0f - Method_0x0040a520();
}

void Bike::UnknownVirtualSlot96()
{
    field_0x524[0] = 0.675f;
    field_0x63c = 1.0f;
    field_0x524[1] = 1.0f;
    field_0x524[2] = 1.0f;
    field_0x524[3] = 1.0f;
    field_0x524[4] = 1.0f;
    field_0x524[5] = 1.0f;
}

static inline float BikeAbsRef(const float& x)
{
    float r = x;
    if (r < 0.0f)
        r = -r;
    return r;
}

int Bike::UnknownVirtualSlot5(int arg)
{
    if (arg)
        return BikeAbsRef(field_0x48) > 0.2618f;
    return BikeAbsRef(field_0x48) > 0.2618f || BikeAbsRef(field_0x4c) > 1.05f;
}

void Bike::UnknownVirtualSlot49(float arg)
{
    Vehicle::UnknownVirtualSlot49(arg);
    UnknownVirtualSlot102(arg);
}

void Bike::UnknownVirtualSlot44()
{
    Vehicle::UnknownVirtualSlot44();
    field_0x5c4->c_0x1a0->Method_0x004444e0();
}

void Bike::UnknownVirtualSlot71(int arg)
{
    if (field_0x444 == 0) {
        if (field_0x108) {
            if (arg == 0 && field_0x59c == 0) {
                field_0x714 = arg;
                Vehicle::UnknownVirtualSlot71(arg);
                return;
            }
        } else if (arg) {
            field_0x720 = 1;
            field_0x718 = (field_0x48 < 0.0f) ? -1.0f : 1.0f;
            field_0x71c = field_0xd8.z;
        }
    }
    Vehicle::UnknownVirtualSlot71(arg);
}

static inline float BikeAbs(float x)
{
    if (x < 0.0f)
        x = -x;
    return x;
}

float Bike::UnknownVirtualSlot57()
{
    if (field_0x444 == 0 && field_0xb8 < field_0x724)
        return field_0x48;
    float t = field_0x47c->field_0x04;
    float a = (t < 0.0f) ? -t : t;
    float r = field_0x4ac;
    if (a < 0.5236f)
        r = r - (field_0x4ac - field_0x48) * ((0.5236f - a) * 1.9098593f);
    return r;
}

void Bike::UnknownVirtualSlot56(Vec3* a, int b, Vec3* c)
{
    Vehicle::UnknownVirtualSlot56(a, b, c);
    if (b) {
        field_0x5f4->w_0x2a4 = field_0x4b0;
        field_0x5f0->w_0x2a4 = field_0x4b0;
    }
}

float Bike::UnknownVirtualSlot59()
{
    if (field_0x658 == 10)
        return field_0x47c->field_0x08;
    float r = field_0x47c->field_0x08 -
              FastSqrt(field_0xb8 / field_0x438 * (field_0x47c->field_0x08 * field_0x47c->field_0x08));
    r = (0.0f > r) ? 0.0f : r;
    float m = field_0x47c->field_0x08;
    return (m < r) ? m : r;
}

void Bike::UnknownVirtualSlot67()
{
    int i;
    for (i = 0; i < field_0x130; i++) {
        BikeElem* e = ((BikeElem**)field_0x12c)[i];
        if (e->h_0x4 != 0)
            e->h_0xa4 = 0;
    }
    field_0x444 = 0;
    field_0x454 = 0.0f;
    field_0x5b4 = 0;
    field_0x608 = 0;
    field_0x61c = g_BikeVec3_005778a8;
}

float Bike::UnknownVirtualSlot73(const Vec3* a, const Vec3* b)
{
    float r = Vehicle::UnknownVirtualSlot73(a, b);
    float d = a->y * b->y + a->x * b->x;
    d += a->z * b->z;
    return r * d;
}

void Bike::UnknownVirtualSlot90(int* flag, float arg)
{
    if (UnknownVirtualSlot100(field_0x13c, 0, 0)) {
        *flag = 1;
        field_0x1e0 = 0.0f;
    } else if (*flag == 1 && field_0x1e0 <= 0.0001f) {
        UnknownVirtualSlot89(arg);
    }
}

Vec3 Bike::UnknownVirtualSlot16(const Vec3* v)
{
    Vec3 r;
    if (field_0x444 == 0) {
        r.x = field_0xe4.x * v->x;
        r.y = 0.0f;
        r.z = 0.0f;
    } else {
        r.x = field_0xf0.x * v->x;
        r.y = field_0xf0.y * v->y;
        r.z = field_0xf0.z * v->z;
    }
    return r;
}

void Bike::UnknownVirtualSlot1(float arg)
{
    Vehicle::UnknownVirtualSlot1(arg);
    BikeA644* p = field_0x644;
    if (p) {
        float v = field_0x704;
        p->m_0x0 = 0;
        if (v != FLT_MAX) {
            p->m_0x4 = v;
            p->m_0x8 = 1.0f;
        }
        p->m_0xc = 1.0f;
        p->m_0x10 = -1.0f;
    }
    field_0x6fc = 0;
    field_0x520 = 1;
    field_0x634 = 0;
    field_0x638 = 0;
    field_0x628 = 0;
    field_0x62c = 0;
    field_0x630 = 0;
    field_0x650 = 1;
    field_0x65c = 0.5f;
    field_0x654 = 0;
    field_0x660 = 0;
    field_0x664 = 0;
    field_0x668 = 0;
    field_0x658 = 5;
    field_0x608 = 0;
    field_0x718 = 0;
    field_0x71c = 0;
    field_0x720 = 0;
    field_0x700 = 0;
}

int Bike::UnknownVirtualSlot33(const Vec3* a, const Vec3* b, const Vec3* c,
                               const Vec3* d, int e, float f)
{
    Matrix4 tmp;
    Method_00525C60();
    int r = Vehicle::UnknownVirtualSlot33(a, b, c, d, e, f);
    int mode = field_0x604->a_0x44;
    if (mode == 0 || mode == 1) {
        d3d_field_0x1a0->GetMatrixIn(0, &tmp);
        field_0x5c4->c_0x1a0->Method_0x004fb8c0(0, &tmp);
    }
    field_0x604->a_0x38->Method_0x00435fe0();
    return r;
}

int Bike::UnknownVirtualSlot62(float arg)
{
    if (field_0x700)
        return 2;
    if (field_0x5a4 && field_0x444 == 0) {
        if (field_0x5a8 || field_0x43c != 0.0f || arg != 0.0f)
            return 1;
        if (field_0x5f4->w_0x260) {
            if (field_0x5f0->w_0x150 > -5.2f)
                return 3;
        } else if (field_0x5f0->w_0x260) {
            if (field_0x5f4->w_0x150 > -5.2f)
                return 1;
        }
    }
    return 0;
}

int Bike::UnknownVirtualSlot100(float a, int b, float c)
{
    float thr = 0.766f;
    if (((BikeA1F4*)field_0x1f4)->i_0xbe8 != 1)
        thr = 0.342f;
    field_0x444 = UnknownVirtualSlot99(a, thr, b, c);
    if (field_0x444) {
        field_0x574 = Vec3(field_0xa0.x, 0.0f, field_0xa0.z);
        field_0x433 = 0;
        int neg = field_0xac.y < 0.0f;
        field_0x45c = field_0x50;
        field_0x464 = neg;
        field_0x138 = 1;
        UnknownVirtualSlot69();
        UnknownVirtualSlot41();
        field_0x431 = 0;
    }
    return field_0x444;
}

static inline Vec3 BikeNormalized(const Vec3& v)
{
    float lenSq = v.y * v.y + v.x * v.x;
    lenSq += v.z * v.z;
    if (lenSq == 1.0f)
        return v;
    float s = FastInvSqrt(lenSq);
    return v * s;
}

Vec3* Bike::UnknownVirtualSlot55(Vec3* out, Vec3* pos)
{
    *pos = d3d_field_0x1a0->WorldToLocalPoint(field_0x5f4->w_0xcc);
    const Vec3* b = &field_0x5f4->w_0xcc;
    const Vec3* a = &field_0x5f0->w_0xcc;
    Vec3 d;
    d.x = a->x - b->x;
    d.y = a->y - b->y;
    d.z = a->z - b->z;
    *out = BikeNormalized(d);
    return out;
}

Vec3* Bike::UnknownVirtualSlot54(Vec3* out)
{
    if (field_0x4a8 != 0) {
        BikeWheel* a = field_0x5f0;
        if (a->w_0x260 != 0) {
            BikeWheel* b = field_0x5f4;
            if (b->w_0x260 != 0) {
                Vec3 s(a->w_0xe4.x + b->w_0xe4.x, a->w_0xe4.y + b->w_0xe4.y, a->w_0xe4.z + b->w_0xe4.z);
                Vec3 mid(s.x * 0.5f, s.y * 0.5f, s.z * 0.5f);
                *out = BikeNormalized(mid);
                return out;
            }
            *out = a->w_0xe4;
            return out;
        }
    }
    *out = field_0x5f4->w_0xe4;
    return out;
}

int Bike::UnknownVirtualSlot39(float arg)
{
    int r = Vehicle::UnknownVirtualSlot39(arg);
    if (r)
        return r;
    if (field_0x444) {
        if (field_0x454 != 0.0f && field_0x604->a_0xac) {
            field_0x454 -= g_Bike_0056e26c->g_0x2f0;
            if (field_0x454 > field_0x710) {
                field_0x604->a_0xac = 0;
            } else if (field_0x454 <= 0.0f) {
                if (field_0x454 == 0.0f)
                    field_0x454 = -0.1f;
                if (field_0x5f4->w_0xe4.y > 0.866 || field_0xb8 < 1.0f) {
                    UnknownVirtualSlot67();
                    return 1;
                }
                if (field_0x454 <= -2.0f) {
                    UnknownVirtualSlot67();
                    return 1;
                }
            }
        }
        if (field_0x444 == 0) {
            UnknownVirtualSlot67();
            return 1;
        }
    }
    return 0;
}

void Bike::UnknownVirtualSlot65(float arg)
{
    if (field_0x588 <= 5.0f || arg == 0.0f)
        return;
    BikeWheel* rear = field_0x5f4;
    if (rear->w_0x268 == 0 && field_0x5f0->w_0x268 == 0)
        return;
    arg = (field_0x504.y - field_0x510.y) / arg;
    if (field_0x504.y <= 0.4f || arg <= 11.0f)
        return;
    float t = field_0x588 * 0.6f;
    t = BikeMin(t, 100.0f);
    t = t * arg * 5.0f;
    t = BikeMin(t, 10000.0f);
    if (rear->w_0x268 && rear->w_0x148 > 30.0f) {
        rear->w_0x2ac->q_0x94 = t;
        return;
    }
    BikeWheel* front = field_0x5f0;
    if (front->w_0x268 && front->w_0x148 > 30.0f)
        front->w_0x2b0->q_0x94 = t;
}

void Bike::UnknownVirtualSlot8()
{
    field_0x1a0 = field_0x42c->WorldToLocalPoint(field_0x18);
    if (field_0x444 == 0 && field_0x640) {
        float steer = (field_0x640->l_0x0 - 0.5f) * 4.0f;
        field_0x1a0.x = 0.0f;
        field_0x1a0.z += steer;
        field_0x18 = field_0x42c->LocalToWorldPoint(field_0x1a0);
    }
    field_0x194 = field_0x1a0;
    field_0x700 = (field_0x4a8 > 0 && field_0x5f0->w_0x150 < -2.5f);
}

static inline void BikeScale(Vec3* v, float s)
{
    v->x *= s;
    v->y *= s;
    v->z *= s;
}

Vec3 Bike::UnknownVirtualSlot76(const Vec3* a, const Vec3* b)
{
    Vec3 v = Vehicle::UnknownVirtualSlot76(a, b);
    if (field_0xbc < 10.0f && field_0x4c > 0.8727f && field_0x5f0->w_0x260) {
        v = g_BikeVec3_005778a8;
        if (field_0xd8.x < 0.0f)
            field_0xd8.x = 0;
    } else if (field_0xd8.x < -1.0f) {
        v *= 0.2f;
    } else {
        float lean = field_0x4c;
        if (lean < 0.0f)
            lean = -lean;
        lean = BikeMin(lean, 1.0f);
        float k = -(field_0x504.y + 0.125f);
        k = (0.0f > k) ? 0.0f : k;
        BikeScale(&v, (1.4f - lean) * k * 1.6f);
    }
    return v;
}

void Bike::UnknownVirtualSlot29(int)
{
    if (field_0x6fc == 0 && field_0x430 == 0) {
        field_0xa0 = field_0x88;
        field_0xac = field_0x94;
        field_0x50 = field_0x34;
        field_0x4c = field_0x30;
        field_0x48 = field_0x2c;
        field_0x54 = field_0x38;
        field_0x58 = field_0x3c;
        field_0x60 = field_0x44;
        field_0x5c = field_0x40;
    } else {
        field_0x42c->GetAxesIn(0, &field_0xa0, &field_0xac);
        OrientationAnglesFromVectors(field_0xa0, field_0xac, &field_0x50, &field_0x4c, &field_0x48,
                            &field_0x54, &field_0x58, &field_0x60, &field_0x5c);
    }
    int mode = field_0x604->a_0x44;
    if (mode == 0 || mode == 1) {
        Matrix4 tmp;
        d3d_field_0x1a0->GetMatrixIn(0, &tmp);
        field_0x5c4->c_0x1a0->Method_0x004fb8c0(0, &tmp);
    }
}

void Bike::UnknownVirtualSlot41()
{
    if (field_0x6fc == 0 && field_0x430 == 0)
        return;
    field_0x42c->GetAxesIn(0, &field_0xa0, &field_0xac);
    d3d_field_0x1a0->SetAxesIn(0, &field_0xa0, &field_0xac, 1, 0);
    D3DIMSoultreeCharacter::Method_0x004a8b00();
    field_0x430 = 0;
    UnknownVirtualSlot34();
    OrientationAnglesFromVectors(field_0x88, field_0x94, &field_0x34, &field_0x30, &field_0x2c,
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
    field_0x6fc = 0;
    if (field_0x504.x <= 0.0f) {
        if (field_0x504.x < -0.9f)
            field_0x520 = 2;
        else if (field_0x504.x > -0.032f)
            field_0x520 = 1;
        else
            field_0x520 = 3;
    } else {
        if (field_0x504.x > 0.9f)
            field_0x520 = 4;
        else if (field_0x504.x < 0.032f)
            field_0x520 = 1;
        else
            field_0x520 = 5;
    }
    for (int i = 0; i < field_0x544; i++)
        field_0x53c[i]->Method_00515660();
}

void Bike::UnknownVirtualSlot102(float)
{
    if (field_0x444) {
        float t = -(field_0x47c->field_0x04 / field_0x47c->field_0x08);
        t = (t + 1.0f) * 0.25f;
        t += 0.25f;
        t = (t > 0.25f) ? BikeMin(t, 0.75f) : 0.25f;
        field_0x65c = t;
        D3DIMSoultreeCharacter::Method_0x004a8bf0(field_0x6b4[1], t);
        return;
    }
    if (field_0x430)
        return;
    int cnt = field_0x658;
    int step = field_0x6fc;
    int idx = field_0x650;
    int next;
    if (step) {
        next = idx + 1;
        if (next > 13)
            next = 13;
    } else if (idx > 2) {
        if (!(cnt == 10 && idx == 5)) {
            float sp = field_0x588;
            sp = (sp < 32.2f) ? ((sp > -32.2f) ? sp : -32.2f) : 32.2f;
            field_0x660 = (sp * 0.0310559f + 1.0f) * 0.5f;
        }
        next = idx + 2;
        if (idx == 10)
            next = idx;
    } else {
        next = idx + 1;
        cnt += 2;
    }
    float w = 1.0f - field_0x654;
    field_0x5c4->Method_0x004a8c50(field_0x66c[idx], field_0x66c[next], field_0x65c, w);
    D3DIMSoultreeCharacter::Method_0x004a8c50(field_0x6b4[idx], field_0x6b4[next], field_0x65c, w);
    int mode = field_0x604->a_0x44;
    if (mode == 0 || mode == 1) {
        Matrix4 tmp;
        d3d_field_0x1a0->GetMatrixIn(0, &tmp);
        field_0x5c4->c_0x1a0->Method_0x004fb8c0(0, &tmp);
    }
}

// Member-form cross product: VC6 keeps source multiplicand order here, where the free
// function form canonicalises it (same observation as Vehicle.cpp's VehV3).
static inline Vec3 BikeCrossMixed(const Vec3& a, const Vec3& n)
{
    Vec3 c;
    c.x = a.z * n.y - a.y * n.z;
    c.y = a.x * n.z - n.x * a.z;
    c.z = n.x * a.y - a.x * n.y;
    return c;
}

void Bike::UnknownVirtualSlot72(Vec3* out, VehicleWheel*)
{
    if (field_0x444)
        return;
    BikeWheel* a = field_0x5f0;
    if (!(a->w_0x260 || (field_0x5f4->w_0x260 && a->w_0x150 > -0.2f) || field_0x700)) {
        *out = field_0x5f4->w_0xe4;
    } else {
        *out = a->w_0xe4;
        BikeWheel* b = field_0x5f4;
        float weight = field_0x5f0->w_0x144;
        if (b->w_0x150 > -0.2f) {
            out->x += b->w_0xe4.x;
            out->y += b->w_0xe4.y;
            out->z += b->w_0xe4.z;
            weight += field_0x5f4->w_0x144;
            float lenSq = out->y * out->y + out->x * out->x;
            lenSq += out->z * out->z;
            if (lenSq == 0.0f) {
                *out = g_BikeVec3_005778a8;
            } else {
                float s = FastInvSqrt(lenSq);
                out->x *= s;
                out->y *= s;
                out->z *= s;
            }
            weight *= 0.5f;
            const Vec3& n = field_0x5f0->w_0x230;
            Vec3 c;
            c.x = out->z * n.y - out->y * n.z;
            c.y = out->x * n.z - n.x * out->z;
            c.z = n.x * out->y - out->x * n.y;
            Vec3 perp = BikeNormalized(c);
            float scale;
            if (field_0x700) {
                float k = field_0x60 * 0.35f;
                k = (0.001f > k) ? 0.001f : k;
                scale = 1.0f / k;
            } else {
                scale = 1.0f;
            }
            float w = (weight < 1.0f) ? weight : 1.0f;
            field_0x4b8 = UnknownVirtualSlot74(&field_0x5f0->w_0x23c, &perp, w, scale);
            field_0x43c = field_0x4b8 * field_0x140;
            return;
        }
    }
    field_0x4b8 = 0.0f;
    field_0x43c = 0.0f;
}

float Bike::UnknownVirtualSlot61(float threshold)
{
    float result = 0.0f;
    if (field_0x5a8) {
        BikeWheel* rear = field_0x5f4;
        if (rear->w_0x148 > 7.0f && rear->w_0x288 >= 0.0001f && field_0x5f0->w_0x288 < 0.5f) {
            Vec3 c;
            c.x = rear->w_0x108.z * rear->w_0x230.y - rear->w_0x108.y * rear->w_0x230.z;
            c.y = rear->w_0x108.x * rear->w_0x230.z - rear->w_0x108.z * rear->w_0x230.x;
            c.z = rear->w_0x108.y * rear->w_0x230.x - rear->w_0x108.x * rear->w_0x230.y;
            field_0x1ac = c;
            float lenSq = field_0x1ac.y * field_0x1ac.y + field_0x1ac.x * field_0x1ac.x + field_0x1ac.z * field_0x1ac.z;
            if (lenSq != 1.0f) {
                float len = (float)sqrt(lenSq);
                if (len < 0.4795f) {
                    len = (float)asin(len);
                    if (_finite(len)) {
                        float sign = (field_0x1ac.y < 0.0f) ? -1.0f : 1.0f;
                        float cap = field_0x13c * 0.8f;
                        float m = (len < cap) ? len : cap;
                        result = sign * m;
                        float mag = (result < 0.0f) ? -result : result;
                        if (threshold > mag)
                            result = 0.0f;
                    }
                }
            }
        }
    }
    bool decay = false;
    if (field_0x5a8) {
        BikeWheel* rear = field_0x5f4;
        if (rear->w_0x2b8 < 0.9f && rear->w_0x280 > 0.0f) {
            float t = field_0x4ec * 0.5f;
            float u = (t + t * rear->w_0x2bc) * field_0xe4.y * field_0x13c * rear->w_0x2bc * rear->w_0x280;
            float a = field_0x4ac;
            if (a < 0.0f)
                a = -a;
            if (a > 0.005f) {
                u *= 32.0f;
                u = BikeMin(u, 1.57f);
                float b = a * 5.0f;
                b = BikeMin(b, 1.5f);
                u = u * field_0x13c * b;
                field_0x628 = (field_0x4ac < 0.0f) ? -u : u;
                field_0x62c = field_0x1e4 * -1.333f * field_0x628;
                field_0x630 = (int)(0.75f / field_0x1e4 - 0.5f);
            } else {
                field_0x628 = 0.0f;
            }
        } else if (field_0x628 != 0.0f) {
            decay = true;
        }
    } else if (field_0x5a4 == 0 || field_0x628 == 0.0f) {
        field_0x628 = 0.0f;
    } else {
        decay = true;
    }
    if (decay) {
        float old = field_0x628;
        field_0x628 = field_0x628 + field_0x62c;
        int count = --field_0x630;
        if (old < 0.0f) {
            if (field_0x628 > 0.0f || count < 0)
                field_0x628 = 0.0f;
        } else {
            if (field_0x628 < 0.0f || count < 0)
                field_0x628 = 0.0f;
        }
    }
    if (field_0x628 != 0.0f)
        return field_0x628;
    return result;
}

// ---------------------------------------------------------------------------
// Second pass: larger dynamics functions (all provisional, tier 3 semantics).
// ---------------------------------------------------------------------------

// Length of a vector from its squared length; FastInvSqrt is 1/sqrt.
static inline float BikeLength(float lenSq)
{
    if (lenSq == 0.0f)
        return 0.0f;
    if (lenSq == 1.0f)
        return 1.0f;
    return 1.0f / FastInvSqrt(lenSq);
}

static inline float BikeMagnitude(const Vec3& v)
{
    float lenSq = BikeDot(&v, &v);
    if (lenSq == 0.0f)
        return 0.0f;
    if (lenSq == 1.0f)
        return 1.0f;
    return 1.0f / FastInvSqrt(lenSq);
}

static inline Vec3 BikeDirection(const Vec3& v)
{
    float lenSq = BikeDot(&v, &v);
    if (lenSq == 0.0f)
        return g_BikeVec3_005778a8;
    float s = FastInvSqrt(lenSq);
    return Vec3(s * v.x, s * v.y, s * v.z);
}

// Slot 97: builds the rider ragdoll ("rider.col") and attaches the 15 body
// bones by name (Pelvis, UprTorso, Head, arms, legs, feet) with local offsets.
// BikeAttachBone is __forceinline because retail expands the same sequence at each of the 15
// call sites; the original was probably a macro or an inline member (tier 3), and a plain
// call does not reproduce the bytes.
static __forceinline void BikeAttachBone(Bike* self, const char* name, float x, float y, float z)
{
    Vec3 offset;
    offset.x = x;
    offset.y = y;
    offset.z = z;
    int bone = self->field_0x604->a_0x34->r_0x1a0->Method_0x004fdae0(name);
    self->field_0x604->Method_0x00532900(offset, bone);
}

void Bike::UnknownVirtualSlot97()
{
    field_0x604 = new(__FILE__, 2023) BikeA604(1);
    if (field_0x604->Method_0x00530190(GameObject::field_0x18, field_0x5c4, "rider.col")) {
        GameObject::Method_0x00469190(field_0x604, -1);
        field_0x604->a_0x38->n_0xe4 = ((BikeA1F4*)field_0x1f4);
        field_0x604->a_0x38->n_0x64 = 0x65;
        BikeAttachBone(this, "Pelvis", 0.0f, 0.0f, -0.35f);
        BikeAttachBone(this, "UprTorso", 0.0f, 0.0f, 0.0f);
        BikeAttachBone(this, "Head", 0.0f, 0.75f, 0.0f);
        BikeAttachBone(this, "UprArmL", 0.0f, 0.0f, 0.0f);
        BikeAttachBone(this, "UprArmR", 0.0f, 0.0f, 0.0f);
        BikeAttachBone(this, "LwrArmL", 0.0f, 0.0f, 0.0f);
        BikeAttachBone(this, "HandL", 0.0f, -0.5f, 0.0f);
        BikeAttachBone(this, "LwrArmR", 0.0f, 0.0f, 0.0f);
        BikeAttachBone(this, "HandR", 0.0f, -0.5f, 0.0f);
        BikeAttachBone(this, "LwrLegL", 0.0f, 0.0f, 0.0f);
        BikeAttachBone(this, "FootL", 0.0f, -0.5f, 0.75f);
        BikeAttachBone(this, "FootL", 0.0f, -1.0f, 0.0f);
        BikeAttachBone(this, "LwrLegR", 0.0f, 0.0f, 0.0f);
        BikeAttachBone(this, "FootR", 0.0f, -0.5f, 0.75f);
        BikeAttachBone(this, "FootR", 0.0f, -1.0f, 0.0f);
        field_0x604->a_0x11c = field_0x124;
    }
}

// Slot 99: crash test.  Classifies the current impact/attitude and, if the
// rider is thrown, records the crash direction (field_0x448: 1..5) and the
// reason code (field_0x460) and returns 1.
int Bike::UnknownVirtualSlot99(float a, float b, int c, float d)
{
    if (field_0x444 != 0)
        return 0;
    BikeA604* rider = field_0x604;
    if (rider->a_0xd8) {
        field_0x1ac = rider->a_0x34->r_0x1a0->d_0x140->WorldToLocalPoint(rider->a_0xdc);
        if (field_0x1ac.y > 1.0f) {
            if (field_0x1ac.x < -1.0f) {
                field_0x448 = 5;
                field_0x460 = 11;
                return 1;
            }
            if (field_0x1ac.x > 1.0f) {
                field_0x448 = 4;
                field_0x460 = 11;
                return 1;
            }
            if (field_0x1ac.z < -0.9f)
                field_0x448 = 1;
            else
                field_0x448 = 3;
            field_0x460 = 11;
            return 1;
        }
    }
    // front wheel planted, rear wheel not: nose-over crash
    if ((field_0x108 || field_0x6fc == 0) && field_0x5f0->w_0x260 && !field_0x5f4->w_0x260
        && field_0x4b4 < b && field_0xd8.x > 0.0f) {
        field_0x448 = 1;
        field_0x460 = 7;
        return 1;
    }
    float d1 = BikeDot(&g_BikeVec3_005778c8, &field_0xa0);
    if (d1 < 0.0f)
        d1 = -d1;
    float lieLimit = field_0x708;
    if (d1 > lieLimit) {
        // bike is lying on its side
        if (field_0xa0.y > 0.0f) {
            field_0x448 = 3;
            field_0x460 = 8;
            return 1;
        }
        field_0x448 = 1;
        field_0x460 = 8;
        return 1;
    }
    float roll = field_0x48;
    if (roll < 0.0f)
        roll = -roll;
    if (roll > 1.91986f) {
        // rolled past ~110 degrees
        if (field_0x48 > 0.0f) {
            field_0x448 = 5;
            field_0x460 = 6;
            return 1;
        }
        field_0x448 = 4;
        field_0x460 = 6;
        return 1;
    }
    if (field_0x5f4->w_0x288 > 0.707f && field_0x4b0 < 0.707f) {
        if (field_0x48 < 0.0f) {
            field_0x448 = 4;
            field_0x460 = 13;
            return 1;
        }
        field_0x448 = 5;
        field_0x460 = 13;
        return 1;
    }
    if (a < field_0x1e4) {
        float m = field_0x1e4 * 0.8f;
        if (m > a)
            a = m;
    }
    if (UnknownVirtualSlot51()) {
        field_0x608 = 0;
        return 0;
    }
    if (UnknownVirtualSlot70(a)) {
        field_0x448 = 1;
        field_0x460 = 4;
        return 1;
    }
    if (c && d * field_0x24 > a * field_0x44c) {
        if (field_0xd8.z > 0.0f) {
            field_0x448 = 4;
            field_0x460 = 10;
            return 1;
        }
        field_0x448 = 5;
        field_0x460 = 10;
        return 1;
    }
    if (field_0x608) {
        // collision with another vehicle: compare headings
        Vehicle* other = field_0x608;
        float speed = field_0xbc;
        float inv = 1.0f / speed;
        Vec3 u1(inv * field_0x64.x, inv * field_0x64.y, inv * field_0x64.z);
        float otherSpeed = other->field_0xbc;
        float inv2 = 1.0f / otherSpeed;
        Vec3 u2(inv2 * other->field_0x64.x, inv2 * other->field_0x64.y, inv2 * other->field_0x64.z);
        float dot = BikeDot(&u1, &u2);
        if (dot > 0.707f) {
            field_0x448 = 1;
            field_0x460 = 2;
            return 1;
        }
        if (dot < -0.707f) {
            field_0x448 = 3;
            field_0x460 = 2;
            return 1;
        }
        if (field_0xd8.z > 0.0f) {
            field_0x448 = 4;
            field_0x460 = 2;
            return 1;
        }
        field_0x448 = 5;
        field_0x460 = 2;
        return 1;
    }
    return 0;
}

// Slot 7: distributes an applied force/direction over the tyre contacts.
// Mode field_0x1d0: weights the active contact elements by distance.
// Otherwise splits the force between front and rear wheel.
void Bike::UnknownVirtualSlot7(const Vec3* dir)
{
    field_0x570 = field_0x1cc + field_0x4a8;
    if (field_0x1d0) {
        float dist[124];
        float sum = 0.0f;
        int active = 0;
        int last = 0;
        int i;
        for (i = 0; i < field_0x130; i++) {
            BikeElem* e = ((BikeElem**)field_0x12c)[i];
            if (e->h_0xa4) {
                Vec3 dv(field_0x18.x - e->h_0x14.x, field_0x18.y - e->h_0x14.y, field_0x18.z - e->h_0x14.z);
                float len = BikeLength(BikeDot(&dv, &dv));
                dist[i] = len;
                sum += len;
                active++;
                last = i;
            } else {
                e->h_0xa0 = 0.0f;
                e->h_0x44 = g_BikeVec3_005778a8;
            }
        }
        if (active == 1) {
            ((BikeElem**)field_0x12c)[last]->h_0xa0 = 1.0f;
            ((BikeElem**)field_0x12c)[last]->h_0x44 = *dir;
            return;
        }
        for (i = 0; active > 0; i++) {
            BikeElem* e = ((BikeElem**)field_0x12c)[i];
            if (e->h_0xa4) {
                float w = 1.0f - dist[i] / sum;
                e->h_0xa0 = w;
                e->h_0x44 = Vec3(w * dir->x, w * dir->y, w * dir->z);
                active--;
            }
        }
        return;
    }
    BikeA640* p = field_0x640;
    if (field_0x430) {
        float f = field_0x13c;
        if (!(f < p->l_0x4))
            f = p->l_0x4;
        float r = f / p->l_0x4;
        p->l_0x8 = r;
        p->l_0x0 = (0.5f - p->l_0x0) * r + p->l_0x0;
        field_0x5f4->w_0x158 = field_0x5f4->w_0x294;
        field_0x5f4->w_0xfc = Vec3(field_0x5f4->w_0x158 * dir->x, field_0x5f4->w_0x158 * dir->y, field_0x5f4->w_0x158 * dir->z);
        field_0x5f0->w_0x158 = field_0x5f0->w_0x294;
        field_0x5f0->w_0xfc = Vec3(field_0x5f0->w_0x158 * dir->x, field_0x5f0->w_0x158 * dir->y, field_0x5f0->w_0x158 * dir->z);
        return;
    }
    float target = field_0x504.y * 0.25f + 0.5f;
    int mode = field_0x4a8;
    {
        float f = field_0x13c;
        if (!(f < p->l_0x4))
            f = p->l_0x4;
        float r = f / p->l_0x4;
        p->l_0x8 = r;
        p->l_0x0 = (target - p->l_0x0) * r + p->l_0x0;
    }
    if (mode > 1) {
        // weight rear/front by distance from the centre position
        Vec3 df(field_0x18.x - field_0x5f0->w_0xcc.x, field_0x18.y - field_0x5f0->w_0xcc.y, field_0x18.z - field_0x5f0->w_0xcc.z);
        float dfSq = BikeDot(&df, &df);
        float lf = BikeLength(dfSq);
        Vec3 dr(field_0x18.x - field_0x5f4->w_0xcc.x, field_0x18.y - field_0x5f4->w_0xcc.y, field_0x18.z - field_0x5f4->w_0xcc.z);
        float drSq = BikeDot(&dr, &dr);
        float lr = BikeLength(drSq);
        field_0x5f4->w_0x158 = drSq / (lr + lf);
        field_0x5f4->w_0xfc = Vec3(field_0x5f4->w_0x158 * dir->x, field_0x5f4->w_0x158 * dir->y, field_0x5f4->w_0x158 * dir->z);
        field_0x5f0->w_0x158 = 1.0f - field_0x5f4->w_0x158;
        field_0x5f0->w_0xfc = Vec3(field_0x5f0->w_0x158 * dir->x, field_0x5f0->w_0x158 * dir->y, field_0x5f0->w_0x158 * dir->z);
    } else if (mode == 1) {
        if (field_0x5f0->w_0x260) {
            field_0x5f0->w_0x158 = 1.0f;
            field_0x5f0->w_0xfc = *dir;
            field_0x5f4->w_0x158 = 0.0f;
            field_0x5f4->w_0xfc = g_BikeVec3_005778a8;
        } else {
            field_0x5f0->w_0x158 = 0.0f;
            field_0x5f0->w_0xfc = g_BikeVec3_005778a8;
            field_0x5f4->w_0x158 = 1.0f;
            field_0x5f4->w_0xfc = *dir;
        }
    } else {
        field_0x5f4->w_0x158 = 0.0f;
        field_0x5f4->w_0xfc = g_BikeVec3_005778a8;
        field_0x5f0->w_0x158 = 0.0f;
        field_0x5f0->w_0xfc = g_BikeVec3_005778a8;
    }
}

// Slot 91: per-step drive/lateral force.  Builds a force from throttle and
// wheel state, turns the velocity toward the heading, applies the force
// through the transform, then decays the accumulated lean terms.
void Bike::UnknownVirtualSlot91()
{
    float dt = field_0x13c * 0.83f;
    float f = field_0x524[1] * field_0x504.y * dt;
    Vec3 force;
    if (field_0x430)
        force = Vec3(f * 0.5f, 0.0f, 0.0f);
    else
        force = Vec3(f, field_0x504.x * field_0x13c * -0.2f, 0.0f);
    if (field_0x6fc && field_0x5f4->w_0x150 < -2.0f) {
        if (field_0x5c0 && field_0x5f4->w_0x29c > 0.0f)
            force.x += dt * field_0x5f4->w_0x29c;
        if (field_0x5bc && field_0x478 && field_0x480->field_0x00 > 0.0f && !field_0x479)
            force.x -= dt * field_0x480->field_0x00;
        float speed = field_0xbc;
        float inv = 1.0f / speed;
        Vec3* heading = &field_0x1ac;
        *heading = Vec3(inv * field_0x64.x, inv * field_0x64.y, inv * field_0x64.z);
        float d = BikeDot(&field_0xa0, heading);
        if (d > 0.55f && d < 0.984f) {
            // steer the velocity toward the lean plane
            field_0x1b8 = Vec3(field_0x1ac.z, 0.0f, -heading->x);
            float s = (field_0x65c < 0.5f) ? 1.0f : -1.0f;
            float k = s * ((1.0f - d) * 3.41296911f * field_0xbc * 0.075f) * field_0x13c;
            field_0x64 += field_0x1b8 * k;
            field_0x64 *= field_0xbc / (k + field_0xbc);
            float lenSq = BikeDot(&field_0x64, &field_0x64);
            field_0xbc = (lenSq == 1.0f) ? 1.0f : (float)sqrt(lenSq);
        }
    }
    float mag = BikeMagnitude(force);
    if (_finite(mag) && ((mag < 0.0f) ? -mag : mag) >= 0.0001f)
        d3d_field_0x1a0->RotateAboutPoint(field_0x194, BikeDirection(force), mag);
    field_0xd8.x = field_0xd8.x * 0.9f;
    field_0xd8.y = field_0xd8.y * 0.9f;
    field_0xd8.z = field_0xd8.z * 0.9f;
    // wobble: ramps a short-lived oscillation on field_0xd8.z (cnt 1..2)
    float t = field_0x714 + field_0x13c;
    int cnt = field_0x720;
    field_0x714 = t;
    if (cnt > 0 && cnt <= 2) {
        float sgn = (field_0x2c < 0.0f) ? -1.0f : 1.0f;
        float v = field_0x140 * field_0x2c;
        if (v < 0.0f)
            v = -v;
        float cube = t * t * t * 0.357792467f;
        if (cube < v)
            v = cube;
        field_0xd8.z = sgn * v + field_0x71c;
        field_0x71c = field_0x71c * 0.9f;
        if (sgn != field_0x718) {
            field_0x720 = cnt + 1;
            if (field_0x720 > 2)
                field_0x720 = 1;
            field_0x718 = sgn;
            field_0x71c = field_0xd8.z;
            field_0x714 = t * 0.5f;
        }
    }
}

// Vec3 whose (x, y, z) constructor is out of line in retail (0x00404e60,
// thiscall, ret 0xc); declared only, so VC6 emits the call instead of inlining.
struct BikeOolVec3 : public Vec3 {
    BikeOolVec3() {}
    BikeOolVec3(float x_, float y_, float z_);
};

static inline BikeOolVec3 BikeOolSub(const Vec3& a, const Vec3& b)
{
    return BikeOolVec3(a.x - b.x, a.y - b.y, a.z - b.z);
}
static inline BikeOolVec3 BikeOolAdd(const Vec3& a, const Vec3& b)
{
    return BikeOolVec3(a.x + b.x, a.y + b.y, a.z + b.z);
}
static inline BikeOolVec3 BikeOolHalf(const Vec3& v)
{
    return BikeOolVec3(v.x * 0.5f, v.y * 0.5f, v.z * 0.5f);
}

// Slot 46: focus/anchor point of the bike in local space.  Airborne or flagged
// bikes return the stored anchor (field_0x194); otherwise it is the front
// wheel position (t != 0) or a blend toward the rear wheel, relative to the
// body, mapped through the transform.
Vec3* Bike::UnknownVirtualSlot46(Vec3* out, float t)
{
    if (field_0x108 || field_0x1d0) {
        *out = field_0x194;
        return out;
    }
    if (field_0x4a8 == 2) {
        if (t != 0.0f) {
            *out = d3d_field_0x1a0->WorldToLocalDirection(field_0x1ac = field_0x5f0->w_0xcc - field_0x0c);
            return out;
        } else if (field_0x5f4->w_0x2b8 < 0.9f) {
            float s = field_0x5f4->w_0x2b8 - 0.4f;
            if (!(s > 0.0f))
                s = 0.0f;
            Vec3 d = field_0x5f4->w_0xcc - field_0x5f0->w_0xcc;
            Vec3 p = d * s + field_0x5f0->w_0xcc;
            *out = d3d_field_0x1a0->WorldToLocalDirection(field_0x1ac = p - field_0x0c);
            return out;
        } else {
            // the midpoint chain goes through the out-of-line Vec3 constructor
            Vec3 d = BikeOolSub(field_0x5f4->w_0xcc, field_0x5f0->w_0xcc);
            Vec3 half = BikeOolHalf(d);
            Vec3 mid = BikeOolAdd(half, field_0x5f0->w_0xcc);
            Vec3 rel = BikeOolSub(mid, field_0x0c);
            *out = d3d_field_0x1a0->WorldToLocalDirection(field_0x1ac = rel);
            return out;
        }
    } else {
        Vec3 d = BikeOolSub(field_0x5f4->w_0xcc, field_0x5f0->w_0xcc);
        Vec3 half = BikeOolHalf(d);
        Vec3 mid, rel;
        Vec3AddCall(&mid, &field_0x5f0->w_0xcc, &half);
        Vec3SubtractCall(&rel, &mid, &field_0x0c);
        *out = d3d_field_0x1a0->WorldToLocalDirection(field_0x1ac = rel);
        return out;
    }
}

// Moves a stored value toward zero by 1.5 * delta without crossing zero.
static inline void BikeCancelToward(float* value, float delta)
{
    if (delta > 0.0f) {
        if (*value < 0.0f) {
            float n = delta * 1.5f + *value;
            if (n > 0.0f)
                n = 0.0f;
            *value = n;
        }
    } else if (*value > 0.0f) {
        float n = delta * 1.5f + *value;
        if (!(n >= 0.0f))
            n = 0.0f;
        *value = n;
    }
}

// Slot 92: steering-torque limiter.  Derives a target steering angle from
// speed/lean terms, clamps it (rider lean, field_0x580), computes the residual
// vs the current angle (slot 57) and applies a corrective force through the
// transform, then bleeds off the accumulated lean impulses.
void Bike::UnknownVirtualSlot92(const Vec3* worldDir, const Vec3* point)
{
    float target = (field_0x47c->field_0x04 * 0.05f + field_0x43c * 0.2f) * field_0x4b4;
    field_0x4e0 = target;
    if (target < 0.0f)
        target = -target;
    if (field_0x628 != 0.0f) {
        float lim = ((field_0x628 < 0.0f) ? -1.0f : 1.0f) * 0.087f;
        float absLim = lim;
        if (absLim < 0.0f)
            absLim = -absLim;
        if (target < absLim)
            field_0x4e0 = lim;
    }
    if (target > field_0x580) {
        if (field_0x4e0 < 0.0f)
            field_0x4e0 = -field_0x580;
        else
            field_0x4e0 = field_0x580;
    }
    field_0x4e4 = UnknownVirtualSlot57() - field_0x4e0;
    if (_finite(field_0x4e4) && ((field_0x4e4 < 0.0f) ? -field_0x4e4 : field_0x4e4) >= 0.0001f) {
        int errSign = (field_0x4e4 < 0.0f) ? -1 : 1;
        int steerSign = (field_0x2c < 0.0f) ? -1 : 1;
        float lim = field_0x584;
        if (errSign == steerSign)
            lim = lim * 0.8f;
        float mag = (field_0x4e4 < 0.0f) ? -field_0x4e4 : field_0x4e4;
        mag = mag * field_0x140;
        if (mag > lim)
            field_0x4e4 = ((field_0x4e4 < 0.0f) ? -1.0f : 1.0f) * field_0x13c * lim;
        if (field_0x5a4) {
            Vec3 w = d3d_field_0x1a0->WorldToLocalDirection(*worldDir);
            float az = (w.z < 0.0f) ? -w.z : w.z;
            float ay = (w.y < 0.0f) ? -w.y : w.y;
            float ax = (w.x < 0.0f) ? -w.x : w.x;
            if (az > 0.001f || ay > 0.001f || ax > 0.001f) {
                d3d_field_0x1a0->RotateAboutPoint(w, *point, field_0x4e4);
                float k = field_0x4e4;
                float tx = k * w.x;
                float ty = k * w.y;
                float tz = k * w.z;
                field_0x1b8 = Vec3(tx * field_0x140, ty * field_0x140, tz * field_0x140);
                BikeCancelToward(&field_0xd8.x, field_0x1b8.x);
                BikeCancelToward(&field_0xd8.y, field_0x1b8.y);
                BikeCancelToward(&field_0xd8.z, field_0x1b8.z);
            }
        }
    }
}

// ---- slot 38: collision-response dispatch (tier 3 names) ----
// Bike's version of Vehicle slot 38 (0x005268d0).  `b` is the collision kind, `c` the event
// record whose +0x60 member is the other party.  The kind selects what the other party looks
// like (another vehicle, a static body, a terrain node) and how the contact is described; the
// tail then hands the contact to the impulse solver (slot 4 with a body, slot 3 without) and
// optionally notifies via slot 100.  Kind 0x67 and unknown kinds return immediately.
struct BikeContactSet {
    char pad_0x00[0xA0];
    Vec3 field_0xa0;            // contact point
    Vec3 field_0xac;            // contact normal
};
struct BikeCollisionEvent {
    char pad_0x00[0x60];
    Vehicle* field_0x60;        // other party
};
// Polymorphic body reached through the other party's +0xc4 (kind 0x3ea); slot 43 (+0xac)
// returns a velocity vector.  The placeholder slots only exist to number it.
struct BikeBodyObj {
    virtual void S0(); virtual void S1(); virtual void S2(); virtual void S3();
    virtual void S4(); virtual void S5(); virtual void S6(); virtual void S7();
    virtual void S8(); virtual void S9(); virtual void S10(); virtual void S11();
    virtual void S12(); virtual void S13(); virtual void S14(); virtual void S15();
    virtual void S16(); virtual void S17(); virtual void S18(); virtual void S19();
    virtual void S20(); virtual void S21(); virtual void S22(); virtual void S23();
    virtual void S24(); virtual void S25(); virtual void S26(); virtual void S27();
    virtual void S28(); virtual void S29(); virtual void S30(); virtual void S31();
    virtual void S32(); virtual void S33(); virtual void S34(); virtual void S35();
    virtual void S36(); virtual void S37(); virtual void S38(); virtual void S39();
    virtual void S40(); virtual void S41(); virtual void S42();
    virtual Vec3* UnknownVirtualSlot43(Vec3* out);
    char pad_0x004[0x150];
    Vec3 f154;                  // current point
    char pad_0x160[0x0c];
    Vec3 f16c;
    float f17c;
    char pad_0x180[0x50];
    float f1d0;
    char pad_0x1d4[0x10];
    float f1e4;
    char pad_0x1e8[0x0c];
    float f1f8;
    char pad_0x1fc[0x2c];
    Vec3 f228;
    SoultreeObject* f234;
    Vec3 f23c;                  // previous point
};
struct BikeKindInfo {           // reached through the other party's +0x5c
    char pad_0x00[0x24];
    float field_0x24;
};

void Bike::UnknownVirtualSlot38(int a, int b, void* c)
{
    float out;                  // slot 3/4 result, forwarded to slot 100
    Vec3 s;                     // contact mask / scale (1,1,1 for static parties)
    Vec3 velA;                  // other party's linear velocity (field_0xcc)
    Vec3 velB;                  // other party's second vector (field_0xe4/0xf0)
    Vec3 leverC;                // contact point relative to the other party
    Vec3 rel;                   // contact point relative to us
    Vec3 blend;                 // kind 0x3ea: other point interpolated by its kind info
    Vec3 tmp;
    float k = 1.0f;             // angle-based response scale (kind 100)
    float l10 = 0.0f;           // other party's field_0x24
    int hasBody = 0;
    int notifyOther = 0;
    Vehicle* other = 0;
    int ctx = 0;
    Vec3* otherVel = 0;
    Vec3* velPtr = 0;

    switch (b) {
    case 100: {
        notifyOther = 1;
        other = ((BikeCollisionEvent*)c)->field_0x60;
        l10 = other->field_0x24;
        velA = other->field_0xcc;
        velB = other->field_0x444 ? other->field_0xf0 : other->field_0xe4;
        hasBody = 1;
        otherVel = &other->field_0x64;
        ctx = *(int*)((char*)other + 0x3bc);
        Vec3 cp = ((BikeContactSet*)field_0x128)->field_0xa0;
        leverC.x = cp.x - other->field_0x0c.x;
        leverC.y = cp.y - other->field_0x0c.y;
        leverC.z = cp.z - other->field_0x0c.z;
        velPtr = &other->field_0xd8;
        if (other->field_0x0c.y - field_0x0c.y > 1.5f)
            field_0x608 = other;
        else if (field_0x0c.y - other->field_0x0c.y > 1.5f)
            ((Bike*)other)->field_0x608 = this;
        {
            Vec3 w = ((BikeContactSet*)field_0x128)->field_0xa0;
            float ang = (float)atan2(w.x - field_0x0c.x, w.z - field_0x0c.z) - field_0x50;
            if (ang < 0.0f)
                ang = -ang;
            if (ang > 3.1415927f)
                ang -= 6.2831853f;
            if (ang > 2.62f)
                k = 0.2f;
            else if (ang > 2.09f)
                k = 0.6f;
            else if (ang > 0.5236f)
                k = 0.8f;
            else
                k = 1.0f;
        }
        if (field_0x124 && (field_0x124->field_0x25 & 1)) {
            Vec3 pos = ((BikeContactSet*)field_0x128)->field_0xa0;
            ((VehicleImpactSink*)field_0x5ac)->Method_004B9DC0(pos);
            Vec3 p;
            p.x = 0.0f; p.y = 12.0f; p.z = 0.0f;
            ((VehicleImpactSink*)field_0x5ac)->field_0x74 = p;
            ((VehicleImpactSink*)field_0x5ac)->field_0x60 = 1;
        }
        break;
    }
    case 0x68:
        notifyOther = 1;
        other = ((BikeCollisionEvent*)c)->field_0x60;
        hasBody = 1;
        l10 = other->field_0x24;
        ctx = (int)other->field_0x08;
        otherVel = &other->field_0x64;
        {
            Vec3 cp = ((BikeContactSet*)field_0x128)->field_0xa0;
            leverC.x = cp.x - other->field_0x18.x;
            leverC.y = cp.y - other->field_0x18.y;
            leverC.z = cp.z - other->field_0x18.z;
        }
        velA = other->field_0xcc;
        velPtr = &other->field_0xd8;
        velB = other->field_0xe4;
        break;
    case 0x65:
        break;
    case 0:
    case 1:
    case 0x66:
    case 0x6a:
        s.x = 1.0f; s.y = 1.0f; s.z = 1.0f;
        if (field_0x124 && (field_0x124->field_0x25 & 1)) {
            Vec3 pos = ((BikeContactSet*)field_0x128)->field_0xa0;
            ((VehicleImpactSink*)field_0x5ac)->Method_004B9DC0(pos);
            Vec3 p;
            p.x = 0.0f; p.y = 12.0f; p.z = 0.0f;
            ((VehicleImpactSink*)field_0x5ac)->field_0x74 = p;
            ((VehicleImpactSink*)field_0x5ac)->field_0x60 = 1;
        }
        break;
    case 0x69: {
        char* obj = (char*)((BikeCollisionEvent*)c)->field_0x60;
        ctx = *(int*)(obj + 0x34);
        velA = g_BikeVec3_005778a8; velB = g_BikeVec3_005778a8; leverC = g_BikeVec3_005778a8;
        s.x = 1.0f; s.y = 1.0f; s.z = 1.0f;
        l10 = 0.0f;
        velPtr = 0;
        otherVel = (Vec3*)(obj + 0x40);
        hasBody = 1;
        break;
    }
    case 0x3e8: {
        Vec3 r;
        r.x = ((BikeContactSet*)field_0x128)->field_0xa0.x - field_0x18.x;
        r.y = ((BikeContactSet*)field_0x128)->field_0xa0.y - field_0x18.y;
        r.z = ((BikeContactSet*)field_0x128)->field_0xa0.z - field_0x18.z;
        UnknownVirtualSlot3(&((BikeContactSet*)field_0x128)->field_0xac, &field_0x64, &r, &s, 0x3e8, 0, &out);
        return;
    }
    case 0x3e9:
        s.x = 1.0f; s.y = 1.0f; s.z = 1.0f;
        break;
    case 0x3ea: {
        Vehicle* o = ((BikeCollisionEvent*)c)->field_0x60;
        BikeKindInfo* kind = *(BikeKindInfo**)((char*)o + 0x5c);
        BikeBodyObj* body = *(BikeBodyObj**)((char*)o + 0xc4);
        velA = *body->UnknownVirtualSlot43(&tmp);
        // other point: interpolate body->f23c toward body->f154 by kind->field_0x24
        float t = kind->field_0x24;
        float dx = body->f154.x - body->f23c.x;
        float dy = body->f154.y - body->f23c.y;
        float dz = body->f154.z - body->f23c.z;
        blend.x = dx * t + body->f23c.x;
        blend.y = dy * t + body->f23c.y;
        blend.z = dz * t + body->f23c.z;
        otherVel = &blend;
        velB.x = body->f1d0;
        velB.y = body->f1e4;
        velB.z = body->f1f8;
        l10 = body->f17c;
        velPtr = &body->f16c;
        Vec3 wp = body->f234->LocalToWorldPoint(body->f228);
        Vec3 cp = ((BikeContactSet*)field_0x128)->field_0xa0;
        leverC.x = cp.x - wp.x;
        leverC.y = cp.y - wp.y;
        leverC.z = cp.z - wp.z;
        ctx = (int)body->f234;
        hasBody = 1;
        break;
    }
    case 0x2711: {
        char* obj = (char*)((BikeCollisionEvent*)c)->field_0x60;
        ctx = *(int*)(obj + 0x1a0);
        velA = g_BikeVec3_005778a8; velB = g_BikeVec3_005778a8; leverC = g_BikeVec3_005778a8;
        s.x = 1.0f; s.y = 1.0f; s.z = 1.0f;
        l10 = 0.0f;
        velPtr = 0;
        otherVel = (Vec3*)(obj + 0x224);
        hasBody = 1;
        break;
    }
    default:
        return;
    }

    rel.x = ((BikeContactSet*)field_0x128)->field_0xa0.x - field_0x18.x;
    rel.y = ((BikeContactSet*)field_0x128)->field_0xa0.y - field_0x18.y;
    rel.z = ((BikeContactSet*)field_0x128)->field_0xa0.z - field_0x18.z;
    if (hasBody) {
        int l14 = (b == 0x2711 || b == 0x69 || b == 0x3ea) ? 0 : (int)otherVel;
        UnknownVirtualSlot4(&((BikeContactSet*)field_0x128)->field_0xac, &field_0x64, &field_0xcc, &rel, b, l10,
                            ctx, otherVel, &velA, &leverC, &velB, velPtr, l14, &out, k);
        if (other) {
            other->field_0x10a = 0;
            float len2 = otherVel->x * otherVel->x + otherVel->y * otherVel->y + otherVel->z * otherVel->z;
            other->field_0xbc = (len2 == 1.0f) ? 1.0f : (float)sqrt(len2);
            other->field_0xcc = other->field_0x08->LocalToWorldDirection(*velPtr);
        }
    } else {
        Vec3 t = field_0xcc;
        field_0x1ac.x = t.y * rel.z - t.z * rel.y;
        field_0x1ac.y = t.z * rel.x - rel.z * t.x;
        field_0x1ac.z = rel.y * t.x - t.y * rel.x;
        Vec3 p;
        p.x = field_0x1ac.x + field_0x64.x;
        p.y = field_0x64.y + field_0x1ac.y;
        p.z = field_0x64.z + field_0x1ac.z;
        UnknownVirtualSlot3(&((BikeContactSet*)field_0x128)->field_0xac, &p, &rel, &s, b, 0, &out);
        if (field_0xb8 < 0.001f && field_0xbc < 0.1f) {
            field_0x64 = g_BikeVec3_005778a8;
            field_0xd8.y = 0.0f;
            field_0xbc = 0.0f;
        }
        out = 0.0f;
    }
    if (a)
        UnknownVirtualSlot100(field_0x1e4, hasBody, out);
    if (notifyOther && other->UnknownVirtualSlot52())
        ((Bike*)other)->UnknownVirtualSlot100(((Bike*)other)->field_0x1e4, 1, out);
}

// ---- slot 89: rider pose blend (tier 3 names) ----
// Reached from slot 90 (and Vehicle's per-frame code) with the frame time.  Derives a lean
// value from the wheel state, picks a pose pair (field_0x650/field_0x658) with a blend weight
// (field_0x654/field_0x660) from the animation parameter at field_0x640, then advances a
// smoothed value (field_0x644) toward field_0x504 and maps it through a small 5-state machine
// (field_0x520) to field_0x65c.  Retail converts the pose index with an inlined fistp helper
// (round-to-nearest after the -0.5); plain (int) is used here.
static inline float BikeRange(float x, float lo, float hi)
{
    if (x <= lo)
        return lo;
    return x < hi ? x : hi;
}

int Bike::UnknownVirtualSlot89(float t)
{
    int r = UnknownVirtualSlot66();
    if (r)
        return r;

    float lean;
    if (field_0x108 || field_0x5f4->w_0x260)
        lean = -field_0x5f0->w_0x150;
    else
        lean = 0.0f;

    if (field_0x108 && lean > 2.0f && field_0x5f4->w_0x150 < -1.0f) {
        float u = (0.75f - field_0x640->l_0x0) * 3.99f;
        field_0x650 = (int)(u - 0.5f);
        int idx = field_0x650;
        field_0x650 = idx + 11;
        field_0x654 = (0.75f - field_0x640->l_0x0) * 3.99f - idx;
        if (field_0x6fc == 0) {
            BikeA644* p = field_0x644;
            float v = field_0x704;
            *(float*)&p->m_0x0 = 0.0f;
            if (v != FLT_MAX) {
                p->m_0x4 = v;
                p->m_0x8 = 1.0f;
            }
            p->m_0xc = 1.0f;
            p->m_0x10 = -1.0f;
            field_0x520 = 1;
        }
        field_0x6fc = 1;
    } else {
        UnknownVirtualSlot41();
        if (!field_0x108 && lean > field_0x64c) {
            field_0x654 = 1.0f;
            field_0x650 = 10;
            field_0x658 = 10;
        } else if (!field_0x108 && lean > field_0x648) {
            field_0x650 = 5;
            field_0x658 = 10;
            field_0x654 = (field_0x504.y + 1.0f) * 0.5f;
            field_0x660 = (lean - field_0x648) / (field_0x64c - field_0x648);
        } else if (field_0x5f4->w_0x27c > 14.0f) {
            field_0x650 = (int)((0.75f - field_0x640->l_0x0) * 3.99f - 0.5f);
            int idx = field_0x650;
            if (idx == 2) {
                if (field_0x5f4->w_0x27c > 32.0f)
                    field_0x654 = (0.75f - field_0x640->l_0x0) * 3.99f - 2.0f;
                else
                    field_0x654 = (field_0x5f4->w_0x27c - 14.0f) * 0.0555f;
            } else {
                field_0x654 = (0.75f - field_0x640->l_0x0) * 3.99f - idx;
            }
            field_0x658 = idx * 2 + 4;
            field_0x650 = field_0x658 - 1;
        } else {
            field_0x650 = (int)((0.75f - field_0x640->l_0x0) * 3.99f - 0.5f);
            int idx = field_0x650;
            field_0x658 = idx * 2 + 3;
            field_0x654 = (0.75f - field_0x640->l_0x0) * 3.99f - idx;
            field_0x660 = (14.0f - field_0x5f4->w_0x27c) * 0.0714285746f;
        }
    }

    float p = field_0x504.x;
    float x;
    if (field_0x6fc) {
        // advance the smoothed value toward p, limited by its rate m_0x4
        BikeA644* q = field_0x644;
        float d = p - *(float*)&q->m_0x0;
        if (d < 0.0f) {
            if (d <= q->m_0x10)
                d = q->m_0x10;
        } else if (d >= q->m_0xc) {
            d = q->m_0xc;
        }
        float step = t < q->m_0x4 ? t : q->m_0x4;
        q->m_0x8 = step / q->m_0x4;
        *(float*)&q->m_0x0 = d * q->m_0x8 + *(float*)&q->m_0x0;
        t = *(float*)&q->m_0x0;
        switch (field_0x520) {
        case 1:
            x = t * -0.25f;
            field_0x65c = x;
            if (x < 0.0f) {
                field_0x520 = 3;
                field_0x65c = 0.5f - x;
            } else if (x > 0.225f) {
                field_0x520 = 2;
                field_0x65c = 0.5f - x;
            }
            break;
        case 2:
            x = t * 0.25f + 0.5f;
            field_0x65c = x;
            if (x > 0.5f)
                field_0x520 = 3;
            break;
        case 3:
            x = t * 0.25f + 0.5f;
            field_0x65c = x;
            if (x < 0.5f) {
                field_0x65c = 0.5f - x;
                field_0x520 = 1;
            } else if (x > 0.725f) {
                field_0x65c = 1.5f - x;
                field_0x520 = 4;
            }
            break;
        case 4:
            x = t * -0.25f + 1.0f;
            field_0x65c = x;
            if (x > 1.0f) {
                field_0x65c = x - 1.0f;
                field_0x520 = 1;
            }
            break;
        }
    } else {
        switch (field_0x520) {
        case 1:
            if (p < 0.0f) {
                if (p < -0.9f)
                    field_0x520 = 2;
                x = p * -0.25f;
                field_0x65c = BikeRange(x, 0.0f, 0.25f);
            } else {
                if (p > 0.9f)
                    field_0x520 = 4;
                x = p * 0.25f + 0.5f;
                field_0x65c = BikeRange(x, 0.5f, 0.75f);
            }
            break;
        case 2:
            if (p > field_0x510.x) {
                field_0x520 = 3;
                x = p * 0.25f + 0.5f;
                field_0x65c = BikeRange(x, 0.25f, 0.5f);
            } else {
                x = p * -0.25f;
                field_0x65c = BikeRange(x, 0.0f, 0.25f);
            }
            break;
        case 3:
            if (p > -0.032f) {
                field_0x520 = 1;
                if (p < 0.0f) {
                    x = p * -0.25f;
                    field_0x65c = BikeRange(x, 0.0f, 0.25f);
                } else {
                    x = p * 0.25f + 0.5f;
                    field_0x65c = BikeRange(x, 0.5f, 0.75f);
                }
            } else {
                x = p * 0.25f + 0.5f;
                field_0x65c = BikeRange(x, 0.25f, 0.5f);
            }
            break;
        case 4:
            if (p < field_0x510.x) {
                field_0x520 = 5;
                x = p * -0.25f + 1.0f;
                field_0x65c = BikeRange(x, 0.75f, 1.0f);
            } else {
                x = p * 0.25f + 0.5f;
                field_0x65c = BikeRange(x, 0.5f, 0.75f);
            }
            break;
        case 5:
            if (p < 0.032f) {
                field_0x520 = 1;
                if (p < 0.0f) {
                    x = p * 0.25f + 0.5f;
                    field_0x65c = BikeRange(x, 0.5f, 0.75f);
                } else {
                    x = p * -0.25f;
                    field_0x65c = BikeRange(x, 0.0f, 0.25f);
                }
            } else {
                x = p * -0.25f + 1.0f;
                field_0x65c = BikeRange(x, 0.75f, 1.0f);
            }
            break;
        }
    }
    return 0;
}

// Out-of-line vector helpers (retail 0x40ae00 / 0x40ae30). The scale helper is
// a thiscall member of a three-float vector; the dot product is a cdecl free
// function taking two pointers.
struct BikeVec3Ops
{
    float x, y, z;
    BikeVec3Ops& ScaleBy(float factor);
};

BikeVec3Ops& BikeVec3Ops::ScaleBy(float factor)
{
    x *= factor;
    y *= factor;
    z *= factor;
    return *this;
}

float BikeDotProduct(const BikeVec3Ops* lhs, const BikeVec3Ops* rhs)
{
    float sum = lhs->y * rhs->y + lhs->x * rhs->x;
    sum += lhs->z * rhs->z;
    return sum;
}
