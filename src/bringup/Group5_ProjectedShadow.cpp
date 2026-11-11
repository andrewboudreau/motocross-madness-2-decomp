// Group5 bring-up definitions (best-guess, link frontier). Not strict matches.
//
// ProjectedShadow::Init (0x004da7b0), ComputeBounds (0x004db0d0), RenderShadow
// (0x004db8c0), Present (0x004dbd30) and TintCasterVertices (0x004dc2b0), from
// samples/physics/shadow/ProjectedShadowNearMisses.cpp.
//  * Init and ComputeBounds are the sample's text. Their call views are bound by
//    link_aliases to the providers at the same retail addresses: ShadowTexture's
//    constructor to PCTextureMap's (0x004c5f00, both 0x80 bytes), ShadowCameraProxy's
//    to ShadowCamera's (0x004da520, both 0x220 bytes), ShadowCameraProxy::SetLensScale
//    to Camera::SetFieldOfView (0x0042e930), ShadowRenderTarget::SetOwner to
//    RenderTarget::SetCamera (0x004e8cf0), ShadowMatrixMultiplyInto to MatrixMult
//    (0x004a1860, hidden result pointer first) and the ShadowVec3Call constructor to
//    Vec3Call's (0x00404e60). KrustyUI.cpp calls Init as UnknownKrustyUIShadow::Init
//    (it reinterprets the 0x134-byte ProjectedShadow), aliased here too. The former
//    copies (Group3_KrustyUIShadow.cpp, which called the providers under their own
//    names, and an inline-constructor ComputeBounds) linked at 0.892 and 0.924.
//  * RenderShadow and TintCasterVertices keep a deliberate difference from the
//    sample: retail rounds the projected coordinates with an inline `fistp`
//    (current FPU rounding mode), and X87FloatToInt reproduces that without inline
//    assembly. The sample's ProjectedShadowFloatToInt (an (int) cast through __ftol
//    and a ties-to-even fix) scores higher (0.688 and 0.638 against 0.545 and
//    0.511, because X87FloatToInt inlines to about 62 instructions a site) and
//    gives the same results in the default round-to-nearest mode, but ignores the
//    FPU rounding mode, so these two are not switched. They were compared with
//    retail instruction by instruction and agree apart from the rounding sites.
//    Present has no rounding site and is the same in both (0.701).
//  * The sample is not linked as a whole: it includes the canonical
//    ProjectedShadow.cpp, which the game build compiles on its own.
#include "TrackGameGlobal.h"
#include "shadow/ProjectedShadow.h"
#include "X87FloatToInt.h"

#include <math.h>
#include <stdio.h>

// The per-file stand-in vector constructor (as in ProjectedShadow.cpp).
struct ShadowConstVec3 : ShadowVec3 {
    ShadowConstVec3(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }
};

