#pragma once

// Griddraw.h -- the reconstructed part of D:\aardvark\VC\krusty2\Griddraw.cpp
// (code 0x0047dda0..0x0048389f). See docs/GRIDDRAW.md for the evidence.
//
// Confirmed (tier 1): RTTI .?AVGridNode@@ (COL 0x0055cb08, vtable 0x00553ec0,
// 2 slots) and .?AVDrawableGridNode@@ (COL 0x0055cad8, vtable 0x00553e98,
// 9 slots, single base GridNode at mdisp 0). The DrawableGridNode constructor
// 0x0047e540 writes its vptr at 0x0047e562; the destructor 0x0047ec60 writes
// it again and calls the GridNode destructor 0x004838a0.
// Member, method and type names are provisional (tier 3) unless noted.

#include "Gridbase.h"

class UnknownTextureStream;
struct GridVertex;
struct GridCamera;

// AgeManager (D:\aardvark\VC\krusty2\AgeManager.cpp, tier 1 by its
// destructor's __FILE__ use) as seen from this file: 0x00401050 registers a
// 0x14-byte entry with an eviction callback, 0x004010d0 unregisters it.
struct GridAgeEntry {
    int age;                                   // +0x00 set from the manager
    int (*callback)(void* owner);              // +0x04
    void* owner;                               // +0x08
    int field_0x0c;                            // +0x0c
    int size;                                  // +0x10 (nonzero while registered)
};
class GridAgeManager {
public:
    void UnknownFunction401050(GridAgeEntry* entry, int (*callback)(void* owner), void* owner,
                               int a, int size);    // 0x00401050 (ret 0x14)
    void UnknownFunction4010d0(GridAgeEntry* entry);  // 0x004010d0 (ret 4)
    void UnknownFunction401250(GridAgeEntry* entry);  // 0x00401250 (ret 4), marks the entry used
};

// Boundary view of the renderer at Terrain+0x18 (thiscall virtuals). Slot 7
// takes (stage, state, value) like a texture-stage state, slot 8 three
// values, slot 15 draws an indexed list (primitive 4, vertex format, vertices,
// count, indices, count, flags). Names tier 3.
class GridRenderDevice {
public:
    virtual void UnknownVirtualSlot0() = 0;
    virtual void UnknownVirtualSlot1() = 0;
    virtual void UnknownVirtualSlot2() = 0;
    virtual void UnknownVirtualSlot3() = 0;
    virtual void UnknownVirtualSlot4() = 0;
    virtual void UnknownVirtualSlot5() = 0;
    virtual void UnknownVirtualSlot6() = 0;
    virtual void UnknownVirtualSlot7(int stage, int state, int value) = 0;
    virtual void UnknownVirtualSlot8(int a, int b, int c) = 0;
    virtual void UnknownVirtualSlot9() = 0;
    virtual void UnknownVirtualSlot10() = 0;
    virtual void UnknownVirtualSlot11() = 0;
    virtual void UnknownVirtualSlot12() = 0;
    virtual void UnknownVirtualSlot13() = 0;
    virtual void UnknownVirtualSlot14() = 0;
    virtual int UnknownVirtualSlot15(int primitive, int format, void* vertices, int vertexCount,
                                     unsigned short* indices, int indexCount, int flags) = 0;

    void* field_0x04;
    GridCamera* field_0x08;                    // +0x08 the camera the box test takes (0x00481de0)
};

// Boundary view of ManagedTexture (src/reconstructed binds 0x00510910 as
// ManagedTexture::UnknownFunction510910): bit 0 of +0x68 enables the texture
// coordinate update that 0x00510910 applies to a vertex range.
class GridManagedTexture {
public:
    int UnknownFunction510910(float* a, float* b, float* c, float* u, float* v, int count,
                              unsigned int stride);  // 0x00510910 (ret 0x1c)
    unsigned char field_0x00[0x68];
    unsigned char field_0x68;
};

// Object embedded at Terrain+0x2c; 0x00484f10 (the next TU) prepares a
// vertex range before an unlit draw. Tier 3.
class GridVertexLighter {
public:
    void UnknownFunction484f10(GridVertex* vertices, int count);  // 0x00484f10 (ret 8)
    int field_0x00;
    int field_0x04;
    int field_0x08;
};

// The game object at 0x0056e26c as this file sees it (other views name it
// TrackGame). +0x2d0 disables terrain drawing; +0x550/+0x554 are the
// texture-stage values restored after a close-up draw.
struct GridGameSettings {
    unsigned char field_0x000[0x2d0];
    int field_0x2d0;
    unsigned char field_0x2d4[0x550 - 0x2d4];
    int field_0x550;
    int field_0x554;
};
extern GridGameSettings* g_gridGameSettings;          // 0x0056e26c

