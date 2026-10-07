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

#include "../../reconstructed/PeakHold.h"

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
// Tier 1: its destructor frees +0x0c with __FILE__ AgeManager.cpp line 0x11, so this is an AgeManager;
// tier 3 member names.
class TerrainOwned {
public:
    ~TerrainOwned();                                            // 0x00401020
    int Purge(int target);                                      // 0x004011b0 (ret 4; 0x00505600 calls it with 0 between --age and ++age)
    void MarkUsed(void* entry);                                 // 0x00401250 (ret 4; AgeManager::MarkUsed; TerrainShadow slot 30 passes a cell mesh's +0x144)
    int age;                                                    // +0x00 decremented around Purge by Terrain::AcquireOwnedObject
};

// Boundary views of PCTextureMap and ManagedTexture, created by 0x00505600.  Base ctor 0x004c5f00 (ret 8,
// forwards to 0x0050a4e0, vptr 0x00555fec, size 0x80), derived ctor 0x00510500 (ret 4, vptr
// 0x00558430, size 0xbc).  Virtual slot 4 (+0x10) takes seventeen stack arguments (tier 3 role:
// create the surface).
class TextureMapManager;

// ABI view of PCTextureMap; canonical identity is established independently.
class TerrainSurfaceBase : public BaseObject {
public:
    TerrainSurfaceBase(TextureMapManager* manager, int flags);                           // 0x004c5f00
    virtual int CreateSurface(int a1, int a2, int a3, int a4, int a5, int a6, int a7, BaseObject* a8,
                              int a9, TerrainComObject* a10, int a11, int a12, int a13, int a14,
                              void* a15, int a16, int a17);     // slot 4, +0x10
    char pad_0x08[0x80 - 0x08];
};
class TerrainSurface : public TerrainSurfaceBase {
public:
    explicit TerrainSurface(TextureMapManager* manager);                             // 0x00510500
    char pad_0x80[0xbc - 0x80];
};
// Boundary view of ManagedTextureGroup: 0x0050c6c0 registers a ManagedTexture.
class TerrainSurfaceRegistry {
public:
    void Register(TerrainSurface* surface);                 // 0x0050c6c0
};
struct TerrainSurfaceDesc {
    TextureMapManager* manager;                                // +0x00 passed to PCTextureMap/ManagedTexture constructors
    char pad_0x04[0x14];
};

struct TerrainVec3;

// PROVISIONAL: object destroyed through its scalar deleting dtor (vtable slot 0) after a
// call to 0x0047edb0(1).
// PROVISIONAL cell of the height field's quadtree (0x00484d70 returns one): +0x24/+0x26 hold the
// 16-bit cell coordinates compared with the lookup arguments, +0x08 is a pointer returned as the
// key, +0x38 the cell's mesh (TerrainShadow slot 30 reads its lit vertices, index list and
// 16 section records; +0x121 bit 3 selects the sectioned layout).  Tier 2 offsets, tier 3 names.
struct TerrainCellSection {
    short vertexBase;                                          // +0x00 vertex base of the following section
    short indexEnd;                                            // +0x02 cumulative index count
    char pad_0x04[0x0c - 0x04];
};
struct TerrainCellOwner {
    char pad_0x00[0x121];
    unsigned char flags_0x121;                                 // +0x121 bit 3: 16 sections instead of one
    char pad_0x122[0x134 - 0x122];
    void* vertices;                                            // +0x134 32-byte lit vertices (x, y, z first)
    short* indices;                                            // +0x138
    short vertexCount;                                         // +0x13c (0 selects the cell key in TerrainShadow slot 29)
    short indexCount;                                          // +0x13e
    char pad_0x140[0x144 - 0x140];
    char ageEntry[0x17c - 0x144];                              // +0x144 passed to TerrainOwned::MarkUsed
    TerrainCellSection sections[0x10];                         // +0x17c
};
struct TerrainCell {
    char pad_0x00[8];
    void* key;                                                 // +0x08
    char pad_0x0c[0x38 - 0x0c];
    TerrainCellOwner* owner;                                   // +0x38
    char pad_0x3c[4];
    int field_0x40;                                            // +0x40 nonzero lets TerrainShadow slot 30 emit the cell's vertices
};

