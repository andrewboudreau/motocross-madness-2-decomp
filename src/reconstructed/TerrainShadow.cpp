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
#include "PCTextureMap.h"
#include "PCRenderTarget.h"
#include "Griddraw.h"

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

// 0x0050a1a0 (slot 14): draws the collected terrain vertices (+0x34, vertexCount of them) with the
// shadow texture and modulating texture stages; D3DIMSoultreeShadow slot 14 without the world
// matrix.  The address mode and render state 4 are restored afterwards; format 0x613 textures
// also switch render state 0x21 on around the draw.
int TerrainShadow::Render3D()
{
    if (vertexCount) {
        shadow->texture->Bind();
        int address;
        ((PCRenderTarget*)renderContext)->GetTextureStageState(0, 0xc, &address);
        if (address != 3)
            ((PCRenderTarget*)renderContext)->SetTextureStageState(0, 0xc, 3);
        if (shadow->surfaceFormat == 0x613) {
            ((PCRenderTarget*)renderContext)->SetRenderState(0x21, 1, 0);
            ((PCRenderTarget*)renderContext)->SetRenderState(0x1b, 1, 0);
            if (((PCRenderTarget*)renderContext)->triTextureBlendCaps & 4) {
                ((PCRenderTarget*)renderContext)->SetTextureStageState(0, 1, 4);
                ((PCRenderTarget*)renderContext)->SetTextureStageState(0, 2, 2);
                ((PCRenderTarget*)renderContext)->SetTextureStageState(0, 3, 0);
                ((PCRenderTarget*)renderContext)->SetTextureStageState(0, 4, 4);
                ((PCRenderTarget*)renderContext)->SetTextureStageState(0, 5, 2);
                ((PCRenderTarget*)renderContext)->SetTextureStageState(0, 6, 0);
            }
        } else {
            ((PCRenderTarget*)renderContext)->SetRenderState(0x1b, 1, 0);
            ((PCRenderTarget*)renderContext)->SetTextureStageState(0, 1, 4);
            ((PCRenderTarget*)renderContext)->SetTextureStageState(0, 2, 2);
            ((PCRenderTarget*)renderContext)->SetTextureStageState(0, 3, 0);
            ((PCRenderTarget*)renderContext)->SetTextureStageState(0, 4, 4);
            ((PCRenderTarget*)renderContext)->SetTextureStageState(0, 5, 2);
            ((PCRenderTarget*)renderContext)->SetTextureStageState(0, 6, 0);
        }
        int blend;
        ((PCRenderTarget*)renderContext)->GetRenderState(4, &blend);
        ((PCRenderTarget*)renderContext)->SetRenderState(4, 1, 0);
        ((PCRenderTarget*)renderContext)->SetRenderState(0xe, 0, 0);
        ((PCRenderTarget*)renderContext)->SetAlphaTest(0);
        ((PCRenderTarget*)renderContext)->DrawIndexedPrimitive(4, 0x1e2, (int)vertices, vertexCount, (int)indexTable,
                                                           vertexCount, 0);
        ((PCRenderTarget*)renderContext)->SetRenderState(0xe, 1, 0);
        ((PCRenderTarget*)renderContext)->SetRenderState(0x1b, 0, 0);
        if (address != 3)
            ((PCRenderTarget*)renderContext)->SetTextureStageState(0, 0xc, address);
        ((PCRenderTarget*)renderContext)->SetRenderState(4, blend, 0);
        if (shadow->surfaceFormat == 0x613)
            ((PCRenderTarget*)renderContext)->SetRenderState(0x21, 0, 0);
    }
    return 1;
}