// 0x004da7b0: binds the shadow to its render target, picks a surface format from the device
// capabilities (and the DriverInfo\<driver>\DestColorShadow setting), creates the shadow
// texture and camera and retargets the render target at the texture while the camera is built.
// Returns this, or 0 after releasing the object when no usable format or allocation exists.
// Near miss: block layout, frame and the tail match (the clip rectangle stored left, right,
// top, bottom reproduces retail's reload of the texture size).  Nesting the texture and camera
// success path under one shared release-and-return-0 gives retail's two failure blocks (the
// surface-format failure keeps its own copy).  Left: register choice in the sprintf/QueryFlag
// argument set-up and the saved render-target fields (retail keeps width in ebx, surface in
// ebp and spills height; VC6 here spills width), 841 bytes against 842.
ProjectedShadow* ProjectedShadow::Init(ShadowRenderTarget* target, void* textureOwner)
{
    GameObject::SetRenderContext((void*)target);
    field_0x5c = (int)target;

    char name[0x100];
    sprintf(name, "DriverInfo\\%s\\DestColorShadow", Host()->display->driverName);
    if (reinterpret_cast<ShadowSettings*>(g_TrackGame)->QueryFlag(name, 0) && (Host()->capsA & 0x100) && (Host()->capsB & 1)) {
        formatA = 9;
        formatB = 1;
    } else {
        unsigned int caps = Host()->capsA;
        if ((caps & 0x10) && (Host()->capsB & 0x20)) {
            formatA = 5;
            formatB = 6;
        } else if ((caps & 1) && (Host()->capsB & 0x10)) {
            formatA = 1;
            formatB = 5;
        } else if (caps & 0x800) {
            formatA = 0xc;
            formatB = 2;
        } else {
            Release();
            return 0;
        }
    }

    if (Host()->IsFormatSupported(0x115c)) {
        surfaceFormat = 0x115c;
    } else if (Host()->IsFormatSupported(0x613) && (Host()->capsC & 0x2000)) {
        surfaceFormat = 0x613;
        field_0x98 = 0;
    } else {
        Release();
        return 0;
    }

    texture = new(__FILE__, 0xaa) ShadowTexture(textureOwner, 1);
    if (texture
        && texture->Create(0, 0x80, 0x80, 0x80, 0x80, surfaceFormat, surfaceFormat, 0, 8, 0, 0, 0,
                           formatA, formatB, 0, 0x80, 0xff00ff)) {
        texture->UnknownVirtualSlot8(1, 0, 1);
        field_0x60 = texture->pixels;
        if (field_0x60) {
            texture->surface->PageLock(0);

            int savedHeight = Host()->height;
            void* savedSurface = Host()->surface;
            int savedWidth = Host()->width;
            Host()->width = texture->size;
            Host()->height = texture->height;
            Host()->surface = texture->pixels;
            ShadowOwner* owner = Host()->owner;

            camera = new(__FILE__, 0xcd) ShadowCameraProxy(1);
            if (camera && camera->SetRenderContext((void*)Host())) {
                Host()->surface = savedSurface;
                Host()->width = savedWidth;
                Host()->height = savedHeight;
                Host()->SetOwner(owner);
                Host()->device->SetTransformLike(owner->block_0x1a0);
                camera->SetLensScale(0.3f);
                for (int bits = 0x40; bits; bits >>= 1)
                    sizeShift++;
                clipRect.left = 0;
                clipRect.right = texture->size - 1;
                clipRect.top = 0;
                clipRect.bottom = texture->size - 1;
                return this;
            }
        }
    }
    Release();
    return 0;
}

// 0x004dc2b0 (352 B): paints shadowColor into the colour word (+0x10 of 0x20-byte vertices) of
// every caster vertex whose projected position lands on a height-map cell equal to colorKeyLow.
// Audited against retail for the linked game (2026-11-11): PartCount/GetPart arguments, the
// running vertex offset, the bounds (inclusive of texture->size, as retail), the signed height
// cell against the zero-extended key and the colour store agree. The size gap (255 built vs 121
// retail instructions) is the two X87FloatToInt expansions and retail's ebp frame.
void ProjectedShadow::TintCasterVertices()
{
    int totalVertices = 0;
    for (int c = 0; c < casterCount; c++) {
        if (!casters[c]->mesh)
            continue;
        for (int part = 0; part < casters[c]->PartCount(-1); part++) {
            char* vertices;
            int count;
            unsigned short* indices;
            int indexCount;
            casters[c]->GetPart(part, (void**)&vertices, &count, &indices, &indexCount, 0, -1);
            for (int i = 0; i < count; i++) {
                int x = X87FloatToInt(vertexBuffer[totalVertices + i].x - 0.5f);
                int y = X87FloatToInt(vertexBuffer[totalVertices + i].y - 0.5f);
                if (x >= 0 && y >= 0 && x <= texture->size && y <= texture->size
                    && heightMap[(y << sizeShift) + x] == colorKeyLow)
                    *(unsigned int*)(vertices + i * 0x20 + 0x10) = shadowColor;
            }
            totalVertices += count;
        }
    }
}

