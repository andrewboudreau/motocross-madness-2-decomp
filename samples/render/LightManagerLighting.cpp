// LightManagerLighting.cpp -- LightManager::LightVertices (0x0049e4a0, ret 0x18) and its
// three private workers LightAny 0x0049e6b0, LightOne 0x0049f1c0 and LightAmbientPlusOne
// 0x0049f8d0, the terrain's vertex lighting, of D:\aardvark\VC\krusty2\LightEmitter.cpp
// (literal __FILE__ 0x0056ddf0; canonical source src/reconstructed/LightEmitter.cpp, whose
// own colour casts truncate through __ftol, does not define them).  Status:
// reviewed-nonexact.  The game build links this file with /QIfist (config/game_build.json
// candidate_sources) in place of the former src/bringup/Group4_GridLighting.cpp copy.
//
// LightManager::LightVertices (LightEmitter.h) is called by the terrain
// through Terrain+0xc40, the particle renderer and D3DIMSoulTree. The three
// workers 0x0049e6b0 (any lights), 0x0049f1c0 (one light) and 0x0049f8d0
// (ambient plus one light) are private LightManager members; nothing else
// calls them.
//
// Retail rounds light terms with an inlined x87 `fistp` (current rounding
// mode, round-to-nearest-even) and converts the light colours with _ftol
// (truncation); the `- 0.5f` sites are retail's own. The game build compiles
// this file with /QIfist, so FistpFloatToInt's cast is a FISTP under the current
// rounding mode. The truncating light-colour casts live in LightColorByte.cpp
// next to it, compiled without the flag, so every float-to-int conversion left
// here is a fistp site.
// Range: each term is a colour byte (a light colour times 255, truncated) times
// a facing value, the dot product of a vertex normal with a unit light vector
// (point and positional lights normalise it; a directional light uses its
// direction rotated by the rigid inverse) or a per-vertex sum / 256 times a
// colour byte, so the values stay far inside int for any light colour near the
// 0..1 range (inference from the data flow; colours far outside it would be
// needed to leave int).
//
// The workers follow retail's out-of-line calls (confirmed call targets):
// 0x0040ae30 Vec3Dot, 0x00460c00 FastInvSqrt, 0x005015b0 Vec3ScaleCall,
// 0x00404e60 Vector3's constructor, 0x0049f180 unary operator- and 0x0042a450
// Vec3TransformNormal. They matter to behaviour, not only to shape: VC6 groups
// the three-term sums of an out-of-line body differently from the same sums
// written inline (0x0042a450 adds z*m31 + y*m21 first, the inlined rotation in
// 0x0049f8d0 adds x*m11 + z*m31 first), and each grouping rounds differently
// in the last bit, which can move a colour by one.
#include "../../src/reconstructed/LightEmitter.h"
#include "../../src/reconstructed/Matrix4.h"
#include "core/DebugAlloc.h"
#include "math/FastMath.h"
#include "../common/FistpFloatToInt.h"

// LightEmitter.h's lightType values the workers distinguish.
enum {
    kLightPoint = 1,        // position, per-vertex direction
    kLightPositional = 2,   // position used as one direction
    kLightDirectional = 4,  // lightDirection
    kLightAmbient = 6       // constant colour
};

// 0x0040ae30 (cdecl, (a.y*b.y + a.x*b.x) + a.z*b.z); config/game_build.json
// links this view to Vec3DotCall.
float Vec3Dot(const Vector3* a, const Vector3* b);
// 0x005015b0 (cdecl, hidden result pointer): v * s; linked to Vec3ScaleCall.
Vector3 Vec3ScaleCall(const Vector3& v, float s);
// 0x0042a450 (cdecl, Vector3 by value): v times the matrix's upper 3x3; linked
// to the Vec3 definition in src/krusty2/bvh/BoundingBoxTreeQuery.cpp.
void Vec3TransformNormal(Vector3* out, Vector3 v, const Matrix4* m);
// Vector3::Vector3(float, float, float), 0x00404e60: the out-of-line copy
// retail calls here; linked to Vec3Call's constructor (the same body).
struct Vector3Call : Vector3 {
    Vector3Call(float x_, float y_, float z_);
};