class TerrainShutdownObject {
public:
    // 0x00484d70 (thiscall, ret 8): walks the cell quadtree (child table +0x04, shift +0x29, nibble
    // table at 0x0056c2ac) down to the leaf whose +0x24/+0x26 halfwords equal (x, z); null if none.
    TerrainCell* LookupCell(int x, int z);
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
    void* cellTable;                                           // +0x04 cell table read by 0x00483910
    char pad_0x08[0x18 - 0x08];
    float heightMin;                                           // +0x18 lower height bound of the grid (compared with segment y)
    float heightMax;                                           // +0x1c upper height bound
    char pad_0x20[0x28 - 0x20];
    unsigned char field_0x28;
    unsigned char gridShift;                                   // +0x29 log2 shift: grid edge = 16 << gridShift cells
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
    int drawDistanceSetting;  // +0x00 SelectQuality passes it to SetField0xbec (the >=0 clamped value stored in Terrain::field_0xbec, ctor 1000)
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
// Tier 2: retail constructs this global with 0x004bed80 (PCCamera(int), argument 1) and registers
// the 0x004624d0 destructor stub (a tail jump to ~Camera), so it is a PCCamera (0x220 bytes);
// the stand-in keeps the ctor/dtor external and the words as an opaque copyable block.
struct TerrainSharedState {
    int words[0x88];
    explicit TerrainSharedState(int flags);   // 0x004bed80 (PCCamera)
    ~TerrainSharedState();                    // 0x004624d0
};

// The timer's identity and layout are already reconstructed in PeakHold.h.
// Keep the local alias for the unknown roles of these ten Terrain timers.
typedef UnknownPeakHold TerrainPeakHold;
struct TerrainHost {
    int field_0x00;
    int field_0x04;
    TerrainSharedState* sharedState;  // +0x08 pointer to the 0x220-byte block copied to g_terrainSharedState by slot 23
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
// 0x0043c890 multiplies by the reciprocal (fld 1.0; fdiv s; then three fmul).  Written with a
// direct constructor call (not via operator*) so the constructor is one inline level below the
// call site: CastSegment's prologue keeps it inlined, as retail does (see the inline-budget
// note above Terrain::CastSegment).
inline TerrainVec3 operator/(const TerrainVec3& v, float s)
{
    float inv = 1.0f / s;
    return TerrainVec3(inv * v.x, inv * v.y, inv * v.z);
}

float TerrainDot(const TerrainVec3& a, const TerrainVec3& b);   // 0x0040ae30 (cdecl, out of line)
// 0x004a11e0 (cdecl): unit normal of the triangle (a, b, c) written to *out; the fifth
// argument, when non-null, receives the plane offset (tier 3: the tail is `-dot(n, a)`).
void TerrainTriangleNormal(const TerrainVec3* a, const TerrainVec3* b, const TerrainVec3* c,
                           TerrainVec3* out, float* planeD);

// 0x00507510 / 0x00507590: inline helpers whose out-of-line COMDAT copies retail kept because
// CastSegment's inline budget ran out (tier 2: cdecl, no ecx, `ret`; the by-value TerrainVec3
// return is the hidden out pointer in arg 0).  References are ABI-identical to pointers here;
// they let CastSegment pass the `-dir` temporary straight through (retail `push eax` after the
// out-of-line operator- at 0x005073a2).
inline TerrainVec3 TerrainClipRayToPlaneY(TerrainVec3& origin, const TerrainVec3& dir, float y);
inline TerrainVec3 TerrainClipRayToPlaneZ(TerrainVec3& origin, const TerrainVec3& dir, float z);

class Terrain : public GameObject, public GroundFogableObject {
public:
    // 0x00507c10 (thiscall, ret 0x10; 58 callers).  Snaps pos->y to the terrain surface under
    // (pos->x, pos->z), optionally writes the surface normal and a per-cell surface byte.
    // The third argument is 0 at nearly every caller and 1 at one (0x004b1567): tier 3 name
    // "flatShaded" = use the triangle's face normal instead of blending corner normals.
    void QueryGround(TerrainVec3* pos, TerrainVec3* normal, int flatShaded, unsigned char* surface);
    // 0x00506e90 (thiscall, ret 0x18).  Casts the world segment from->to against the terrain grid:
    // rescales to grid space (1 / gridCellSize), clips it to the grid box, hands it to
    // heightField->CastSegment, and on a hit returns the world-space hit point in *out.
    int CastSegment(const TerrainVec3* from, const TerrainVec3* to, TerrainVec3* out, int a, int b, int c);
    // GameObject slot overrides (vtable 0x0055825c, tier 1 addresses).
    virtual int GameObjectVirtualSlot19(int a);                 // 0x00507920 returns 0
    void GetHeightRange(float* outMin, float* outMax);          // 0x00508970
    virtual int GameObjectVirtualSlot22(int a, int b);          // 0x004dc4c0 returns 0 (shared stub)
    // Slot 23 (0x00508850, ret 8): debug key handler; tier 3 name kept in the comment only
    // so the override has the base's name.  First argument is the input event.
    virtual int GameObjectVirtualSlot23(int event, int unused);
    // 0x00507bb0 (plain thiscall, no args): derives four floats from the ints at 0x6c / 0x70
    // (tier 3: a = appliedDrawDistance, b = field_0x70): 0x74 = a/b, 0x78 = (a*a)/(b*b), 0x7c = 1/0x78,
    // 0x80 = 1/0x74.
    void ComputeRatios();
    // 0x005057d0 (ret 4, tier 3 name): when `object` is in the ownedObjectArray[] array, swaps the last
    // element into its place, decrements the count at ownedObjectArrayCount and stores `object` in the freed
    // last slot, i.e. moves it behind the live prefix.
    BaseObject* AcquireOwnedObject();                           // 0x00505600
    void RetireOwnedObject(BaseObject* object);
    void SetField0xbec(int value);                              // 0x00507930 (ret 4)
    void SelectQuality(int index);                              // 0x00507960 (ret 4)
    explicit Terrain(int a);                                    // 0x00505830 (ret 4), forwards a to GameObject(int)
    virtual ~Terrain();                                         // 0x005059b0 -> core 0x005079f0

