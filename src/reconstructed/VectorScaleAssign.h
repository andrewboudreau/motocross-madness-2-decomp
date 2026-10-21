#pragma once
#include "../krusty2/math/Vec3.h"

// Existing Bike call view of the out-of-line vector scale operator.
// The name is provisional; callers retain their 12-byte primary receiver.
struct BikeVec3ScaleAssign : Vec3 {
    BikeVec3ScaleAssign& operator*=(float scale);
};