// Object at Terrain+0xc40; 0x0049e4a0 is called with (&Terrain+0xc44, count,
// buffer + 0xc, buffer, 0x20, 0) after a node rebuilds its vertex buffer.
class GridVertexSink {
public:
    void UnknownFunction49e4a0(void* matrix, int count, void* vertices, void* buffer,
                               int stride, int flags);  // 0x0049e4a0
};

// Boundary view of the camera at Terrain+0xc8c: its matrix at +0xec is
// passed to the box test (see src/krusty2/visibility/VisibilityQuadTree.h).
struct GridCamera {
    unsigned char field_0x000[0xec];
    float matrix[16];                          // +0xec
};

// Boundary view of VisibilityClipper (src/krusty2/visibility): 0x0052f570
// tests a box (centre, half extents) against the camera frustum.
class GridVisibilityClipper {
public:
    int TestBox(GridCamera* camera, const float* matrix, const float* center, const float* extent,
                int* screenRect, int* cornersInside, int* a6);  // 0x0052f570 (ret 0x1c)
};
extern GridVisibilityClipper* g_visibilityClipper;   // 0x00575a98

// Boundary view of Terrain (RTTI .?AVTerrain@@; reconstructed in
// src/krusty2/broadphase/Terrain.h). Only the members this file touches.
struct GridTerrain {
    unsigned char field_0x000[0x18];
    GridRenderDevice* field_0x18;              // +0x18
    unsigned char field_0x01c[0x2c - 0x1c];
    GridVertexLighter field_0x2c;              // +0x2c
    int field_0x38;                            // +0x38 enables texture coordinate updates
    unsigned char field_0x03c[0x40 - 0x3c];
    float gridCellSize;                        // +0x40
    unsigned char field_0x044[0x60 - 0x44];
    float field_0x60;                          // +0x60..+0x68 viewer position (0x00481cc0)
    float field_0x64;
    float field_0x68;
    int field_0x6c;
    int field_0x70;
    float field_0x74;
    float field_0x78;                          // +0x78 distance factor (Terrain.h: ratio squared)
    unsigned char field_0x07c[0x88 - 0x7c];
    int field_0x88;                            // +0x88 indices drawn
    int field_0x8c;                            // +0x8c vertices drawn
    int field_0x90;                            // +0x90 incremented per vertex error test
    int field_0x94;
    int field_0x98;                            // +0x98 incremented per detail update (0x004815e0)
    int field_0x9c;                            // +0x9c incremented by 0x00482dd0
    int field_0xa0;                            // +0xa0 incremented by 0x00482c90
    int field_0xa4;                            // +0xa4 incremented per buffer rebuild (0x0047f840)
    int field_0xa8;                            // +0xa8 vertices pinned by 0x00482760
    int field_0xac;                            // +0xac vertices released by 0x00482760
    int field_0xb0;                            // +0xb0 incremented per vertex test (0x00482a40)
    int field_0xb4;                            // +0xb4 copied to the draw data's +0x130
    int field_0xb8[17 * 17];                   // +0xb8 cleared by the first node constructor
    int field_0x53c;                           // +0x53c compared with field_0xb8[i] (0x00482f00)
    unsigned char field_0x540[0xc14 - 0x540];
    GridBaseBlock** blocks;                    // +0xc14
    unsigned char field_0xc18[0xc34 - 0xc18];
    unsigned char field_0xc34;                 // +0xc34 bit 0 copied into the node flags
    unsigned char field_0xc35[3];
    int field_0xc38;                           // +0xc38 stream offset of the child tables
    int field_0xc3c;
    GridVertexSink* field_0xc40;               // +0xc40
    float field_0xc44[16];                     // +0xc44 transform
    int field_0xc84;
    GridAgeManager* field_0xc88;               // +0xc88
    GridCamera* field_0xc8c;                   // +0xc8c
    unsigned char field_0xc90[0xc9c - 0xc90];
    union {
        float field_0xc9c;                     // +0xc9c
        int unlitColor;                        // copied into unlit vertices (0x00480c90)
    };
    union {
        float field_0xca0;                     // +0xca0
        int unlitSpecular;
    };
    int field_0xca4;                           // +0xca4 detail limit for the close-up stage state
    int field_0xca8;
    int field_0xcac;
    int field_0xcb0;                           // +0xcb0
    int field_0xcb4;                           // +0xcb4 close-up stage state set
};

