// TerrainShadow.cpp -- TerrainShadow (vtable 0x005582d4).
// Ownership is uncertain, so this lives in samples/: no __FILE__ xref lies in 0x00508ae0..0x0050a58f
// (Terrain.cpp's last is 0x00507b38, Texmap.cpp's first 0x0050a6bc). The vector $E block at
// 0x005089a0 just before it closes Terrain.cpp (docs/INITIALIZERS.md), so this code starts a unit
// of its own or continues Terrain.cpp (tier 3).
#include "TerrainShadow.h"

// ---------------------------------------------------------------------------------------------
// TerrainShadow (vtable 0x005582d4).  owner: bracket only.  No __FILE__ xref lies in
// 0x00508ae0..0x0050a58f (Terrain.cpp's last is 0x00507b38; Texmap.cpp's first is 0x0050a6bc).
// Terrain.cpp's vector $E block at 0x005089a0 precedes it (tier 3).
#include <math.h>

// 0x00508ae0 (ret 4).
TerrainShadow::TerrainShadow(int flags)
    : ShadowReceiver(flags)
{
    caster = 0;
    field_0x6654 = 0;
    for (int i = 0; i < 0x300; i++)
        indexTable[i] = (short)i;
    minX = 0x7fffffff;
    maxX = 0x80000000;
    minZ = 0x7fffffff;
    maxZ = 0x80000000;
    field_0x6668 = 3.4028235e38f;
    field_0x666c = -3.4028235e38f;
    field_0x6670 = 3.4028235e38f;
    field_0x6674 = -3.4028235e38f;
    cellCount = 0;
    field_0x677c = 0.0078125f;
}

// 0x00508b80 (ret 0xc).
TerrainShadow* TerrainShadow::Attach(int host, Terrain* c, ProjectedShadow* s)
{
    if (s) {
        GameObject::GameObjectVirtualSlot8(host);
        if (this) {
            caster = c;
            shadow = s;
            s->AddReceiver(this);
            return this;
        }
    }
    return 0;
}

// 0x005099c0 (slot 29): collects the distinct cell keys the shadow's grid rectangle
// (minX..maxX, minZ..maxZ in steps of 16) touches in the terrain's height field.  Returns whether
// any were found; when 0x40 are gathered it resets (tier 3 semantics).
int TerrainShadow::UnknownVirtualSlot29()
{
    cellCount = 0;
    for (int z = minZ; z <= maxZ; z += 16) {
        for (int x = minX; x <= maxX; x += 16) {
            TerrainCell* cell = caster->heightField->LookupCell(x, z);
            if (cell) {
                if (cell->owner->selector == 0)
                    cell = (TerrainCell*)cell->key;
                void* key = cell;
                int i = 0;
                while (i < cellCount && cellKeys[i] != key)
                    i++;
                if (i == cellCount) {
                    cellKeys[cellCount] = key;
                    cellCount++;
                }
                if (cellCount == 0x40) {
                    cellCount = 0;
                    field_0x6654 = 0;
                    return 0;
                }
            }
        }
    }
    return cellCount > 0;
}

// 0x005097d0 (slot 28): sizes the shadow's light matrix to the terrain rectangle (tier 3).  Same
// two-mode matrix build as ProjectedShadow::ComputeBounds: mode 1 is a perspective camera whose
// field of view comes from the texture area over the larger rectangle edge, otherwise an
// orthographic scale by lensScale.  Returns 0 when the rectangle does not fit the texture.
// Explicit-destination forms of 0x004a1410 / 0x004a1860 (cdecl, the destination is the first
// argument and is returned), as Math3D.h's MatrixMultiply(Matrix4* out, a, b) declares them.
ShadowMatrix* TerrainMatrixIdentityInto(ShadowMatrix* out);                       // 0x004a1410
ShadowMatrix* TerrainMatrixMultiplyInto(ShadowMatrix* out, ShadowMatrix a, ShadowMatrix b);  // 0x004a1860

int TerrainShadow::UnknownVirtualSlot28()
{
    float k;
    if (shadow->field_0x98)
        k = 0.5f;
    else
        k = 1.0f;
    field_0x677c = k / shadow->texture->size;
    float extentX = field_0x666c - field_0x6668;
    float extent = field_0x6674 - field_0x6670;
    if (extentX > extent)
        extent = extentX;
    if (shadow->texture->size > extent) {
        float fov;
        ShadowMatrix m;
        if (shadow->mode == 1) {
            fov = (float)atan2((float)(shadow->texture->size * shadow->texture->size) / extent * 0.5,
                               shadow->camera->field_0x198) * 114.59156f;
            shadow->camera->SetLookAt(&shadow->camera->eyePosition, 0, 0, 0, &fov);
            shadow->camera->UnknownVirtualSlot28();
            m = *TerrainMatrixIdentityInto(&m);
            m.m[0][0] = (float)shadow->texture->size;
            m.m[1][1] = (float)shadow->texture->size;
            shadow->lightMatrix = *TerrainMatrixMultiplyInto(&m, m, shadow->camera->matrix_0xec);
        } else {
            m = *TerrainMatrixIdentityInto(&m);
            m.m[0][0] = 0.5f / shadow->lensScale * fov;
            m.m[1][1] = -m.m[0][0];
            m.m[3][0] = (float)shadow->texture->size * 0.5f;
            m.m[3][1] = (float)shadow->texture->size * 0.5f;
            shadow->lightMatrix = *TerrainMatrixMultiplyInto(&m, m, shadow->camera->matrix_0xac);
        }
        return 1;
    }
    return 0;
}

