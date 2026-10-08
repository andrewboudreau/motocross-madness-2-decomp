// OrientationAxes.cpp -- 0x004b5d00: the inverse of OrientationAngles.cpp's
// 0x004b5a60, in the same unit between OptionProcs.cpp (last literal
// reference 0x004b5119) and overlay.cpp (0x004b60a2); ownership tier 3.
//
// Near miss: 265/274 bytes, 91 of 91 instructions in order. The only
// difference is where VC6 schedules `mov eax, 1`: retail puts it before the
// last faddp, this build puts it between the two closing fstps. The split
// `t = s * z; t += c * y;` in the pitch step is load-bearing (one expression
// multiplies by c first); an early `if (!up || !forward) return 0;`, a nested
// `if (up && forward)` and splitting the last yaw product the same way do not
// move the mov.
#include <math.h>

#include "../../../src/reconstructed/MatrixUtil.h"

// 0x004b5d00 (cdecl): builds the forward (0, 0, 1) and up (0, 1, 0) axes
// rolled about z (up only: forward is the axis), pitched about x and yawed
// about y. SceneManager passes (heading, pitch, 0); KrustyBike passes a
// racer's (roll, pitch, yaw). Returns 0 when either pointer is null.
int UnknownFunction4b5d00(Vector3* forward, Vector3* up, float yaw, float pitch, float roll)
{
    if (!up || !forward) {
        return 0;
    }
    float c = (float)cos(roll);
    up->x = 0.0f;
    up->y = 1.0f;
    up->z = 0.0f;
    forward->x = 0.0f;
    forward->y = 0.0f;
    forward->z = 1.0f;
    float s = (float)sin(roll);
    float t = c * up->y - s * up->x;
    up->x = c * up->x + s * up->y;
    up->y = t;

    c = (float)cos(pitch);
    s = (float)sin(pitch);
    t = s * up->z;
    t += c * up->y;
    up->z = c * up->z - s * up->y;
    up->y = t;
    t = s * forward->z;
    t += c * forward->y;
    forward->z = c * forward->z - s * forward->y;
    forward->y = t;

    c = (float)cos(yaw);
    s = (float)sin(yaw);
    t = c * up->z - s * up->x;
    up->x = c * up->x + s * up->z;
    up->z = t;
    t = c * forward->z - s * forward->x;
    forward->x = c * forward->x + s * forward->z;
    forward->z = t;
    return 1;
}
