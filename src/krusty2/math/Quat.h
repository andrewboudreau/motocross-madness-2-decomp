// Quat.h -- the quaternion storage type, split out of Math3D.h so that
// rigidbody/PhysicsBody.h can declare its derivative arrays without pulling in
// Math3D.h's per-TU constant vectors (Wrecker.cpp, which includes PhysicsBody.h
// through Wrecker.h, defines its own set). Math3D.h includes this header.
#ifndef MCM2_PHYSICS_COMMON_QUAT_H
#define MCM2_PHYSICS_COMMON_QUAT_H

// Quaternion {w, x, y, z}: scalar part FIRST. Tier 2: QuatDerivative (0x004a1f90)
// computes result[+0] = -0.5 * (q[+4]*w.x + q[+8]*w.y + q[+12]*w.z), which is the
// scalar part of 0.5 * q (x) (0, w) only when +0 is the scalar and +4..+12 the vector.
// The same order is the one QuatToMatrix (0x004a1e10) needs to produce a rotation.
struct Quat {
    float w, x, y, z;
};

#endif
