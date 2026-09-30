// Terrain.h -- Terrain (D:\aardvark\VC\krusty2\Terrain.cpp).
//
// Evidence (tier 1 unless noted):
//  * RTTI .?AVTerrain@@ COL 0x0055f338, vtable 0x0055825c (27 slots, object_offset 0), direct bases
//    GameObject (mdisp 0) and GroundFogableObject (mdisp 44 = 0x2c); no vbptr (pdisp -1).
//  * Overridden slots (vtable_overrides.json): 0 (deleting dtor 0x005059b0 -> core 0x005079f0),
//    12 0x00507610, 14 0x00506220, 19 0x00507920, 22 0x004dc4c0 (shared return stub), 23 0x00508850.
//  * Ctor 0x00505830 (ret 4, one argument forwarded to the GameObject ctor 0x00468ca0), calls
//    GroundFogableObject ctor 0x004aae20 on this+0x2c (writes a zero dword), then vptr 0x55825c.
//  * __FILE__ 0x00574720 "D:\aardvark\VC\krusty2\Terrain.cpp"; debug deletes at lines 0x4b3 / 0x4d0.
//  * Object size >= 0xcc4 (last ctor store at +0xcc0).
// Member names are field_0xNN (tier 3 semantics documented in the .cpp).
#ifndef BROADPHASE_TERRAIN_H
#define BROADPHASE_TERRAIN_H

#include "../soultree_base/GameObject.h"

void operator delete(void* p, const char* file, int line);   // 0x004a2e60

// PROVISIONAL stand-in: the 0x004aae20 ctor only stores 0 in its first dword.
class GroundFogableObject {
public:
    GroundFogableObject();                      // 0x004aae20: stores 0 in field_0x00
    int field_0x00;
};

// PROVISIONAL: DirectX-style interface at Terrain+0x34; the dtor calls slot 2 with `this`
// pushed on the stack (`mov ecx,[eax]; push eax; call [ecx+8]`), i.e. an IUnknown::Release.
class TerrainComObject {
public:
    virtual long __stdcall QueryInterface(void* iid, void** out);
    virtual unsigned long __stdcall AddRef();
    virtual unsigned long __stdcall Release();
};

// PROVISIONAL: owned object with a real (non-virtual) dtor at 0x00401020, freed with plain delete.
class TerrainOwned {
public:
    ~TerrainOwned();                                            // 0x00401020
};

// PROVISIONAL: object destroyed through its scalar deleting dtor (vtable slot 0) after a
// call to 0x0047edb0(1).
class TerrainShutdownObject {
public:
    virtual ~TerrainShutdownObject();
    void Shutdown(int flag);                                    // 0x0047edb0
};

// PROVISIONAL: 0x0056df04 holds a pointer to an object whose 0x004a2d00(const char*) / 0x004a2d90(x)
// bracket the Terrain dtor body (named scope enter/leave; tier 3 semantics).
class TerrainScopeMgr {
public:
    void* Enter(const char* name);                              // 0x004a2d00
    void Leave(void* token);                                    // 0x004a2d90
};
extern TerrainScopeMgr* g_pTerrainScopeMgr;                     // 0x0056df04

// 16-dword block copied into +0xc44 from 0x004a1410's return value (a 4x4 matrix, tier 2:
// 0x40 bytes).  Provisional.
struct TerrainMatrix {
    float m[16];
};
TerrainMatrix* GetIdentityMatrix(TerrainMatrix* out);           // 0x004a1410 (fills *out, returns it; tier 3 name)

// PROVISIONAL: 16-byte entries of the table at *0x0068a32c indexed by Terrain::field_0xbf0
// (`shl eax,4`, fields at +0, +4, +8, +0xc read by 0x00507960; tier 2 layout, tier 3 use).
struct TerrainQualityEntry {
    int field_0x00;
    int field_0x04;
    int field_0x08;
    int field_0x0c;
};
extern TerrainQualityEntry* g_pTerrainQualityTable;             // 0x0068a32c
extern int g_terrainQualityValue;                               // 0x0057471c (Terrain.cpp .data)

// PROVISIONAL: 0x0043caa0 tests an input event; cdecl (kind, 0, event, 0x80) at the call in 0x00508850.
int TestInputEvent(int kind, int zero, int event, int mask);    // 0x0043caa0 (tier 3 name)