// Per-node draw data (DebugCalloc'd 0x1c0 or 0x540 bytes by 0x0047e600;
// freed with plain delete by the destructor). Offsets tier 2.
struct GridNodeDrawData {
    unsigned char field_0x000[17 * 17];        // +0x000 per-vertex bytes (bit 7 tested)
    unsigned char b0 : 1;                      // +0x121 bit 0
    unsigned char b1 : 1;                      // +0x121 bit 1
    unsigned char b2 : 1;                      // +0x121 bit 2
    unsigned char b3 : 1;                      // +0x121 bit 3
    unsigned short field_0x122;
    unsigned short field_0x124;
    unsigned short field_0x126;
    unsigned short field_0x128;
    short field_0x12a;
    short field_0x12c;
    short field_0x12e;
    int field_0x130;
    void* field_0x134;                         // +0x134 buffer (DebugFree'd, line 0x330/0x35a)
    void* field_0x138;                         // +0x138 buffer (line 0x337/0x361)
    short field_0x13c;
    short field_0x13e;
    short field_0x140;                         // +0x140 size charged to g_gridDrawMemory for +0x134
    short field_0x142;                         // +0x142 size charged for +0x138
    GridAgeEntry ageEntry;                     // +0x144
    float field_0x158;
    float field_0x15c;
    float field_0x160;
    float field_0x164;
    float field_0x168;
    float field_0x16c;
    float field_0x170;
    float field_0x174;
    short field_0x178;
    short field_0x17a;
    void* field_0x17c;
};

// Three floats; the constructor builds a temporary that is then copied
// (0x0047e600 sets the extent to (0.1, 0.1, 0.1) through a stack copy).
struct GridVec3 {
    float x;
    float y;
    float z;
    GridVec3() {}
    GridVec3(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }
};

class GridNode {
public:
    virtual ~GridNode();                                        // 0x004838a0 (slot 0 0x004838f0)
    virtual int UnknownVirtualSlot1(void* a, void* b, void* out, int c, int d, int e); // 0x00483d40
    // 0x00484d70: the leaf whose grid position is (x, z), or 0 (GridNode.cpp).
    GridNode* UnknownFunction484d70(int x, int z);

    GridNode** children;                       // +0x04 256 entries (0x400 bytes)
    GridNode* parent;                          // +0x08
    GridBaseBlock* block;                      // +0x0c
    float field_0x10;
    float field_0x14;
    float field_0x18;
    float field_0x1c;
    unsigned short field_0x20;                 // +0x20 compared with the draw data's +0x122
    unsigned short field_0x22;                 // +0x22 index into Terrain::blocks
    unsigned short gridX;                      // +0x24 position in units of 16 << shift
    unsigned short gridZ;                      // +0x26
    unsigned char level;                       // +0x28
    unsigned char shift;                       // +0x29
    unsigned char field_0x2a;
    unsigned char field_0x2b;
    int field_0x2c;
    int* field_0x30;                           // +0x30 child stream offsets (0x400 bytes)
};

// 0x2c-byte records at GridNodeExtra::field_0x38 (one per 4 x 4 block);
// +0 comes from the node's stream header, +4 starts at 0.
struct GridBlockRecord {
    int field_0x00;
    int field_0x04;
    unsigned char field_0x08[0x1c - 0x08];
    float field_0x1c;                          // +0x1c set to 1.0 after a rebuild (0x0047f210)
    float field_0x20;
    float field_0x24;
    float field_0x28;                          // +0x28 detail of the block
};

// Block at DrawableGridNode+0x3c (inside the draw data at +0x180, or +0x240
// for nodes with per-block ranges). Offsets tier 2.
struct GridNodeExtra {
    int field_0x00;                            // +0x00 byte count read from the stream
    void* field_0x04;                          // +0x04 DebugMalloc'd, field_0x00 bytes
    GridManagedTexture* field_0x08;            // +0x08 tested before slot 4(-1)
    int field_0x0c;
    int field_0x10;
    unsigned char field_0x14[0x28 - 0x14];
    float field_0x28;                          // +0x28 set to 1.0 after a rebuild
    float field_0x2c;
    float field_0x30;
    float field_0x34;                          // +0x34 detail of the whole node
    GridBlockRecord* field_0x38;               // +0x38 sixteen records or null
    unsigned char field_0x3c[4];
};

// Twelve-byte entries at GridNodeDrawData::field_0x17c, one per 4 x 4 block
// in Z order; +0 is the running end of the block's index range.
struct GridBlockRange {
    short end;                                 // +0x00 running end of the vertex range
    short indexEnd;                            // +0x02 running end of the index range
    float centerY;                             // +0x04 box centre height (0x00481a20)
    float extentY;                             // +0x08 box half height
};