static inline float ShadowMagnitude(const ShadowVec3& v)
{
    float y = v.y, x = v.x, z = v.z;
    float sq = z * z + (x * x + y * y);
    if (sq == 0.0f)
        return 0.0f;
    if (sq == 1.0f)
        return 1.0f;
    return 1.0f / FastInvSqrt(sq);
}
static inline void ShadowTransformCoord(ShadowVec3* r, ShadowVec3 v, const ShadowMatrix& m)
{
    r->x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + m.m[3][0];
    r->y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + m.m[3][1];
    r->z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + m.m[3][2];
}
static inline ShadowVec3 ShadowDifference(const ShadowVec3& a, const ShadowVec3& b) { return ShadowConstVec3(a.x - b.x, a.y - b.y, a.z - b.z); }
static inline ShadowVec3 ShadowSum(const ShadowVec3& a, const ShadowVec3& b) { return ShadowConstVec3(a.x + b.x, a.y + b.y, a.z + b.z); }
static inline ShadowVec3 ShadowScale(const ShadowVec3& a, float s) { return ShadowConstVec3(a.x * s, a.y * s, a.z * s); }
// Out-of-line call views, declared only and bound to the retail copies: ShadowMatrixMultiply
// 0x004a1860 with its hidden result pointer spelt out (ComputeBounds copies straight from the
// returned pointer, so both modes share one tail), and the Vec3(float, float, float) COMDAT
// constructor 0x00404e60 (thiscall, ret 0xc) that ComputeBounds calls instead of inlining.
ShadowMatrix* ShadowMatrixMultiplyInto(ShadowMatrix* out, ShadowMatrix a, ShadowMatrix b);
struct ShadowVec3Call : ShadowVec3 {
    ShadowVec3Call(float x_, float y_, float z_);
};
static inline float AbsF(float v) { return v < 0.0f ? -v : v; }

static inline void ShadowTransformExtents(ShadowVec3* out, ShadowVec3 h, const ShadowCaster* k)
{
    float a00 = h.x * k->world.m[0][0];
    float a01 = h.x * k->world.m[0][1];
    float a02 = h.x * k->world.m[0][2];
    float a10 = h.y * k->world.m[1][0];
    float a11 = h.y * k->world.m[1][1];
    float a12 = h.y * k->world.m[1][2];
    float a20 = h.z * k->world.m[2][0];
    float a21 = h.z * k->world.m[2][1];
    float a22 = h.z * k->world.m[2][2];
    out->x = AbsF(a00) + AbsF(a10) + AbsF(a20);
    out->y = AbsF(a01) + AbsF(a11) + AbsF(a21);
    out->z = AbsF(a02) + AbsF(a12) + AbsF(a22);
}

