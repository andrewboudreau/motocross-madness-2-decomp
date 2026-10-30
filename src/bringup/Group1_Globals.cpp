// Group1_Globals.cpp -- bring-up storage for data globals that reconstructed code
// declares extern but no translation unit defines yet.  None of these is a byte-match
// claim; they exist so the source-built game links.  Retail addresses are given for
// each; all of them lie in the zero-filled .bss tail of retail's .data section unless
// noted, so zero initialization is faithful.
//
// Not provided here: ?g_CollisionScratchPoints@@3PAUCollisionVec3@@A (0x00579068).
// BoundingBoxTreeQuery.cpp already defines the same retail array as Vec3[128]; the
// CollisionVec3 spelling used by CollisionObject.cpp is bound to it by the
// `link_aliases` weak external in config/game_build.json, so both share one storage.

#include "visibility/VisibilityClipper.h"
#include "ManagedTexture.h"
#include "effects/ParticleManager.h"

// 0x00575a98 (initialized .data): retail stores 0x0068acd4, the address of a .bss object
// that no method reads fields from (see docs/CLIPPER_GLOBAL_CONNECTIONS.md).
// best guess: an empty file-static VisibilityClipper (the "second empty static" whose
// CRT initializer 0x0052f080 is a bare return) stands in for the object at 0x0068acd4.
static VisibilityClipper s_visibilityClipperObject;
VisibilityClipper* g_visibilityClipper = &s_visibilityClipperObject;

// Grid texture use table (Grid1.cpp RecordTextureUse).  Retail places the sizes at
// 0x0065b65c and the textures at 0x0066047c: 0x4e20 bytes apart.
// best guess: 5000 entries each, the largest capacity the retail layout allows for the
// sizes table; retail never bounds-checks the count.
int g_gridTextureSizes[5000];          // 0x0065b65c
ManagedTexture* g_gridTextures[5000];  // 0x0066047c
int g_gridTextureCount;                // 0x0068a2ec

// 0x0068a084: Griddraw's per-frame box test counter (debug statistic).
int g_gridBoxTests;

// 0x00689158: the 61 sprite-cell texture rectangles ParticleManager slot 27 fills.
ParticleUVRect g_particleCellUVs[61];

// VisibilityQuadTree.cpp's two views of the debug box strip.  Retail has ONE 16-entry
// 0x20-byte vertex array at 0x0068a768 (x, y, z, then reserved/colour/specular/uv); the
// entry view at 0x0068a774 is the same storage offset by 0xc, so the constructor's
// colour reset (0x2000ffff) lands in the vertices DrawBox emits.
// best guess: two separate arrays (C++ cannot alias them); DrawBox's debug lines then use
// zero colours.  The layouts repeat VisibilityQuadTree.cpp's local declarations.
struct VisibilityGlobalEntry {
    int field_0x00;
    int field_0x04;
    int field_0x08;
    char field_0x0c[0x14];
};
struct VisibilityBoxVertex {
    float x, y, z;
    char field_0x0c[0x14];
};
VisibilityBoxVertex g_visibilityBoxVertices[16];   // 0x0068a768
VisibilityGlobalEntry g_visibilityEntries[16];     // 0x0068a774 (overlaps the above in retail)
