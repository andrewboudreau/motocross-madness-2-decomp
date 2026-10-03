// Candidate: relocation evidence is incomplete; see docs/PHYSICS_VALIDATION.md.
// ProjectedShadow.cpp -- reconstruction of D:\aardvark\VC\krusty2\ProjectedShadow.cpp.
#include "shadow/ProjectedShadow.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

ProjectedShadow::ProjectedShadow(int flags)
    : GameObject(flags)
{
    casters = 0;
    casterCount = 0;
    casterCapacity = 0;
    heightMap = 0;
    lightDir = g_vec3Zero;
    lightVec2 = g_vec3Zero;
    field_0x50 = 0;
    texture = 0;
    camera = 0;
    field_0x5c = 0;
    memset(&clipRect, 0, sizeof(clipRect));
    mode = 1;
    field_0x98 = 1;
    updateThisFrame = 1;
    updatePeriod = 1;
    prevMaxX = 0x80000000;
    prevMaxY = 0x80000000;
    sizeShift = 0;
    field_0x94 = 0;
    vertexCapacity = 0;
    vertexBuffer = 0;
    shadowColor = 0xff000000;
    frameCounter = 0;
    receiverCount = 0;
    receivers = 0;
    receiverCapacity = 0;
    prevMinX = 0x7fffffff;
    prevMinY = 0x7fffffff;
    center = g_vec3Zero;
    halfExtents = g_vec3Zero;
    dirtyFlags = 0;
    lensScale = 1.0f;
}

ProjectedShadow::~ProjectedShadow()
{
    if (camera)
        camera->BaseObjectVirtualSlot2();
    if (texture) {
        if (texture->surface)
            texture->surface->PageUnlock(0);
        texture->BaseObjectVirtualSlot2();
    }
    if (vertexBuffer)
        operator delete(vertexBuffer, __FILE__, 0x5e);
    if (casters)
        operator delete(casters, __FILE__, 0x5f);
    if (receivers)
        operator delete(receivers, __FILE__, 0x60);
}

// 0x004dab00: appends a caster, growing the pointer array by one entry at a time (debug
// alloc at line 0xfe, realloc at 0x102), then resizes the vertex buffer.
void ProjectedShadow::AddCaster(ShadowCaster* caster)
{
    if (!casters) {
        casters = (ShadowCaster**)DebugMalloc(casterCount * 4 + 4, __FILE__, 0xfe);
        casterCapacity = casterCount + 1;
    } else if (casterCapacity < casterCount + 1) {
        casters = (ShadowCaster**)DebugRealloc(casters, (casterCount + 1) * 4, __FILE__, 0x102);
        casterCapacity = casterCount + 1;
    }
    casters[casterCount] = caster;
    casterCount++;
    ResizeVertexBuffer();
}

// 0x004dab90.
void ProjectedShadow::ClearCasters()
{
    casterCount = 0;
}

// 0x004daba0: same growth pattern for the receiver list (lines 0x115 / 0x119).
void ProjectedShadow::AddReceiver(ShadowReceiver* receiver)
{
    if (!receivers) {
        receivers = (ShadowReceiver**)DebugMalloc(receiverCount * 4 + 4, __FILE__, 0x115);
        receiverCapacity = receiverCount + 1;
    } else if (receiverCapacity < receiverCount + 1) {
        receivers = (ShadowReceiver**)DebugRealloc(receivers, (receiverCount + 1) * 4, __FILE__, 0x119);
        receiverCapacity = receiverCount + 1;
    }
    receivers[receiverCount] = receiver;
    receiverCount++;
}

// 0x004dac50: sums the casters' vertex counts and (re)allocates the 12-byte-per-vertex buffer
// (lines 0x131 / 0x134), growing only.
void ProjectedShadow::ResizeVertexBuffer()
{
    int total = 0;
    for (int i = 0; i < casterCount; i++)
        total += casters[i]->GetVertexCount();
    if (!vertexBuffer) {
        vertexCapacity = total;
        vertexBuffer = (ShadowVec3*)DebugMalloc(total * 12, __FILE__, 0x131);
    } else if (total > vertexCapacity) {
        vertexBuffer = (ShadowVec3*)DebugRealloc(vertexBuffer, total * 12, __FILE__, 0x134);
        vertexCapacity = total;
    }
}