// Lit vertices are 0x20 bytes: position at +0, packed diffuse at +0x10.
// Normals come from a separate pointer and stride (the terrain passes the
// same buffer + 0xc with stride 0x20).
static inline const Vector3* LightingVertexPosition(void* buffer, int i) {
    return (const Vector3*)((char*)buffer + i * 0x20);
}
static inline unsigned int* LightingVertexDiffuse(void* buffer, int i) {
    return (unsigned int*)((char*)buffer + i * 0x20 + 0x10);
}
static inline const Vector3* LightingVertexNormal(void* normals, int stride, int i) {
    return (const Vector3*)((char*)normals + i * stride);
}

static inline int LightingClamp255(int value) {
    return value < 0xff ? value : 0xff;
}

// Opaque packed colour; the red term is or-ed with 0xffffff00 first, so an
// in-range red gives alpha 0xff.
static inline unsigned int LightingPackOpaque(int r, int g, int b) {
    return (((r | 0xffffff00) << 8 | g) << 8) | b;
}

// __forceinline here and on LightingTransformPointAny: 0x0049e6b0 expands
// both at every site, but its later sites fall outside VC6's per-function
// inline budget (docs/VC6_INLINE_BUDGET.md) and would otherwise be calls.
static __forceinline float LightingDot(const Vector3& a, const Vector3* b) {
    return a.z * b->z + (a.x * b->x + a.y * b->y);
}

// Retail's facing values outlive the inline-asm fistp blocks that follow
// them, so VC6 kept them in memory: `fst [facing]` compares the register
// value with 0 and every colour multiply reads the binary32 copy. Without the
// asm blocks VC6 keeps the unrounded register value instead, which moves
// exact ties such as 63 * 1.0000000477 - 0.5 by one (0.6f/0.8f axis normals
// under 53-bit precision). The memcpy gives the binary32 copy (see
// FistpFloatToInt.h).
static inline float LightingBinary32(float value) {
    float stored;
    memcpy(&stored, &value, sizeof(stored));
    return stored;
}

// Retail's inline fistp helper (FISTP under the current rounding mode).
static inline int LightingRound(float value) {
    int result;
    FistpFloatToInt(&result, value);
    return result;
}

// One channel of a light's colour scaled to 0..255 (retail: _ftol of
// colour * 255, inlined). Defined in LightColorByte.cpp, which is compiled
// without /QIfist so the cast keeps retail's truncation.
int LightingColorByte(float channel);

// Retail fills the per-vertex sums sixteen ints at a time, entering the first
// block through a jump table on the remainder (Duff's device; the stores run
// from the block's last int down to its first).
static inline void LightingFill(int* p, int value, int n) {
    if (n > 0) {
        do {
            switch (n) {
            default: p[15] = value;
            case 15: p[14] = value;
            case 14: p[13] = value;
            case 13: p[12] = value;
            case 12: p[11] = value;
            case 11: p[10] = value;
            case 10: p[9] = value;
            case 9: p[8] = value;
            case 8: p[7] = value;
            case 7: p[6] = value;
            case 6: p[5] = value;
            case 5: p[4] = value;
            case 4: p[3] = value;
            case 3: p[2] = value;
            case 2: p[1] = value;
            case 1: p[0] = value;
            }
            p += 16;
            n -= 16;
        } while (n > 0);
    }
}

