#include "MovingPart.h"

// 0x004a2350. The component-wise copy into a local gives retail's stack
// temporary; returning the row through a Vector3 cast copies it directly.
Vector3 GetMatrixRow(const Matrix4* m, int row)
{
    Vector3 v;
    v.x = m->m[row][0];
    v.y = m->m[row][1];
    v.z = m->m[row][2];
    return v;
}

// 0x004a23a0
MovingPart::MovingPart(const char* name, SoultreeObject* root, int field_0x44_)
{
    node = root->FindByName(name);
    node->GetLocalMatrix(&matrix);
    field_0x48 = 0;
    field_0x44 = field_0x44_;
}