// 0x004dafc0 (slot 10): counts frames and flags every updatePeriod-th one, then forwards the
// time step to the camera.
int ProjectedShadow::GameObjectVirtualSlot10(float dt)
{
    frameCounter++;
    updateThisFrame = 0;
    if (frameCounter % updatePeriod == 0)
        updateThisFrame = 1;
    return camera->GameObjectVirtualSlot10(dt);
}

// 0x004dace0: converts an 8 bit shadow strength into the 4 bit alpha of the surface format
// (rounded, 16 clamped to 15) and fills the matching colour key and tint.  Only the 0x115c
// (4444) surface formats use the quantised value; every other format keeps the full 0x8000.
void ProjectedShadow::SetShadowStrength(int strength)
{
    if (surfaceFormat == 0x115c) {
        if (formatA == 9) {
            field_0x94 = ((strength & 0xff) + 8) / 16;
            if (field_0x94 == 16)
                field_0x94 = 15;
            colorKeyLow = 0xfff;
            colorKeyPair = 0xfff0fff;
            shadowColor = 0xff404040;
            field_0x94 = (((field_0x94 << 4) | field_0x94) << 4) | field_0x94;
        } else if (formatB == 5) {
            field_0x94 = ((strength & 0xff) + 8) / 16;
            if (field_0x94 == 16)
                field_0x94 = 15;
            field_0x94 <<= 12;
            colorKeyLow = (short)0xffff;
            colorKeyPair = 0xffffffff;
            shadowColor = 0xff404040;
        } else {
            field_0x94 = ((strength & 0xff) + 8) / 16;
            if (field_0x94 == 16)
                field_0x94 = 15;
            field_0x94 <<= 12;
            shadowColor = 0xff404040;
            colorKeyLow = 0;
            colorKeyPair = 0;
        }
    } else {
        field_0x94 = (short)0x8000;
        colorKeyLow = 0x7fff;
        colorKeyPair = 0x7fff7fff;
        shadowColor = 0xff404040;
    }
}

// 0x004dae30: grey level -> opaque 0xffvvvvvv colour.
void ProjectedShadow::SetShadowGrey(int level)
{
    shadowColor = (((0xffffff00 | level) << 8 | level) << 8) | level;
}

// 0x004dae50: mode 1 sets a normalised light direction plus a look-at point on the camera,
// mode 3 only the direction.  Returns 0 for other modes.
int ProjectedShadow::SetLight(int newMode, const ShadowVec3* dir, const ShadowVec3* pos)
{
    if (newMode == 1) {
        ShadowVec3 n;
        float lengthSq = dir->x * dir->x + dir->y * dir->y + dir->z * dir->z;
        if (lengthSq == 1.0f) {
            n = *dir;
        } else {
            float inv = FastInvSqrt(lengthSq);
            n.x = inv * dir->x;
            n.y = inv * dir->y;
            n.z = inv * dir->z;
        }
        lightDir = n;
        lightVec2 = *pos;
        camera->SetLookAt(pos, 0, 0, 0, 0);
        mode = newMode;
        return 1;
    }
    if (newMode == 3) {
        ShadowVec3 n;
        float lengthSq = dir->x * dir->x + dir->y * dir->y + dir->z * dir->z;
        if (lengthSq == 1.0f) {
            n = *dir;
        } else {
            float inv = FastInvSqrt(lengthSq);
            n.x = inv * dir->x;
            n.y = inv * dir->y;
            n.z = inv * dir->z;
        }
        mode = newMode;
        lightDir = n;
        return 1;
    }
    return 0;
}

