// Group1_ParticleRender.cpp -- bring-up body for ParticleManager slot 14 (0x004bac60,
// 2495 bytes, thiscall, no arguments, returns 1): the particle sprite renderer.
// Written from the decoded retail function (D:\aardvark\VC\krusty2\Particles.cpp's
// code range; ParticleManager.h records the slot).  Behaviour, in retail order:
//
//  1. Nothing when no particle is live or g_TrackGame+0x2d0 is set.
//  2. The render target's camera gets an identity world matrix (a function static
//     built once by IdentityMatrix 0x004a1410; Camera slot 30).
//  3. Every live particle's position (y + 0.5) goes into the untransformed buffer
//     (+0x1f88), which ProcessVertices (D3DVOP_TRANSFORM) transforms into +0x1f84
//     with the target's device (+0x50).  +0x1f84 stays locked and each particle's
//     +0x44 points at its transformed vertex.
//  4. Particles with 0 <= z <= 1 whose screen square (camera +0x198 focal length x
//     size x rhw) overlaps the camera's target rectangle are collected in visible[]
//     (the square is stored at particle +0x24 top, +0x28 bottom, +0x2c left,
//     +0x30 right) and sorted by depth (CompareParticleDepth 0x004bac20).
//  5. The four sprite corners are lit by the manager's light manager (+0x2054,
//     LightManager::LightVertices 0x0049e4a0) in a camera-aligned frame placed 100
//     units in front of the scene camera (g_TrackGame+0x10 -> camera); the corner
//     colours keep only their rgb.
//  6. Each visible particle becomes a screen-clipped quad in the 4000-vertex buffer
//     (+0x1f8c): per-vertex fog in the specular alpha unless the device has table
//     fog without vertex fog, alpha from age/lifetime (flag 2), flag 1 keeps the
//     previous particle's colour, flag 0x800 uses the particle colour on all four
//     corners instead of the lit corner colours.  Clipping at the screen edges
//     interpolates the sprite cell's texture coordinates.
//  7. Both buffers unlock, the texture binds (TextureMap slot 19) and the quads are
//     drawn as an indexed triangle list with alpha blending (render state 0x1b) and,
//     for textures other than format 0x115c, render state 0x21; texture blend 4,
//     restored to 2 afterwards.
//
// Retail rounds the fog and fade alpha with an inline x87 fistp (0x004bb271,
// 0x004bb2ab).  The game build compiles this unit with /QIfist (config/game_build.json
// bringup_flags), so FistpFloatToInt's cast is a FISTP under the current rounding mode
// (ties to even, not up); the two are the unit's only float-to-int conversions.  Only
// the low byte of either result is kept (`<< 24`): the fog is clamped to [0, 1]; the
// fade lies in [-0.5, 127] while age <= lifetime (ParticleManager::Tick retires a
// particle once its age passes its lifetime), and leaving int would take an age of
// over 1.6e7 lifetimes; an infinity or NaN (lifetime 0) leaves a zero low byte in both
// retail's 32-bit indefinite and the 64-bit one.  So the 64-bit store gives retail's
// colours.
// The differential run (tools/emu_cases/particle_render3d.py) found last-bit
// differences in the light frame and the FastInvSqrt argument (both precisions) and,
// under 53-bit precision, in the left-clipped u (retail keeps the unrounded value on
// the x87 stack).  Retail's grouping is reproduced by the by-value Vector3 cross and
// normalize helpers, the z*z + (x*x + y*y) length, the scaled view vector and the
// left-clip assignment order; after them no run differs under 53- or 24-bit precision.
// Faithful retail quirk kept: when no particle is visible the function returns with
// the transformed buffer (+0x1f84) still locked.

#include <math.h>

#include "TrackGameGlobal.h"
#include "effects/ParticleManager.h"
#include "Camera.h"
#include "PCRenderTarget.h"
#include "TextureMap.h"
#include "../../samples/common/FistpFloatToInt.h"

extern "C" void qsort(void* base, unsigned int num, unsigned int width,
                      int (*compare)(const void*, const void*));

// 0x00460c00 (math/FastMath.h).
float FastInvSqrt(float x);

// 0x00689158 (Group1_Globals.cpp).
extern ParticleUVRect g_particleCellUVs[61];

// src/reconstructed/LightEmitter.h's LightManager, reduced to the one method used here.
class LightManager {
public:
    void LightVertices(void* matrix, int count, void* vertices, void* buffer, int stride,
                       void* colors);   // 0x0049e4a0
};

