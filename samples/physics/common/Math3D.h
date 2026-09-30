// Math3D.h -- shared 3D math types and helpers used by the MCM2 physics code.
//
// Owner: opus_rigidbody (samples/physics/common). Other areas include this as
// "../common/Math3D.h". The API is append-only: names below will not be renamed.
//
// Evidence summary (see also SoultreeObject.h):
//  * Vec3 is three consecutive floats (12 bytes). Tier 1: every helper below reads
//    and writes x/y/z at +0/+4/+8, and 12-byte vectors are always returned through
//    a hidden result pointer (VC6 returns only 1/2/4/8-byte structs in registers).
//  * Matrix4 is a 4x4 float matrix, row-major, row-vector convention (D3D style,
//    v' = v * M, translation in row 3 = elements 12..14). Tier 1 for size and element
//    offsets (rep movsd of 16 dwords; SoultreeObject's constructor sets elements
//    0/5/10/15 to 1.0f and the rest to 0). Tier 2 for the convention: world = local *
//    parent (0x004fb4f0) and SetPosition writes elements 12..14 of the local matrix.
//  * The layout is identical to D3DVECTOR / D3DMATRIX, and the inline operators
//    mirror d3dvec.inl (D3D_OVERLOADS). Whether the game used the SDK types with
//    D3D_OVERLOADS is tier 3; codegen is the same either way for these helpers.
//
// VC6 note (/O2, no /Op): the compiler reassociates float sums, e.g. the source
// sum a*b + c*d + e*f is emitted as (c*d + e*f) + a*b. Write sums in natural x,y,z
// order and let the compiler reorder them; do not permute terms to chase bytes.
// Exception: explicit parentheses do constrain VC6. Vec3Normalize (0x005087b0)
// needs 'z*z + (x*x + y*y)' to get retail's (x*x + y*y) + z*z. Treat such
// grouping as evidence of how the source was written, not as term shuffling.
//
// Exact byte-match bodies for MatrixZero, IntegrateAdamsBashforth2, QuatDerivative
// and Vec3Normalize are in Math3D.cpp (samples/physics/common/targets.json).
#ifndef MCM2_PHYSICS_COMMON_MATH3D_H
#define MCM2_PHYSICS_COMMON_MATH3D_H

struct Vec3 {
    float x, y, z;

    Vec3() {}
    Vec3(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }

    float& operator[](int i) { return (&x)[i]; }
    const float& operator[](int i) const { return (&x)[i]; }

