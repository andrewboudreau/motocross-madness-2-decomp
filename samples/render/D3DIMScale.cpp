// Existing rendering-model call view of the complete nonexact scale operation.
// Retail 0x004fd340, 630 bytes; original method spelling and TU are provisional.
#include <string.h>
#include "../../src/reconstructed/D3DIMSoulTree.h"
#include "../physics/helpers/SoultreeScaleProduct.h"

void D3DIMSoultreeObject::Scale(float x, float y, float z)
{
    Matrix4* local = &localMatrix;
    Matrix4 s;
    memset(&s, 0, sizeof(Matrix4));
    s._11 = x;
    s._22 = y;
    s._33 = z;
    s._44 = 1.0f;
    ScaleProduct(local, *local, s);
    InvalidateWorldMatrix();
}
