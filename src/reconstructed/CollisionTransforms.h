#pragma once

struct Vec3;
struct Matrix4;

// Complete out-of-line helpers; original names and source unit are provisional.
void MoveBox(Vec3* center, Vec3* halfExtents, const Matrix4* matrix); // 0x00428db0
void InvertRigidTransform(Matrix4* matrix);                        // 0x0042e2b0