    Vec3& operator+=(const Vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    Vec3& operator-=(const Vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    Vec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
    Vec3& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }
};

inline Vec3 operator-(const Vec3& v) { return Vec3(-v.x, -v.y, -v.z); }
inline Vec3 operator+(const Vec3& a, const Vec3& b) { return Vec3(a.x + b.x, a.y + b.y, a.z + b.z); }
inline Vec3 operator-(const Vec3& a, const Vec3& b) { return Vec3(a.x - b.x, a.y - b.y, a.z - b.z); }
inline Vec3 operator*(const Vec3& v, float s) { return Vec3(s * v.x, s * v.y, s * v.z); }
inline Vec3 operator*(float s, const Vec3& v) { return Vec3(s * v.x, s * v.y, s * v.z); }
inline Vec3 operator/(const Vec3& v, float s) { return Vec3(v.x / s, v.y / s, v.z / s); }

inline float DotProduct(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline float SquareMagnitude(const Vec3& v) { return v.x * v.x + v.y * v.y + v.z * v.z; }

// Same component formula as d3dvec.inl CrossProduct. Confirmed shape: PhysicsRigidBody
// slot 36 (0x004cc230) computes r.y*F.z - r.z*F.y, r.z*F.x - r.x*F.z, r.x*F.y - r.y*F.x.
inline Vec3 CrossProduct(const Vec3& a, const Vec3& b)
{
    Vec3 r;
    r.x = a.y * b.z - a.z * b.y;
    r.y = a.z * b.x - a.x * b.z;
    r.z = a.x * b.y - a.y * b.x;
    return r;
}

// 4x4 row-major matrix (D3DMATRIX layout). _RC names: row R, column C.
// The empty user-declared default constructor matters for codegen (tier 2).
// D3DMATRIX under D3D_OVERLOADS has one too. With it, VC6 assigns a Matrix4 returned
// through a hidden pointer straight from the returned pointer, as retail does in
// PhysicsBody::SetInertia 0x004cbe40. Without it (a POD struct), VC6 inserts an
// extra 64-byte copy.
struct Matrix4 {
    Matrix4() {}
    union {
        struct {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };
        float m[4][4];
    };
};

// ---------------------------------------------------------------------------
// Free helpers (cdecl, float result in st(0)).
// ---------------------------------------------------------------------------

// 0x00460b50, 85 bytes. Table-driven square root: returns 0 for x == 0, otherwise
// builds the result from the halved exponent and a 256-entry mantissa table at
// 0x005dafe0 (top 7 mantissa bits + exponent parity). No Newton step. Tier 2
// semantics (decoded bit arithmetic), tier 3 name.
float FastSqrt(float x);

// 0x00460c00, 107 bytes. Reciprocal square root: table estimate (0x005daf5c,
// 128 entries, exponent 0x5f000000 - (e << 22)) refined by two Newton steps
// y = 0.5 * y * (3 - x*y*y). Tier 2 semantics, tier 3 name.
float FastInvSqrt(float x);

// ---------------------------------------------------------------------------
// Quaternions and matrix helpers (added after the first release; append-only).
// These live in the 0x004a1700..0x004a2300 matrix/quaternion library range. All are
// __cdecl. Struct results use a hidden result pointer passed as the FIRST stack
// argument (caller cleans), so e.g. QuatNormalize is called as push q; push &ret.
// ---------------------------------------------------------------------------

// Quaternion {w, x, y, z}: scalar part FIRST. Tier 2: QuatDerivative (0x004a1f90)
// computes result[+0] = -0.5 * (q[+4]*w.x + q[+8]*w.y + q[+12]*w.z), which is the
// scalar part of 0.5 * q (x) (0, w) only when +0 is the scalar and +4..+12 the vector.
// The same order is the one QuatToMatrix (0x004a1e10) needs to produce a rotation.
struct Quat {
    float w, x, y, z;
};

// 0x004a13e0, 41 bytes: returns an all-zero matrix (rep stosd into a local, copied
// out). This is NOT an identity matrix. Tier 1 behavior, tier 3 name.
Matrix4 MatrixZero();

// 0x004a18e0, 362 bytes: returns the inverse of m, taken BY VALUE (64 bytes on
// the stack; with the hidden pointer the caller cleans 0x44). The body is a
// cofactor/determinant computation. Tier 2 for "inverse". Whether it is a full 4x4
// or a 3x3 (rotation/inertia) inverse has not been checked.
Matrix4 MatrixInverse(Matrix4 m);

// 0x004a2040, 381 bytes: rotation matrix (upper 3x3 of m) -> unit quaternion by the
// trace/largest-diagonal (Shepperd) method (constants 1.0f and 0.25f). Tier 2.
Quat QuatFromMatrix(const Matrix4& m);

// 0x004a1e10, 196 bytes: quaternion -> rotation matrix (D3D row-vector convention,
// translation row zero). The scale is s = 2/|q|^2 when |q|^2 > 0, else 0. Tier 2.
Matrix4 QuatToMatrix(const Quat& q);

// 0x004a1d60, 173 bytes: q / |q|. Returns the identity quaternion (global at
// 0x006850b0, set up at run time) when |q|^2 == 0.0 (double compare). Tier 2.
Quat QuatNormalize(const Quat& q);

// 0x004a1f90, 169 bytes: time derivative of an orientation quaternion under the
// angular velocity w, dq/dt = 0.5 * q (x) (0, w). Tier 2 (decoded arithmetic).
Quat QuatDerivative(const Quat& q, const Vec3& w);

// State integrator callback, as stored in PhysicsBody (+0x238) and called by
// PhysicsRigidBody's step (__cdecl, 4 args, caller cleans 0x10). 'deriv' points at
// TWO consecutive n-float derivative blocks: deriv[0..n-1] (current) and
// deriv[n..2n-1] (previous step).
typedef void (*IntegrateFn)(float dt, int n, float* state, float* deriv);

// 0x004a1d10, 71 bytes: the default IntegrateFn installed by the PhysicsBody
// constructor. It is a two-step Adams-Bashforth step:
//   state[i] += (3*deriv[i] - deriv[n+i]) * dt * 0.5f;  deriv[n+i] = deriv[i];
// Tier 1 for the arithmetic (decoded), tier 3 for the name.
void IntegrateAdamsBashforth2(float dt, int n, float* state, float* deriv);

// Identity quaternion {1,0,0,0} at 0x006850b0 (.bss, filled at run time). Read by
// QuatNormalize for a zero-length input and by PhysicsRigidBody::ResetState.
// Tier 2 for the value (from its uses), tier 3 for the name.
extern Quat g_QuatIdentity;

// Row-vector rotation v * M using only the upper 3x3 (no translation):
//   r.x = v.x*_11 + v.y*_21 + v.z*_31, and so on.
// Inlined in the PhysicsRigidBody step, both for I*w (inertia) and for
// invInertia*tau. v is taken by value; retail copies the source vector to a
// stack local before the multiply. Tier 2 shape, tier 3 name.
inline Vec3 RotateVector(Vec3 v, const Matrix4& m)
{
    Vec3 r;
    r.x = v.x * m._11 + v.y * m._21 + v.z * m._31;
    r.y = v.x * m._12 + v.y * m._22 + v.z * m._32;
    r.z = v.x * m._13 + v.y * m._23 + v.z * m._33;
    return r;
}

// Per-translation-unit vector constants. Tier 2: 73 retail translation units
// (0x00403c20 ... 0x00533000) begin with the same four VC6 dynamic initializers,
// which build (0,0,0), (1,0,0), (0,1,0) and (0,0,1) in a stack temporary and copy
// it to a TU-private 12-byte .bss global. That shape is a namespace-scope
// 'static const Vec3 name(a,b,c);' in a widely included header: the user-declared
// constructor forces dynamic initialization, and VC6 emits $E initializers before
// the TU's own functions (verified by compiling a probe). PhysicsBody uses its TU's
// zero at 0x006899c0 and PhysicsRigidBody uses its TU's zero at 0x00689a00. Names
// are tier 3.
static const Vec3 kVec3Zero(0.0f, 0.0f, 0.0f);
static const Vec3 kVec3XAxis(1.0f, 0.0f, 0.0f);
static const Vec3 kVec3YAxis(0.0f, 1.0f, 0.0f);
static const Vec3 kVec3ZAxis(0.0f, 0.0f, 1.0f);

// ---------------------------------------------------------------------------
// More out-of-line helpers (added in the third revision; append-only).
// ---------------------------------------------------------------------------

// 0x005087b0, 150 bytes, __cdecl, hidden result pointer first. Returns v unchanged
// when |v|^2 == 1.0f exactly. Otherwise returns v * FastInvSqrt(|v|^2)
// (0x00460c00). Tier 1 arithmetic. The name is tier 3 and was chosen so it does not
// collide with local helpers in other areas. Body in Math3D.cpp.
Vec3 Vec3Normalize(const Vec3& v);

// Out-of-line (non-inlined COMDAT) instances of the inline helpers above. These
// are the same functions, so they get no separate declarations (tier 2, identical
// decoded bodies):
//  * 0x00404e60: Vec3::Vec3(float x, float y, float z), __thiscall, ret 0xc.
//  * 0x00515600: CrossProduct(const Vec3&, const Vec3&), __cdecl, hidden result
//    pointer first. Its components match the inline formula.
//
// 0x0042a1a0: 4x4 matrix product. __cdecl, a destination pointer first, then two
// Matrix4 BY VALUE (add esp,0x84 at the callers). SoultreeObject 0x004fc050 passes
// &localMatrix straight in as the destination and does not copy afterwards, so
// this is modelled as an out-parameter rather than a struct return (tier 2).
// Operand order and naming are tier 3: out = a * b in the row-vector convention.
void MatrixMultiply(Matrix4* out, Matrix4 a, Matrix4 b);

// 0x004b5a60, 657 bytes, __cdecl: (Vec3 a, Vec3 b, float* out0 ... ) with seven
// pointer arguments after the two by-value vectors. It returns at once if any of
// the first three pointers is null. It derives three angles with the CRT x87
// intrinsic at 0x00535240 (an _CIacos/_CIasin-style helper, argument in st(0)).
// The first angle comes from (a.x, a.z) / sqrt(a.x^2 + a.z^2), with a fallback of
// (1, 0) at zero length. Not reconstructed: the exact pointer roles and the second
// vector's role are open. The name and parameters are provisional (tier 3).
void UnknownVectorsToAngles_4b5a60(Vec3 a, Vec3 b, float* out0, float* out1,
                                   float* out2, float* out3, float* out4,
                                   float* out5, float* out6);

// 0x004cb6e0, 892 bytes, __cdecl: per-axis settle/clamp of two accumulators.
// Arguments: (float* a, float* b, const float* c, float dt, unsigned char mask), each
// pointer being 3 floats. For each axis k (bit 1<<k of mask), when c[k] != 0:
//   * mask bit clear and a[k] == 0: if b[k] != 0 and sign(b[k]) != sign(c[k]),
//     then b[k] += c[k] if |b[k]| > |c[k]|, else b[k] = 0. Nothing happens when the
//     signs are equal or b[k] == 0.
//   * otherwise, with t = a[k] + dt*b[k] and d = dt*c[k]: b[k] += c[k] when
//     sign(t) == sign(d), when |t| > |d|, or when t == 0. Otherwise a[k] = b[k] = 0.
// This reads like a static-friction or stop-at-zero integrator guard. Tier 2
// arithmetic (decoded from the x axis; y and z repeat it with bits 2 and 4). The
// name and parameter roles are tier 3.
void UnknownAxisSettle_4cb6e0(float* a, float* b, const float* c, float dt,
                              unsigned char mask);

// Scene-graph node helpers (local/world transforms) are SoultreeObject methods;
// see SoultreeObject.h, which physics code reaches through node pointers.
#include "SoultreeObject.h"

#endif
