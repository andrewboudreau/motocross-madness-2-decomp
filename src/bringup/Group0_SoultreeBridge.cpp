// Group0_SoultreeBridge.cpp -- see Group0_SoultreeBridge.h.
#include "math/Math3D.h"
#include "Group0_SoultreeBridge.h"

void Group0_SoultreeGetPosition(void* node, float* out)
{
    ((SoultreeObject*)node)->GetPosition((Vec3*)out);
}

void Group0_SoultreeTranslateIn(void* node, void* frame, const float* delta)
{
    Vec3 value(delta[0], delta[1], delta[2]);
    ((SoultreeObject*)node)->TranslateIn((SoultreeObject*)frame, value);
}

void Group0_SoultreeSetMatrixIn(void* node, void* frame, void* matrix)
{
    ((SoultreeObject*)node)->SetMatrixIn((SoultreeObject*)frame, (Matrix4*)matrix);
}

void Group0_SoultreeSetAxesIn(void* node, void* frame, const float* axisZ, const float* axisY,
                              int orthogonalize, int keepZ)
{
    ((SoultreeObject*)node)->SetAxesIn((SoultreeObject*)frame, (const Vec3*)axisZ,
                                       (const Vec3*)axisY, orthogonalize, keepZ);
}

void Group0_SoultreeSetPositionIn(void* node, void* frame, const float* p)
{
    ((SoultreeObject*)node)->SetPositionIn((SoultreeObject*)frame, (const Vec3*)p);
}

void Group0_SoultreeMarkSubtreeDirty(void* node)
{
    ((SoultreeObject*)node)->MarkSubtreeDirty();
}
