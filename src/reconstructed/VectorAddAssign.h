#pragma once
#include "../krusty2/math/Vec3.h"

// Existing BVH call view of the out-of-line in-place vector addition.
// Primary xyz storage only; the original class and header ownership are unproven.
struct Vec3AddAssignCall : Vec3 {
    Vec3AddAssignCall() {}
    Vec3AddAssignCall& operator+=(const Vec3& v);
};