// 0x004db0d0 (2020 B): recomputes the light-space bounds of all casters and derives the shadow
// camera set-up (mode 3 directional: orthographic; mode 1 point light: perspective with a
// field of view from atan2(lensScale, distance)). Every exit returns 1.
int ProjectedShadow::ComputeBounds()
{
    if (!casters)
        return 1;
    ShadowMatrix product;
    ShadowVec3 boundsMin, boundsMax;
    boundsMin.x = boundsMin.y = boundsMin.z = 3.4028235e38f;
    boundsMax.x = boundsMax.y = boundsMax.z = -3.4028235e38f;
    for (int i = 0; i < casterCount; i++) {
        ShadowVec3 casterCenter, ext;
        casters[i]->GetBounds(&casterCenter, &ext);
        ShadowTransformExtents(&ext, ext, casters[i]);
        ShadowTransformCoord(&casterCenter, casterCenter, casters[i]->world);
        ShadowVec3 lo = ShadowDifference(casterCenter, ext);
        ShadowVec3 hi = ShadowSum(casterCenter, ext);
        if (lo.x < boundsMin.x) boundsMin.x = lo.x;
        if (lo.y < boundsMin.y) boundsMin.y = lo.y;
        if (lo.z < boundsMin.z) boundsMin.z = lo.z;
        if (hi.x > boundsMax.x) boundsMax.x = hi.x;
        if (hi.y > boundsMax.y) boundsMax.y = hi.y;
        if (hi.z > boundsMax.z) boundsMax.z = hi.z;
    }
    center = ShadowScale(ShadowSum(boundsMax, boundsMin), 0.5f);
    halfExtents = ShadowScale(ShadowDifference(boundsMax, boundsMin), 0.5f);
    lensScale = ShadowMagnitude(halfExtents);

    if (mode == 3) {
        ShadowMatrix projection = ShadowMatrixIdentity();
        float half = (float)texture->size * 0.5f;
        projection.m[0][0] = half / lensScale;
        projection.m[1][1] = -(half / lensScale);
        projection.m[3][0] = half;
        projection.m[3][1] = half;
        ShadowVec3 eye = ShadowDifference(center, ShadowScale(lightDir, 1000.0f));
        camera->SetLookAt(&eye, 0, 0, 0, 0);
        camera->UnknownVirtualSlot29(center);
        camera->UnknownVirtualSlot28();
        lightMatrix = *ShadowMatrixMultiplyInto(&product, projection, camera->matrix_0xac);
        return 1;
    } else if (mode == 1) {
        lightDir = ShadowDifference(center, lightVec2);
        float dist = ShadowMagnitude(lightDir);
        float fov = (float)atan2(lensScale, dist) * 114.59156f;
        camera->SetLookAt(&lightVec2, 0, 0, 0, &fov);
        ShadowVec3& dir = lightDir;
        float y = dir.y, x = dir.x, z = dir.z;
        float sq = z * z + (x * x + y * y);
        ShadowVec3 unitDir;
        if (sq == 1.0f) {
            unitDir = dir;
        } else {
            float inv = FastInvSqrt(sq);
            unitDir = ShadowVec3Call(dir.x * inv, dir.y * inv, dir.z * inv);
        }
        dir = unitDir;
        camera->UnknownVirtualSlot29(center);
        camera->UnknownVirtualSlot28();
        ShadowMatrix projection = ShadowMatrixIdentity();
        projection.m[0][0] = (float)texture->size;
        projection.m[1][1] = (float)texture->size;
        lightMatrix = *ShadowMatrixMultiplyInto(&product, projection, camera->matrix_0xec);
    }
    return 1;
}

// Fills count shorts at p with value: one short to reach dword alignment, then dword pairs
// (pair = value in both halves), then the last odd short (retail unrolls the pair loop 16
// times with a Duff's device).
static inline void FillShortsUnrolled(short* p, int count, short value, unsigned int pair)
{
    if (count > 0) {
        short* aligned = p;
        if ((int)p & 2) {
            *p = value;
            aligned = p + 1;
            count--;
        }
        int pairs = count >> 1;
        if (pairs > 0) {
            unsigned int* words = (unsigned int*)aligned;
            int k = pairs - 1;
            unsigned int passes = (unsigned int)(pairs + 15) >> 4;
            do {
                switch (k) {
                default: words[15] = pair;
                case 14: words[14] = pair;
                case 13: words[13] = pair;
                case 12: words[12] = pair;
                case 11: words[11] = pair;
                case 10: words[10] = pair;
                case 9: words[9] = pair;
                case 8: words[8] = pair;
                case 7: words[7] = pair;
                case 6: words[6] = pair;
                case 5: words[5] = pair;
                case 4: words[4] = pair;
                case 3: words[3] = pair;
                case 2: words[2] = pair;
                case 1: words[1] = pair;
                case 0: words[0] = pair;
                }
                k -= 16;
                words += 16;
            } while (--passes);
        }
        if (count & 1)
            aligned[count - 1] = value;
    }
}

