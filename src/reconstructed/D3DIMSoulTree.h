#pragma once

// D3DIMSoulTree.h -- D:\aardvark\VC\krusty2\D3DIMSoulTree.CPP (literal
// __FILE__ at 0x00568994, xrefs 0x0043f2ee..0x00444f6b). Code
// 0x0043f160..0x0044523f: after cursor.cpp (ends 0x0043f15f) and before
// D3DIMSoultreeModifier.cpp (constructor 0x00445240). The unit's vector
// set sits mid-file at 0x00442e00..0x00442f3b (zero 0x0057ef08, x
// 0x0057ef18, y 0x0057ef28, z 0x0057eed8).
//
// Confirmed (RTTI): D3DIMSoultreeObject : SoultreeObject : QuadTreeObject
// (+0), GameObject (+0x0c), all non-virtual. COL 0x0055b2b8 / vtable
// 0x005513ec (object offset 0, 10 slots) and COL 0x0055b248 / vtable
// 0x0055137c (object offset 0x0c, 27 slots). The constructor 0x0043f160 and
// the destructor 0x0043f2b0 write both vptrs. SoultreeObject's own tables
// are 0x00557c18 (+0, 9 slots) and 0x00557c40 (+0x0c); D3DIMSoultreeObject
// overrides primary slots 2..8, adds slot 9 and overrides GameObject slots
// 0, 4, 5, 10, 12 and 14. operator new size 0x2d8 (0x004440c8).
//
// SoultreeObject (src/krusty2/core/SoultreeObject.h) and QuadTreeObject
// (src/krusty2/broadphase/QuadTreeObject.h) are the shared declarations;
// their code is soultree.cpp's (0x004fb2b0..) and Quadtree.cpp's. Member
// names are provisional (tier 3).
// LightEmitter.h includes this header, so LightEmitter.cpp,
// ArcadeObject.cpp, D3DIMSoultreeModifier.cpp, MorphBastardModifier.cpp and
// QuarryStuntEvent.cpp see the same class.

#include "GameObject.h"
struct TextureContext;
#include "MatrixUtil.h"
#include "SoultreeLodSettings.h"

struct Vec3;
class LightManager;
class RenderTarget;
class SoultreeMaterial;
class TextureMapManager;
class ParameterBlock;
class FileStream;

// RTTI SoultreeObject : QuadTreeObject (+0), GameObject (+0x0c): constructor
// 0x004fb2b0, code in soultree.cpp. One declaration for every unit.
#include "../krusty2/core/SoultreeObject.h"

// One 0x20-byte vertex in the D3DLVERTEX layout (drawn with FVF 0x1e2):
// position, reserved, colour, specular, texture coordinates.
struct SoultreeVertex {
    Vector3 position;
    int reserved;
    unsigned int diffuse;
    unsigned int specular;
    float tu;
    float tv;
};

// One 0x14-byte vertex group of a surface: the vertices one node moves.
// 0x00440d40 transforms vertexCount local vertices by the node's world matrix into vertices.
struct SoultreeVertexGroup {
    SoultreeObject* node;                      // node
    int vertexCount;                           // vertex count
    SoultreeVertex* vertices;                  // this group's range of the surface's drawn vertices
    SoultreeVertex* localVertices;             // the same range of its local (node-space) vertices
    Vector3* normals;                          // normals
};

// The texture coordinates of one vertex.
struct SoultreeUV {
    float u;
    float v;
};

// One 0x38-byte surface of a level of detail ("LOD %i - Surface %i").
// The destructor frees +0x1c, +0x04, +0x10, +0x14, +0x18, +0x28 and +0x24.
struct SoultreeSurface {
    int groupCount;                            // face group count
    SoultreeVertexGroup* groups;
    int vertexCount;                           // vertex count
    int faceCount;                             // face count
    SoultreeVertex* vertices;                  // drawn vertices (world space; DrawIndexedPrimitive)
    SoultreeVertex* localVertices;             // node-space vertices as loaded; 0x00440d40 transforms them into vertices
    Vector3* normals;                          // normals
    unsigned short* indices;                   // indices, three per face
    int materialCount;                         // material count
    int* materialIndices;                      // material indices
    SoultreeUV* uvs;                           // texture coordinates
    float mappingScale;                        // 1.0 after loading
    int mappingOffsetU;
    int mappingOffsetV;
};