// IDirect3DVertexBuffer7 as slot 14 uses it (slots 3 Lock, 4 Unlock, 5 ProcessVertices).
class Group1ParticleVertexBuffer {
public:
    virtual long __stdcall QueryInterface(void* iid, void** out) = 0;
    virtual unsigned long __stdcall AddRef() = 0;
    virtual unsigned long __stdcall Release() = 0;
    virtual long __stdcall Lock(unsigned long flags, void** data, unsigned long* size) = 0;
    virtual long __stdcall Unlock() = 0;
    virtual long __stdcall ProcessVertices(unsigned long op, unsigned long destIndex,
                                           unsigned long count,
                                           Group1ParticleVertexBuffer* source,
                                           unsigned long sourceIndex, void* device,
                                           unsigned long flags) = 0;
};

// The 0x20-byte transformed-and-lit vertex (FVF 0x1c4: xyzrhw, diffuse, specular, tex1).
struct Group1ParticleTLVertex {
    float x, y, z, rhw;
    unsigned int color;      // +0x10
    unsigned int specular;   // +0x14 (fog in the alpha byte)
    float u, v;              // +0x18, +0x1c
};

// The screen square slot 14 stores in Particle +0x24..+0x30.
struct Group1ParticleRect {
    float top, bottom, left, right;
};

// g_TrackGame as slot 14 reads it.
struct Group1ParticleGame {
    char field_0x00[0x10];
    RenderTarget* renderTarget;    // +0x10 the scene render target (its camera lights the corners)
    char field_0x14[0x2d0 - 0x14];
    int hidden;                    // +0x2d0 nonzero: no particles are drawn
};

// The camera fields slot 14 reads (Camera.h keeps them protected; same offsets).
struct Group1ParticleCamera {
    char field_0x000[0xec];
    float screenMatrix[16];        // +0xec screen x view matrix (column 2 gives view depth)
    char field_0x12c[0x170 - 0x12c];
    Vector3 eyePosition;           // +0x170
    Vector3 viewDirection;         // +0x17c
    Vector3 upDirection;           // +0x188
    char roll[0x198 - 0x194]; // +0x194: Camera::roll (bytes kept)
    float focalLength;             // +0x198
    char field_0x19c[0x1bc - 0x19c];
    float nearPlane;               // +0x1bc
    float farPlane;                // +0x1c0
    int targetWidth;               // +0x1c4
    int targetHeight;              // +0x1c8
};

static inline Group1ParticleCamera* Group1CameraView(Camera* camera)
{
    return reinterpret_cast<Group1ParticleCamera*>(camera);
}

static inline Group1ParticleRect* Group1Rect(Particle* particle)
{
    return reinterpret_cast<Group1ParticleRect*>(particle->screenRect);
}

static inline unsigned int& Group1CornerColor(ParticleCornerVertex& corner)
{
    return *reinterpret_cast<unsigned int*>(corner.field_0x0c + 4);   // +0x10
}

// The normalize retail inlines twice: unit vectors are kept as they are.
static inline Vector3 Group1Normalize(float x, float y, float z)
{
    float lengthSquared = z * z + (x * x + y * y);
    if (lengthSquared == 1.0f)
        return Vector3(x, y, z);
    float scale = FastInvSqrt(lengthSquared);
    return Vector3(scale * x, scale * y, scale * z);
}