// 0x004db000 (slot 12): per-frame update.  Recomputes the caster bounds, then on update frames
// asks every receiver to prepare and report dirtiness; when any is dirty it renders the
// shadow, finishes the receivers and presents.
int ProjectedShadow::GameObjectVirtualSlot12()
{
    ComputeBounds();
    dirtyFlags = 0;
    if (updateThisFrame) {
        for (int i = 0; i < receiverCount; i++) {
            if (receivers[i]->UnknownVirtualSlot27()) {
                receivers[i]->UnknownVirtualSlot28();
                dirtyFlags |= receivers[i]->UnknownVirtualSlot29();
            }
        }
        if (dirtyFlags) {
            RenderShadow();
            for (int i = 0; i < receiverCount; i++)
                receivers[i]->UnknownVirtualSlot30();
            if (updatePeriod == 1)
                TintCasterVertices();
            Present();
        }
    }
    return 1;
}

// 0x004dc410 (slot 13): after a dirty update, uploads the union of the previous and current
// dirty rectangles (previous 0x78..0x84, current clipRect) to the texture, then makes the
// current one the previous one.
int ProjectedShadow::GameObjectVirtualSlot13()
{
    if (updateThisFrame && dirtyFlags) {
        int rect[4];
        rect[0] = prevMinX < clipRect.left ? prevMinX : clipRect.left;
        rect[2] = prevMaxX + 1 > clipRect.right ? prevMaxX + 1 : clipRect.right;
        rect[1] = prevMinY < clipRect.top ? prevMinY : clipRect.top;
        rect[3] = prevMaxY + 1 > clipRect.bottom ? prevMaxY + 1 : clipRect.bottom;
        texture->UnknownVirtualSlot9(rect, -1);
        prevMinX = clipRect.left;
        prevMaxX = clipRect.right;
        prevMinY = clipRect.top;
        prevMaxY = clipRect.bottom;
    }
    return 1;
}

// 0x004da7b0: binds the shadow to its render target, picks a surface format from the device
// capabilities (and the DriverInfo\<driver>\DestColorShadow setting), creates the shadow
// texture and camera and retargets the render target at the texture while the camera is built.
// Returns this, or 0 after releasing the object when no usable format or allocation exists.
ProjectedShadow* ProjectedShadow::Init(ShadowRenderTarget* target, void* textureOwner)
{
    GameObject::GameObjectVirtualSlot8((int)target);
    field_0x5c = (int)target;

    char name[0x100];
    sprintf(name, "DriverInfo\\%s\\DestColorShadow", Host()->display->driverName);
    if (g_pShadowSettings->QueryFlag(name, 0) && (Host()->capsA & 0x100) && (Host()->capsB & 1)) {
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
            BaseObjectVirtualSlot2();
            return 0;
        }
    }

    if (Host()->IsFormatSupported(0x115c)) {
        surfaceFormat = 0x115c;
    } else if (Host()->IsFormatSupported(0x613) && (Host()->capsC & 0x2000)) {
        surfaceFormat = 0x613;
        field_0x98 = 0;
    } else {
        BaseObjectVirtualSlot2();
        return 0;
    }

    texture = new(__FILE__, 0xaa) ShadowTexture(textureOwner, 1);
    if (!texture
        || !texture->Create(0, 0x80, 0x80, 0x80, 0x80, surfaceFormat, surfaceFormat, 0, 8, 0, 0, 0,
                            formatA, formatB, 0, 0x80, 0xff00ff)) {
        BaseObjectVirtualSlot2();
        return 0;
    }
    texture->UnknownVirtualSlot8(1, 0, 1);
    field_0x60 = texture->pixels;
    if (!field_0x60) {
        BaseObjectVirtualSlot2();
        return 0;
    }
    texture->surface->PageLock(0);

    int savedHeight = Host()->height;
    void* savedSurface = Host()->surface;
    int savedWidth = Host()->width;
    Host()->width = texture->size;
    Host()->height = texture->height;
    Host()->surface = texture->pixels;
    ShadowOwner* owner = Host()->owner;

    camera = new(__FILE__, 0xcd) ShadowCameraProxy(1);
    if (!camera || !camera->GameObjectVirtualSlot8((int)Host())) {
        BaseObjectVirtualSlot2();
        return 0;
    }
    Host()->surface = savedSurface;
    Host()->width = savedWidth;
    Host()->height = savedHeight;
    Host()->SetOwner(owner);
    Host()->device->SetTransformLike(owner->block_0x1a0);
    camera->SetLensScale(0.3f);
    for (int bits = 0x40; bits; bits >>= 1)
        sizeShift++;
    clipRect.left = 0;
    clipRect.top = 0;
    clipRect.right = texture->size - 1;
    clipRect.bottom = texture->size - 1;
    return this;
}

