// Near-miss ProjectedShadow.cpp candidates (D:\aardvark\VC\krusty2\ProjectedShadow.cpp), kept
// out of src/krusty2/shadow/ProjectedShadow.cpp until they match.  The canonical file is included
// first so the vector set and the class are the same.  Each function's note records what still
// differs; the scores are in targets.json and docs/NEAR_MISS_INDEX.md.
#include "../../../src/reconstructed/TrackGameGlobal.h"
#include "shadow/ProjectedShadow.cpp"

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
// PARTIAL by design: retail converts with an inline fistp helper (fstp/fld float temp, then
// "fistp [ptr]" through a local address) that is inline asm; the no-asm rule forces the
// (int) cast, which calls __ftol instead.  Same situation as Terrain QueryGround 0x507c10.
// Verified exclusion: retail 0x004dc351 and 0x004dc36c are bare `fistp dword ptr [eax]` in an
// ebp frame (docs/REBUILD_GAPS.md inline-asm table).
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
                int x = (int)(vertexBuffer[totalVertices + i].x - 0.5f);
                int y = (int)(vertexBuffer[totalVertices + i].y - 0.5f);
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
// camera set-up.  Each caster's local bounds (centre c, half extents h) are carried through the
// caster's world matrix at +0xf8 (extents by the sum of absolute row contributions, centre as a
// row vector); the running min/max give this->center / this->halfExtents, and lensScale is the
// length of halfExtents.  Mode 3 (directional) builds an orthographic matrix and looks along
// -lightDir*1000; mode 1 (point light) derives lightDir from the centre, a field of view from
// atan2(lensScale, distance) and normalises the direction.  The result, camera matrix times a
// local scale/translate matrix, goes to lightMatrix (+0xd8).  Every exit returns 1 (retail's
// shared epilogue sets eax = 1).  The extents and centre are transformed through out-pointer
// helpers with by-value inputs, centre/halfExtents, the mode-3 eye and the mode-1 direction are
// built through by-value temporaries, and the mode-1 normalisation is written out with the
// out-of-line constructor.  PARTIAL: retail copies the GetBounds extents into three separate
// slots before the products (VC6 elides that copy here), which shifts every frame slot.
// One product buffer t serves both modes (0x134 -> 0xf4, 177 -> 188 strict).
// Retail's frame (tools/frame_layout.py, 0xc0 against 0xf4 here) has no 64-byte buffer beside m:
// the identity call and the product's result pointer are both m's own slot (0x64), with
// fov 0xc, c 0x1c, mn 0x28, mx 0x34, ext 0x40, eye 0x4c and sq 0x78, and 12-byte temporaries
// at 0, 0x10 and 0x58.  Passing &m as the product's result instead keeps a 64-byte identity
// temporary (0xf4, 1970 bytes, 185 strict, 244 differing instructions); `m = ShadowMatrixIdentity()` after the declaration
// compiles the same as the initialisation.
int ProjectedShadow::ComputeBounds()
{
    if (!casters)
        return 1;
    ShadowMatrix t;
    ShadowVec3 mn, mx;
    mn.x = mn.y = mn.z = 3.4028235e38f;
    mx.x = mx.y = mx.z = -3.4028235e38f;
    for (int i = 0; i < casterCount; i++) {
        ShadowVec3 c, ext;
        casters[i]->GetBounds(&c, &ext);
        ShadowTransformExtents(&ext, ext, casters[i]);
        ShadowTransformCoord(&c, c, casters[i]->world);
        ShadowVec3 lo = ShadowDifference(c, ext);
        ShadowVec3 hi = ShadowSum(c, ext);
        if (lo.x < mn.x) mn.x = lo.x;
        if (lo.y < mn.y) mn.y = lo.y;
        if (lo.z < mn.z) mn.z = lo.z;
        if (hi.x > mx.x) mx.x = hi.x;
        if (hi.y > mx.y) mx.y = hi.y;
        if (hi.z > mx.z) mx.z = hi.z;
    }
    center = ShadowScale(ShadowSum(mx, mn), 0.5f);
    halfExtents = ShadowScale(ShadowDifference(mx, mn), 0.5f);
    lensScale = ShadowMagnitude(halfExtents);

    if (mode == 3) {
        ShadowMatrix m = ShadowMatrixIdentity();
        float half = (float)texture->size * 0.5f;
        m.m[0][0] = half / lensScale;
        m.m[1][1] = -(half / lensScale);
        m.m[3][0] = half;
        m.m[3][1] = half;
        ShadowVec3 eye = ShadowDifference(center, ShadowScale(lightDir, 1000.0f));
        camera->SetLookAt(&eye, 0, 0, 0, 0);
        camera->UnknownVirtualSlot29(center);
        camera->UnknownVirtualSlot28();
        lightMatrix = *ShadowMatrixMultiplyInto(&t, m, camera->matrix_0xac);
        return 1;
    } else if (mode == 1) {
        lightDir = ShadowDifference(center, lightVec2);
        float dist = ShadowMagnitude(lightDir);
        float fov = (float)atan2(lensScale, dist) * 114.59156f;
        camera->SetLookAt(&lightVec2, 0, 0, 0, &fov);
        ShadowVec3& d = lightDir;
        float y = d.y, x = d.x, z = d.z;
        float sq = z * z + (x * x + y * y);
        ShadowVec3 n;
        if (sq == 1.0f) {
            n = d;
        } else {
            float inv = FastInvSqrt(sq);
            n = ShadowVec3Call(d.x * inv, d.y * inv, d.z * inv);
        }
        d = n;
        camera->UnknownVirtualSlot29(center);
        camera->UnknownVirtualSlot28();
        ShadowMatrix m = ShadowMatrixIdentity();
        m.m[0][0] = (float)texture->size;
        m.m[1][1] = (float)texture->size;
        lightMatrix = *ShadowMatrixMultiplyInto(&t, m, camera->matrix_0xec);
    }
    return 1;
}