// 0x004db8c0 (1074 B): renders the casters into the 16-bit height texture: clears the previous
// dirty rectangle, transforms each part's vertices into vertexBuffer, culls back-facing and
// out-of-range triangles, grows clipRect/minDepth and rasterises with shadowPixel.
void ProjectedShadow::RenderShadow()
{
    int vertexBase = 0;
    heightMap = texture->LockPixels(0, 0, 0);
    if (!heightMap)
        return;

    if (clipRect.right >= clipRect.left) {
        short* row = heightMap + clipRect.top * texture->size + clipRect.left;
        for (int y = clipRect.top; y <= clipRect.bottom; y++) {
            FillShortsUnrolled(row, clipRect.right - clipRect.left + 1, colorKeyLow, colorKeyPair);
            row += texture->size;
        }
    }

    clipRect.left = texture->size - 1;
    clipRect.right = 0;
    clipRect.top = texture->size - 1;
    clipRect.bottom = 0;
    minDepth = 3.4028235e38f;

    for (int c = 0; c < casterCount; c++) {
        int partCount = casters[c]->PartCount(-1);
        for (int part = 0; part < partCount; part++) {
            char* vertices;
            int count;
            unsigned short* indices;
            int indexCount;
            casters[c]->GetPart(part, (void**)&vertices, &count, &indices, &indexCount, 0, -1);
            if (mode == 3)
                ShadowTransformPointsOrtho(vertexBuffer + vertexBase, vertices, &lightMatrix, count, 0xc, 0x20);
            else
                ShadowTransformPointsProjective(vertexBuffer + vertexBase, vertices, &lightMatrix, count, 0xc, 0x20);

            for (int i = 0; i < indexCount; i += 3) {
                ShadowVec3* v0 = &vertexBuffer[indices[i + 2] + vertexBase];
                ShadowVec3* v1 = &vertexBuffer[indices[i + 1] + vertexBase];
                ShadowVec3* v2 = &vertexBuffer[indices[i] + vertexBase];
                float area = (v0->y - v2->y) * (v1->x - v2->x) - (v1->y - v2->y) * (v0->x - v2->x);
                if (area <= 0.0f)
                    continue;

                int points[6];
                bool inside = true;
                for (int k = 0; k < 3; k++) {
                    ShadowVec3* vertex = &vertexBuffer[*(indices + i + k) + vertexBase];
                    points[k * 2] = X87FloatToInt(vertex->x - 0.5f);
                    points[k * 2 + 1] = X87FloatToInt(vertex->y - 0.5f);
                    if (points[k * 2] <= 1 || points[k * 2] >= texture->size - 1
                        || points[k * 2 + 1] <= 1 || points[k * 2 + 1] >= texture->size - 1) {
                        inside = false;
                        break;
                    }
                }
                if (!inside)
                    continue;

                for (int j = 0; j < 3; j++) {
                    ShadowVec3* vertex = &vertexBuffer[indices[i + j] + vertexBase];
                    int x = points[j * 2];
                    int y = points[j * 2 + 1];
                    clipRect.left = clipRect.left < x ? clipRect.left : x;
                    clipRect.top = clipRect.top < y ? clipRect.top : y;
                    clipRect.right = clipRect.right > x ? clipRect.right : x;
                    clipRect.bottom = clipRect.bottom > y ? clipRect.bottom : y;
                    minDepth = vertex->z > minDepth ? minDepth : vertex->z;
                }
                ShadowFillTriangle(sizeShift, heightMap, shadowPixel, points, 1);
            }
            vertexBase += count;
        }
        if (casters[c]->field_0x27c != -1)
            casters[c]->ReleaseParts();
    }
    texture->UnlockPixels(0);
}