// 0x004dc2b0 (352 B): paints shadowColor into the colour word (+0x10 of 0x20-byte vertices) of
// every caster vertex whose projected position lands on a height-map cell equal to colorKeyLow.
// PARTIAL by design: retail converts with an inline fistp helper (fstp/fld float temp, then
// "fistp [ptr]" through a local address) that is inline asm; the no-asm rule forces the
// (int) cast, which calls __ftol instead.  Same situation as Terrain QueryGround 0x507c10.
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

static inline float AbsF(float v) { return v < 0.0f ? -v : v; }

// 0x004db0d0 (2020 B): recomputes the light-space bounds of all casters and derives the shadow
// camera set-up.  Each caster's local bounds (centre c, half extents h) are carried through the
// caster's world matrix at +0xf8 (extents by the sum of absolute row contributions, centre as a
// row vector); the running min/max give this->center / this->halfExtents, and lensScale is the
// length of halfExtents.  Mode 3 (directional) builds an orthographic matrix and looks along
// -lightDir*1000; mode 1 (point light) derives lightDir from the centre, a field of view from
// atan2(lensScale, distance) and normalises the direction.  The result, camera matrix times a
// local scale/translate matrix, goes to lightMatrix (+0xd8).  PARTIAL: the FPU scheduling of the
// inlined vector helpers differs from retail.
void ProjectedShadow::ComputeBounds()
{
    if (!casters)
        return;
    ShadowVec3 mn, mx;
    mn.x = mn.y = mn.z = 3.4028235e38f;
    mx.x = mx.y = mx.z = -3.4028235e38f;
    for (int i = 0; i < casterCount; i++) {
        ShadowVec3 c, ext;
        casters[i]->GetBounds(&c, &ext);
        const ShadowMatrix& M = casters[i]->world;
        float a00 = ext.x * M.m[0][0];
        float a01 = ext.x * M.m[0][1];
        float a02 = ext.x * M.m[0][2];
        float a10 = ext.y * M.m[1][0];
        float a11 = ext.y * M.m[1][1];
        float a12 = ext.y * M.m[1][2];
        float a20 = ext.z * M.m[2][0];
        float a21 = ext.z * M.m[2][1];
        float a22 = ext.z * M.m[2][2];
        ext.x = AbsF(a00) + AbsF(a10) + AbsF(a20);
        ext.y = AbsF(a01) + AbsF(a11) + AbsF(a21);
        ext.z = AbsF(a02) + AbsF(a12) + AbsF(a22);
        ShadowVec3 w;
        w.x = c.z * M.m[2][0] + c.y * M.m[1][0] + c.x * M.m[0][0] + M.m[3][0];
        w.y = c.z * M.m[2][1] + c.y * M.m[1][1] + c.x * M.m[0][1] + M.m[3][1];
        w.z = c.z * M.m[2][2] + c.y * M.m[1][2] + c.x * M.m[0][2] + M.m[3][2];
        float lx = w.x - ext.x, ly = w.y - ext.y, lz = w.z - ext.z;
        float hx = w.x + ext.x, hy = w.y + ext.y, hz = w.z + ext.z;
        if (lx < mn.x) mn.x = lx;
        if (ly < mn.y) mn.y = ly;
        if (lz < mn.z) mn.z = lz;
        if (hx > mx.x) mx.x = hx;
        if (hy > mx.y) mx.y = hy;
        if (hz > mx.z) mx.z = hz;
    }
    center.x = (mx.x + mn.x) * 0.5f;
    center.y = (mx.y + mn.y) * 0.5f;
    center.z = (mx.z + mn.z) * 0.5f;
    halfExtents.x = (mx.x - mn.x) * 0.5f;
    halfExtents.y = (mx.y - mn.y) * 0.5f;
    halfExtents.z = (mx.z - mn.z) * 0.5f;
    float lenSq = halfExtents.x * halfExtents.x + halfExtents.y * halfExtents.y + halfExtents.z * halfExtents.z;
    lensScale = lenSq == 0.0f ? 0.0f : (lenSq == 1.0f ? 1.0f : 1.0f / FastInvSqrt(lenSq));

    if (mode == 3) {
        ShadowMatrix m = ShadowMatrixIdentity();
        float half = (float)texture->size * 0.5f;
        m.m[0][0] = half / lensScale;
        m.m[1][1] = -(half / lensScale);
        m.m[3][0] = half;
        m.m[3][1] = half;
        ShadowVec3 eye;
        eye.x = center.x - lightDir.x * 1000.0f;
        eye.y = center.y - lightDir.y * 1000.0f;
        eye.z = center.z - lightDir.z * 1000.0f;
        camera->SetLookAt(&eye, 0, 0, 0, 0);
        camera->UnknownVirtualSlot29(center);
        camera->UnknownVirtualSlot28();
        lightMatrix = ShadowMatrixMultiply(m, camera->matrix_0xac);
    } else if (mode == 1) {
        lightDir.x = center.x - lightVec2.x;
        lightDir.y = center.y - lightVec2.y;
        lightDir.z = center.z - lightVec2.z;
        float lenSq2 = lightDir.x * lightDir.x + lightDir.y * lightDir.y + lightDir.z * lightDir.z;
        float dist = lenSq2 == 0.0f ? 0.0f : (lenSq2 == 1.0f ? 1.0f : 1.0f / FastInvSqrt(lenSq2));
        float fov = (float)atan2(lensScale, dist) * 114.59156f;
        camera->SetLookAt(&lightVec2, 0, 0, 0, &fov);
        float lenSq3 = lightDir.x * lightDir.x + lightDir.y * lightDir.y + lightDir.z * lightDir.z;
        if (lenSq3 != 1.0f) {
            float inv = FastInvSqrt(lenSq3);
            ShadowVec3 n;
            n.x = inv * lightDir.x;
            n.y = inv * lightDir.y;
            n.z = inv * lightDir.z;
            lightDir = n;
        }
        camera->UnknownVirtualSlot29(center);
        camera->UnknownVirtualSlot28();
        ShadowMatrix m = ShadowMatrixIdentity();
        m.m[0][0] = (float)texture->size;
        m.m[1][1] = (float)texture->size;
        lightMatrix = ShadowMatrixMultiply(m, camera->matrix_0xec);
    }
}

