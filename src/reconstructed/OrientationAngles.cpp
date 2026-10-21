// OrientationAngles.cpp -- 0x004b5a60: Euler-style angle extraction from a pair of direction
// vectors. Translation-unit ownership is PROVISIONAL (tier 3): nearest __FILE__ references are
// OptionProcs.cpp / overlay.cpp (proximity only). Callers: 0x0040bec0 and five others.
//
// All names are tier 3, derived from the arithmetic. The routine takes a "forward" vector a
// and an "up" vector b (both by value, 3 floats each). It returns 0 if any of the first three
// pointers is null, otherwise 1.
#include <math.h>
#include "math/Math3D.h"

static inline float AbsF(float x) { return x < 0.0f ? -x : x; }

int OrientationAnglesFromVectors(Vec3 a, Vec3 b, float* yaw, float* pitch, float* roll,
                                 float* sinRoll, float* cosRoll, float* cosPitch,
                                 float* sinPitch)
{
    if (!yaw || !pitch || !roll)
        return 0;

    // The components are read into locals first; that is what makes VC6 load the heading
    // terms c and s before b's components and the rotated az before a.y (their leaves are
    // older, docs/VC6_OPERAND_ORDER.md).
    float ax = a.x, ay = a.y, az = a.z;
    float bx = b.x, by = b.y, bz = b.z;

    // Heading of a in the x/z plane: (cos, sin) = (az, ax) / |(az, ax)|.
    float c, s;
    float len = (float)sqrt(ax * ax + az * az);
    if (len == 0.0f) {
        c = 1.0f;
        s = 0.0f;
    } else {
        c = az / len;
        s = ax / len;
    }
    *yaw = (float)acos(c);
    if (ax < 0.0f)
        *yaw = -*yaw;

    // Turn a and b about y by the heading, then take the pitch of a.
    float fz = c * az + s * ax;
    float uz = c * bz + s * bx;
    float ux = c * bx - s * bz;
    len = (float)sqrt(fz * fz + ay * ay);
    if (len == 0.0f) {
        *cosPitch = 1.0f;
        *sinPitch = 0.0f;
    } else {
        *cosPitch = fz / len;
        *sinPitch = ay / len;
    }
    *pitch = (float)acos(*cosPitch);
    if (ay < 0.0f)
        *pitch = -*pitch;

    // Roll of b about the pitched axis.
    float uy = by * *cosPitch - uz * *sinPitch;
    len = (float)sqrt(uy * uy + ux * ux);
    if (len == 0.0f) {
        *cosRoll = 1.0f;
        *sinRoll = 0.0f;
    } else {
        *cosRoll = uy / len;
        *sinRoll = ux / len;
    }
    *roll = (float)acos(*cosRoll);
    if (ux < 0.0f)
        *roll = -*roll;

    // Near straight up/down the yaw and roll axes coincide: fold roll into yaw.
    if (AbsF(*pitch - 1.5707964f) < 0.1745f) {
        *yaw = *yaw - *roll;
        if (*yaw < 0.0)
            *yaw += 6.2831855f;
        *roll = 0.0f;
    } else if (AbsF(*pitch + 1.5707964f) < 0.1745f) {
        *yaw = *yaw + *roll;
        if (*yaw > 6.2831855f)
            *yaw -= 6.2831855f;
        *roll = 0.0f;
    }
    return 1;
}
