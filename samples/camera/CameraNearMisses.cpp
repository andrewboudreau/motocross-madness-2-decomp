// Near-miss Camera candidates (src/reconstructed/Camera.cpp's class), kept out
// of src/reconstructed until they match. See docs/OVERLAY.md.
//
// Camera::UnknownVirtualSlot28 (0x0042ee30, 492 bytes, 487/492): one
// scheduling difference: retail emits `sub esp, 0x40` before the `rep movsd`
// that copies the projection matrix (MatrixMult's second argument), VC6 after.
// Assigning through a named product, a nested assignment, m[][] stores, the
// viewport built by an inline helper and the store order do not move it.
// Also unchanged (487): a projection copy (local, Matrix4(...) temporary,
// through a pointer or this->), a viewport temporary, `Matrix4 viewport;`
// declared apart or at the top, the stores through a float pointer or a
// reference, a named 0.5f constant, -viewport(0, 0) for the -0.5 and a block
// around the viewport. Chained stores, a projection local assigned after
// the viewport, the view matrix built after it and the products nested into
// the second MatrixMult call are worse.
// The half field of view needs the (float) cast: without it VC6 folds
// 0.5f * 0.01745329f into one constant, while retail multiplies twice.
// 0..15 dead initialised int locals at the top of the function leave the `sub esp, 0x40` in place.
#include <math.h>

#include "../../src/reconstructed/Camera.h"

// 0x0042ee30: rebuilds the view and projection matrices from the camera
// frame, the field of view (degrees) and the viewport, the viewport-scaled
// projection (+0x12c), its product with the view (+0xec) and the projection
// scale +0x198.
void Camera::UnknownVirtualSlot28() {
    viewMatrix = ViewMatrix(field_0x170, field_0x17c, field_0x188, roll);
    projectionMatrix = ProjectionMatrix(field_0x1bc, field_0x1c0, field_0x16c * 0.017453290f,
                                        (float)(unsigned int)field_0x1a0[2] / (unsigned int)field_0x1a0[3]);
    Matrix4 viewport = IdentityMatrix();
    viewport(0, 0) = 0.5f;
    viewport(1, 1) = -0.5f;
    viewport(3, 0) = 0.5f;
    viewport(3, 1) = 0.5f;
    field_0xec[1] = MatrixMult(viewport, projectionMatrix);
    field_0xec[0] = MatrixMult(field_0xec[1], viewMatrix);
    focalLength = (float)(unsigned int)field_0x1a0[2] * 0.5f / (float)tan((float)(field_0x16c * 0.5f) * 0.017453290f);
}