// 0x004db8c0 (1074 B): renders the casters into the 16-bit height texture.  Locks the texture,
// clears the previous dirty rectangle to colorKeyLow, resets the rectangle to "empty" and the
// depth to FLT_MAX, then for every part of every caster transforms its vertices into
// vertexBuffer (ortho for mode 3, projective otherwise), culls back-facing triangles by the
// signed screen area, requires all three projected points to lie inside (1, size-1), grows
// clipRect and minDepth, and rasterises the triangle with the height word field_0x94.
// PARTIAL: retail's 16-bit clear is an unrolled dword fill through a 15-entry jump table and
// the vertex helpers are inlined differently.
void ProjectedShadow::RenderShadow()
{
    int vertexBase = 0;
    heightMap = texture->LockPixels(0, 0, 0);
    if (!heightMap)
        return;

    if (clipRect.right >= clipRect.left) {
        short* row = heightMap + clipRect.top * texture->size + clipRect.left;
        for (int y = clipRect.top; y <= clipRect.bottom; y++) {
            int n = clipRect.right - clipRect.left + 1;
            for (int i = 0; i < n; i++)
                row[i] = (short)colorKeyLow;
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
                ShadowFillTriangle(sizeShift, heightMap, field_0x94, points, 1);
            }
            vertexBase += count;
        }
        if (casters[c]->field_0x27c != -1)
            casters[c]->ReleaseParts();
    }
    texture->UnlockPixels(0);
}