// Inverse of a rigid object transform (row-major 4x4, row vector times
// matrix), in place on the workers' copy: the rotation transposed and the
// translation rotated back, all three components computed before any is
// stored.
//
// The parentheses below are retail's groupings, decoded from each worker's
// fadd order. VC6 /O2 regroups an unparenthesised three-term sum by its own
// register heuristics (0x0042a450's source order x, y, z compiles to
// (z + y) + x), and the workers do not all agree: 0x0049e6b0 and 0x0049f8d0
// translate with (x*m11 + z*m31) + y*m21 and (z*m32 + y*m22) + x*m12, 0x0049f1c0
// with (z*m31 + y*m21) + x*m11 and (y*m22 + x*m12) + z*m32. Each grouping
// rounds differently in the last bit, and at the 24-bit precision Direct3D
// sets that can move a colour by one, so they are spelled out.
static inline void LightingInvertRigid(Matrix4& m) {
    float t;
    t = m._12;
    m._12 = m._21;
    m._21 = t;
    t = m._13;
    m._13 = m._31;
    m._31 = t;
    t = m._23;
    m._23 = m._32;
    m._32 = t;
    float x = -((m._41 * m._11 + m._43 * m._31) + m._42 * m._21);
    float y = -((m._43 * m._32 + m._42 * m._22) + m._41 * m._12);
    float z = -((m._43 * m._33 + m._42 * m._23) + m._41 * m._13);
    m._41 = x;
    m._42 = y;
    m._43 = z;
}
static inline void LightingInvertRigidOne(Matrix4& m) {
    float t;
    t = m._12;
    m._12 = m._21;
    m._21 = t;
    t = m._13;
    m._13 = m._31;
    m._31 = t;
    t = m._23;
    m._23 = m._32;
    m._32 = t;
    float x = -((m._43 * m._31 + m._42 * m._21) + m._41 * m._11);
    float y = -((m._42 * m._22 + m._41 * m._12) + m._43 * m._32);
    float z = -((m._43 * m._33 + m._42 * m._23) + m._41 * m._13);
    m._41 = x;
    m._42 = y;
    m._43 = z;
}

// A point times the matrix, translation included (retail copies the point
// first, as for a by-value argument). Retail's groupings again: 0x0049e6b0
// sums the second column as (y*m22 + z*m32) + x*m12, the other two workers as
// (y*m22 + x*m12) + z*m32.
static __forceinline void LightingTransformPointAny(Vector3* out, Vector3 v, const Matrix4& m) {
    out->x = ((v.z * m._31 + v.y * m._21) + v.x * m._11) + m._41;
    out->y = ((v.y * m._22 + v.z * m._32) + v.x * m._12) + m._42;
    out->z = ((v.z * m._33 + v.y * m._23) + v.x * m._13) + m._43;
}
static inline void LightingTransformPoint(Vector3* out, Vector3 v, const Matrix4& m) {
    out->x = ((v.z * m._31 + v.y * m._21) + v.x * m._11) + m._41;
    out->y = ((v.y * m._22 + v.x * m._12) + v.z * m._32) + m._42;
    out->z = ((v.z * m._33 + v.y * m._23) + v.x * m._13) + m._43;
}

// The normalisations retail skips when the squared length is exactly 1. The
// first scales through 0x005015b0, the second through Vector3's out-of-line
// constructor (both appear in retail).
static inline Vector3 LightingNormalizedByScaleCall(const Vector3& v) {
    float lengthSquared = Vec3Dot(&v, &v);
    if (lengthSquared == 1.0f)
        return v;
    return Vec3ScaleCall(v, FastInvSqrt(lengthSquared));
}
static inline Vector3 LightingNormalizedByConstructor(const Vector3& v) {
    float lengthSquared = Vec3Dot(&v, &v);
    if (lengthSquared == 1.0f)
        return v;
    float scale = FastInvSqrt(lengthSquared);
    return Vector3Call(v.x * scale, v.y * scale, v.z * scale);
}

