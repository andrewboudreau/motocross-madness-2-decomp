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

#include "TrackGameGlobal.h"
#include "Gridbase.h"

class UnknownTextureStream;
struct GridVertex;
struct GridCamera;

// The grid caches share AgeManager's 0x14-byte manager and entry records.
// Eviction receives owner and context; callbacks may ignore the second value.
#include "AgeEntry.h"
#include "AgeManager.h"

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
    GridCamera* camera;                        // +0x08 the camera the box test takes (0x00481de0)
};

// Boundary view of ManagedTexture (src/reconstructed binds 0x00510910 as
// ManagedTexture::RemapTexCoordsIfMoved): bit 0 of +0x68 enables the texture
// coordinate update that 0x00510910 applies to a vertex range.
class GridManagedTexture {
public:
    int RemapTexCoordsIfMoved(float* a, float* b, float* c, float* u, float* v, int count,
                              unsigned int stride);  // 0x00510910 (ret 0x1c)
    unsigned char field_0x00[0x68];
    unsigned char mapFlags;  // +0x68  TextureMap.h mapFlags; bit 0 (ManagedTexture) enables the texture coordinate remap
};

// Object embedded at Terrain+0x2c; 0x00484f10 (the next TU) prepares a
// vertex range before an unlit draw. Tier 3.
class GridVertexLighter {
public:
    void FogVertices(GridVertex* vertices, int count);  // 0x00484f10 (ret 8)
    int field_0x00;
    int field_0x04;
    int field_0x08;
};

// The game object at 0x0056e26c as this file sees it (other views name it
// TrackGame). +0x2d0 disables terrain drawing; +0x550/+0x554 are the
// texture-stage values restored after a close-up draw.
struct GridGameSettings {
    unsigned char field_0x000[0x2d0];
    int softwareRenderer;
    unsigned char field_0x2d4[0x550 - 0x2d4];
    int minFilter;
    int mipFilter;
};

