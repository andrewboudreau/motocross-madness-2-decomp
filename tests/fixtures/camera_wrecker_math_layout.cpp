#include <stddef.h>
#include "../../src/reconstructed/FollowCamera.h"
#include "../../src/reconstructed/Wrecker.h"
#include "../../src/krusty2/math/Vec3.h"

// FollowCamera's existing Vector3 inputs cross the shared Vec3 reference ABI.
// Both constructors and the complete callers also prove the scalar stores.
typedef char vector_size[sizeof(Vector3) == 12 && sizeof(Vec3) == 12 ? 1 : -1];
typedef char vector_x[offsetof(Vector3, x) == 0 && offsetof(Vec3, x) == 0 ? 1 : -1];
typedef char vector_y[offsetof(Vector3, y) == 4 && offsetof(Vec3, y) == 4 ? 1 : -1];
typedef char vector_z[offsetof(Vector3, z) == 8 && offsetof(Vec3, z) == 8 ? 1 : -1];

// The Wrecker caller writes rotation rows through this existing matrix view.
typedef char matrix_size[sizeof(UnknownWreckerMatrix) == 64 ? 1 : -1];
typedef char matrix_up[offsetof(UnknownWreckerMatrix, _21) == 16 ? 1 : -1];
typedef char matrix_forward[offsetof(UnknownWreckerMatrix, _31) == 32 ? 1 : -1];