// Two 16-byte blocks at +0x298 and +0x2a8, copied whole by slot 8 and
// cleared by 0x004444e0. The one at +0x298 is a screen rectangle: SelectLod
// (0x00443de0) passes it to the clipper's TestBox (0x0052f570) and
// DrawScreenRect (0x004439c0) outlines x in [+0x00, +0x08) and y in
// [+0x04, +0x0c) on the locked render surface.
struct SoultreeScreenRect {
    int left;                                  // +0x00
    int top;                                   // +0x04
    int right;                                 // +0x08
    int bottom;                                // +0x0c
};

// One 8-byte level of detail at +0x28c.
struct SoultreeLod {
    int surfaceCount;                          // surface count
    SoultreeSurface* surfaces;
};

// The camera fields 0x004439c0 reads (RenderTarget+0x08): the viewport
// origin (ObjectPicker.h's PickerCameraView names the same fields).
struct D3DIMSoultreeCameraView {
    unsigned char field_0x000[0xec];
    Matrix4 viewMatrix;                        // view matrix (slot 12's visibility test)
    unsigned char field_0x12c[0x170 - 0x12c];
    Vector3 position;                          // position
    Vector3 viewDirection;                     // view direction (drawing mode 5)
    Vector3 upDirection;                      // +0x188 Camera upDirection
    float roll;                               // +0x194 Camera roll
    float imagePlaneDistance;                  // image plane distance
    int displayAspect;
    unsigned int viewportX;                    // viewport x
    unsigned int viewportY;                    // viewport y
};


// 0x004a1b00 (cdecl): transforms `count` points from `source` by `matrix` into `target`;
// `sourceStride` 0 means `targetStride` (MatrixUtil.h's ProjectPoints takes the same arguments).
void TransformPoints(void* target, const void* source, const Matrix4* matrix, int count,
                     int targetStride, int sourceStride);

class D3DIMSoultreeObject : public SoultreeObject {
public:
    explicit D3DIMSoultreeObject(int flags);       // 0x0043f160 (ret 4)
    virtual ~D3DIMSoultreeObject();                // 0x0043f2b0 (deleting wrapper 0x0043f280)
    virtual void LoadHierarchy(FileStream* stream); // 0x0043f950
    virtual void LoadHierarchyFromParameters();                               // 0x0043fe40
    virtual void CreateNode(SoultreeObject** out);           // 0x004440a0
    virtual void UpdateLocalBounds();                        // 0x00444140
    virtual void ResetLocalMatrices();                               // 0x004450c0
    virtual void CloneFrom(SoultreeObject* source);         // 0x00444560
    virtual void CopySubtree(SoultreeObject* source, SoultreeObject* target); // 0x00444b60
    // Slot 9 (0x0043f4b0, ret 0x14): loads the model `name` (".slb" from
    // the resource manager, else ".slt") and returns the GameObject base;
    // callers hand it straight to GameObject 0x00469190. `owner` goes to
    // GameObject slot 8, `lights` is the LightManager kept at +0x1bc,
    // `options` points to the six-dword texture options copied to +0x248
    // when `useOptions` is set, and `useOptions` is also the load flag
    // handed to the materials. The int types are
    // kept from the first (LightEmitter.cpp, ArcadeObject.cpp) callers.
    virtual GameObject* LoadModel(void* owner, const char* name, int lights, int options,
                                  int useOptions);

    // GameObject slots (`this` is the GameObject subobject).
    virtual void Disable();                        // 0x00444540
    virtual void Enable();                         // 0x00444520
    virtual int Tick(float frameTime);                 // 0x00443490
    virtual int PrepareGeometry();                 // 0x00443aa0
    virtual int Render3D();            // 0x00444030

    void AddModifier(GameObject* modifier);        // 0x00444d80: appends to the first list (+0x264)
    // Seen from D3DIMSoultreeModifier.cpp: 0x00444de0 and 0x00444f10 (both
    // ret 4) remove a modifier from the first (+0x264) or second (+0x26c)
    // modifier list.
    void RemoveModifier(GameObject* modifier);
    void RemoveSecondModifier(GameObject* modifier);