// 0x004dbd30 (1218 B): halves the dirty rectangle and box-filters the 2x2 height blocks of the
// locked texture into it (format 9: the four-sample sum is mapped to a blended 4444 value;
// otherwise the average); each row's tail and the rows no longer covered are cleared.
void ProjectedShadow::Present()
{
    if (surfaceFormat == 0x613)
        return;
    if (!field_0x98)
        return;

    heightMap = texture->LockPixels(0, 0, 0);
    int oldBottom = clipRect.bottom;
    int oldLeft = clipRect.left;
    int oldRight = clipRect.right;
    int oldTop = clipRect.top;
    int newBottom = oldBottom >> 1;
    clipRect.bottom = newBottom;
    clipRect.left = oldLeft >> 1;
    clipRect.top = oldTop >> 1;
    clipRect.right = oldRight >> 1;

    int index = clipRect.left + texture->size * clipRect.top;
    short* dstRow = heightMap + index;
    unsigned short* srcRow = (unsigned short*)heightMap + index * 2;
    int rowTail = oldRight - clipRect.right;
    int y;
    if (formatA == 9) {
        unsigned int key = colorKeyLow;
        unsigned int alpha = (unsigned short)shadowPixel;
        unsigned int keyLevel = key & 0xf;
        unsigned int sumMixA = (alpha + key) * 2;
        unsigned int sumMixB = key + alpha * 2 + alpha;
        unsigned int sumAlpha = alpha * 4;
        unsigned int halfShade = (unsigned char)((unsigned char)shadowPixel >> 1) & 7;
        unsigned int quarterShade = (unsigned char)((unsigned char)shadowPixel >> 2) & 3;
        unsigned short outMixA = (unsigned short)(keyLevel * 0x111 - halfShade * 0x111);
        unsigned short outMixB = (unsigned short)(keyLevel * 0x111 - (quarterShade + halfShade) * 0x111);
        unsigned short outMixC = (unsigned short)(keyLevel * 0x111 - quarterShade * 0x111);
        for (y = clipRect.top; y <= clipRect.bottom; y++) {
            unsigned short* srcTop = srcRow;
            unsigned short* srcBottom = srcRow + texture->size;
            short* dst = dstRow;
            int span = clipRect.right - clipRect.left;
            for (int x = 0; x <= span; x++) {
                unsigned int sum = srcTop[0] + srcTop[1] + srcBottom[0] + srcBottom[1];
                short out;
                if (sum == key * 4)
                    out = colorKeyLow;
                else if (sum == sumAlpha)
                    out = shadowPixel;
                else if (sum == sumMixA)
                    out = outMixA;
                else
                    out = sum == sumMixB ? outMixB : outMixC;
                *dst++ = out;
                srcTop += 2;
                srcBottom += 2;
            }
            FillShortsUnrolled(dst, rowTail, colorKeyLow, colorKeyPair);
            dstRow += texture->size;
            srcRow += texture->size * 2;
        }
    } else {
        for (y = clipRect.top; y <= clipRect.bottom; y++) {
            unsigned short* srcTop = srcRow;
            unsigned short* srcBottom = srcRow + texture->size;
            short* dst = dstRow;
            int span = clipRect.right - clipRect.left;
            for (int x = 0; x <= span; x++) {
                *dst++ = (short)((int)(srcTop[0] + srcTop[1] + srcBottom[0] + srcBottom[1]) >> 2);
                srcTop += 2;
                srcBottom += 2;
            }
            FillShortsUnrolled(dst, rowTail, colorKeyLow, colorKeyPair);
            dstRow += texture->size;
            srcRow += texture->size * 2;
        }
    }

    short* clearRow = dstRow + (oldLeft - clipRect.left);
    int clearCount = oldRight - oldLeft + 1;
    for (; y <= oldBottom; y++) {
        FillShortsUnrolled(clearRow, clearCount, colorKeyLow, colorKeyPair);
        clearRow += texture->size;
    }
    texture->UnlockPixels(0);
}