// The render target at GameObject::field_0x18 and the shadow texture as slot 14 uses them (the
// same local views as D3DIMSoultreeShadow.cpp's slot 14; RTTI PCRenderTarget / PCTextureMap).
// Tier 3.
class TerrainShadowTextureView {
public:
    virtual void Slot0(); virtual void Slot1(); virtual void Slot2(); virtual void Slot3();
    virtual void Slot4(); virtual void Slot5(); virtual void Slot6(); virtual void Slot7();
    virtual void Slot8(); virtual void Slot9(); virtual void Slot10(); virtual void Slot11();
    virtual void Slot12(); virtual void Slot13(); virtual void Slot14(); virtual void Slot15();
    virtual void Slot16(); virtual void Slot17(); virtual void Slot18();
    virtual void Bind();                                          // slot 19
};

class TerrainShadowRenderTarget {
public:
    virtual void Slot0(); virtual void Slot1(); virtual void Slot2(); virtual void Slot3();
    virtual void Slot4(); virtual void Slot5();
    virtual long GetTextureStageState(int stage, int type, int* value);  // slot 6
    virtual long SetTextureStageState(int stage, int type, int value);   // slot 7
    virtual void SetRenderState(int state, int value, int force);        // slot 8
    virtual long GetRenderState(int state, int* value);                  // slot 9
    virtual void Slot10(); virtual void Slot11(); virtual void Slot12(); virtual void Slot13();
    virtual void Slot14();
    // slot 15: indexed draw (type, FVF, vertices, vertex count, indices, index count, flags).
    virtual int DrawIndexed(int type, int fvf, void* vertices, int vertexCount, short* indices,
                            int indexCount, int flags);
    virtual void Slot16(); virtual void Slot17();
    virtual void Slot18(int value);
    char pad_0x04[0x1c8 - 0x04];
    unsigned char field_0x1c8;                   // bit 2 tested by slot 14
};

// 0x0050a1a0 (slot 14): draws the field_0x6654 shadow vertices at +0x34 with the shadow texture
// and modulating texture stages, as D3DIMSoultreeShadow's slot 14 does without its world matrix.
int TerrainShadow::GameObjectVirtualSlot14()
{
    if (field_0x6654) {
        ((TerrainShadowTextureView*)shadow->texture)->Bind();
        int address;
        ((TerrainShadowRenderTarget*)field_0x18)->GetTextureStageState(0, 0xc, &address);
        if (address != 3)
            ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 0xc, 3);
        if (shadow->surfaceFormat == 0x613) {
            ((TerrainShadowRenderTarget*)field_0x18)->SetRenderState(0x21, 1, 0);
            ((TerrainShadowRenderTarget*)field_0x18)->SetRenderState(0x1b, 1, 0);
            if (((TerrainShadowRenderTarget*)field_0x18)->field_0x1c8 & 4) {
                ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 1, 4);
                ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 2, 2);
                ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 3, 0);
                ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 4, 4);
                ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 5, 2);
                ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 6, 0);
            }
        } else {
            ((TerrainShadowRenderTarget*)field_0x18)->SetRenderState(0x1b, 1, 0);
            ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 1, 4);
            ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 2, 2);
            ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 3, 0);
            ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 4, 4);
            ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 5, 2);
            ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 6, 0);
        }
        int blend;
        ((TerrainShadowRenderTarget*)field_0x18)->GetRenderState(4, &blend);
        ((TerrainShadowRenderTarget*)field_0x18)->SetRenderState(4, 1, 0);
        ((TerrainShadowRenderTarget*)field_0x18)->SetRenderState(0xe, 0, 0);
        ((TerrainShadowRenderTarget*)field_0x18)->Slot18(0);
        ((TerrainShadowRenderTarget*)field_0x18)->DrawIndexed(4, 0x1e2, field_0x34, field_0x6654, indexTable,
                                                              field_0x6654, 0);
        ((TerrainShadowRenderTarget*)field_0x18)->SetRenderState(0xe, 1, 0);
        ((TerrainShadowRenderTarget*)field_0x18)->SetRenderState(0x1b, 0, 0);
        if (address != 3)
            ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 0xc, address);
        ((TerrainShadowRenderTarget*)field_0x18)->SetRenderState(4, blend, 0);
        if (shadow->surfaceFormat == 0x613)
            ((TerrainShadowRenderTarget*)field_0x18)->SetRenderState(0x21, 0, 0);
    }
    return 1;
}
