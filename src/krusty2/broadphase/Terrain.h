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

#include "core/GameObject.h"
#include "math/FastMath.h"
#include "core/MemTag.h"
#include "core/DebugAlloc.h"

// Second direct base of Terrain at mdisp 0x2c (RTTI .?AVGroundFogableObject@@, tier 1); it has no
// vfptr.  Its contents are PROVISIONAL: the 0x004aae20 ctor only stores 0 in its first dword.
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

struct TerrainVec3;

// PROVISIONAL: object destroyed through its scalar deleting dtor (vtable slot 0) after a
// call to 0x0047edb0(1).
class TerrainShutdownObject {
public:
    virtual ~TerrainShutdownObject();
    // vtable slot 1 (called `mov ecx,[obj]; call [ecx+4]` at 0x005074d1, callee pops 6 args):
    // casts the grid-space segment (a, b) against the terrain; on a hit writes the grid-space hit
    // point to *out.  The last three ints are forwarded from Terrain::CastSegment (tier 3 role).
    virtual int CastSegment(TerrainVec3* a, TerrainVec3* b, TerrainVec3* out, int p3, int p4, int p5);
    void Shutdown(int flag);                                    // 0x0047edb0
    // 0x00483910 (thiscall, ret 0x14; tier 2: field_0x44 is the same object the dtor shuts
    // down).  Fetches the four corners of grid cell (ix, iz): heights[4], optional normals[4]
    // and per-corner surface bytes[4] (three output arrays, see Terrain::QueryGround).
    void GetCellCorners(int ix, int iz, float* heights, TerrainVec3* normals,
                        unsigned char* surface);

    // Members read by Terrain::CastSegment (tier 2 offsets, tier 3 names).
    void* field_0x04;                                           // cell table read by 0x00483910
    char pad_0x08[0x18 - 0x08];
    float field_0x18;                                           // lower height bound of the grid (compared with segment y)
    float field_0x1c;                                           // upper height bound
    char pad_0x20[0x28 - 0x20];
    unsigned char field_0x28;
    unsigned char field_0x29;                                   // log2 shift: grid edge = 16 << field_0x29 cells
};

// 0x0056df04 / 0x004a2d00 / 0x004a2d90 bracket the Terrain dtor body: MemTagStack in
// core/MemTag.h.

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
// objects would add $E initializers that Terrain.cpp does not have).  The out-of-line
// copies 0x00404e60 (ctor), 0x00421cb0 (+), 0x0043c890 (/ scalar), 0x005015b0 (* scalar)
// exist in retail as COMDATs; VC6 inlines them at most call sites of QueryGround.
struct TerrainVec3 {
    float x, y, z;
    TerrainVec3() {}
    TerrainVec3(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }
    TerrainVec3& operator*=(float s);                            // 0x0040ae00 (out of line at its one call site)
};
inline TerrainVec3 operator-(const TerrainVec3& a, const TerrainVec3& b)
{
    return TerrainVec3(a.x - b.x, a.y - b.y, a.z - b.z);
}
inline TerrainVec3 operator+(const TerrainVec3& a, const TerrainVec3& b)
{
    return TerrainVec3(a.x + b.x, a.y + b.y, a.z + b.z);
}
inline TerrainVec3 operator-(const TerrainVec3& v) { return TerrainVec3(-v.x, -v.y, -v.z); }
inline TerrainVec3 operator*(const TerrainVec3& v, float s)
{
    return TerrainVec3(s * v.x, s * v.y, s * v.z);
}
// 0x0043c890 multiplies by the reciprocal (fld 1.0; fdiv s; then three fmul).
inline TerrainVec3 operator/(const TerrainVec3& v, float s) { return v * (1.0f / s); }

float TerrainDot(const TerrainVec3& a, const TerrainVec3& b);   // 0x0040ae30 (cdecl, out of line)
// 0x004a11e0 (cdecl): unit normal of the triangle (a, b, c) written to *out; the fifth
// argument, when non-null, receives the plane offset (tier 3: the tail is `-dot(n, a)`).
void TerrainTriangleNormal(const TerrainVec3* a, const TerrainVec3* b, const TerrainVec3* c,
                           TerrainVec3* out, float* planeD);
extern TerrainVec3 g_terrainRefDir;                              // 0x0068a058 (dotted with the face normal to orient it)

// 0x00507510 / 0x00507590: file-static cdecl helpers (tier 2: no ecx, `ret`; the by-value TerrainVec3 return is the hidden out pointer in arg 0).
TerrainVec3 TerrainClipRayToPlaneY(TerrainVec3* origin, const TerrainVec3* dir, float y);
TerrainVec3 TerrainClipRayToPlaneZ(TerrainVec3* origin, const TerrainVec3* dir, float z);

class Terrain : public GameObject, public GroundFogableObject {
public:
    // 0x00507c10 (thiscall, ret 0x10; 58 callers).  Snaps pos->y to the terrain surface under
    // (pos->x, pos->z), optionally writes the surface normal and a per-cell surface byte.
    // The third argument is 0 at nearly every caller and 1 at one (0x004b1567): tier 3 name
    // "flatShaded" = use the triangle's face normal instead of blending corner normals.
    void QueryGround(TerrainVec3* pos, TerrainVec3* normal, int flatShaded, unsigned char* surface);
    // 0x00506e90 (thiscall, ret 0x18).  Casts the world segment from->to against the terrain grid:
    // rescales to grid space (1 / field_0x40), clips it to the grid box, hands it to
    // field_0x44->CastSegment, and on a hit returns the world-space hit point in *out.
    int CastSegment(const TerrainVec3* from, const TerrainVec3* to, TerrainVec3* out, int a, int b, int c);
    // GameObject slot overrides (vtable 0x0055825c, tier 1 addresses).
    virtual int GameObjectVirtualSlot19(int a);                 // 0x00507920 returns 0
    virtual int GameObjectVirtualSlot22(int a, int b);          // 0x004dc4c0 returns 0 (shared stub)
    // Slot 23 (0x00508850, ret 8): debug key handler; tier 3 name kept in the comment only
    // so the override has the base's name.  First argument is the input event.
    virtual int GameObjectVirtualSlot23(int event, int unused);
    void SetField0xbec(int value);                              // 0x00507930 (ret 4)
    void SelectQuality(int index);                              // 0x00507960 (ret 4)
    explicit Terrain(int a);                                    // 0x00505830 (ret 4), forwards a to GameObject(int)
    virtual ~Terrain();                                         // 0x005059b0 -> core 0x005079f0

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