// PROVISIONAL: GameObject::field_0x18 points at a host object whose +8 member points at a 0x220
// byte block that 0x00508850 copies to 0x0068a090.
struct TerrainSharedState { int words[0x88]; };
struct TerrainHost {
    int field_0x00;
    int field_0x04;
    TerrainSharedState* field_0x08;
};
extern TerrainSharedState g_terrainSharedState;                 // 0x0068a090
extern int g_terrainToggle314;                                  // 0x0068a314
extern int g_terrainToggle718;                                  // 0x00574718

// PROVISIONAL stand-in for the shared Vec3 (Math3D.h is not included: its static const
// objects would add $E initializers that Terrain.cpp does not have).
struct TerrainVec3 {
    float x, y, z;
};
// 0x00507510 / 0x00507590: file-static cdecl helpers (tier 2: no ecx, `ret`; the by-value TerrainVec3 return is the hidden out pointer in arg 0).
TerrainVec3 TerrainClipRayToPlaneY(TerrainVec3* origin, const TerrainVec3* dir, float y);
TerrainVec3 TerrainClipRayToPlaneZ(TerrainVec3* origin, const TerrainVec3* dir, float z);

class Terrain : public GameObject {
public:
    int HandleInput(int event, int unused);                     // 0x00508850 (slot 23 override, ret 8)
    int UnknownVirtualSlot19(int a);                            // 0x00507920 returns 0 (shared stub)
    int UnknownVirtualSlot22(int a, int b);                     // 0x004dc4c0 returns 0 (shared stub)
    void SetField0xbec(int value);                              // 0x00507930 (ret 4)
    void SelectQuality(int index);                              // 0x00507960 (ret 4)
    Terrain(int a);                                             // 0x00505830 (ret 4)
    virtual ~Terrain();                                         // 0x005059b0 -> core 0x005079f0

    GroundFogableObject fogBase;                                // +0x2c (non-polymorphic base at mdisp 0x2c)
    BaseObject* field_0x30;                                     // slot 2 called (0x005079f0)
    TerrainComObject* field_0x34;                               // Release()d, then zeroed
    int field_0x38;
    int field_0x3c;
    float field_0x40;                                           // ctor 1.0f
    TerrainShutdownObject* field_0x44;
    char field_0x48[0x24];
    int field_0x6c;
    int field_0x70;
    int field_0x74;
    int field_0x78;
    int field_0x7c;
    int field_0x80;
    int field_0x84;
    int field_0x88;
    int field_0x8c;
    int field_0x90;
    int field_0x94;
    int field_0x98;
    int field_0x9c;
    int field_0xa0;
    int field_0xa4;
    int field_0xa8;
    int field_0xac;
    int field_0xb0;
    int field_0xb4;
    char field_0xb8[0x53c - 0xb8];
    int field_0x53c;
    int field_0x540;                                            // count of field_0x544[]
    BaseObject* field_0x544[(0xbec - 0x544) / 4];
    int field_0xbec;                                            // ctor 1000
    int field_0xbf0;
    int field_0xbf4;
    int field_0xbf8;
    int field_0xbfc;
    int field_0xc00;
    int field_0xc04;
    int field_0xc08;
    int field_0xc0c;
    int field_0xc10;                                            // count of field_0xc14[]
    void** field_0xc14;                                         // array of plain-new pointers
    char field_0xc18[0x18];
    float field_0xc30;                                          // ctor 1.0f
    int field_0xc34;
    int field_0xc38;
    BaseObject* field_0xc3c;                                    // slot 2 called
    int field_0xc40;
    TerrainMatrix field_0xc44;                                  // copy of the matrix from 0x004a1410
    TerrainOwned* field_0xc84;
    TerrainOwned* field_0xc88;
    int field_0xc8c;
    char field_0xc90[0xca4 - 0xc90];
    int field_0xca4;
    int field_0xca8;
    int field_0xcac;
    int field_0xcb0;
    int field_0xcb4;
    BaseObject** field_0xcb8;                                   // array; each element slot 2 called
    int field_0xcbc;                                            // count of field_0xcb8[]
    int field_0xcc0;
};

#endif
