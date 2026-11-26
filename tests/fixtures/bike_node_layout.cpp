#include "../../src/krusty2/vehicle/Bike.h"
#include <stddef.h>

typedef char CanonicalModelOffset[offsetof(D3DIMSoultreeCharacter, modelNode) == 0x1a0 ? 1 : -1];
typedef char ChildCanonicalOffset[offsetof(SoultreeObject, firstChild) == 0x140 ? 1 : -1];
typedef char LocalMatrixOffset[offsetof(SoultreeObject, localMatrix) == 0xb8 ? 1 : -1];
typedef char VectorExtent[sizeof(Vec3) == 12 ? 1 : -1];
typedef char MatrixExtent[sizeof(Matrix4) == 64 ? 1 : -1];
typedef char BonePointerExtent[sizeof(int) == sizeof(SoultreeObject*) ? 1 : -1];

// The rider model is a primary node pointer, not its +0xc GameObject base.
SoultreeObject* RiderModel(D3DIMSoultreeCharacter* rider) {
    return rider->modelNode;
}