// 0x004db8c0 (1074 B): renders the casters into the 16-bit height texture.  Locks the texture,
// clears the previous dirty rectangle to colorKeyLow, resets the rectangle to "empty" and the
// depth to FLT_MAX, then for every part of every caster transforms its vertices into
// vertexBuffer (ortho for mode 3, projective otherwise), culls back-facing triangles by the
// signed screen area, requires all three projected points to lie inside (1, size-1), grows
// clipRect and minDepth, and rasterises the triangle with the height word shadowPixel.
// PARTIAL: the 16-bit clear goes through FillShortsUnrolled (retail's 15-entry jump table) and
// the size matches; the vertex helpers are inlined differently.  Verified exclusion: retail 0x004dbb5c and
// 0x004dbb8b are bare `fistp dword ptr [eax]` in an ebp frame (inline-asm rounding).
// s15 reference diff: the two extra __ftol calls are those roundings; the missing immediates
// (0xb980004d ...) are the jump table after the ret at 0x004dbcf1 read as code (false positive).
// Fills count shorts at p with value: one short to reach dword alignment, then dword pairs
// (pair = value in both halves) unrolled 16 per pass with a Duff's device, then the last
// odd short.
static inline void FillShortsUnrolled(short* p, int count, short value, unsigned int pair)
{
    if (count > 0) {
        short* q = p;
        if ((int)p & 2) {
            *p = value;
            q = p + 1;
            count--;
        }
        int pairs = count >> 1;
        if (pairs > 0) {
            unsigned int* d = (unsigned int*)q;
            int k = pairs - 1;
            unsigned int passes = (unsigned int)(pairs + 15) >> 4;
            do {
                switch (k) {
                default: d[15] = pair;
                case 14: d[14] = pair;
                case 13: d[13] = pair;
                case 12: d[12] = pair;
                case 11: d[11] = pair;
                case 10: d[10] = pair;
                case 9: d[9] = pair;
                case 8: d[8] = pair;
                case 7: d[7] = pair;
                case 6: d[6] = pair;
                case 5: d[5] = pair;
                case 4: d[4] = pair;
                case 3: d[3] = pair;
                case 2: d[2] = pair;
                case 1: d[1] = pair;
                case 0: d[0] = pair;
                }
                k -= 16;
                d += 16;
            } while (--passes);
        }
        if (count & 1)
            q[count - 1] = value;
    }
}

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
                    ShadowVec3* v = &vertexBuffer[*(indices + i + k) + vertexBase];
                    points[k * 2] = (int)(v->x - 0.5f);
                    points[k * 2 + 1] = (int)(v->y - 0.5f);
                    if (points[k * 2] <= 1 || points[k * 2] >= texture->size - 1
                        || points[k * 2 + 1] <= 1 || points[k * 2 + 1] >= texture->size - 1) {
                        inside = false;
                        break;
                    }
                }
                if (!inside)
                    continue;

                for (int j = 0; j < 3; j++) {
                    ShadowVec3* v = &vertexBuffer[indices[i + j] + vertexBase];
                    int x = points[j * 2];
                    int y = points[j * 2 + 1];
                    clipRect.left = clipRect.left < x ? clipRect.left : x;
                    clipRect.top = clipRect.top < y ? clipRect.top : y;
                    clipRect.right = clipRect.right > x ? clipRect.right : x;
                    clipRect.bottom = clipRect.bottom > y ? clipRect.bottom : y;
                    minDepth = v->z > minDepth ? minDepth : v->z;
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

// 0x004dbd30 (1218 B plus three 15-entry jump tables): after rendering, halves the dirty
// rectangle and box-filters the 2x2 height blocks of the locked texture into it.  For
// formatA == 9 the four-sample sum is matched against the exact sums of colorKeyLow /
// shadowPixel combinations and mapped to a blended 4-bit-per-channel value (n*0x111 - ...),
// otherwise the average (sum >> 2) is stored; retail has one row loop per case.  Each row's
// tail and the rows the halved rectangle no longer covers are cleared to colorKeyLow with
// the Duff's-device fill (its three expansions are the three jump tables).
// Near miss: block structure, the fill expansions and the counted inner loops match; left:
// register assignment of the clip fields and of the format-9 constants (retail computes the
// nibble masks with byte operations on the low byte of shadowPixel).
// s15 reference diff (masked match): its missing immediates lie in the three jump tables after
// the ret at 0x004dc1f1, read as code (false positive).
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
        unsigned int n = key & 0xf;
        unsigned int sumMixA = (alpha + key) * 2;
        unsigned int sumMixB = key + alpha * 2 + alpha;
        unsigned int sumAlpha = alpha * 4;
        unsigned int m = (unsigned char)((unsigned char)shadowPixel >> 1) & 7;
        unsigned int pp = (unsigned char)((unsigned char)shadowPixel >> 2) & 3;
        unsigned short outMixA = (unsigned short)(n * 0x111 - m * 0x111);
        unsigned short outMixB = (unsigned short)(n * 0x111 - (pp + m) * 0x111);
        unsigned short outMixC = (unsigned short)(n * 0x111 - pp * 0x111);
        for (y = clipRect.top; y <= clipRect.bottom; y++) {
            unsigned short* s = srcRow;
            unsigned short* s2 = srcRow + texture->size;
            short* d = dstRow;
            int w = clipRect.right - clipRect.left;
            for (int x = 0; x <= w; x++) {
                unsigned int sum = s[0] + s[1] + s2[0] + s2[1];
                short out;
                if (sum == key * 4)
                    out = colorKeyLow;
                else if (sum == sumAlpha)
                    out = shadowPixel;
                else if (sum == sumMixA)
                    out = outMixA;
                else
                    out = sum == sumMixB ? outMixB : outMixC;
                *d++ = out;
                s += 2;
                s2 += 2;
            }
            FillShortsUnrolled(d, rowTail, colorKeyLow, colorKeyPair);
            dstRow += texture->size;
            srcRow += texture->size * 2;
        }
    } else {
        for (y = clipRect.top; y <= clipRect.bottom; y++) {
            unsigned short* s = srcRow;
            unsigned short* s2 = srcRow + texture->size;
            short* d = dstRow;
            int w = clipRect.right - clipRect.left;
            for (int x = 0; x <= w; x++) {
                *d++ = (short)((int)(s[0] + s[1] + s2[0] + s2[1]) >> 2);
                s += 2;
                s2 += 2;
            }
            FillShortsUnrolled(d, rowTail, colorKeyLow, colorKeyPair);
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