static inline Vector3 Group1Cross(const Vector3& a, const Vector3& b)
{
    return Vector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

// 0x004bac60
// Retail's inline-asm fistp helper gives this function an EBP frame; the
// pragma keeps the frame pointer for it alone (docs/GAME_BUILD.md#frame-pointer-units).
#pragma optimize("y", off)
int ParticleManager::Render3D()
{
    int i;
    unsigned int color = 0;
    if (activeCount <= 0 || reinterpret_cast<Group1ParticleGame*>(g_TrackGame)->hidden)
        return 1;

    static Matrix4 s_particleWorld = IdentityMatrix();
    PCRenderTarget* target = (PCRenderTarget*)renderContext;
    target->currentCamera->SetWorldMatrix(&s_particleWorld);

    // 3. Positions in, transformed vertices out.
    Group1ParticleVertexBuffer* source = (Group1ParticleVertexBuffer*)indexBufferObject;
    Group1ParticleVertexBuffer* transformed = (Group1ParticleVertexBuffer*)vertexBuffer;
    char* data;
    source->Lock(1, (void**)&data, 0);
    for (i = 0; i < activeCount; i++) {
        float* out = (float*)(data + i * 0x20);
        out[0] = particles[i]->position.x;
        out[1] = particles[i]->position.y + 0.5f;
        out[2] = particles[i]->position.z;
    }
    source->Unlock();
    transformed->ProcessVertices(1, 0, activeCount, source, 0, target->device, 0);
    char* screen;
    transformed->Lock(1, (void**)&screen, 0);
    for (i = 0; i < activeCount; i++)
        particles[i]->vertex = (ParticleVertex*)(screen + i * 0x20);

    // 4. Visible particles.
    visibleCount = 0;
    for (i = 0; i < activeCount; i++) {
        Particle* particle = particles[i];
        Group1ParticleTLVertex* vertex = (Group1ParticleTLVertex*)particle->vertex;
        if (vertex->z > 1.0f || vertex->z < 0.0f)
            continue;
        Group1ParticleCamera* camera = Group1CameraView(target->currentCamera);
        float half = camera->focalLength * particle->size * vertex->rhw;
        Group1ParticleRect* rect = Group1Rect(particle);
        rect->left = vertex->x - half;
        if (rect->left > (float)Group1CameraView(target->currentCamera)->targetWidth)
            continue;
        rect->right = half + vertex->x;
        if (rect->right < 0.0f)
            continue;
        rect->top = vertex->y - half;
        if (rect->top > (float)Group1CameraView(target->currentCamera)->targetHeight)
            continue;
        rect->bottom = half + vertex->y;
        if (rect->bottom < 0.0f)
            continue;
        visible[visibleCount] = particle;
        visibleCount++;
    }
    if (visibleCount == 0)
        return 1;
    qsort(visible, visibleCount, 4, CompareParticleDepth);

    // Per-vertex fog unless the device fogs by table without vertex fog.
    int vertexFog = 1;
    if ((target->triRasterCaps & 0x100) && !(target->triRasterCaps & 0x80))
        vertexFog = 0;

    // 5. Light the corners in a frame 100 units in front of the scene camera.
    Group1ParticleCamera* scene =
        Group1CameraView(reinterpret_cast<Group1ParticleGame*>(g_TrackGame)->renderTarget->currentCamera);
    Vector3 scaled;
    scaled.x = scene->viewDirection.x * 100.0f;
    scaled.y = scene->viewDirection.y * 100.0f;
    scaled.z = scene->viewDirection.z * 100.0f;
    float lightX = scaled.x + scene->eyePosition.x;
    float lightY = scaled.y + scene->eyePosition.y;
    float lightZ = scaled.z + scene->eyePosition.z;
    Vector3 backward(-scene->viewDirection.x, -scene->viewDirection.y, -scene->viewDirection.z);
    Vector3 upward = scene->upDirection;
    // side = up x back; up' = back x side (up made perpendicular to the view).
    Vector3 side = Group1Cross(upward, backward);
    Vector3 perp = Group1Cross(backward, side);
    Vector3 up = Group1Normalize(perp.x, perp.y, perp.z);
    Vector3 back = Group1Normalize(backward.x, backward.y, backward.z);
    Matrix4 frame;
    frame.m[0][0] = up[1] * back[2] - up[2] * back[1];
    frame.m[0][1] = back[0] * up[2] - up[0] * back[2];
    frame.m[0][2] = up[0] * back[1] - back[0] * up[1];
    frame.m[0][3] = 0.0f;
    frame.m[1][0] = up[0];
    frame.m[1][1] = up[1];
    frame.m[1][2] = up[2];
    frame.m[1][3] = 0.0f;
    frame.m[2][0] = back[0];
    frame.m[2][1] = back[1];
    frame.m[2][2] = back[2];
    frame.m[2][3] = 0.0f;
    frame.m[3][0] = lightX;
    frame.m[3][1] = lightY;
    frame.m[3][2] = lightZ;
    frame.m[3][3] = 1.0f;
    reinterpret_cast<LightManager*>(lightManager)->LightVertices(&frame, 4, cornerNormals,
                                                                 corners, 0xc, 0);
    for (i = 0; i < 4; i++)
        Group1CornerColor(corners[i]) &= 0xffffff;

    // 6. Quads.
    Group1ParticleCamera* camera = Group1CameraView(target->currentCamera);
    float fogScale = 1.0f / (camera->farPlane - camera->nearPlane);
    unsigned int alpha = 0;
    unsigned int specular = 0;
    Group1ParticleTLVertex* quads;
    ((Group1ParticleVertexBuffer*)quadVertexBuffer)->Lock(1, (void**)&quads, 0);
    for (i = 0; i < visibleCount; i++) {
        Particle* particle = visible[i];
        Group1ParticleTLVertex* vertex = (Group1ParticleTLVertex*)particle->vertex;
        float z = vertex->z;
        float rhw = vertex->rhw;
        if (vertexFog) {
            const float* m = camera->screenMatrix;
            float viewZ = m[10] * particle->position.z + m[6] * particle->position.y +
                          m[2] * particle->position.x + m[14];
            float fog = 1.0f - (viewZ - camera->nearPlane) * fogScale;
            if (fog < 0.0f)
                fog = 0.0f;
            else if (fog > 1.0f)
                fog = 1.0f;
            int fogLevel;
            FistpFloatToInt(&fogLevel, fog * 255.0f);
            specular = (unsigned int)fogLevel << 24;
        }
        unsigned int flags = particle->flags;
        if (flags & 2) {
            if (!(flags & 1)) {
                int fade;
                FistpFloatToInt(&fade, (1.0f - particle->age / particle->lifetime) * 127.5f - 0.5f);
                alpha = (unsigned int)fade << 24;
                color = particle->color | alpha;
            }
        } else if (!(flags & 1)) {
            alpha = 0xff000000;
            color = particle->color | 0xff000000;
        }

        Group1ParticleTLVertex* out = quads + i * 4;
        if (particle->flags & 0x800) {
            out[0].color = color;
            out[1].color = color;
            out[2].color = color;
            out[3].color = color;
        } else {
            out[0].color = Group1CornerColor(corners[0]) | alpha;
            out[1].color = Group1CornerColor(corners[1]) | alpha;
            out[2].color = Group1CornerColor(corners[2]) | alpha;
            out[3].color = Group1CornerColor(corners[3]) | alpha;
        }

        // Clip the square to the target and the cell's texture coordinates with it.
        const ParticleUVRect& cell = g_particleCellUVs[particle->frame];
        Group1ParticleRect* rect = Group1Rect(particle);
        float vTopLeft, vTopRight, vBottomRight, vBottomLeft;
        float uTopLeft, uTopRight, uBottomRight, uBottomLeft;
        if (rect->top < 0.0f) {
            float t = -rect->top / (rect->bottom - rect->top);
            vTopLeft = (cell.v3 - cell.v0) * t + cell.v0;
            rect->top = 0.0f;
            vTopRight = vTopLeft;
        } else {
            vTopLeft = cell.v0;
            vTopRight = cell.v1;
        }
        int height = camera->targetHeight;
        if ((float)height < rect->bottom) {
            float t = ((float)height - rect->top) / (rect->bottom - rect->top);
            vBottomRight = (cell.v3 - vTopLeft) * t + vTopLeft;
            vBottomLeft = vBottomRight;
            rect->bottom = (float)(height - 1);
        } else {
            vBottomLeft = cell.v3;
            vBottomRight = cell.v2;
        }
        if (rect->left < 0.0f) {
            float t = -rect->left / (rect->right - rect->left);
            uBottomLeft = (cell.u1 - cell.u0) * t + cell.u0;
            rect->left = 0.0f;
            uTopLeft = uBottomLeft;
        } else {
            uBottomLeft = cell.u3;
            uTopLeft = cell.u0;
        }
        int width = camera->targetWidth;
        if ((float)width <= rect->right) {
            float t = ((float)width - rect->left) / (rect->right - rect->left);
            uTopRight = (cell.u1 - uTopLeft) * t + uTopLeft;
            uBottomRight = uTopRight;
            rect->right = (float)(width - 1);
        } else {
            uBottomRight = cell.u2;
            uTopRight = cell.u1;
        }

        out[0].specular = specular;
        out[0].u = uTopLeft;
        out[0].v = vTopLeft;
        out[0].x = rect->left;
        out[0].y = rect->top;
        out[0].z = z;
        out[0].rhw = rhw;
        out[1].u = uTopRight;
        out[1].v = vTopRight;
        out[1].specular = specular;
        out[1].x = rect->right;
        out[1].y = rect->top;
        out[1].z = z;
        out[1].rhw = rhw;
        out[2].v = vBottomRight;
        out[2].u = uBottomRight;
        out[2].specular = specular;
        out[2].x = rect->right;
        out[2].y = rect->bottom;
        out[2].z = z;
        out[2].rhw = rhw;
        out[3].u = uBottomLeft;
        out[3].v = vBottomLeft;
        out[3].specular = specular;
        out[3].x = rect->left;
        out[3].y = rect->bottom;
        out[3].z = z;
        out[3].rhw = rhw;
    }
    transformed->Unlock();
    ((Group1ParticleVertexBuffer*)quadVertexBuffer)->Unlock();

    // 7. Draw.
    ((TextureMap*)texture)->Bind();
    target->SetRenderState(0x1b, 1, 0);
    if (((TextureMap*)texture)->mapFormat != 0x115c)
        target->SetRenderState(0x21, 1, 0);
    target->SetTextureBlend(4, 0);
    target->DrawIndexedPrimitiveVB(4, (int)quadVertexBuffer, 0, visibleCount * 4, (int)indices,
                                   visibleCount * 6, 0);
    if (((TextureMap*)texture)->mapFormat != 0x115c)
        target->SetRenderState(0x21, 0, 0);
    target->SetRenderState(0x1b, 0, 0);
    target->SetTextureBlend(2, 0);
    return 1;
}
#pragma optimize("", on)
