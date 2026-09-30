// Bike.cpp: reconstruction of the motorcycle physics layer (see Bike.h).
#include <float.h>
#include <math.h>
#include "Bike.h"

#define BIKE_MIN(a, b) ((a) < (b) ? (a) : (b))

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

static inline float BikeDot(const BikeVec3* a, const BikeVec3* b)
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

void Bike::UnknownVirtualSlot56(BikeVec3* a, int b, BikeVec3* c)
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
              BikeMath_0x00460b50(field_0xb8 / field_0x438 * (field_0x47c->field_0x08 * field_0x47c->field_0x08));
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

float Bike::UnknownVirtualSlot73(const BikeVec3* a, const BikeVec3* b)
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

BikeVec3 Bike::UnknownVirtualSlot16(const BikeVec3* v)
{
    BikeVec3 r;
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

int Bike::UnknownVirtualSlot33(const BikeVec3* a, const BikeVec3* b, const BikeVec3* c,
                               const BikeVec3* d, int e, float f)
{
    float tmp[16];
    Method_00525C60();
    int r = Vehicle::UnknownVirtualSlot33(a, b, c, d, e, f);
    int mode = field_0x604->a_0x44;
    if (mode == 0 || mode == 1) {
        ((BikeXform*)d3d_field_0x1a0)->Method_0x004fca80(0, tmp);
        field_0x5c4->c_0x1a0->Method_0x004fb8c0(0, tmp);
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
        field_0x574 = BikeVec3(field_0xa0.x, 0.0f, field_0xa0.z);
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

static inline BikeVec3 BikeNormalized(const BikeVec3& v)
{
    float lenSq = v.y * v.y + v.x * v.x;
    lenSq += v.z * v.z;
    if (lenSq == 1.0f)
        return v;
    float s = BikeMath_0x00460c00(lenSq);
    BikeVec3 r;
    r.x = v.x;
    r.y = v.y;
    r.z = v.z;
    r.x *= s;
    r.y *= s;
    r.z *= s;
    return r;
}

BikeVec3* Bike::UnknownVirtualSlot55(BikeVec3* out, BikeVec3* pos)
{
    BikeVec3 tmp;
    *pos = *((BikeXform*)d3d_field_0x1a0)->Method_0x004fd7f0(&tmp, &field_0x5f4->w_0xcc);
    const BikeVec3* b = &field_0x5f4->w_0xcc;
    const BikeVec3* a = &field_0x5f0->w_0xcc;
    BikeVec3 d;
    d.x = a->x - b->x;
    d.y = a->y - b->y;
    d.z = a->z - b->z;
    *out = BikeNormalized(d);
    return out;
}

BikeVec3* Bike::UnknownVirtualSlot54(BikeVec3* out)
{
    if (field_0x4a8 == 0) {
        *out = field_0x5f4->w_0xe4;
        return out;
    }
    BikeWheel* a = field_0x5f0;
    if (a->w_0x260 == 0) {
        *out = field_0x5f4->w_0xe4;
        return out;
    }
    BikeWheel* b = field_0x5f4;
    if (b->w_0x260 == 0) {
        *out = b->w_0xe4;
        return out;
    }
    BikeVec3 s(a->w_0xe4.x + b->w_0xe4.x, a->w_0xe4.y + b->w_0xe4.y, a->w_0xe4.z + b->w_0xe4.z);
    BikeVec3 mid(s.x * 0.5f, s.y * 0.5f, s.z * 0.5f);
    *out = BikeNormalized(mid);
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
    t = BIKE_MIN(t, 100.0f);
    t = t * arg * 5.0f;
    t = BIKE_MIN(t, 10000.0f);
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
    BikeVec3 tmp;
    field_0x1a0 = *((BikeXform*)field_0x42c)->Method_0x004fd7f0(&tmp, &field_0x18);
    if (field_0x444 == 0 && field_0x640) {
        float steer = (field_0x640->l_0x0 - 0.5f) * 4.0f;
        field_0x1a0.x = 0.0f;
        field_0x1a0.z += steer;
        field_0x18 = *((BikeXform*)field_0x42c)->Method_0x004fd660(&tmp, &field_0x1a0);
    }
    field_0x194 = field_0x1a0;
    field_0x700 = (field_0x4a8 > 0 && field_0x5f0->w_0x150 < -2.5f);
}

BikeVec3 Bike::UnknownVirtualSlot76(const BikeVec3* a, const BikeVec3* b)
{
    BikeVec3 v = Vehicle::UnknownVirtualSlot76(a, b);
    if (field_0xbc < 10.0f && field_0x4c > 0.8727f && field_0x5f0->w_0x260) {
        v = g_BikeVec3_005778a8;
        if (field_0xd8.x < 0.0f)
            field_0xd8.x = 0;
    } else if (field_0xd8.x < -1.0f) {
        v.x *= 0.2f;
        v.y *= 0.2f;
        v.z *= 0.2f;
    } else {
        float lean = field_0x4c;
        if (lean < 0.0f)
            lean = -lean;
        lean = BIKE_MIN(lean, 1.0f);
        float k = -(field_0x504.y + 0.125f);
        k = (0.0f > k) ? 0.0f : k;
        k = (1.4f - lean) * k * 1.6f;
        v.x *= k;
        v.y *= k;
        v.z *= k;
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
        ((BikeXform*)field_0x42c)->Method_0x004fc540(0, &field_0xa0, &field_0xac);
        BikeFunc_0x004b5a60(field_0xa0, field_0xac, &field_0x50, &field_0x4c, &field_0x48,
                            &field_0x54, &field_0x58, &field_0x60, &field_0x5c);
    }
    int mode = field_0x604->a_0x44;
    if (mode == 0 || mode == 1) {
        float tmp[16];
        ((BikeXform*)d3d_field_0x1a0)->Method_0x004fca80(0, tmp);
        field_0x5c4->c_0x1a0->Method_0x004fb8c0(0, tmp);
    }
}

void Bike::UnknownVirtualSlot41()
{
    if (field_0x6fc == 0 && field_0x430 == 0)
        return;
    ((BikeXform*)field_0x42c)->Method_0x004fc540(0, &field_0xa0, &field_0xac);
    ((BikeXform*)d3d_field_0x1a0)->Method_0x004fc050(0, &field_0xa0, &field_0xac, 1, 0);
    D3DIMSoultreeCharacter::Method_0x004a8b00();
    field_0x430 = 0;
    UnknownVirtualSlot34();
    BikeFunc_0x004b5a60(field_0x88, field_0x94, &field_0x34, &field_0x30, &field_0x2c,
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
        t = (t > 0.25f) ? BIKE_MIN(t, 0.75f) : 0.25f;
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
        float tmp[16];
        ((BikeXform*)d3d_field_0x1a0)->Method_0x004fca80(0, tmp);
        field_0x5c4->c_0x1a0->Method_0x004fb8c0(0, tmp);
    }
}

void Bike::UnknownVirtualSlot72(BikeVec3* out, VehicleWheel*)
{
    if (field_0x444)
        return;
    BikeWheel* a = field_0x5f0;
    if (!(a->w_0x260 || (field_0x5f4->w_0x260 && a->w_0x150 > -0.2f) || field_0x700)) {
        *out = field_0x5f4->w_0xe4;
        field_0x4b8 = 0.0f;
        field_0x43c = 0.0f;
        return;
    }
    *out = a->w_0xe4;
    BikeWheel* b = field_0x5f4;
    float weight = field_0x5f0->w_0x144;
    if (b->w_0x150 <= -0.2f) {
        field_0x4b8 = 0.0f;
        field_0x43c = 0.0f;
        return;
    }
    out->x += b->w_0xe4.x;
    out->y += b->w_0xe4.y;
    out->z += b->w_0xe4.z;
    weight += field_0x5f4->w_0x144;
    float lenSq = out->y * out->y + out->x * out->x + out->z * out->z;
    if (lenSq == 0.0f) {
        *out = g_BikeVec3_005778a8;
    } else {
        float s = BikeMath_0x00460c00(lenSq);
        out->x *= s;
        out->y *= s;
        out->z *= s;
    }
    weight *= 0.5f;
    const BikeVec3* n = &field_0x5f0->w_0x230;
    BikeVec3 c;
    c.x = out->z * n->y - out->y * n->z;
    c.y = out->x * n->z - n->x * out->z;
    c.z = n->x * out->y - out->x * n->y;
    BikeVec3 perp = BikeNormalized(c);
    float scale;
    if (field_0x700) {
        float k = field_0x60 * 0.35f;
        k = (0.001f > k) ? 0.001f : k;
        scale = 1.0f / k;
    } else {
        scale = 1.0f;
    }
    weight = BIKE_MIN(weight, 1.0f);
    field_0x4b8 = UnknownVirtualSlot74(&field_0x5f0->w_0x23c, &perp, weight, scale);
    field_0x43c = field_0x4b8 * field_0x140;
}

float Bike::UnknownVirtualSlot61(float threshold)
{
    float result = 0.0f;
    if (field_0x5a8) {
        BikeWheel* rear = field_0x5f4;
        if (rear->w_0x148 > 7.0f && rear->w_0x288 >= 0.0001f && field_0x5f0->w_0x288 < 0.5f) {
            BikeVec3 c;
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
                u = BIKE_MIN(u, 1.57f);
                float b = a * 5.0f;
                b = BIKE_MIN(b, 1.5f);
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

// Length of a vector from its squared length; BikeMath_0x00460c00 is 1/sqrt.
static inline float BikeLength(float lenSq)
{
    if (lenSq == 0.0f)
        return 0.0f;
    if (lenSq == 1.0f)
        return 1.0f;
    return 1.0f / BikeMath_0x00460c00(lenSq);
}

// Slot 97: builds the rider ragdoll ("rider.col") and attaches the 15 body
// bones by name (Pelvis, UprTorso, Head, arms, legs, feet) with local offsets.
static inline void BikeAttachBone(BikeA604* rider, const char* name, float x, float y, float z)
{
    BikeVec3 offset(x, y, z);
    int bone = rider->a_0x34->r_0x1a0->Method_0x004fdae0(name);
    rider->Method_0x00532900(offset, bone);
}

void Bike::UnknownVirtualSlot97()
{
    field_0x604 = new(__FILE__, 2023) BikeA604(1);
    if (field_0x604->Method_0x00530190(GameObject::field_0x18, field_0x5c4, "rider.col")) {
        GameObject::Method_0x00469190(field_0x604, -1);
        field_0x604->a_0x38->n_0xe4 = ((BikeA1F4*)field_0x1f4);
        field_0x604->a_0x38->n_0x64 = 0x65;
        BikeAttachBone(field_0x604, "Pelvis", 0.0f, 0.0f, -0.35f);
        BikeAttachBone(field_0x604, "UprTorso", 0.0f, 0.0f, 0.0f);
        BikeAttachBone(field_0x604, "Head", 0.0f, 0.75f, 0.0f);
        BikeAttachBone(field_0x604, "UprArmL", 0.0f, 0.0f, 0.0f);
        BikeAttachBone(field_0x604, "UprArmR", 0.0f, 0.0f, 0.0f);
        BikeAttachBone(field_0x604, "LwrArmL", 0.0f, 0.0f, 0.0f);
        BikeAttachBone(field_0x604, "HandL", 0.0f, -0.5f, 0.0f);
        BikeAttachBone(field_0x604, "LwrArmR", 0.0f, 0.0f, 0.0f);
        BikeAttachBone(field_0x604, "HandR", 0.0f, -0.5f, 0.0f);
        BikeAttachBone(field_0x604, "LwrLegL", 0.0f, 0.0f, 0.0f);
        BikeAttachBone(field_0x604, "FootL", 0.0f, -0.5f, 0.75f);
        BikeAttachBone(field_0x604, "FootL", 0.0f, -1.0f, 0.0f);
        BikeAttachBone(field_0x604, "LwrLegR", 0.0f, 0.0f, 0.0f);
        BikeAttachBone(field_0x604, "FootR", 0.0f, -0.5f, 0.75f);
        BikeAttachBone(field_0x604, "FootR", 0.0f, -1.0f, 0.0f);
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
        BikeVec3 tmp;
        field_0x1ac = *rider->a_0x34->r_0x1a0->d_0x140->Method_0x004fd7f0(&tmp, &rider->a_0xdc);
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
    float d1 = g_BikeVec3_005778c8.y * field_0xa0.y + g_BikeVec3_005778c8.x * field_0xa0.x;
    d1 += g_BikeVec3_005778c8.z * field_0xa0.z;
    if (d1 < 0.0f)
        d1 = -d1;
    if (field_0x708 < d1) {
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
    if (c && a * field_0x44c < d * field_0x24) {
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
        float inv = 1.0f / field_0xbc;
        BikeVec3 u1(inv * field_0x64.x, inv * field_0x64.y, inv * field_0x64.z);
        float inv2 = 1.0f / other->field_0xbc;
        BikeVec3 u2(inv2 * other->field_0x64.x, inv2 * other->field_0x64.y, inv2 * other->field_0x64.z);
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
void Bike::UnknownVirtualSlot7(const BikeVec3* dir)
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
                BikeVec3 dv(field_0x18.x - e->h_0x14.x, field_0x18.y - e->h_0x14.y, field_0x18.z - e->h_0x14.z);
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
                e->h_0x44 = BikeVec3(w * dir->x, w * dir->y, w * dir->z);
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
        field_0x5f4->w_0xfc = BikeVec3(field_0x5f4->w_0x158 * dir->x, field_0x5f4->w_0x158 * dir->y, field_0x5f4->w_0x158 * dir->z);
        field_0x5f0->w_0x158 = field_0x5f0->w_0x294;
        field_0x5f0->w_0xfc = BikeVec3(field_0x5f0->w_0x158 * dir->x, field_0x5f0->w_0x158 * dir->y, field_0x5f0->w_0x158 * dir->z);
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
        BikeVec3 df(field_0x18.x - field_0x5f0->w_0xcc.x, field_0x18.y - field_0x5f0->w_0xcc.y, field_0x18.z - field_0x5f0->w_0xcc.z);
        float dfSq = BikeDot(&df, &df);
        float lf = BikeLength(dfSq);
        BikeVec3 dr(field_0x18.x - field_0x5f4->w_0xcc.x, field_0x18.y - field_0x5f4->w_0xcc.y, field_0x18.z - field_0x5f4->w_0xcc.z);
        float drSq = BikeDot(&dr, &dr);
        float lr = BikeLength(drSq);
        field_0x5f4->w_0x158 = drSq / (lr + lf);
        field_0x5f4->w_0xfc = BikeVec3(field_0x5f4->w_0x158 * dir->x, field_0x5f4->w_0x158 * dir->y, field_0x5f4->w_0x158 * dir->z);
        field_0x5f0->w_0x158 = 1.0f - field_0x5f4->w_0x158;
        field_0x5f0->w_0xfc = BikeVec3(field_0x5f0->w_0x158 * dir->x, field_0x5f0->w_0x158 * dir->y, field_0x5f0->w_0x158 * dir->z);
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
    float fx, fy, fz;
    if (field_0x430) {
        fx = f * 0.5f;
        fy = 0.0f;
        fz = 0.0f;
    } else {
        fx = f;
        fy = field_0x504.x * field_0x13c * -0.2f;
        fz = 0.0f;
    }
    if (field_0x6fc && field_0x5f4->w_0x150 < -2.0f) {
        if (field_0x5c0 && field_0x5f4->w_0x29c > 0.0f)
            fx = dt * field_0x5f4->w_0x29c + fx;
        if (field_0x5bc && field_0x478 && field_0x480->field_0x00 > 0.0f && !field_0x479)
            fx = fx - dt * field_0x480->field_0x00;
        float inv = 1.0f / field_0xbc;
        field_0x1ac = BikeVec3(inv * field_0x64.x, inv * field_0x64.y, inv * field_0x64.z);
        float d = field_0xa0.y * field_0x1ac.y + field_0x1ac.x * field_0xa0.x + field_0xa0.z * field_0x1ac.z;
        if (d > 0.55f && d < 0.984f) {
            // steer the velocity toward the lean plane
            field_0x1b8 = BikeVec3(field_0x1ac.z, 0.0f, -field_0x1ac.x);
            float s = (field_0x65c < 0.5f) ? 1.0f : -1.0f;
            float k = s * ((1.0f - d) * 3.41296911f * field_0xbc * 0.075f) * field_0x13c;
            float kx = k * field_0x1b8.x;
            float ky = k * field_0x1b8.y;
            float kz = k * field_0x1b8.z;
            field_0x64.x = kx + field_0x64.x;
            field_0x64.y = ky + field_0x64.y;
            field_0x64.z = kz + field_0x64.z;
            float scale = field_0xbc / (k + field_0xbc);
            field_0x64.x = scale * field_0x64.x;
            field_0x64.y = scale * field_0x64.y;
            field_0x64.z = scale * field_0x64.z;
            float lenSq = field_0x64.y * field_0x64.y + field_0x64.x * field_0x64.x + field_0x64.z * field_0x64.z;
            field_0xbc = (lenSq == 1.0f) ? 1.0f : (float)sqrt(lenSq);
        }
    }
    float forceSq = fy * fy + fx * fx + fz * fz;
    float mag = BikeLength(forceSq);
    if (_finite(mag) && ((mag < 0.0f) ? -mag : mag) >= 0.0001f) {
        BikeVec3 dir;
        if (forceSq == 0.0f) {
            dir = g_BikeVec3_005778a8;
        } else {
            float rs = BikeMath_0x00460c00(forceSq);
            dir = BikeVec3(rs * fx, rs * fy, rs * fz);
        }
        ((BikeXform*)d3d_field_0x1a0)->Method_0x004fd1f0(field_0x194, dir, mag);
    }
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

// Slot 46: focus/anchor point of the bike in local space.  Airborne or flagged
// bikes return the stored anchor (field_0x194); otherwise it is the front
// wheel position (t != 0) or a blend toward the rear wheel, relative to the
// body, mapped through the transform.
BikeVec3* Bike::UnknownVirtualSlot46(BikeVec3* out, float t)
{
    if (field_0x108 || field_0x1d0) {
        *out = field_0x194;
        return out;
    }
    BikeWheel* front = field_0x5f0;
    BikeWheel* rear = field_0x5f4;
    BikeVec3 tmp;
    if (field_0x4a8 == 2) {
        if (t != 0.0f) {
            field_0x1ac = BikeVec3(front->w_0xcc.x - field_0x0c.x, front->w_0xcc.y - field_0x0c.y, front->w_0xcc.z - field_0x0c.z);
        } else if (rear->w_0x2b8 < 0.9f) {
            float s = rear->w_0x2b8 - 0.4f;
            if (!(s > 0.0f))
                s = 0.0f;
            BikeVec3 d(rear->w_0xcc.x - front->w_0xcc.x, rear->w_0xcc.y - front->w_0xcc.y, rear->w_0xcc.z - front->w_0xcc.z);
            BikeVec3 p(d.x * s + front->w_0xcc.x, d.y * s + front->w_0xcc.y, d.z * s + front->w_0xcc.z);
            field_0x1ac = BikeVec3(p.x - field_0x0c.x, p.y - field_0x0c.y, p.z - field_0x0c.z);
        } else {
            BikeVec3 d(rear->w_0xcc.x - front->w_0xcc.x, rear->w_0xcc.y - front->w_0xcc.y, rear->w_0xcc.z - front->w_0xcc.z);
            BikeVec3 half(d.x * 0.5f, d.y * 0.5f, d.z * 0.5f);
            BikeVec3 mid(half.x + front->w_0xcc.x, half.y + front->w_0xcc.y, half.z + front->w_0xcc.z);
            field_0x1ac = BikeVec3(mid.x - field_0x0c.x, mid.y - field_0x0c.y, mid.z - field_0x0c.z);
        }
    } else {
        BikeVec3 d(rear->w_0xcc.x - front->w_0xcc.x, rear->w_0xcc.y - front->w_0xcc.y, rear->w_0xcc.z - front->w_0xcc.z);
        BikeVec3 half(d.x * 0.5f, d.y * 0.5f, d.z * 0.5f);
        BikeVec3 mid, rel;
        BikeVecAdd_0x00421cb0(&mid, &front->w_0xcc, &half);
        BikeVecSub_0x00421d00(&rel, &mid, &field_0x0c);
        field_0x1ac = rel;
    }
    *out = *((BikeXform*)d3d_field_0x1a0)->Method_0x004fd710(&tmp, &field_0x1ac);
    return out;
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
void Bike::UnknownVirtualSlot92(const BikeVec3* worldDir, const BikeVec3* point)
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
            BikeVec3 tmp;
            BikeVec3 w = *((BikeXform*)d3d_field_0x1a0)->Method_0x004fd710(&tmp, worldDir);
            float az = (w.z < 0.0f) ? -w.z : w.z;
            float ay = (w.y < 0.0f) ? -w.y : w.y;
            float ax = (w.x < 0.0f) ? -w.x : w.x;
            if (az > 0.001f || ay > 0.001f || ax > 0.001f) {
                ((BikeXform*)d3d_field_0x1a0)->Method_0x004fd1f0(w, *point, field_0x4e4);
                float k = field_0x4e4;
                float tx = k * w.x;
                float ty = k * w.y;
                float tz = k * w.z;
                field_0x1b8 = BikeVec3(tx * field_0x140, ty * field_0x140, tz * field_0x140);
                BikeCancelToward(&field_0xd8.x, field_0x1b8.x);
                BikeCancelToward(&field_0xd8.y, field_0x1b8.y);
                BikeCancelToward(&field_0xd8.z, field_0x1b8.z);
            }
        }
    }
}