// 0x0049e6b0: any number of lights. The first ambient light seeds the
// per-vertex sums (zero without one); point, positional and directional
// lights add their rounded diffuse terms where they face the vertex; the sums
// are clamped and packed.
// Retail's inline-asm fistp helper gives each function below an EBP frame; the
// pragma keeps the frame pointer for them (docs/GAME_BUILD.md#frame-pointer-units).
#pragma optimize("y", off)
void LightManager::LightAny(const Matrix4* matrix, int count, void* normals, void* buffer,
                              int stride) {
    LightManager* manager = this;
    Matrix4 inverse = *matrix;
    int i;
    int light;
    LightingInvertRigid(inverse);

    for (light = 0; light < manager->lightCount; light++) {
        if (manager->lights[light]->lightType == kLightAmbient)
            break;
    }
    if (light != manager->lightCount) {
        LightEmitter* ambient = manager->lights[light];
        int ambientR = LightingColorByte(ambient->ambientRGBA[0]);
        int ambientG = LightingColorByte(ambient->ambientRGBA[1]);
        int ambientB = LightingColorByte(ambient->ambientRGBA[2]);
        LightingFill(manager->vertexRed, ambientR, count);
        LightingFill(manager->vertexGreen, ambientG, count);
        LightingFill(manager->vertexBlue, ambientB, count);
    } else {
        LightingFill(manager->vertexRed, 0, count);
        LightingFill(manager->vertexGreen, 0, count);
        LightingFill(manager->vertexBlue, 0, count);
    }

    for (light = 0; light < manager->lightCount; light++) {
        LightEmitter* emitter = manager->lights[light];
        int r = LightingColorByte(emitter->ambientRGBA[0]);
        int g = LightingColorByte(emitter->ambientRGBA[1]);
        int b = LightingColorByte(emitter->ambientRGBA[2]);
        switch (emitter->lightType) {
        case kLightDirectional: {
            Vector3 direction;
            Vec3TransformNormal(&direction, -emitter->lightDirection, &inverse);
            for (i = 0; i < count; i++) {
                float dot = Vec3Dot(LightingVertexNormal(normals, stride, i), &direction);
                float facing = LightingBinary32(dot);
                if (dot > 0.0f) {
                    manager->vertexRed[i] += LightingRound((float)r * facing);
                    manager->vertexGreen[i] += LightingRound((float)g * facing);
                    manager->vertexBlue[i] += LightingRound((float)b * facing);
                }
            }
            break;
        }
        case kLightPositional: {
            Vector3 position;
            LightingTransformPointAny(&position, emitter->lightPosition, inverse);
            position = LightingNormalizedByScaleCall(position);
            for (i = 0; i < count; i++) {
                float dot = LightingDot(position, LightingVertexNormal(normals, stride, i));
                float facing = LightingBinary32(dot);
                if (dot > 0.0f) {
                    manager->vertexRed[i] += LightingRound((float)r * facing);
                    manager->vertexGreen[i] += LightingRound((float)g * facing);
                    manager->vertexBlue[i] += LightingRound((float)b * facing);
                }
            }
            break;
        }
        case kLightPoint: {
            Vector3 position;
            LightingTransformPointAny(&position, emitter->lightPosition, inverse);
            for (i = 0; i < count; i++) {
                const Vector3* vertex = LightingVertexPosition(buffer, i);
                Vector3 direction = LightingNormalizedByScaleCall(
                    Vector3Call(position.x - vertex->x, position.y - vertex->y,
                                position.z - vertex->z));
                float dot = LightingDot(direction, LightingVertexNormal(normals, stride, i));
                float facing = LightingBinary32(dot);
                if (dot > 0.0f) {
                    manager->vertexRed[i] += LightingRound((float)r * facing);
                    manager->vertexGreen[i] += LightingRound((float)g * facing);
                    manager->vertexBlue[i] += LightingRound((float)b * facing);
                }
            }
            break;
        }
        }
    }

    for (i = 0; i < count; i++) {
        *LightingVertexDiffuse(buffer, i) = LightingPackOpaque(
            LightingClamp255(manager->vertexRed[i]), LightingClamp255(manager->vertexGreen[i]),
            LightingClamp255(manager->vertexBlue[i]));
    }
}