class DrawableGridNode : public GridNode {
public:
    DrawableGridNode(int a0, int a1, GridNode* parent, GridTerrain* terrain, int a4, int a5,
                     int a6, int a7, int a8, int a9, int a10);  // 0x0047e540 (ret 0x2c)
    virtual ~DrawableGridNode();                                // 0x0047ec60 (slot 0 0x0047e5e0)
    DrawableGridNode* UnknownFunction47e600(UnknownTextureStream* stream, int a1, DrawableGridNode* parent,
                                            int level, int shift, int x, int z, int index,
                                            float* boundsMin, float* boundsMax); // 0x0047e600 (ret 0x28)
    virtual GridNode* UnknownVirtualSlot2(UnknownTextureStream* stream, int a1, DrawableGridNode* parent,
                                          int level, int shift, int x, int z, int index,
                                          void* a8, void* a9) { return 0; }   // 0x004806e0
    virtual void UnknownVirtualSlot3(float a) {}                // 0x004806f0 (shared)
    virtual void UnknownVirtualSlot4(int a) {}                  // 0x004806f0 (shared)
    virtual void UnknownVirtualSlot5(UnknownTextureStream* stream) {} // 0x004806f0 (shared)
    virtual void UnknownVirtualSlot6() {}                       // 0x0044d710 (shared)
    virtual void UnknownVirtualSlot7(int a) {}                  // 0x004806f0 (shared)
    virtual int UnknownVirtualSlot8(int a) { return 0; }        // 0x004da550 (shared)

    void UnknownFunction47edb0(int recurse);                    // 0x0047edb0 (ret 4)
    int UnknownFunction47ef70();                                // 0x0047ef70
    int UnknownFunction47ef80(int x, int z, int size, int quad); // 0x0047ef80 (ret 0x10)
    int UnknownFunction47f210();                                // 0x0047f210
    int UnknownFunction47f840();                                // 0x0047f840
    int UnknownFunction47fce0(int level, int n, int x, int z, int dx, int dz, int* closed); // 0x0047fce0 (ret 0x1c)
    void UnknownFunction47fe70(int x, int z, int dx, int dz, int n); // 0x0047fe70 (ret 0x14)
    int UnknownFunction480200(int level, int n, int x, int z, int dx, int dz, int* closed); // 0x00480200 (ret 0x1c)
    int UnknownFunction480700(int n, int x, int z, int dx, int dz); // 0x00480700 (ret 0x14)
    int UnknownFunction480900(void* target);                    // 0x00480900 (ret 4)
    int UnknownFunction480920(void* target, int a5);            // 0x00480920 (ret 8)
    int UnknownFunction480940(void* target, int x, int z, int size, int quad, int a5); // 0x00480940 (ret 0x18)
    int UnknownFunction480ad0(void* target, int x, int z, int size, int quad, int a5); // 0x00480ad0 (ret 0x18)
    void UnknownFunction480c90(int a5, GridVertex* vertices, int vertexCount, unsigned short* indices,
                               int indexCount, int block);      // 0x00480c90 (ret 0x18)
    void UnknownFunction480fb0(int a5, GridVertex* vertices, int vertexCount, unsigned short* indices,
                               int indexCount, int block);      // 0x00480fb0 (ret 0x18)
    int UnknownFunction481170();                                // 0x00481170
    int UnknownFunction481180(int x, int z, int size, int quad); // 0x00481180 (ret 0x10)
    int UnknownFunction481300(int x, int z, int size, int quad); // 0x00481300 (ret 0x10)
    // 0x004813e0: counts seams where neighbouring children disagree (the
    // count is never used) and recurses into the children.
    void UnknownFunction4813e0();
    void UnknownFunction481580();                               // 0x00481580
    int UnknownFunction481a20(int quad);                        // 0x00481a20 (ret 4)
    float UnknownFunction481b30(int block);                     // 0x00481b30 (ret 4)
    float UnknownFunction4815e0(int coarse);                    // 0x004815e0 (ret 4)
    int UnknownFunction481cc0(int x, int z);                    // 0x00481cc0 (ret 8)
    int UnknownFunction481db0(void* a0, void* a1);              // 0x00481db0 (ret 8)
    int UnknownFunction481de0(void* a0, void* a1, float* center, float* extent, int a4, int a5,
                              int size, int a7, int a8, int a9); // 0x00481de0 (ret 0x28)
    void UnknownFunction4824c0(UnknownTextureStream* stream, void* a1, int recurse,
                               int coarse);                     // 0x004824c0 (ret 0x10)
    void UnknownFunction4826d0();                               // 0x004826d0
    void UnknownFunction482760();                               // 0x00482760
    void UnknownFunction482a40(int x, int z);                   // 0x00482a40 (ret 8)
    void UnknownFunction482ae0();                               // 0x00482ae0
    void UnknownFunction482b40(int x0, int z0, int x1, int z1, int half, int start, int step,
                               int rowStep);                    // 0x00482b40 (ret 0x20)
    void UnknownFunction482c90(int x, int z, int flag);         // 0x00482c90 (ret 0xc)
    void UnknownFunction482dd0(int x, int z, int flag, int dir, GridBaseCell* origin); // 0x00482dd0 (ret 0x14)
    int UnknownFunction482f00(int x, int z);                    // 0x00482f00 (ret 8)
    int UnknownFunction483040();                                // 0x00483040
    int UnknownFunction483100();                                // 0x00483100
    void UnknownFunction483200(int x, int z, int flag, int dir, GridBaseCell* origin,
                               int minLevel);                   // 0x00483200 (ret 0x18)
    void UnknownFunction4835c0(DrawableGridNode* child, int x, int z, int flag, int dir,
                               GridBaseCell* cell);             // 0x004835c0 (ret 0x18)
    void UnknownFunction464e90();                               // 0x00464e90 (shared empty body)

