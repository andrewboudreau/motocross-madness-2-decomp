// Near-miss Camera candidates (src/reconstructed/Camera.cpp's class), kept out
// of src/reconstructed until they match. See docs/OVERLAY.md.
//
// Camera::UpdateMatrices (0x0042ee30, 492 bytes, 487/492): one
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
// In the second MatrixMult both compilers hoist the `sub esp, 0x40` over the
// copy of `b`, whose space is left over from the first call; in the first
// call `b`'s space comes from `sub esp, 0x28` plus deferred argument pops, and
// only retail hoists there. Also worse: building the viewport (identity, or
// identity and stores) before the projection (29-40 of 492) and /G6
// (42 of 492); a `projection` copy right before the product is inert (487).
#include <math.h>

#include "../../src/reconstructed/Camera.h"

// 0x0042ee30: rebuilds the view and projection matrices from the camera
// frame, the field of view (degrees) and the viewport, the viewport-scaled
// projection (+0x12c), its product with the view (+0xec) and the projection
// scale +0x198.
void Camera::UpdateMatrices() {
    viewMatrix = ViewMatrix(eyePosition, viewDirection, upDirection, roll);
    projectionMatrix = ProjectionMatrix(nearPlane, farPlane, fieldOfView * 0.017453290f,
                                        (float)(unsigned int)viewport[2] / (unsigned int)viewport[3]);
    Matrix4 viewportScale = IdentityMatrix();
    viewportScale(0, 0) = 0.5f;
    viewportScale(1, 1) = -0.5f;
    viewportScale(3, 0) = 0.5f;
    viewportScale(3, 1) = 0.5f;
    screenMatrix[1] = MatrixMult(viewportScale, projectionMatrix);
    screenMatrix[0] = MatrixMult(screenMatrix[1], viewMatrix);
    focalLength = (float)(unsigned int)viewport[2] * 0.5f / (float)tan((float)(fieldOfView * 0.5f) * 0.017453290f);
}