// 0x0049f1c0: a single light writes the diffuse colours directly (the
// per-vertex sums are left alone). Point and positional lights leave
// back-facing vertices unwritten; a directional light writes every vertex,
// back-facing ones with negative terms (retail does not clamp them).
void LightManager::LightOne(const Matrix4* matrix, int count, void* normals, void* buffer,
                              int stride) {
    LightManager* manager = this;
    LightEmitter* emitter = manager->lights[0];
    int i;
    int r = LightingColorByte(emitter->ambientRGBA[0]);
    int g = LightingColorByte(emitter->ambientRGBA[1]);
    int b = LightingColorByte(emitter->ambientRGBA[2]);
    switch (emitter->lightType) {
    case kLightPoint: {
        Matrix4 inverse = *matrix;
        Vector3 position;
        LightingInvertRigidOne(inverse);
        LightingTransformPoint(&position, emitter->lightPosition, inverse);
        for (i = 0; i < count; i++) {
            const Vector3* vertex = LightingVertexPosition(buffer, i);
            Vector3 direction = LightingNormalizedByScaleCall(Vector3(
                position.x - vertex->x, position.y - vertex->y, position.z - vertex->z));
            float dot = LightingDot(direction, LightingVertexNormal(normals, stride, i));
            float facing = LightingBinary32(dot);
            if (dot > 0.0f) {
                *LightingVertexDiffuse(buffer, i) =
                    LightingPackOpaque(LightingRound((float)r * facing - 0.5f),
                                         LightingRound((float)g * facing - 0.5f),
                                         LightingRound((float)b * facing - 0.5f));
            }
        }
        break;
    }
    case kLightPositional: {
        Matrix4 inverse = *matrix;
        Vector3 position;
        LightingInvertRigidOne(inverse);
        LightingTransformPoint(&position, emitter->lightPosition, inverse);
        position = LightingNormalizedByScaleCall(position);
        for (i = 0; i < count; i++) {
            float dot = LightingDot(position, LightingVertexNormal(normals, stride, i));
            float facing = LightingBinary32(dot);
            if (dot > 0.0f) {
                *LightingVertexDiffuse(buffer, i) =
                    LightingPackOpaque(LightingRound((float)r * facing - 0.5f),
                                         LightingRound((float)g * facing - 0.5f),
                                         LightingRound((float)b * facing - 0.5f));
            }
        }
        break;
    }
    case kLightDirectional: {
        Matrix4 inverse = *matrix;
        Vector3 direction;
        LightingInvertRigidOne(inverse);
        Vec3TransformNormal(&direction, -emitter->lightDirection, &inverse);
        float red = (float)r;
        float green = (float)g;
        float blue = (float)b;
        for (i = 0; i < count; i++) {
            float facing =
                LightingBinary32(Vec3Dot(LightingVertexNormal(normals, stride, i), &direction));
            *LightingVertexDiffuse(buffer, i) =
                LightingPackOpaque(LightingRound(red * facing - 0.5f),
                                     LightingRound(green * facing - 0.5f),
                                     LightingRound(blue * facing - 0.5f));
        }
        break;
    }
    case kLightAmbient: {
        unsigned int color = LightingPackOpaque(r, g, b);
        for (i = 0; i < count; i++)
            *LightingVertexDiffuse(buffer, i) = color;
        break;
    }
    }
}

// One vertex of 0x0049f8d0: ambient plus the other light's rounded term where
// the vertex faces it (`dot` > 0), otherwise the ambient colour alone. When a
// colour array is pending (storeVertexColors) the clamped sums are kept per vertex
// too. __forceinline: retail expands it in each of the three loops.
static __forceinline void LightingOverAmbient(LightManager* manager, void* buffer, int i,
                                                    float dot, int r, int g, int b,
                                                    int ambientR, int ambientG, int ambientB,
                                                    unsigned int ambientColor) {
    float facing = LightingBinary32(dot);
    if (dot > 0.0f) {
        int red = LightingRound((float)r * facing) + ambientR;
        int green = LightingRound((float)g * facing) + ambientG;
        int blue = LightingRound((float)b * facing) + ambientB;
        *LightingVertexDiffuse(buffer, i) = LightingPackOpaque(
            LightingClamp255(red), LightingClamp255(green), LightingClamp255(blue));
        if (manager->storeVertexColors) {
            manager->vertexRed[i] = LightingClamp255(red);
            manager->vertexGreen[i] = LightingClamp255(green);
            manager->vertexBlue[i] = LightingClamp255(blue);
        }
    } else {
        *LightingVertexDiffuse(buffer, i) = ambientColor;
        if (manager->storeVertexColors) {
            manager->vertexRed[i] = ambientR;
            manager->vertexGreen[i] = ambientG;
            manager->vertexBlue[i] = ambientB;
        }
    }
}

