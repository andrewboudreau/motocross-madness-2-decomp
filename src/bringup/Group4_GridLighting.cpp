// Bring-up definition (best-guess-unreviewed) of the terrain's vertex lighting
// call, 0x0049e4a0 in LightEmitter.cpp (literal __FILE__ 0x0056ddf0).
//
// Griddraw.h's GridVertexSink is the terrain's view of the LightManager at
// Terrain+0xc40 (LightEmitter.h, LightManager::LightVertices). Only the
// GridVertexSink name is referenced, so this unit defines it and treats
// `this` as the LightManager. The three workers 0x0049e6b0 (any lights),
// 0x0049f1c0 (one light) and 0x0049f8d0 (ambient plus one light) are
// unit-private helpers here; nothing else calls them.
//
// Retail rounds light terms with an inlined x87 `fistp` (current rounding
// mode, round-to-nearest-even) and converts the light colours with _ftol
// (truncation). best guess: Group4Vis_RoundFloat reproduces the fistp
// without inline assembly.
#include <math.h>

#include "LightEmitter.h"
#include "core/DebugAlloc.h"
#include "math/FastMath.h"

// Griddraw.h's boundary declaration (same class key and signature, so the
// definition mangles as ?LightVertices@GridVertexSink@@QAEXPAXH00HH@Z). The
// last argument is the optional colour array the results modulate.
class GridVertexSink {
public:
    void LightVertices(void* matrix, int count, void* vertices, void* buffer,
                       int stride, int flags);  // 0x0049e4a0
};

// LightEmitter.h's lightType values the workers distinguish.
enum {
    kGroup4LightPoint = 1,        // position, per-vertex direction
    kGroup4LightPositional = 2,   // position used as one direction
    kGroup4LightDirectional = 4,  // lightDirection
    kGroup4LightAmbient = 6       // constant colour
};

// Lit vertices are 0x20 bytes: position at +0, packed diffuse at +0x10.
// Normals come from a separate pointer and stride (the terrain passes the
// same buffer + 0xc with stride 0x20).
static inline const float* Group4Vis_VertexPosition(void* buffer, int i) {
    return (const float*)((char*)buffer + i * 0x20);
}
static inline unsigned int* Group4Vis_VertexDiffuse(void* buffer, int i) {
    return (unsigned int*)((char*)buffer + i * 0x20 + 0x10);
}
static inline const float* Group4Vis_VertexNormal(void* normals, int stride, int i) {
    return (const float*)((char*)normals + i * stride);
}

// Round to nearest, ties to even (the x87 default mode retail's fistp uses).
static int Group4Vis_RoundFloat(float value) {
    double floorValue = floor((double)value);
    double fraction = (double)value - floorValue;
    int result = (int)floorValue;
    if (fraction > 0.5 || (fraction == 0.5 && (result & 1)))
        result++;
    return result;
}

static inline int Group4Vis_Clamp255(int value) {
    return value < 0xff ? value : 0xff;
}

// Opaque packed colour; the red term is or-ed with 0xffffff00 first, so an
// in-range red gives alpha 0xff.
static inline unsigned int Group4Vis_PackOpaque(int r, int g, int b) {
    return (((r | 0xffffff00) << 8 | g) << 8) | b;
}

