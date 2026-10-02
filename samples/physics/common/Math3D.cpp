// Math3D.cpp -- out-of-line bodies of small Math3D.h helpers (byte-match samples).
//
// This is a sample grouping, not a claim about the retail translation unit. The
// helpers sit in different retail ranges: 0x004a13e0 and 0x004a1d10..0x004a2040
// belong to the matrix/quaternion library, and 0x005087b0 is elsewhere. Which file
// owned each one is unknown (tier 3).
#include <string.h>
#include "Math3D.h"

// 0x004a13e0. rep stosd of a zeroed local, then a copy to the hidden result.
Matrix4 MatrixZero()
{
    Matrix4 m;
    memset(&m, 0, sizeof(m));
    return m;
}

// 0x004a1d10. Two-step Adams-Bashforth update. The constants are 3.0f (0x00550774)
// and 0.5f (0x005507f4). Retail forms deriv + n before the n > 0 test
// ('lea edx,[eax+esi*4]'), which the named 'prev' pointer reproduces. Indexing
// deriv[n + i] directly does not.
void IntegrateAdamsBashforth2(float dt, int n, float* state, float* deriv)
{
    float* prev = deriv + n;
    for (int i = 0; i < n; i++) {
        state[i] += (deriv[i] * 3.0f - prev[i]) * dt * 0.5f;
        prev[i] = deriv[i];
    }
}

// 0x004a1f90. dq/dt = 0.5 * q (x) (0, w). The scalar part is scaled by -0.5f
// (0x00550758) rather than negating the sum.
Quat QuatDerivative(const Quat& q, const Vec3& w)
{
    Quat r;
    r.w = (q.x * w.x + q.y * w.y + q.z * w.z) * -0.5f;
    r.x = (q.w * w.x + q.y * w.z - q.z * w.y) * 0.5f;
    r.y = (q.w * w.y + q.z * w.x - q.x * w.z) * 0.5f;
    r.z = (q.w * w.z + q.x * w.y - q.y * w.x) * 0.5f;
    return r;
}

// 0x005087b0. Unit vector using the table rsqrt. A vector whose squared length is
// already exactly 1.0f is returned unchanged. Retail loads y, x, z into the FPU and
// sums (x*x + y*y) + z*z. Only the explicit grouping below (or an equivalent
// running sum) keeps VC6 from reassociating it to (z*z + x*x) + y*y. The
// declaration order y, x, z gives the load order.
Vec3 Vec3Normalize(const Vec3& v)
{
    float y = v.y, x = v.x, z = v.z;
    float sq = z * z + (x * x + y * y);
    if (sq == 1.0f)
        return v;
    float inv = FastInvSqrt(sq);
    return Vec3(v.x * inv, v.y * inv, v.z * inv);
}
