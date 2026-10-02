// SteeringControl.cpp -- reconstruction of D:\aardvark\VC\krusty2\SteeringControl.cpp.
#include "core/GameObject.h"
#include "core/DebugAlloc.h"
#include "../../../samples/physics/common/Math3D.h"
#include "SteeringControl.h"
#include <math.h>

// Inline absolute value; the `v < 0 ? -v : v` form (not fabs) reproduces retail's fchs sequence.
static inline float Abs(float v) { return v < 0.0f ? -v : v; }

// Retail has two sets of the four per-TU vector initialisers here (0x00504f70..0x005051a0: bodies store
// to 0x00689f70/80/90/60 and 0x00689fb0/c0/d0/a0).  Math3D.h supplies the first set; the second set
// comes from another header that is included here too.  Tier 3 stand-in with the same copy-initialised shape.
static const Vec3 kSecondVec3Zero = Vec3(0.0f, 0.0f, 0.0f);
static const Vec3 kSecondVec3XAxis = Vec3(1.0f, 0.0f, 0.0f);
static const Vec3 kSecondVec3YAxis = Vec3(0.0f, 1.0f, 0.0f);
static const Vec3 kSecondVec3ZAxis = Vec3(0.0f, 0.0f, 1.0f);

extern "C" int _finite(double);   // 0x00534d07

SteeringControl::SteeringControl(float scale, float t, const Vec3* axisZ, const Vec3* axisY)
{
    angle = 0.0f;
    axis = kVec3Zero;
    float s = t > 0.0f ? t : 0.0f;
    if (s >= 1.0f)
        s = 1.0f;
    field_0x08 = scale * s;
    node = new(__FILE__, 0x17) SoultreeObject(1);
    node->SetPosition(0.0f, 0.0f, 0.0f);
    node->SetAxesPtr(axisZ, axisY, 1, 0);
}

void SteeringControl::Release()
{
    if (node) {
        GameObject* base = (GameObject*)((char*)node + 0xc);
        base->BaseObjectVirtualSlot2();
        node = 0;
    }
}

int SteeringControl::SetAxisFromDirection(const Vec3* worldDir, SoultreeObject* frame)
{
    float lenSq = worldDir->x * worldDir->x + worldDir->y * worldDir->y;
    lenSq += worldDir->z * worldDir->z;
    float len = lenSq == 1.0f ? 1.0f : (float)sqrt(lenSq);
    float inv = 1.0f / len;
    Vec3 dir = Vec3(worldDir->x * inv, worldDir->y * inv, worldDir->z * inv);
    if (frame) {
        axis = frame->WorldToLocalDirection(dir);
        return 1;
    }
    return 0;
}

int SteeringControl::SetAxisFromPoints(SoultreeObject* from, SoultreeObject* to, SoultreeObject* frame)
{
    Vec3 fromPos;
    Vec3 toPos;
    from->GetPositionIn(0, &fromPos);
    to->GetPositionIn(0, &toPos);
    Vec3 d = fromPos - toPos;
    float lenSq = d.x * d.x + d.y * d.y;
    lenSq += d.z * d.z;
    float len = lenSq == 1.0f ? 1.0f : (float)sqrt(lenSq);
    float inv = 1.0f / len;
    Vec3 dir = Vec3(d.x * inv, d.y * inv, d.z * inv);
    if (frame) {
        axis = frame->WorldToLocalDirection(dir);
        return 1;
    }
    return 0;
}

void SteeringControl::SetAngle(float a, SoultreeObject* frame)
{
    if (_finite(a)) {
        if (Abs(a) < 0.001f)
            a = 0.0f;
        angle = a;
        Matrix4 m;
        frame->GetMatrixIn(0, &m);
        node->SetMatrixIn(0, &m);
        node->SetPosition(kVec3Zero);
        node->RotateAbout(axis.x, axis.y, axis.z, -angle);
    }
}

void SteeringControl::AddAngle(float delta, SoultreeObject* frame)
{
    angle += delta;
    if (!_finite(angle) || Abs(angle) < 0.0001f)
        angle = 0.0f;
    Matrix4 m;
    frame->GetMatrixIn(0, &m);
    node->SetMatrixIn(0, &m);
    node->SetPosition(kVec3Zero);
    if (angle != 0.0f)
        node->RotateAbout(axis.x, axis.y, axis.z, -angle);
}