static void FillShorts(short* dst, int count, short value)
{
    for (int i = 0; i < count; i++)
        dst[i] = value;
}

// 0x004dbd30 (1218 B plus three jump tables of 45/30/15 entries, which are unrolled dword fills):
// after rendering, halves the dirty rectangle and box-filters the 2x2 height blocks of the
// locked texture into it.  For formatA == 9 the four-sample sum is matched against the exact
// sums of colorKeyLow / field_0x94 combinations and mapped to a blended 4-bit-per-channel value
// (n*0x111 - ...), otherwise the average (sum >> 2) is stored.  Afterwards the part of the old
// rectangle not covered by the halved one is cleared to colorKeyLow and the texture unlocked.
// PARTIAL: retail clears with Duff-style unrolled dword stores through the jump tables; this
// uses plain loops.
void ProjectedShadow::Present()
{
    if (surfaceFormat == 0x613 || !field_0x98)
        return;

    heightMap = texture->LockPixels(0, 0, 0);
    int oldBottom = clipRect.bottom;
    int oldLeft = clipRect.left;
    int oldRight = clipRect.right;
    int oldTop = clipRect.top;
    clipRect.bottom = oldBottom >> 1;
    clipRect.left = oldLeft >> 1;
    clipRect.top = oldTop >> 1;
    clipRect.right = oldRight >> 1;

    int size = texture->size;
    int index = clipRect.top * size + clipRect.left;
    short* dst = heightMap + index;
    unsigned short* src = (unsigned short*)(heightMap + index * 2);
    int rowTail = oldRight - clipRect.right;

    unsigned int key = colorKeyLow;
    unsigned int height = (unsigned short)field_0x94;
    unsigned int n = key & 0xf;
    unsigned int m = (height >> 1) & 7;
    unsigned int pp = (height >> 2) & 3;
    unsigned int sumKey4 = key * 4;
    unsigned int sumHeight4 = height * 4;
    unsigned int sumMixA = (height + key) * 2;
    unsigned int sumMixB = key + height * 3;
    unsigned int outMixA = n * 0x111 - m * 0x111;
    unsigned int outMixB = n * 0x111 - (pp + m) * 0x111;
    unsigned int outMixC = n * 0x111 - pp * 0x111;

    int y = clipRect.top;
    for (; y <= clipRect.bottom; y++) {
        unsigned short* srcNext = src + size;
        int width = clipRect.right - clipRect.left + 1;
        short* d = dst;
        for (int x = 0; x < width && clipRect.right >= clipRect.left; x++) {
            unsigned int sum = srcNext[1] + src[1] + srcNext[0] + src[0];
            if (formatA == 9) {
                short out;
                if (sum == sumKey4)
                    out = colorKeyLow;
                else if (sum == sumHeight4)
                    out = field_0x94;
                else if (sum == sumMixA)
                    out = (short)outMixA;
                else if (sum == sumMixB)
                    out = (short)outMixB;
                else
                    out = (short)outMixC;
                *d++ = out;
            } else {
                *d++ = (short)(sum >> 2);
            }
            src += 2;
            srcNext += 2;
        }
        FillShorts(d, rowTail, colorKeyLow);
        dst += size;
        src += size * 2 - width * 2;
    }

    short* clearRow = dst + (oldLeft - clipRect.left);
    for (int rows = oldBottom - clipRect.bottom; rows > 0; rows--) {
        FillShorts(clearRow, oldRight - oldLeft + 1, colorKeyLow);
        clearRow += texture->size;
    }
    texture->UnlockPixels(0);
}
