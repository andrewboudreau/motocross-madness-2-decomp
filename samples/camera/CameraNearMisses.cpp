// Near-miss Camera candidates (src/reconstructed/Camera.cpp's class), kept out
// of src/reconstructed until they match. See docs/OVERLAY.md.
//
// Camera::UnknownVirtualSlot29 (0x0042eb10, 690 bytes, 659/690): identical
// length, frame and slot layout. Four of the twelve fld/fmul operand pairs of
// the first cross product (up x forward) load the other operand first
// (retail loads up first unless the pair involves up.x). Written operand
// order, member vs operator[] access, Vector3(...) vs member-assigned results,
// swapped parameter roles, pointer parameters and a cross written in the body
// all leave four pairs wrong (or lose the forward pointer in esi).
// By-value Cross parameters (either or both) and a by-value normalisation copy
// the vectors (97..232/690).
// Established: the direction through an inline difference passed straight to
// the out-of-line-dot normalisation, the right vector's z + (x + y) squared
// length and its scale through the 0x00404e60 constructor copy.
//
// Camera::UnknownVirtualSlot28 (0x0042ee30, 492 bytes, 487/492): one
// scheduling difference: retail emits `sub esp, 0x40` before the `rep movsd`
// that copies the projection matrix (MatrixMult's second argument), VC6 after.
// Assigning through a named product, a nested assignment, m[][] stores, the
// viewport built by an inline helper and the store order do not move it.
// The half field of view needs the (float) cast: without it VC6 folds
// 0.5f * 0.01745329f into one constant, while retail multiplies twice.
#include <math.h>

#include "../../src/reconstructed/Camera.h"

float FastInvSqrt(float value); // 0x00460c00

// 0x0040ae30 (cdecl): out-of-line dot product.
float UnknownFunction40ae30(const Vector3* a, const Vector3* b);

// Vector3 seen through the out-of-line (x, y, z) constructor copy 0x00404e60.
struct CameraVec3Call : Vector3 {
    CameraVec3Call(float x_, float y_, float z_);
};

// v scaled to unit length through the out-of-line dot product.
static inline Vector3 NormalizeCall(const Vector3& v) {
    float squared = UnknownFunction40ae30(&v, &v);
    if (squared == 1.0f)
        return v;
    float scale = FastInvSqrt(squared);
    return Vector3(v.x * scale, v.y * scale, v.z * scale);
}

static inline Vector3 Diff(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline Vector3 Cross(const Vector3& a, const Vector3& b) {
    Vector3 r;
    r.x = a.y * b.z - a.z * b.y;
    r.y = a.z * b.x - a.x * b.z;
    r.z = a.x * b.y - a.y * b.x;
    return r;
}

// v scaled to unit length through the constructor copy.
static inline Vector3 NormalizeInline(const Vector3& v) {
    float squared = v.z * v.z + (v.x * v.x + v.y * v.y);
    if (squared == 1.0f)
        return v;
    float scale = FastInvSqrt(squared);
    return CameraVec3Call(scale * v.x, scale * v.y, scale * v.z);
}

// 0x0042eb10: aims the camera at `target`. Returns 0 when the target is the
// camera position; otherwise the forward direction becomes the unit vector to
// the target, the up vector is rebuilt from (0, 1, 0) through the right
// vector (the x axis when looking straight up or down) and the projection is
// marked for refresh.
int Camera::UnknownVirtualSlot29(Vector3 target) {
    if (target.x == field_0x170.x && target.y == field_0x170.y && target.z == field_0x170.z)
        return 0;
    field_0x188.x = 0.0f;
    field_0x188.y = 1.0f;
    field_0x188.z = 0.0f;
    field_0x17c = NormalizeCall(Diff(target, field_0x170));
    float y = field_0x17c.y;
    if (y < 0.0f)
        y = -y;
    Vector3 right;
    if (y != 1.0f)
        right = NormalizeInline(Cross(field_0x188, field_0x17c));
    else
        right = Vector3(1.0f, 0.0f, 0.0f);
    field_0x188 = Cross(field_0x17c, right);
    field_0x1d0 = Owner()->field_0x14 + 1;
    return 1;
}

// 0x0042ee30: rebuilds the view and projection matrices from the camera
// frame, the field of view (degrees) and the viewport, the viewport-scaled
// projection (+0x12c), its product with the view (+0xec) and the projection
// scale +0x198.
void Camera::UnknownVirtualSlot28() {
    viewMatrix = ViewMatrix(field_0x170, field_0x17c, field_0x188, field_0x194);
    projectionMatrix = ProjectionMatrix(field_0x1bc, field_0x1c0, field_0x16c * 0.017453290f,
                                        (float)(unsigned int)field_0x1a0[2] / (unsigned int)field_0x1a0[3]);
    Matrix4 viewport = IdentityMatrix();
    viewport(0, 0) = 0.5f;
    viewport(1, 1) = -0.5f;
    viewport(3, 0) = 0.5f;
    viewport(3, 1) = 0.5f;
    field_0xec[1] = MatrixMult(viewport, projectionMatrix);
    field_0xec[0] = MatrixMult(field_0xec[1], viewMatrix);
    field_0x198 = (float)(unsigned int)field_0x1a0[2] * 0.5f / (float)tan((float)(field_0x16c * 0.5f) * 0.017453290f);
}