    // 0x00440810, the drawing function 0x00440f30, slot 12 and 0x00443de0
    // are defined in D3DIMSoulTree.cpp (0x00440f30 and slot 12 not exact).
    void UpdateMaterialTexelRatios();                  // 0x00440810
    void TransformVertexGroups(int lod);           // 0x00440d40 (ret 4): transforms the vertex groups
    void DrawCurrentLod();                         // 0x00440f30: draws the current level of detail
    void SelectLod(Vector3* center, Vector3* extents);             // 0x00443de0 (ret 8): picks the level of detail
    void DrawScreenRect(SoultreeScreenRect* rect); // 0x004439c0 (ret 4): outlines a screen rectangle
    void ComputeSubtreeBounds();                   // 0x004434b0: bounding boxes of the subtree
    void DrawAxisGizmo();                  // 0x004435b0: axes of the subtree
    void DrawBoxOutline(Vector3 center, Vector3 extents, int flag); // 0x00443740 (ret 0x1c)
    void DrawWireframe(SoultreeSurface* surface);                 // 0x00442f70 (ret 4): wireframe
    void DrawNormals(SoultreeSurface* surface);                   // 0x00442fe0 (ret 4): normals
    void DrawVertexCrosses(SoultreeSurface* surface);             // 0x004431f0 (ret 4): vertex crosses
    void GetPart(int surface, SoultreeVertex** vertices, int* vertexCount,
                               unsigned short** indices, int* indexCount, int unused, int lod); // 0x004433f0
    int ReadLods();                                // 0x00440060: reads the levels of detail
    void ComputeAllBounds();                       // 0x00444440: slot 5 (local bounds) on every node, then subtree bounds
    void SetUseLighting(int value);         // 0x004444c0: sets useLighting and the dirty flag
    void ClearScreenRects();                  // 0x004444e0: zeroes screenRect and +0x2a8, then SetFlag14c
    void CopyMaterials(D3DIMSoultreeObject* source); // 0x00444a40: new materials copied from source
    int ReplaceMaterialTexture(int index, const char* name, TextureContext* textures); // 0x00444c70 (textures unused)
    void SetLod(int lod);           // 0x00444d00
    void SetLowestLod(int lod);           // 0x00444d40
    void SetLodThreshold(float value);             // 0x00444d60
    void RemoveAllModifiers();                     // 0x00444e80
    void AddSecondModifier(GameObject* modifier); // 0x00444eb0
    void RemoveAllSecondModifiers();               // 0x00444fb0
    int GetVertexCount();                   // 0x00444fe0
    int GetSurfaceCount(int lod);         // 0x00445030: surface count
    int NodeMovesVertices(SoultreeObject* node, int lod);     // 0x00445060: node moves vertices
    void ApplyLodSettings(int index);         // 0x004451e0

    int useLighting;                           // +0x1a4 set by 0x004444c0; gates the light manager's vertex lighting
    int showNormals;                           // +0x1a8 DrawNormals per surface (debug)
    int showVertexCrosses;                     // +0x1ac DrawVertexCrosses per surface (debug)
    int showWireframe;                         // +0x1b0 DrawWireframe per surface (debug)
    int texturePerspective;                    // +0x1b4 D3DRENDERSTATE_TEXTUREPERSPECTIVE while drawing
    int gouraudShading;                        // +0x1b8 D3DRENDERSTATE_SHADEMODE: gouraud when set, else flat
    LightManager* lightManager;                // +0x1bc slot 9's `a`; lights the vertices
    char modelName[0x80];                      // +0x1c0 name (0x7f characters)
    TextureMapManager* textureManager;         // +0x240 texture manager (first option dword)
    int textureFormat;                         // +0x244 texture format, 555
    int textureOptions[6];                     // +0x248 texture options
    int field_0x260;                           // +0x260
    int modifierCount;                         // +0x264 first modifier list
    GameObject** modifierList;                 // +0x268
    int secondModifierCount;                   // +0x26c second modifier list
    GameObject** secondModifierList;           // +0x270
    int lodCount;                              // +0x274 level-of-detail count
    int lowestLod;                             // +0x278 lowest level of detail
    int currentLod;                            // +0x27c current level of detail
    float* autoLodDistances;                   // +0x280 "AutoLOD#%i" distances
    float inverseLodThreshold;                 // +0x284 1 / SetLodThreshold's value; SelectLod takes level i + 1 while scale * autoLodDistances[i] < threshold
    int useAutoLod;                            // +0x288 "UseAutoLOD"
    SoultreeLod* lodTable;                     // +0x28c
    SoultreeMaterial** materialTable;          // +0x290 materials
    int materialCount;                         // +0x294 material count
    SoultreeScreenRect screenRect;      // +0x298 projected bounds (SelectLod); slot 14 bumps right and bottom
    SoultreeScreenRect field_0x2a8;     // +0x2a8
    float lastFrameTime;                       // +0x2b8 frame time (slot 10)
    Vector3 lastScrollPosition;                // +0x2bc world position at the last mode 2 texture scroll
    float textureScroll;                       // +0x2c8 texture scroll (drawing modes 1 and 2)
    int loadFlag;                              // +0x2cc load flag (slot 9)
    int showBounds;                            // +0x2d0 slot 14 outlines screenRect and draws bounds and axes (debug)
    int visible;                               // +0x2d4 1 after construction; SelectLod sets it from the clipper test, slot 14 skips drawing when 0
};
