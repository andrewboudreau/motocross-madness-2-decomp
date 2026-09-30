// Terrain.cpp -- reconstruction of D:\aardvark\VC\krusty2\Terrain.cpp.
#include "Terrain.h"

TerrainScopeMgr* g_pTerrainScopeMgr;

Terrain::Terrain(int a)
{
    field_0x40 = 1.0f;
    field_0x3c = 0;
    field_0x84 = 0;
    field_0x88 = 0;
    field_0x90 = 0;
    field_0x94 = 0;
    field_0x98 = 0;
    field_0x9c = 0;
    field_0xa0 = 0;
    field_0xa4 = 0;
    field_0xa8 = 0;
    field_0xac = 0;
    field_0xb0 = 0;
    field_0x34 = 0;
    field_0xb4 = 0;
    field_0x540 = 0;
    field_0x53c = 0;
    field_0x44 = 0;
    field_0xbec = 1000;
    field_0xbf4 = 0;
    field_0x30 = 0;
    field_0xbf8 = 0;
    field_0x8c = 0;
    field_0xbfc = 0;
    field_0xc00 = 0;
    field_0xc04 = 0;
    field_0xc08 = 0;
    field_0xc0c = 0;
    field_0x6c = 0;
    field_0x70 = 0;
    field_0x78 = 0;
    field_0x74 = 0;
    field_0x7c = 0;
    field_0x80 = 0;
    field_0xc10 = 0;
    field_0xc14 = 0;
    field_0xc30 = 1.0f;
    field_0xc34 = 0;
    field_0xc38 = 0;
    field_0xc3c = 0;
    TerrainMatrix tmp;
    field_0xc44 = *GetIdentityMatrix(&tmp);
    field_0xcb8 = 0;
    field_0xcbc = 0;
    field_0xcc0 = 0;
    field_0xcac = 0;
    field_0xcb0 = 0;
    field_0xcb4 = 0;
}

Terrain::~Terrain()
{
    void* scope = g_pTerrainScopeMgr->Enter("Terrain");
    if (field_0xcb8) {
        for (int i = 0; i < field_0xcbc; i++)
            field_0xcb8[i]->BaseObjectVirtualSlot2();
        operator delete(field_0xcb8, __FILE__, 0x4b3);
    }
    if (field_0xc3c)
        field_0xc3c->BaseObjectVirtualSlot2();
    if (field_0x30)
        field_0x30->BaseObjectVirtualSlot2();
    if (field_0x540) {
        for (int i = 0; i < field_0x540; i++) {
            if (field_0x544[i])
                field_0x544[i]->BaseObjectVirtualSlot2();
        }
    }
    if (field_0x44) {
        field_0x44->Shutdown(1);
        if (field_0x44)
            delete field_0x44;
    }
    if (field_0x34) {
        field_0x34->Release();
        field_0x34 = 0;
    }
    if (field_0xc14) {
        for (int i = 0; i < field_0xc10; i++) {
            if (field_0xc14[i])
                delete field_0xc14[i];
        }
        operator delete(field_0xc14, __FILE__, 0x4d0);
    }
    if (field_0xc84)
        delete field_0xc84;
    if (field_0xc88)
        delete field_0xc88;
    g_pTerrainScopeMgr->Leave(scope);
}

TerrainQualityEntry* g_pTerrainQualityTable;
int g_terrainQualityValue;

// Slot 19 (0x00507920) and slot 22 (0x004dc4c0) are the shared "return 0" stubs.
int Terrain::UnknownVirtualSlot19(int)
{
    return 0;
}

int Terrain::UnknownVirtualSlot22(int, int)
{
    return 0;
}

// 0x00507930: clamps at zero, marks field_0xbf4 (a dirty flag, tier 3) when the value changes.
void Terrain::SetField0xbec(int value)
{
    if (value < 0)
        value = 0;
    if (field_0xbec != value) {
        field_0xbec = value;
        field_0xbf4 = 1;
    }
}

void Terrain::SelectQuality(int index)
{
    field_0xbf0 = index;
    g_terrainQualityValue = g_pTerrainQualityTable[index].field_0x0c;
    SetField0xbec(g_pTerrainQualityTable[field_0xbf0].field_0x00);
    field_0xca4 = field_0xcb0 ? g_pTerrainQualityTable[9].field_0x04
                              : g_pTerrainQualityTable[field_0xbf0].field_0x04;
    field_0xca8 = g_pTerrainQualityTable[field_0xbf0].field_0x08;
}

TerrainSharedState g_terrainSharedState;
int g_terrainToggle314;
int g_terrainToggle718;

// Slot 23 (0x00508850): debug key handler.  The three TestInputEvent kinds 0x43, 2 and 3
// each flip one toggle and return 1 (tier 3 semantics).
int Terrain::HandleInput(int event, int)
{
    if (TestInputEvent(0x43, 0, event, 0x80)) {
        g_terrainToggle314 = 1 - g_terrainToggle314;
        if (g_terrainToggle314)
            g_terrainSharedState = *((TerrainHost*)field_0x18)->field_0x08;
        return 1;
    }
    if (TestInputEvent(2, 0, event, 0x80)) {
        if (g_terrainQualityValue && field_0xcac) {
            field_0xcb0 = 1 - field_0xcb0;
            if (field_0xcb0) {
                field_0xca4 = g_pTerrainQualityTable[9].field_0x04;
                return 1;
            }
            field_0xca4 = g_pTerrainQualityTable[field_0xbf0].field_0x04;
        }
        g_terrainQualityValue = 1 - g_terrainQualityValue;
        return 1;
    }
    if (TestInputEvent(3, 0, event, 0x80)) {
        g_terrainToggle718 = 1 - g_terrainToggle718;
        return 1;
    }
    return 0;
}

// 0x00507510 (Y) and 0x00507590 (Z): advance `origin` along `dir` to the plane coordinate
// `limit` when the ray is heading toward it from the outside, then return the result by value
// (tier 3 names: t = (limit - origin.c) / dir.c and the other two components step by t*dir).
TerrainVec3 TerrainClipRayToPlaneY(TerrainVec3* origin, const TerrainVec3* dir, float limit)
{
    if ((dir->y > 0.0f && origin->y < limit) || (dir->y < 0.0f && origin->y > limit)) {
        float t = (limit - origin->y) / dir->y;
        origin->x += t * dir->x;
        origin->y = limit;
        origin->z += t * dir->z;
    }
    return *origin;
}

TerrainVec3 TerrainClipRayToPlaneZ(TerrainVec3* origin, const TerrainVec3* dir, float limit)
{
    if ((dir->z > 0.0f && origin->z < limit) || (dir->z < 0.0f && origin->z > limit)) {
        float t = (limit - origin->z) / dir->z;
        origin->x += t * dir->x;
        origin->y += t * dir->y;
        origin->z = limit;
    }
    return *origin;
}