// 0x0049f8d0: two lights, one of them ambient (the caller guarantees it).
// The colours of both are taken where the search finds the ambient one; each
// light type then has its own vertex loop, as in retail.
void LightManager::LightAmbientPlusOne(const Matrix4* matrix, int count, void* normals,
                                         void* buffer, int stride) {
    LightManager* manager = this;
    Matrix4 inverse = *matrix;
    int i;
    int ambientR, ambientG, ambientB;
    unsigned int ambientColor;
    int other;
    int r, g, b;
    LightingInvertRigid(inverse);
    // Without an ambient light `other` and the colours stay unset (retail
    // reads an unrelated stack value as the index); the caller rules that out.
    for (i = 0; i < manager->lightCount; i++) {
        if (manager->lights[i]->lightType == kLightAmbient) {
            LightEmitter* ambient = manager->lights[i];
            ambientR = LightingColorByte(ambient->ambientRGBA[0]);
            ambientG = LightingColorByte(ambient->ambientRGBA[1]);
            ambientB = LightingColorByte(ambient->ambientRGBA[2]);
            ambientColor = LightingPackOpaque(ambientR, ambientG, ambientB);
            other = 1 - i;
            LightEmitter* emitter = manager->lights[other];
            r = LightingColorByte(emitter->ambientRGBA[0]);
            g = LightingColorByte(emitter->ambientRGBA[1]);
            b = LightingColorByte(emitter->ambientRGBA[2]);
            break;
        }
    }

    LightEmitter* emitter = manager->lights[other];
    switch (emitter->lightType) {
    case kLightDirectional: {
        // Retail inlines the negation and the rotation here, grouping each
        // component as (x*m1j + z*m3j) + y*m2j, (z*m32 + y*m22) + x*m12 and
        // (z*m33 + y*m23) + x*m13.
        float x = -emitter->lightDirection.x;
        float y = -emitter->lightDirection.y;
        float z = -emitter->lightDirection.z;
        Vector3 toLight;
        toLight.x = (x * inverse._11 + z * inverse._31) + y * inverse._21;
        toLight.y = (z * inverse._32 + y * inverse._22) + x * inverse._12;
        toLight.z = (z * inverse._33 + y * inverse._23) + x * inverse._13;
        for (i = 0; i < count; i++) {
            float dot = LightingDot(toLight, LightingVertexNormal(normals, stride, i));
            LightingOverAmbient(manager, buffer, i, dot, r, g, b, ambientR, ambientG,
                                       ambientB, ambientColor);
        }
        break;
    }
    case kLightPositional: {
        Vector3 toLight;
        LightingTransformPoint(&toLight, emitter->lightPosition, inverse);
        toLight = LightingNormalizedByConstructor(toLight);
        for (i = 0; i < count; i++) {
            float dot = LightingDot(toLight, LightingVertexNormal(normals, stride, i));
            LightingOverAmbient(manager, buffer, i, dot, r, g, b, ambientR, ambientG,
                                       ambientB, ambientColor);
        }
        break;
    }
    case kLightPoint: {
        Vector3 position;
        LightingTransformPoint(&position, emitter->lightPosition, inverse);
        for (i = 0; i < count; i++) {
            const Vector3* vertex = LightingVertexPosition(buffer, i);
            Vector3 toLight = LightingNormalizedByConstructor(
                Vector3(position.x - vertex->x, position.y - vertex->y, position.z - vertex->z));
            float dot = LightingDot(toLight, LightingVertexNormal(normals, stride, i));
            LightingOverAmbient(manager, buffer, i, dot, r, g, b, ambientR, ambientG,
                                       ambientB, ambientColor);
        }
        break;
    }
    }
}