// Object at Terrain+0xc40; 0x0049e4a0 is called with (&Terrain+0xc44, count,
// buffer + 0xc, buffer, 0x20, 0) after a node rebuilds its vertex buffer.
class GridVertexSink {
public:
    void LightVertices(void* matrix, int count, void* vertices, void* buffer,
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
#include "../krusty2/visibility/VisibilityClipperGlobal.h"   // 0x00575a98

// Boundary view of Terrain (RTTI .?AVTerrain@@; reconstructed in
// src/krusty2/broadphase/Terrain.h). Only the members this file touches.
struct GridTerrain {
    unsigned char field_0x000[0x18];
    GridRenderDevice* renderer;                // +0x18
    unsigned char field_0x01c[0x2c - 0x1c];
    GridVertexLighter vertexLighter;           // +0x2c
    int updateTextureCoordinates;              // +0x38 enables texture coordinate updates
    unsigned char field_0x03c[0x40 - 0x3c];
    float gridCellSize;                        // +0x40
    unsigned char field_0x044[0x60 - 0x44];
    float viewerX;                             // +0x60..+0x68 viewer position (0x00481cc0)
    float viewerY;
    float viewerZ;
    int appliedDrawDistance;                  // +0x6c (Terrain.h name)
    int cameraFocalLength;                    // +0x70  Terrain cameraFocalLength (0x00507836)
    float drawDistanceRatio;  // +0x74  Terrain 0x00507bb0: appliedDrawDistance / cameraFocalLength; scales the error limits (0x004815e0)
    float drawDistanceRatioSquared;  // +0x78  Terrain 0x00507bb0: square of +0x74; CanDropVertex's limit factor (0x00481cc0)
    unsigned char field_0x07c[0x88 - 0x7c];
    int indicesDrawn;                          // +0x88 indices drawn
    int verticesDrawn;                         // +0x8c vertices drawn
    int vertexErrorTests;                     // +0x90 incremented per vertex error test
    int field_0x94;
    int detailUpdates;                        // +0x98 incremented per detail update (0x004815e0)
    int vertexDirectionSets;  // +0x9C  incremented per SetVertexDirection call (0x00482dd0)
    int vertexChangePropagations;  // +0xA0  incremented per PropagateVertexChange call (0x00482c90)
    int bufferRebuilds;                       // +0xa4 incremented per buffer rebuild (0x0047f840)
    int verticesPinned;                        // +0xa8 vertices pinned by 0x00482760
    int verticesReleased;                      // +0xac vertices released by 0x00482760
    int vertexTests;                          // +0xb0 incremented per vertex test (0x00482a40)
    int frameCount;                           // +0xB4  Terrain +0xb4, incremented by PrepareGeometry 0x00507610 (0x00507861) after the visibility walk
    int vertexRetestStamps[17 * 17];  // +0xB8  per vertex: set to retestStamp by 0x00482760, compared by 0x00482f00; cleared by the first node
    int retestStamp;  // +0x53C  incremented per RetestChangedVertices (0x00482760); stamps vertexRetestStamps
    unsigned char field_0x540[0xc14 - 0x540];
    GridBaseBlock** blocks;                    // +0xc14
    unsigned char field_0xc18[0xc34 - 0xc18];
    unsigned char useVertexNormals;  // +0xC34  Terrain::Load sets it with the +0xc40 sink; bit 0 -> node b2 (normals written, sink called)
    unsigned char field_0xc35[3];
    int childTableOffset;                     // +0xc38 stream offset of the child tables
    int detailTexture;  // +0xC3C  Terrain::Load's 0x0050a590 texture (testnoise.tga); set -> 0x00480c90 writes detail coordinates
    GridVertexSink* vertexSink;                // +0xc40
    float transform[16];                       // +0xc44 transform
    int textureAgeManager;  // +0xC84  Grid1.h's Grid1Terrain view: the node texture AgeManager
    AgeManager* ageManager;                    // +0xc88
    GridCamera* camera;                        // +0xc8c
    unsigned char field_0xc90[0xc9c - 0xc90];
    union {
        float field_0xc9c;                     // +0xc9c
        int unlitColor;                        // copied into unlit vertices (0x00480c90)
    };
    union {
        float field_0xca0;                     // +0xca0
        int unlitSpecular;
    };
    int qualityParamA;                         // +0xca4 detail limit for the close-up stage state
    int qualityParamB;
    int canSinglePassDual;                    // +0xcac Terrain slot 14 prints it as "CanSinglePassDual"
    int lowestQualityOverride;                 // +0xcb0
    int closeUpStateSet;                      // +0xcb4 close-up stage state set
};

// Per-node draw data (DebugCalloc'd 0x1c0 or 0x540 bytes by 0x0047e600;
// freed with plain delete by the destructor). Offsets tier 2.
struct GridNodeDrawData {
    unsigned char vertexFlags[17 * 17];        // +0x000 per-vertex bytes, row g_gridRow17[z] + x (bit 7 tested)
    unsigned char b0 : 1;                      // +0x121 bit 0
    unsigned char b1 : 1;                      // +0x121 bit 1
    unsigned char b2 : 1;                      // +0x121 bit 2
    unsigned char b3 : 1;                      // +0x121 bit 3
    unsigned short lowerErrorLimit;  // +0x122  0x004815e0 from the nearest distance; vertices with curve error below it drop (0x00482f00)
    unsigned short upperErrorLimit;  // +0x124  0x004815e0 from the farthest distance; vertices with curve error above it stay (0x00482f00)
    unsigned short previousLowerErrorLimit;  // +0x126  copy of +0x122 before 0x004815e0 recomputes it (0x00482760 retests the band)
    unsigned short previousUpperErrorLimit;  // +0x128  copy of +0x124 before 0x004815e0 recomputes it (0x00482760 retests the band)
    short triangleCount;                       // +0x12a triangles in this node's index list
    short childTriangleCount;                  // +0x12c children's +0x12a/+0x12c summed into the parent
    short field_0x12e;
    int testFrame;                            // +0x130  GridTerrain::frameCount when 0x004826d0/0x00482760 last tested the node; 0x00482f00 then runs the full drop test
    void* vertexBuffer;                        // +0x134 32-byte vertices (DebugFree'd, line 0x330/0x35a)
    void* indexBuffer;                         // +0x138 16-bit indices (line 0x337/0x361)
    short vertexCount;                         // +0x13c
    short indexCount;                          // +0x13e three per triangle
    short vertexBufferBytes;                   // +0x140 size charged to g_gridDrawMemory for +0x134
    short indexBufferBytes;                    // +0x142 size charged for +0x138
    AgeEntry ageEntry;                         // +0x144
    float texOriginU;  // +0x158  vertex cache u = ((x - origin) * texStep + texOriginU) * 0.9375 + 1/32 (0x0047de90)
    float texOriginV;  // +0x15C  vertex cache v origin (0x0047de90), as texOriginU
    float texStep;  // +0x160  texture coordinate step per grid sample (0x0047de90); x4 per 4x4 block build
    float gridMinX;  // +0x164  gridX << shift (0x0047e600); box and distance bounds in grid units
    float gridMinZ;  // +0x168  gridZ << shift (0x0047e600)
    float gridMaxX;  // +0x16C  gridMinX + (16 << shift) (0x0047e600)
    float gridMaxZ;  // +0x170  gridMinZ + (16 << shift) (0x0047e600)
    float inverseErrorScale;  // +0x174  abs(1 / (heightScale * reliefScale)) (0x0047e600); scales the error limits
    short visibleBlockMask;  // +0x178  bit n set when 4x4 block n passes the frustum test (0x00481a20)
    short field_0x17a;
    void* blockRanges;  // +0x17C  sixteen GridBlockRange records (data + 0x180) or null (0x0047e600)
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
    // 0x00483d40 (ret 0x18, GridNode.cpp): first crossing of the segment p0..p1
    // (node-local x, y grid units; z height) with the height field: walks the
    // grid-line crossings cell by cell, recurses into children whose height
    // range the segment reaches and intersects the two cell triangles in a leaf.
    // Writes *out in parent units, the leaf to *outNode and the rounded hit
    // coordinates to *outX/*outZ (each when non-null); returns 1 on a hit.
    virtual int UnknownVirtualSlot1(const GridVec3* p0, const GridVec3* p1, GridVec3* out, GridNode** outNode,
                                    int* outX, int* outZ); // 0x00483d40
    // 0x00484d70: the leaf whose grid position is (x, z), or 0 (GridNode.cpp).
    GridNode* FindLeaf(int x, int z);
    // 0x00483910 (ret 0x14, GridNode.cpp): descends to the leaf holding sample
    // (x, z) and writes the four corner heights, normals and sample bytes of
    // that cell (order (x,z), (x+1,z), (x,z+1), (x+1,z+1)); returns 1, or 0
    // with heights 0, normals (0,1,0) and bytes 0 when no leaf exists.
    int GetCellCorners(int x, int z, float* heights, GridVec3* normals, unsigned char* bytes);

    GridNode** children;                       // +0x04 256 entries (0x400 bytes)
    GridNode* parent;                          // +0x08
    GridBaseBlock* block;                      // +0x0c
    float reliefScale;  // +0x10  from the stream; scales height - baseHeight (and flips normals when negative)
    float heightScale;  // +0x14  from the stream; scales the whole cell height (0x00483910, 0x00481cc0)
    float minHeight;  // +0x18  from the stream; widens boundsMin.y, box bottom; segment test skips children below
    float maxHeight;  // +0x1C  from the stream; widens boundsMax.y, box top; segment test skips children above
    unsigned short maxError;  // +0x20  from the stream; <= lowerErrorLimit makes the node coarse, releasing children (0x004824c0)
    unsigned short blockIndex;                 // +0x22 index into Terrain::blocks
    unsigned short gridX;                      // +0x24 position in units of 16 << shift
    unsigned short gridZ;                      // +0x26
    unsigned char level;                       // +0x28
    unsigned char shift;                       // +0x29
    unsigned char childIndex;                  // +0x2a slot in the parent's children: x low nibble, z high nibble
    unsigned char field_0x2b;
    int childListOffset;  // +0x2C  from the stream; childOffsets read at terrain childTableOffset + it; 0: no children
    int* childOffsets;                        // +0x30 child stream offsets (0x400 bytes)
};

// 0x2c-byte records at GridNodeExtra::field_0x38 (one per 4 x 4 block);
// +0 comes from the node's stream header, +4 starts at 0.
struct GridBlockRecord {
    int textureReference;  // +0x0  stream header word; Grid1BlockTexture packs own bit and table index (0xfffe/0xffff: none)
    int field_0x04;
    unsigned char field_0x08[0x1c - 0x08];
    float mappingScale;  // +0x1C  RemapTexCoordsIfMoved scale; 1.0 after a rebuild (0x0047f210)
    float mappingOffsetU;  // +0x20  RemapTexCoordsIfMoved offsetU; 0 after a rebuild
    float mappingOffsetV;  // +0x24  RemapTexCoordsIfMoved offsetV; 0 after a rebuild
    float detail;                              // +0x28 detail of the block
};

// Block at DrawableGridNode+0x3c (inside the draw data at +0x180, or +0x240
// for nodes with per-block ranges). Offsets tier 2.
struct GridNodeExtra {
    int byteCount;                             // +0x00 byte count read from the stream
    void* data;                                // +0x04 DebugMalloc'd, byteCount bytes
    GridManagedTexture* texture;               // +0x08 tested before slot 4(-1)
    int textureReference;  // +0xC  stream header; Grid1NodeExtra packs own bit and texture table index
    int field_0x10;
    unsigned char field_0x14[0x28 - 0x14];
    float mappingScale;  // +0x28  RemapTexCoordsIfMoved scale; 1.0 after a rebuild
    float mappingOffsetU;  // +0x2C  RemapTexCoordsIfMoved offsetU; 0 after a rebuild
    float mappingOffsetV;  // +0x30  RemapTexCoordsIfMoved offsetV; 0 after a rebuild
    float detail;                              // +0x34 detail of the whole node
    GridBlockRecord* blockRecords;             // +0x38 sixteen records or null
    unsigned char field_0x3c[4];
};

// Twelve-byte entries at GridNodeDrawData::blockRanges, one per 4 x 4 block
// in Z order; +0 is the running end of the block's index range.
struct GridBlockRange {
    short end;                                 // +0x00 running end of the vertex range
    short indexEnd;                            // +0x02 running end of the index range
    float centerY;                             // +0x04 box centre height (0x00481a20)
    float extentY;                             // +0x08 box half height
};

class DrawableGridNode : public GridNode {
public:
    // Only terrain and parent are used; the rest are DrawableGridNodeSharedTextures' arguments
    // (the stream and bounds passed as ints).
    DrawableGridNode(int stream, int a1, GridNode* parent, GridTerrain* terrain, int level, int shift,
                     int x, int z, int index, int boundsMin, int boundsMax);  // 0x0047e540 (ret 0x2c)
    virtual ~DrawableGridNode();                                // 0x0047ec60 (slot 0 0x0047e5e0)
    DrawableGridNode* ReadNode(UnknownTextureStream* stream, int a1, DrawableGridNode* parent,
                               int level, int shift, int x, int z, int index,
                               float* boundsMin, float* boundsMax); // 0x0047e600 (ret 0x28)
    virtual GridNode* UnknownVirtualSlot2(UnknownTextureStream* stream, int a1, DrawableGridNode* parent,
                                          int level, int shift, int x, int z, int index,
                                          void* boundsMin, void* boundsMax) { return 0; }   // 0x004806e0
    virtual void UnknownVirtualSlot3(float a) {}                // 0x004806f0 (shared)
    virtual void UnknownVirtualSlot4(int a) {}                  // 0x004806f0 (shared)
    virtual void UnknownVirtualSlot5(UnknownTextureStream* stream) {} // 0x004806f0 (shared)
    virtual void UnknownVirtualSlot6() {}                       // 0x0044d710 (shared)
    virtual void UnknownVirtualSlot7(int a) {}                  // 0x004806f0 (shared)
    virtual int UnknownVirtualSlot8(int a) { return 0; }        // 0x004da550 (shared)

    void ReleaseVertexBuffers(int recurse);                    // 0x0047edb0 (ret 4)
    int WalkVisibleFromRoot();                                // 0x0047ef70
    int WalkVisibleQuadrants(int x, int z, int size, int quad); // 0x0047ef80 (ret 0x10)
    int RebuildBlockBuffers();                                // 0x0047f210
    int RebuildWholeNodeBuffers();                                // 0x0047f840
    int EmitDiagonalTriangles(int level, int n, int x, int z, int dx, int dz, int* closed); // 0x0047fce0 (ret 0x1c)
    void EmitLeafEdgeFan(int x, int z, int dx, int dz, int n); // 0x0047fe70 (ret 0x14)
    int EmitEdgeTriangles(int level, int n, int x, int z, int dx, int dz, int* closed); // 0x00480200 (ret 0x1c)
    int AppendCornerTriangle(int n, int x, int z, int dx, int dz); // 0x00480700 (ret 0x14)
    int DrawVisible(void* target);                    // 0x00480900 (ret 4)
    int DrawVisibleWithDetail(void* target, int detailPass);            // 0x00480920 (ret 8)
    int DrawVisibleQuadrants(void* target, int x, int z, int size, int quad, int detailPass); // 0x00480940 (ret 0x18)
    int DrawVisibleBlock(void* target, int x, int z, int size, int quad, int detailPass); // 0x00480ad0 (ret 0x18)
    void SubmitUnlitRange(int detailPass, GridVertex* vertices, int vertexCount, unsigned short* indices,
                               int indexCount, int block);      // 0x00480c90 (ret 0x18)
    void SubmitLitRange(int detailPass, GridVertex* vertices, int vertexCount, unsigned short* indices,
                               int indexCount, int block);      // 0x00480fb0 (ret 0x18)
    int WalkSlot4FromRoot();                                // 0x00481170: WalkSlot4Quadrants(0, 0, 16, 0)
    int WalkSlot4Quadrants(int x, int z, int size, int quad); // 0x00481180 (ret 0x10): quadrant walk calling slot 4
    int WalkSlot4Block(int x, int z, int size, int quad); // 0x00481300 (ret 0x10)
    // 0x004813e0: counts seams where neighbouring children disagree (the
    // count is never used) and recurses into the children.
    void CountEdgeDifferences();
    void TestDroppableVertices();                               // 0x00481580: CanDropVertex over every leaf
    int TestBlockFrustum(int quad);                        // 0x00481a20 (ret 4)
    float BlockDistance(int block);                     // 0x00481b30 (ret 4)
    float ComputeViewerDetail(int coarse);                    // 0x004815e0 (ret 4): from the viewer's distance to the bounds
    int CanDropVertex(int x, int z);                    // 0x00481cc0 (ret 8)
    int TestVisibility(void* stream, void* a1);              // 0x00481db0 (ret 8)
    // 0x00481de0 (ret 0x28): tests the box against the clipper and recurses
    // into the four quadrant boxes (x, z, size; depth and quadrant path)
    // down to the child blocks, updating childMask; returns the visible count.
    int WalkVisibilityQuadrants(void* stream, void* a1, float* center, float* extent, int x, int z,
                                int size, int depth, int path, int coarse);
    void UpdateLevelOfDetail(UnknownTextureStream* stream, void* a1, int recurse,
                               int coarse);                     // 0x004824c0 (ret 0x10)
    void PropagateFullMask();                               // 0x004826d0
    void RetestChangedVertices();                               // 0x00482760
    void TestVertex(int x, int z);                   // 0x00482a40 (ret 8)
    void TestNodeVertices();                               // 0x00482ae0
    void TestRectangleLevel(int x0, int z0, int x1, int z1, int half, int start, int step,
                               int rowStep);                    // 0x00482b40 (ret 0x20)
    void PropagateVertexChange(int x, int z, int flag);         // 0x00482c90 (ret 0xc)
    void SetVertexDirection(int x, int z, int flag, int dir, GridBaseCell* origin); // 0x00482dd0 (ret 0x14)
    int RetestUnpinnedVertex(int x, int z);                    // 0x00482f00 (ret 8)
    int ForceBlockCentres();                                // 0x00483040
    int ReleaseBlockCentres();                                // 0x00483100
    void WalkBorderVertex(int x, int z, int flag, int dir, GridBaseCell* origin,
                          int minLevel);                   // 0x00483200 (ret 0x18)
    void PropagateBorderChange(DrawableGridNode* child, int x, int z, int flag, int dir,
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
int EvictNodeBuffers(void* owner, int context);

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
void FindEdgeNeighbours(int x, int z, int h, int v, int maskX, int maskZ, GridEdge* out);
// 0x0047e430: one subdivision level of the edge table.
void BuildEdgeLevel(int half, int start, int step, int rowStep);

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
        float ny;  // +0x10  normal y (data b2 set) or a second coordinate pair (0x0047de90)
        int color;                             // diffuse colour for unlit draws (0x00480c90)
    };
    union {
        float nz;  // +0x14  normal z (data b2 set) or a second coordinate pair (0x0047de90)
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
    float reliefScale;  // +0x2D44  the node's reliefScale, latched by the first vertex (0x0047de90)
    float texOriginV;  // +0x2D48  the draw data's texOriginV (0x0047de90)
    float texOriginU;  // +0x2D4C  the draw data's texOriginU (0x0047de90)
    float texStep;  // +0x2D50  the draw data's texStep (0x0047de90)
    int field_0x2d54;
    float scale;                               // +0x2d58
    float minY;                                // +0x2d5c
    float maxY;                                // +0x2d60
    int lowestQualityOverride;  // +0x2D64  the terrain's lowestQualityOverride; selects the detail coordinate pair (0x0047de90)
    float unlitColor;  // +0x2D68  the terrain's unlitColor bits, copied into unlit vertices (0x0047de90)
    float unlitSpecular;  // +0x2D6C  the terrain's unlitSpecular bits, copied into unlit vertices (0x0047de90)
    float detailTexScale;  // +0x2D70  g_gridDrawNormalScale (x16 at level 1): step of the detail coordinate pair
};