static inline float Group4Vis_Dot(const float* a, const float* b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

// A light's colour scaled to 0..255 (retail: _ftol of colour * 255).
static void Group4Vis_LightColor(const LightEmitter* light, int* r, int* g, int* b) {
    *r = (int)(light->ambientRGBA[0] * 255.0f);
    *g = (int)(light->ambientRGBA[1] * 255.0f);
    *b = (int)(light->ambientRGBA[2] * 255.0f);
}

// Inverse of the rigid object transform (row-major 4x4, row vector times
// matrix): the transposed rotation and the translation rotated back.
static void Group4Vis_InvertRigid(const float* m, float* inverse) {
    int i;
    for (i = 0; i < 16; i++)
        inverse[i] = m[i];
    inverse[1] = m[4];
    inverse[4] = m[1];
    inverse[2] = m[8];
    inverse[8] = m[2];
    inverse[6] = m[9];
    inverse[9] = m[6];
    inverse[12] = -(m[12] * inverse[0] + m[14] * inverse[8] + m[13] * inverse[4]);
    inverse[13] = -(m[14] * inverse[9] + m[13] * inverse[5] + m[12] * inverse[1]);
    inverse[14] = -(m[14] * inverse[10] + m[13] * inverse[6] + m[12] * inverse[2]);
}

// World light position into object space (with translation).
static void Group4Vis_ToObjectPoint(const float* inverse, const Vector3* p, float* out) {
    out[0] = inverse[8] * p->z + inverse[4] * p->y + inverse[0] * p->x + inverse[12];
    out[1] = inverse[5] * p->y + inverse[9] * p->z + inverse[1] * p->x + inverse[13];
    out[2] = inverse[10] * p->z + inverse[6] * p->y + inverse[2] * p->x + inverse[14];
}

// The direction towards a directional light, in object space (no translation).
static void Group4Vis_ToObjectLightDirection(const float* inverse, const Vector3* direction,
                                             float* out) {
    float x = -direction->x;
    float y = -direction->y;
    float z = -direction->z;
    out[0] = x * inverse[0] + y * inverse[4] + z * inverse[8];
    out[1] = x * inverse[1] + y * inverse[5] + z * inverse[9];
    out[2] = x * inverse[2] + y * inverse[6] + z * inverse[10];
}

// Retail skips the normalisation when the squared length is exactly 1.
static void Group4Vis_NormalizeUnlessUnit(float* v) {
    float lengthSquared = Group4Vis_Dot(v, v);
    if (lengthSquared != 1.0f) {
        float scale = FastInvSqrt(lengthSquared);
        v[0] *= scale;
        v[1] *= scale;
        v[2] *= scale;
    }
}

// Unit vector from a vertex to an object-space light position.
static void Group4Vis_PointLightVector(const float* lightPosition, const float* vertex, float* out) {
    out[0] = lightPosition[0] - vertex[0];
    out[1] = lightPosition[1] - vertex[1];
    out[2] = lightPosition[2] - vertex[2];
    Group4Vis_NormalizeUnlessUnit(out);
}

// 0x0049e6b0: any number of lights. The first ambient light seeds the
// per-vertex sums; point, positional and directional lights add their
// rounded diffuse terms where they face the vertex; the sums are clamped
// and packed.
static void Group4Vis_LightAny(LightManager* manager, const float* matrix, int count, void* normals,
                               void* buffer, int stride) {
    float inverse[16];
    int i;
    int light;
    Group4Vis_InvertRigid(matrix, inverse);

    LightEmitter* ambient = 0;
    for (light = 0; light < manager->lightCount; light++) {
        if (manager->lights[light]->lightType == kGroup4LightAmbient) {
            ambient = manager->lights[light];
            break;
        }
    }
    int ambientR = 0;
    int ambientG = 0;
    int ambientB = 0;
    if (ambient)
        Group4Vis_LightColor(ambient, &ambientR, &ambientG, &ambientB);
    for (i = 0; i < count; i++) {
        manager->vertexRed[i] = ambientR;
        manager->vertexGreen[i] = ambientG;
        manager->vertexBlue[i] = ambientB;
    }

    for (light = 0; light < manager->lightCount; light++) {
        LightEmitter* emitter = manager->lights[light];
        int r, g, b;
        float toLight[3];
        Group4Vis_LightColor(emitter, &r, &g, &b);
        switch (emitter->lightType) {
        case kGroup4LightDirectional:
            Group4Vis_ToObjectLightDirection(inverse, &emitter->lightDirection, toLight);
            break;
        case kGroup4LightPositional:
            Group4Vis_ToObjectPoint(inverse, &emitter->lightPosition, toLight);
            Group4Vis_NormalizeUnlessUnit(toLight);
            break;
        case kGroup4LightPoint:
            Group4Vis_ToObjectPoint(inverse, &emitter->lightPosition, toLight);
            break;
        default:
            continue;
        }
        for (i = 0; i < count; i++) {
            float direction[3];
            const float* lightVector = toLight;
            if (emitter->lightType == kGroup4LightPoint) {
                Group4Vis_PointLightVector(toLight, Group4Vis_VertexPosition(buffer, i), direction);
                lightVector = direction;
            }
            float facing = Group4Vis_Dot(Group4Vis_VertexNormal(normals, stride, i), lightVector);
            if (facing > 0.0f) {
                manager->vertexRed[i] += Group4Vis_RoundFloat((float)r * facing);
                manager->vertexGreen[i] += Group4Vis_RoundFloat((float)g * facing);
                manager->vertexBlue[i] += Group4Vis_RoundFloat((float)b * facing);
            }
        }
    }

    for (i = 0; i < count; i++) {
        *Group4Vis_VertexDiffuse(buffer, i) = Group4Vis_PackOpaque(
            Group4Vis_Clamp255(manager->vertexRed[i]), Group4Vis_Clamp255(manager->vertexGreen[i]),
            Group4Vis_Clamp255(manager->vertexBlue[i]));
    }
}

// 0x0049f1c0: a single light writes the diffuse colours directly (the
// per-vertex sums are left alone). Point and positional lights leave
// back-facing vertices unwritten; a directional light writes every vertex,
// back-facing ones with negative terms (retail does not clamp them).
static void Group4Vis_LightOne(LightManager* manager, const float* matrix, int count, void* normals,
                               void* buffer, int stride) {
    LightEmitter* emitter = manager->lights[0];
    float inverse[16];
    float toLight[3];
    int r, g, b;
    int i;
    Group4Vis_LightColor(emitter, &r, &g, &b);
    switch (emitter->lightType) {
    case kGroup4LightPoint:
        Group4Vis_InvertRigid(matrix, inverse);
        Group4Vis_ToObjectPoint(inverse, &emitter->lightPosition, toLight);
        for (i = 0; i < count; i++) {
            float direction[3];
            Group4Vis_PointLightVector(toLight, Group4Vis_VertexPosition(buffer, i), direction);
            float facing = Group4Vis_Dot(direction, Group4Vis_VertexNormal(normals, stride, i));
            if (facing > 0.0f) {
                *Group4Vis_VertexDiffuse(buffer, i) =
                    Group4Vis_PackOpaque(Group4Vis_RoundFloat((float)r * facing - 0.5f),
                                         Group4Vis_RoundFloat((float)g * facing - 0.5f),
                                         Group4Vis_RoundFloat((float)b * facing - 0.5f));
            }
        }
        break;
    case kGroup4LightPositional:
        Group4Vis_InvertRigid(matrix, inverse);
        Group4Vis_ToObjectPoint(inverse, &emitter->lightPosition, toLight);
        Group4Vis_NormalizeUnlessUnit(toLight);
        for (i = 0; i < count; i++) {
            float facing = Group4Vis_Dot(toLight, Group4Vis_VertexNormal(normals, stride, i));
            if (facing > 0.0f) {
                *Group4Vis_VertexDiffuse(buffer, i) =
                    Group4Vis_PackOpaque(Group4Vis_RoundFloat((float)r * facing - 0.5f),
                                         Group4Vis_RoundFloat((float)g * facing - 0.5f),
                                         Group4Vis_RoundFloat((float)b * facing - 0.5f));
            }
        }
        break;
    case kGroup4LightDirectional:
        Group4Vis_InvertRigid(matrix, inverse);
        Group4Vis_ToObjectLightDirection(inverse, &emitter->lightDirection, toLight);
        for (i = 0; i < count; i++) {
            float facing = Group4Vis_Dot(Group4Vis_VertexNormal(normals, stride, i), toLight);
            *Group4Vis_VertexDiffuse(buffer, i) =
                Group4Vis_PackOpaque(Group4Vis_RoundFloat((float)r * facing - 0.5f),
                                     Group4Vis_RoundFloat((float)g * facing - 0.5f),
                                     Group4Vis_RoundFloat((float)b * facing - 0.5f));
        }
        break;
    case kGroup4LightAmbient: {
        unsigned int color = Group4Vis_PackOpaque(r, g, b);
        for (i = 0; i < count; i++)
            *Group4Vis_VertexDiffuse(buffer, i) = color;
        break;
    }
    default:
        break;
    }
}

// 0x0049f8d0: two lights, one of them ambient (the caller guarantees it).
// Each vertex gets ambient plus the other light's rounded term where it
// faces the light, otherwise the ambient colour alone. When a colour array
// is pending (field_0x114) the clamped sums are kept per vertex too.
static void Group4Vis_LightAmbientPlusOne(LightManager* manager, const float* matrix, int count,
                                          void* normals, void* buffer, int stride) {
    float inverse[16];
    int ambientIndex;
    Group4Vis_InvertRigid(matrix, inverse);
    for (ambientIndex = 0; ambientIndex < manager->lightCount; ambientIndex++) {
        if (manager->lights[ambientIndex]->lightType == kGroup4LightAmbient)
            break;
    }
    // best guess: retail reads an unrelated value as the other light's index
    // when no ambient light exists; the caller makes that unreachable.
    if (ambientIndex >= manager->lightCount)
        return;

    int ambientR, ambientG, ambientB;
    Group4Vis_LightColor(manager->lights[ambientIndex], &ambientR, &ambientG, &ambientB);
    unsigned int ambientColor = Group4Vis_PackOpaque(ambientR, ambientG, ambientB);
    LightEmitter* emitter = manager->lights[1 - ambientIndex];
    int r, g, b;
    Group4Vis_LightColor(emitter, &r, &g, &b);

    float toLight[3];
    switch (emitter->lightType) {
    case kGroup4LightDirectional:
        Group4Vis_ToObjectLightDirection(inverse, &emitter->lightDirection, toLight);
        break;
    case kGroup4LightPositional:
        Group4Vis_ToObjectPoint(inverse, &emitter->lightPosition, toLight);
        Group4Vis_NormalizeUnlessUnit(toLight);
        break;
    case kGroup4LightPoint:
        Group4Vis_ToObjectPoint(inverse, &emitter->lightPosition, toLight);
        break;
    default:
        return;
    }

    for (int i = 0; i < count; i++) {
        float direction[3];
        const float* lightVector = toLight;
        if (emitter->lightType == kGroup4LightPoint) {
            Group4Vis_PointLightVector(toLight, Group4Vis_VertexPosition(buffer, i), direction);
            lightVector = direction;
        }
        float facing = Group4Vis_Dot(lightVector, Group4Vis_VertexNormal(normals, stride, i));
        if (facing > 0.0f) {
            int red = Group4Vis_RoundFloat((float)r * facing) + ambientR;
            int green = Group4Vis_RoundFloat((float)g * facing) + ambientG;
            int blue = Group4Vis_RoundFloat((float)b * facing) + ambientB;
            *Group4Vis_VertexDiffuse(buffer, i) = Group4Vis_PackOpaque(
                Group4Vis_Clamp255(red), Group4Vis_Clamp255(green), Group4Vis_Clamp255(blue));
            if (manager->field_0x114) {
                manager->vertexRed[i] = Group4Vis_Clamp255(red);
                manager->vertexGreen[i] = Group4Vis_Clamp255(green);
                manager->vertexBlue[i] = Group4Vis_Clamp255(blue);
            }
        } else {
            *Group4Vis_VertexDiffuse(buffer, i) = ambientColor;
            if (manager->field_0x114) {
                manager->vertexRed[i] = ambientR;
                manager->vertexGreen[i] = ambientG;
                manager->vertexBlue[i] = ambientB;
            }
        }
    }
}

// 0x0049e4a0 (ret 0x18). Lights `count` vertices of `buffer` (normals at
// `vertices` + i * stride) under the object transform `matrix`, growing the
// per-vertex colour sums first. With a colour array (`flags`, the same
// 0x20-byte vertex layout) each vertex's diffuse becomes that colour scaled
// by the per-vertex sums / 256.
void GridVertexSink::LightVertices(void* matrix, int count, void* vertices, void* buffer, int stride,
                                   int flags) {
    LightManager* manager = reinterpret_cast<LightManager*>(this);
    void* colors = (void*)flags;
    manager->field_0x114 = colors != 0 ? 1 : 0;

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

    const float* m = (const float*)matrix;
    if (manager->lightCount == 1)
        Group4Vis_LightOne(manager, m, count, vertices, buffer, stride);
    else if (manager->lightCount == 2 && (manager->lights[0]->lightType == kGroup4LightAmbient ||
                                          manager->lights[1]->lightType == kGroup4LightAmbient))
        Group4Vis_LightAmbientPlusOne(manager, m, count, vertices, buffer, stride);
    else
        Group4Vis_LightAny(manager, m, count, vertices, buffer, stride);

    if (colors) {
        for (int i = 0; i < count; i++) {
            unsigned int color = *Group4Vis_VertexDiffuse(colors, i);
            int r = Group4Vis_RoundFloat((float)manager->vertexRed[i] * (1.0f / 256.0f) *
                                             (float)(int)((color >> 16) & 0xff) - 0.5f);
            int g = Group4Vis_RoundFloat((float)manager->vertexGreen[i] * (1.0f / 256.0f) *
                                             (float)(int)((color >> 8) & 0xff) - 0.5f);
            int b = Group4Vis_RoundFloat((float)manager->vertexBlue[i] * (1.0f / 256.0f) *
                                             (float)(int)(color & 0xff) - 0.5f);
            *Group4Vis_VertexDiffuse(buffer, i) = ((r << 8 | g) << 8) | b;
        }
    }
}
