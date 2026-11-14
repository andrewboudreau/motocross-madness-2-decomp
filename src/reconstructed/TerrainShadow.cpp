// TerrainShadow.cpp -- TerrainShadow (vtable 0x005582d4): the strict exact constructor, Attach,
// slot 29 and the slot 14 render (Render3D).  The class is declared in TerrainShadow.h.
// Owner TU unknown (bracket only, tier 3): no __FILE__ xref lies in 0x00508ae0..0x0050a58f
// (Terrain.cpp's last is 0x00507b38, Texmap.cpp's first 0x0050a6bc).  The vector $E block at
// 0x005089a0 just before it closes Terrain.cpp (docs/INITIALIZERS.md), so this code starts a unit
// of its own or continues Terrain.cpp; the file therefore has the class's name, not a retail path.
// This TU also emits the empty ~ShadowReceiver COMDAT retail keeps at 0x00508b70 and the
// TerrainShadow vtable.  Not here: slots 27 (0x00508bc0), 28 (0x005097d0) and 30 (0x00509aa0),
// near misses in samples/physics/shadow/TerrainShadow.cpp, which the game build admits.
#include "TerrainShadow.h"

// 0x00508ae0 (ret 4).
TerrainShadow::TerrainShadow(int flags)
    : ShadowReceiver(flags)
{
    caster = 0;
    vertexCount = 0;
    for (int i = 0; i < 0x300; i++)
        indexTable[i] = (short)i;
    minX = 0x7fffffff;
    maxX = 0x80000000;
    minZ = 0x7fffffff;
    maxZ = 0x80000000;
    screenMinX = 3.4028235e38f;
    screenMaxX = -3.4028235e38f;
    screenMinY = 3.4028235e38f;
    screenMaxY = -3.4028235e38f;
    cellCount = 0;
    texelSize = 0.0078125f;
}

// 0x00508b80 (ret 0xc).
TerrainShadow* TerrainShadow::Attach(int host, Terrain* c, ProjectedShadow* s)
{
    if (s) {
        GameObject::SetRenderContext((void*)host);
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
int TerrainShadow::NeedsShadowRender()
{
    cellCount = 0;
    for (int z = minZ; z <= maxZ; z += 16) {
        for (int x = minX; x <= maxX; x += 16) {
            TerrainCell* cell = caster->heightField->LookupCell(x, z);
            if (cell) {
                if (cell->owner->vertexCount == 0)
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
                    vertexCount = 0;
                    return 0;
                }
            }
        }
    }
    return cellCount > 0;
}

// The render target at GameObject::renderContext as slot 14 uses it (RTTI PCRenderTarget; the
// same local view D3DIMSoultreeShadow.cpp keeps).  Tier 3.
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
    unsigned char triTextureBlendCaps;           // bit 2 tested by slot 14
};

// The shadow texture's slot 19 (0x4c) binds it to texture stage 0 (PCTextureMap slot 19).  Tier 3.
class TerrainShadowTextureView {
public:
    virtual void Slot0(); virtual void Slot1(); virtual void Slot2(); virtual void Slot3();
    virtual void Slot4(); virtual void Slot5(); virtual void Slot6(); virtual void Slot7();
    virtual void Slot8(); virtual void Slot9(); virtual void Slot10(); virtual void Slot11();
    virtual void Slot12(); virtual void Slot13(); virtual void Slot14(); virtual void Slot15();
    virtual void Slot16(); virtual void Slot17(); virtual void Slot18();
    virtual void Bind();                                          // slot 19
};

// 0x0050a1a0 (slot 14): draws the collected terrain vertices (+0x34, vertexCount of them) with the
// shadow texture and modulating texture stages; D3DIMSoultreeShadow slot 14 without the world
// matrix.  The address mode and render state 4 are restored afterwards; format 0x613 textures
// also switch render state 0x21 on around the draw.
int TerrainShadow::Render3D()
{
    if (vertexCount) {
        ((TerrainShadowTextureView*)shadow->texture)->Bind();
        int address;
        ((TerrainShadowRenderTarget*)renderContext)->GetTextureStageState(0, 0xc, &address);
        if (address != 3)
            ((TerrainShadowRenderTarget*)renderContext)->SetTextureStageState(0, 0xc, 3);
        if (shadow->surfaceFormat == 0x613) {
            ((TerrainShadowRenderTarget*)renderContext)->SetRenderState(0x21, 1, 0);
            ((TerrainShadowRenderTarget*)renderContext)->SetRenderState(0x1b, 1, 0);
            if (((TerrainShadowRenderTarget*)renderContext)->triTextureBlendCaps & 4) {
                ((TerrainShadowRenderTarget*)renderContext)->SetTextureStageState(0, 1, 4);
                ((TerrainShadowRenderTarget*)renderContext)->SetTextureStageState(0, 2, 2);
                ((TerrainShadowRenderTarget*)renderContext)->SetTextureStageState(0, 3, 0);
                ((TerrainShadowRenderTarget*)renderContext)->SetTextureStageState(0, 4, 4);
                ((TerrainShadowRenderTarget*)renderContext)->SetTextureStageState(0, 5, 2);
                ((TerrainShadowRenderTarget*)renderContext)->SetTextureStageState(0, 6, 0);
            }
        } else {
            ((TerrainShadowRenderTarget*)renderContext)->SetRenderState(0x1b, 1, 0);
            ((TerrainShadowRenderTarget*)renderContext)->SetTextureStageState(0, 1, 4);
            ((TerrainShadowRenderTarget*)renderContext)->SetTextureStageState(0, 2, 2);
            ((TerrainShadowRenderTarget*)renderContext)->SetTextureStageState(0, 3, 0);
            ((TerrainShadowRenderTarget*)renderContext)->SetTextureStageState(0, 4, 4);
            ((TerrainShadowRenderTarget*)renderContext)->SetTextureStageState(0, 5, 2);
            ((TerrainShadowRenderTarget*)renderContext)->SetTextureStageState(0, 6, 0);
        }
        int blend;
        ((TerrainShadowRenderTarget*)renderContext)->GetRenderState(4, &blend);
        ((TerrainShadowRenderTarget*)renderContext)->SetRenderState(4, 1, 0);
        ((TerrainShadowRenderTarget*)renderContext)->SetRenderState(0xe, 0, 0);
        ((TerrainShadowRenderTarget*)renderContext)->Slot18(0);
        ((TerrainShadowRenderTarget*)renderContext)->DrawIndexed(4, 0x1e2, vertices, vertexCount, indexTable,
                                                              vertexCount, 0);
        ((TerrainShadowRenderTarget*)renderContext)->SetRenderState(0xe, 1, 0);
        ((TerrainShadowRenderTarget*)renderContext)->SetRenderState(0x1b, 0, 0);
        if (address != 3)
            ((TerrainShadowRenderTarget*)renderContext)->SetTextureStageState(0, 0xc, address);
        ((TerrainShadowRenderTarget*)renderContext)->SetRenderState(4, blend, 0);
        if (shadow->surfaceFormat == 0x613)
            ((TerrainShadowRenderTarget*)renderContext)->SetRenderState(0x21, 0, 0);
    }
    return 1;
}
