// ProjectedShadow.cpp -- reconstruction of D:\aardvark\VC\krusty2\ProjectedShadow.cpp
// (__FILE__ string 0x00571fd8; xrefs 0x004da745..0x004dacbc in Init, AddCaster, AddReceiver
// and ResizeVertexBuffer; tier 1).
//
// Extent (strong inference): 0x004da570..0x004dc610.  ShadowCamera's methods
// (0x004da520..0x004da560) precede it; RTTI ProjectedShadow : GameObject (vtable 0x005575c8,
// slots 10, 12, 13 and 23); slot 23 is the `xor eax, eax; ret 8` stub 0x004dc4c0 that
// identical-code folding shares with Terrain and StatsOverlay (registered with
// src/krusty2/broadphase/Terrain.cpp); the .CRT$XCU 244-247 vector set 0x004dc4d0..0x004dc60b
// closes the file.  The functions here are the unit's strict exact ones
// (src/krusty2/shadow/targets.json).  Init, ComputeBounds and Present are near misses and
// RenderShadow and TintCasterVertices round with inline-asm fistp (excluded); all five are in
// samples/physics/shadow/ProjectedShadowNearMisses.cpp, which includes this file.
#include "shadow/ProjectedShadow.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

// The four per-file vector constants (0x00689b48, 0x00689b58, 0x00689b68,
// 0x00689b38). Their initializers 0x004dc4d0..0x004dc60b follow this file's
// last function (slot 13, 0x004dc410) and the shared Terrain stub 0x004dc4c0;
// only this file's code reads them (the constructor at 0x004da591 and
// 0x004da65f reads the zero vector), and .CRT$XCU lists them right after
// ProCircuitProcs.cpp's set (see docs/INITIALIZERS.md). In the original source
// they come from a widely included header; the stand-in type is ours.
struct ShadowConstVec3 : ShadowVec3 {
    ShadowConstVec3(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }
};
static const ShadowConstVec3 kVec3Zero = ShadowConstVec3(0.0f, 0.0f, 0.0f);
static const ShadowConstVec3 kVec3XAxis = ShadowConstVec3(1.0f, 0.0f, 0.0f);
static const ShadowConstVec3 kVec3YAxis = ShadowConstVec3(0.0f, 1.0f, 0.0f);
static const ShadowConstVec3 kVec3ZAxis = ShadowConstVec3(0.0f, 0.0f, 1.0f);

ProjectedShadow::ProjectedShadow(int flags)
    : GameObject(flags)
{
    casters = 0;
    casterCount = 0;
    casterCapacity = 0;
    heightMap = 0;
    lightDir = kVec3Zero;
    lightVec2 = kVec3Zero;
    field_0x50 = 0;
    texture = 0;
    camera = 0;
    field_0x5c = 0;
    memset(&clipRect, 0, sizeof(clipRect));
    mode = 1;
    field_0x98 = 1;
    updateThisFrame = 1;
    updatePeriod = 1;
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
    prevMaxX = 0x80000000;
    prevMinY = 0x7fffffff;
    prevMaxY = 0x80000000;
    center = kVec3Zero;
    halfExtents = kVec3Zero;
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
        DebugFree(vertexBuffer, __FILE__, 0x5e);
    if (casters)
        DebugFree(casters, __FILE__, 0x5f);
    if (receivers)
        DebugFree(receivers, __FILE__, 0x60);
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

// Inline normalisation with the shape of Math3D's Vec3Normalize (0x005087b0): the
// components are loaded y, x, z, the squared length is z*z + (x*x + y*y), and a unit vector
// is returned unchanged.  The constructor-built result goes through a stack temporary.
static inline ShadowVec3 ShadowNormalize(const ShadowVec3& v)
{
    float y = v.y, x = v.x, z = v.z;
    float sq = z * z + (x * x + y * y);
    if (sq == 1.0f)
        return v;
    float inv = FastInvSqrt(sq);
    return ShadowConstVec3(v.x * inv, v.y * inv, v.z * inv);
}

// 0x004dae50: mode 1 sets the normalised light direction and the position the camera looks
// at, mode 3 only the direction; both record the mode and return 1.  Other modes return 0.
// Retail keeps newMode in ebx for both `mode` stores, which comes from the single store
// after the if/else chain (with a store per branch VC6 stores the constants 1 and 3).
int ProjectedShadow::SetLight(const ShadowVec3* pos, const ShadowVec3* dir, int newMode)
{
    if (newMode == 1) {
        lightDir = ShadowNormalize(*dir);
        lightVec2 = *pos;
        camera->SetLookAt(pos, 0, 0, 0, 0);
    } else if (newMode == 3) {
        lightDir = ShadowNormalize(*dir);
    } else {
        return 0;
    }
    mode = newMode;
    return 1;
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