// 0x0049e4a0 (ret 0x18). Lights `count` vertices of `buffer` (normals at
// `vertices` + i * stride) under the object transform `matrix`, growing the
// per-vertex colour sums first. With a colour array (`colors`, the same
// 0x20-byte vertex layout) each vertex's diffuse becomes that colour scaled
// by the per-vertex sums / 256.
// Audited against retail for the linked game (2026-11-11): the +0x114 flag, the
// buffer growth (free/alloc lines 0x13c/0x143, capacity cleared on failure), the
// worker dispatch (0x0049f1c0 for one light, 0x0049f8d0 for two with an ambient
// one, else 0x0049e6b0, each called out of line as here) and the modulation loop
// (no alpha) agree. Shape: retail stores the flag with a branch (if/else, not a
// select), calls the workers thiscall on `this` with the five leading arguments,
// converts the colour bytes as unsigned (dead high-dword zero store, fimul) and
// scales red/256 before the byte multiply (unparenthesised, VC6 multiplies the
// byte first; exact either way since 1/256 is a power of two; the parentheses
// raise match_one 58 -> 60 aligned but the linked difflib score, then dominated by
// the old X87FloatToInt emulation, read 0.458 vs 0.477 without). With the
// LightingRound FISTP roundings (/QIfist) the body is 163 built vs 167 retail
// instructions (masked 0.721); the rest is retail's ebp frame and its
// fstp/fld/fistp-dword helper sequence, which comes from the inline-asm fistp.
// Ruled out: `/ 256.0f`, `0.00390625f` (same code as `* (1.0f / 256.0f)`); a
// separate colour pointer local (worse). Session 23: the colour read per channel and
// each channel rounded into its variable makes the colour array the loop's induction
// pointer, as retail (0.728 -> 0.731 masked); VC6 still CSEs the three colour loads
// that retail's asm fistp stores keep apart.
void LightManager::LightVertices(void* matrix, int count, void* vertices, void* buffer, int stride,
                                 void* colors) {
    LightManager* manager = this;
    if (colors)
        manager->storeVertexColors = 1;
    else
        manager->storeVertexColors = 0;

    if (count > manager->vertexCapacity) {
        if (manager->vertexRed)
            DebugFree(manager->vertexRed, "D:\\aardvark\\VC\\krusty2\\LightEmitter.cpp", 0x13c);
        manager->vertexRed = (int*)DebugMalloc(count * 3 * sizeof(int),
                                               "D:\\aardvark\\VC\\krusty2\\LightEmitter.cpp", 0x143);
        manager->vertexGreen = manager->vertexRed + count;
        manager->vertexBlue = manager->vertexGreen + count;
        if (!manager->vertexRed) {
            manager->vertexCapacity = 0;
            return;
        }
        manager->vertexCapacity = count;
    }

    const Matrix4* m = (const Matrix4*)matrix;
    if (manager->lightCount == 1)
        LightOne(m, count, vertices, buffer, stride);
    else if (manager->lightCount == 2 && (manager->lights[0]->lightType == kLightAmbient ||
                                          manager->lights[1]->lightType == kLightAmbient))
        LightAmbientPlusOne(m, count, vertices, buffer, stride);
    else
        LightAny(m, count, vertices, buffer, stride);

    if (colors) {
        for (int i = 0; i < count; i++) {
            // Retail reads the colour per channel and rounds each channel into its
            // variable before the next (the colour array is the loop's pointer).
            int r, g, b;
            FistpFloatToInt(&r, ((float)manager->vertexRed[i] * (1.0f / 256.0f)) *
                                    (float)((*LightingVertexDiffuse(colors, i) >> 16) & 0xff) - 0.5f);
            FistpFloatToInt(&g, ((float)manager->vertexGreen[i] * (1.0f / 256.0f)) *
                                    (float)((*LightingVertexDiffuse(colors, i) >> 8) & 0xff) - 0.5f);
            FistpFloatToInt(&b, ((float)manager->vertexBlue[i] * (1.0f / 256.0f)) *
                                    (float)(*LightingVertexDiffuse(colors, i) & 0xff) - 0.5f);
            *LightingVertexDiffuse(buffer, i) = ((r << 8 | g) << 8) | b;
        }
    }
}
#pragma optimize("", on)