    GridTerrain* terrain;                      // +0x34
    GridNodeDrawData* data;                    // +0x38
    GridNodeExtra* extra;                      // +0x3c points into data
    int childMask;                             // +0x40 bit 1..4: 8-cell quadrants, 5..: 4-cell
    GridVec3 center;                           // +0x44 bounding box centre (world units)
    GridVec3 extent;                           // +0x50 half extents
};

// 0x0047ecc0: AgeManager eviction callback for a node's vertex buffers.
int UnknownFunction47ecc0(void* owner);

// One link of the 17 x 17 vertex grid (0x006754e0, 32 bytes each). The
// first two words are filled by 0x0047e4d0, the rest by 0x0047e340.
struct GridEdge {
    int x;
    int z;
    int x0;
    int z0;
    int x1;
    int z1;
    int dir0;                                   // +0x18 direction bit passed with (x0, z0)
    int dir1;                                   // +0x1c direction bit passed with (x1, z1)
};
extern GridEdge g_gridEdges[17 * 17];                   // 0x006754e0

// 0x0047e340: fills the neighbours of (x, z) for one subdivision step.
void UnknownFunction47e340(int x, int z, int h, int v, int maskX, int maskZ, GridEdge* out);
// 0x0047e430: one subdivision level of the edge table (near miss, see
// samples/render/GriddrawNearMisses.cpp).
void UnknownFunction47e430(int half, int start, int step, int rowStep);

// One cached vertex (32 bytes; tier 2 from 0x0047de90's stores).
struct GridVertex {
    float x;
    float y;
    float z;
    union {
        float nx;                              // normal x (data b2 set)
        int packed;                            // otherwise 0 or (dx << 16) + dz
    };
    union {
        float field_0x10;                      // normal y or a second coordinate pair
        int color;                             // diffuse colour for unlit draws (0x00480c90)
    };
    union {
        float field_0x14;
        int specular;                          // specular colour for unlit draws
    };
    float u;
    float v;
};

struct GridVertexSlot {
    unsigned int stamp;
    int index;
};

// The vertex cache global at 0x00677910 (size 0x2d74). Its constructor is
// inlined in the dynamic initializer 0x0047dde0 (stamp = -1, then Reset(0));
// the destructor 0x0047de10 is empty.
class GridVertexCache {
public:
    GridVertexCache() { stamp = -1; Reset(0); }
    ~GridVertexCache() {}
    void Reset(DrawableGridNode* node);        // 0x0047de20 (ret 4)
    int GetVertex(int x, int z);               // 0x0047de90 (ret 8)

    DrawableGridNode* node;                    // +0x0000
    unsigned int stamp;                        // +0x0004
    GridVertexSlot slots[17 * 17];             // +0x0008
    GridVertex vertices[17 * 17];              // +0x0910
    int count;                                 // +0x2d30
    int originX;                               // +0x2d34
    int originZ;                               // +0x2d38
    int shift;                                 // +0x2d3c
    GridBaseCell* cells;                       // +0x2d40
    float field_0x2d44;
    float field_0x2d48;
    float field_0x2d4c;
    float field_0x2d50;
    int field_0x2d54;
    float scale;                               // +0x2d58
    float minY;                                // +0x2d5c
    float maxY;                                // +0x2d60
    int field_0x2d64;
    float field_0x2d68;
    float field_0x2d6c;
    float field_0x2d70;
};