    BaseObject* field_0x30;                                     // slot 2 called (0x005079f0)
    TerrainComObject* field_0x34;                               // Release()d, then zeroed
    TerrainSurfaceRegistry* field_0x38;                         // +0x38 tested by 0x00505600 (registers new surfaces)
    int field_0x3c;
    float gridCellSize;                                           // +0x40 ctor 1.0f
    TerrainShutdownObject* heightField;  // +0x44 object with CastSegment slot and GetCellCorners; QueryGround returns early when null; dtor Shutdown(1) then deletes
    char field_0x48[0x24];
    int appliedDrawDistance;  // +0x6c slot 12 (0x507610) compares drawDistance with it, sets a dirty flag when different, then stores drawDistance into it; appliedDrawDistance is also the int a of ComputeRatios
    int field_0x70;
    float field_0x74;                                           // ratio of the two ints (0x507bb0)
    float field_0x78;                                           // ratio squared
    float field_0x7c;                                           // 1 / ratio squared
    float field_0x80;                                           // 1 / ratio
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
    int ownedObjectCount;                                            // +0x540 count of ownedObjects[]
    BaseObject* ownedObjects[(0xbec - 0x544) / 4];
    int drawDistance;                                            // +0xbec ctor 1000
    int qualityIndex;  // +0xbf0 SelectQuality arg; indexes g_pTerrainQualityTable
    int drawDistanceDirty;  // +0xbf4 set to 1 when SetField0xbec changes the value
    int field_0xbf8;
    int field_0xbfc;
    int field_0xc00;
    int field_0xc04;
    int field_0xc08;
    int field_0xc0c;
    int blockCount;                                            // +0xc10 count of blocks[]
    void** blocks;                                         // +0xc14 array of plain-new pointers
    TerrainSurfaceDesc field_0xc18;                             // +0xc18 surface description (pointer passed to CreateSurface)
    float invGridCellSize;                                          // +0xc30 ctor 1.0f
    int field_0xc34;
    int field_0xc38;
    BaseObject* field_0xc3c;                                    // slot 2 called
    int field_0xc40;
    TerrainMatrix transform;                                  // +0xc44 copy of the matrix from 0x004a1410
    TerrainOwned* field_0xc84;
    TerrainOwned* field_0xc88;
    int field_0xc8c;
    char field_0xc90[0xca4 - 0xc90];
    int qualityParamA;  // +0xca4 SelectQuality: table[index].field_0x04 (or table[9] when lowestQualityOverride)
    int qualityParamB;  // +0xca8 SelectQuality: table[index].field_0x08
    int field_0xcac;
    int lowestQualityOverride;  // +0xcb0 slot 23 key 2 toggles it; when set qualityParamA comes from table[9]
    int field_0xcb4;
    BaseObject** ownedObjectArray;                                   // +0xcb8 array; each element slot 2 called
    int ownedObjectArrayCount;                                            // +0xcbc count of ownedObjectArray[]
    int field_0xcc0;
};

#endif
