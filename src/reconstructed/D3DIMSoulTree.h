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
// SoultreeObject and QuadTreeObject are declared here only as far as this
// file needs them; their code is soultree.cpp's (0x004fb2b0..) and
// QuadTree's. src/krusty2/core/SoultreeObject.h is the physics code's flat
// view of the same class. Member names are provisional (tier 3).
// LightEmitter.h includes this header, so LightEmitter.cpp,
// ArcadeObject.cpp, D3DIMSoultreeModifier.cpp, MorphBastardModifier.cpp and
// QuarryStuntEvent.cpp see the same class.

#include "GameObject.h"
#include "MatrixUtil.h"

class LightManager;
class RenderTarget;
class SoultreeMaterial;
class TextureMapManager;
class UnknownParameterBlock;
class UnknownTextureStream;

// RTTI QuadTreeObject (vtable 0x005511c4: 0x004dc610 `ret`, 0x00434ce0
// returns 15).
class UnknownSoultreeQuadTreeObject {
public:
    virtual void QuadTreeVirtualSlot0();
    virtual int QuadTreeVirtualSlot1();

    short field_0x04;
    short field_0x06;
    char field_0x08;
};

// RTTI SoultreeObject: constructor 0x004fb2b0 (ret 4), destructor core
// 0x004fb430 (`this` is the GameObject subobject). Slot argument counts
// come from each body's `ret N`.
class SoultreeObject : public UnknownSoultreeQuadTreeObject, public GameObject {
public:
    explicit SoultreeObject(int flags);
    virtual ~SoultreeObject();
    virtual void SoultreeVirtualSlot2(UnknownTextureStream* stream);   // 0x004fdc00 (ret 4): reads a saved node
    virtual void SoultreeVirtualSlot3();                                  // 0x004fddc0: reads the .slt keys
    virtual void SoultreeVirtualSlot4(SoultreeObject** out);              // 0x004fe020 (ret 4): clone
    virtual void SoultreeVirtualSlot5();                                  // 0x00464e90 (shared `ret`)
    virtual void SoultreeVirtualSlot6();                                  // 0x004fec70
    virtual void SoultreeVirtualSlot7(SoultreeObject* source);            // 0x004feb10 (ret 4): copies `source`
    virtual void SoultreeVirtualSlot8(SoultreeObject* source, SoultreeObject* target); // 0x004feb30 (ret 8)

    // soultree.cpp helpers (thiscall).
    int UnknownFunction4fda30();                                   // 0x004fda30: node count of the subtree
    void UnknownFunction4fda60(int* index, SoultreeObject** nodes); // 0x004fda60: collects the subtree
    void UnknownFunction4fdb40();                                  // 0x004fdb40
    void UnknownFunction4fdb50();                                  // 0x004fdb50
    void UnknownFunction4fdb60(UnknownTextureStream* stream, int a); // 0x004fdb60: reads a .slt stream
    void UnknownFunction4fe0a0(Vector3* a, Vector3* b);            // 0x004fe0a0
    void UnknownFunction4fe0f0();                                  // 0x004fe0f0
    void UnknownFunction4fb4f0();                                  // 0x004fb4f0: world matrix
    Vector3 UnknownFunction4fd660(const Vector3& p);               // 0x004fd660: local -> world point
    void UnknownFunction4fedb0();                                  // 0x004fedb0: registers the node
    // Seen from Krusty3DObjects.cpp (src/krusty2/core/SoultreeObject.h
    // describes the same functions): 0x004fc690 (ret 0x10) moves the node
    // by `delta` in `frame`, 0x004fd090 (ret 0x10) sets the rotation about
    // the axis (x, y, z), 0x004fd910 (ret 4) appends `child`, 0x004fd990
    // (ret 4) unlinks `child`.
    void UnknownFunction4fc690(SoultreeObject* frame, Vector3 delta);
    void UnknownFunction4fd090(float x, float y, float z, float angle);
    void UnknownFunction4fd910(SoultreeObject* child);
    void UnknownFunction4fd990(SoultreeObject* child);

    char field_0x038[0x80];                    // name
    Matrix4 field_0x0b8;                       // local matrix
    Matrix4 field_0x0f8;                       // world matrix
    int field_0x138;                           // world matrix valid
    SoultreeObject* field_0x13c;               // parent
    SoultreeObject* field_0x140;               // first child
    SoultreeObject* field_0x144;               // next sibling
    unsigned char field_0x148[0x14c - 0x148];
    int field_0x14c;                           // Krusty3DObjects.cpp: hidden
    int field_0x150;                           // bounds found (slot 5)
    int field_0x154;                           // bounds valid
    Vector3 field_0x158;                       // bounds centre
    Vector3 field_0x164;                       // bounds half extent
    int field_0x170;                           // Krusty3DObjects.cpp 0x0048c8a0 sets 1
    Vector3 field_0x174;                       // Krusty3DObjects.cpp 0x0048c8a0
    Vector3 field_0x180;                       // Krusty3DObjects.cpp 0x0048c8a0
    int field_0x18c;                           // subtree dirty
    unsigned char field_0x190[0x1a0 - 0x190];
    UnknownParameterBlock* field_0x1a0;        // .slt reader
};

// One 0x14-byte vertex group of a surface: the vertices one node moves.
// 0x00440d40 transforms field_0x04 vertices by the node's world matrix.
struct UnknownSoultreeFaceGroup {
    SoultreeObject* field_0x00;                // node
    int field_0x04;                            // vertex count
    void* field_0x08;                          // source vertices
    void* field_0x0c;                          // transformed vertices
    Vector3* field_0x10;                       // normals
};

// One 0x20-byte vertex in the D3DLVERTEX layout (drawn with FVF 0x1e2):
// position, reserved, colour, specular, texture coordinates.
struct UnknownSoultreeVertex {
    Vector3 field_0x00;
    int field_0x0c;
    unsigned int field_0x10;
    unsigned int field_0x14;
    float field_0x18;
    float field_0x1c;
};

// The texture coordinates of one vertex.
struct UnknownSoultreeUV {
    float field_0x00;
    float field_0x04;
};

// One 0x38-byte surface of a level of detail ("LOD %i - Surface %i").
// The destructor frees +0x1c, +0x04, +0x10, +0x14, +0x18, +0x28 and +0x24.
struct UnknownSoultreeSurface {
    int field_0x00;                            // face group count
    UnknownSoultreeFaceGroup* field_0x04;
    int field_0x08;                            // vertex count
    int field_0x0c;                            // face count
    UnknownSoultreeVertex* field_0x10;         // vertices
    UnknownSoultreeVertex* field_0x14;         // drawn vertices
    Vector3* field_0x18;                       // normals
    unsigned short* field_0x1c;                // indices, three per face
    int field_0x20;                            // material count
    int* field_0x24;                           // material indices
    UnknownSoultreeUV* field_0x28;             // texture coordinates
    float field_0x2c;                          // 1.0 after loading
    int field_0x30;
    int field_0x34;
};

// Two 16-byte blocks at +0x298 and +0x2a8, copied whole by slot 8 and
// cleared by 0x004444e0.
struct UnknownSoultreeCounters {
    int field_0x00;
    int field_0x04;
    int field_0x08;
    int field_0x0c;
};

// One 8-byte level of detail at +0x28c.
struct UnknownSoultreeLod {
    int field_0x00;                            // surface count
    UnknownSoultreeSurface* field_0x04;
};

// The camera fields 0x004439c0 reads (RenderTarget+0x08): the viewport
// origin (ObjectPicker.h's UnknownPickCamera names the same fields).
struct UnknownSoultreeCameraView {
    unsigned char field_0x000[0x170];
    Vector3 field_0x170;                       // position
    unsigned char field_0x17c[0x198 - 0x17c];
    float field_0x198;                         // image plane distance
    int field_0x19c;
    unsigned int field_0x1a0;                  // viewport x
    unsigned int field_0x1a4;                  // viewport y
};

// The 16-byte rows of the global table at 0x00689f18 (0x004451e0).
struct UnknownSoultreeLodSetting {
    float field_0x00;
    int field_0x04;
    int field_0x08;
    int field_0x0c;
};
extern UnknownSoultreeLodSetting* g_UnknownSoultreeLodSettings689f18;

// 0x004a1b00 (cdecl): transforms `count` points by `matrix`.
void UnknownFunction4a1b00(const void* source, void* target, const Matrix4* matrix, int count,
                           int sourceStride, int targetStride);

class D3DIMSoultreeObject : public SoultreeObject {
public:
    explicit D3DIMSoultreeObject(int flags);       // 0x0043f160 (ret 4)
    virtual ~D3DIMSoultreeObject();                // 0x0043f2b0 (deleting wrapper 0x0043f280)
    virtual void SoultreeVirtualSlot2(UnknownTextureStream* stream); // 0x0043f950
    virtual void SoultreeVirtualSlot3();                               // 0x0043fe40
    virtual void SoultreeVirtualSlot4(SoultreeObject** out);           // 0x004440a0
    virtual void SoultreeVirtualSlot5();                               // 0x00444140
    virtual void SoultreeVirtualSlot6();                               // 0x004450c0
    virtual void SoultreeVirtualSlot7(SoultreeObject* source);         // 0x00444560
    virtual void SoultreeVirtualSlot8(SoultreeObject* source, SoultreeObject* target); // 0x00444b60
    // Slot 9 (0x0043f4b0, ret 0x14): loads the model `name` (".slb" from
    // the resource manager, else ".slt") and returns the GameObject base;
    // callers hand it straight to GameObject 0x00469190. `owner` goes to
    // GameObject slot 8, `a` is the LightManager kept at +0x1bc, `b` points
    // to the six-dword texture options copied to +0x248 when `c` is set,
    // and `c` is the load flag handed to the materials. The int types are
    // kept from the first (LightEmitter.cpp, ArcadeObject.cpp) callers.
    virtual GameObject* UnknownVirtualSlot9(void* owner, const char* name, int a, int b, int c);

    // GameObject slots (`this` is the GameObject subobject).
    virtual void UnknownVirtualSlot4();            // 0x00444540
    virtual void UnknownVirtualSlot5();            // 0x00444520
    virtual int UnknownVirtualSlot10(float frameTime); // 0x00443490
    virtual int UnknownVirtualSlot12();            // 0x00443aa0
    virtual int UnknownVirtualSlot14();            // 0x00444030

    void UnknownFunction4fc630(Vector3 position);  // 0x004fc630: sets the local translation
    void UnknownFunction4fd340(float x, float y, float z); // 0x004fd340
    // Seen from ArcadeObject.cpp: 0x004fc660 (ret 4) sets the position from
    // a pointer, 0x004fe850 (ret 8) returns two vectors (the second is the
    // half extent ArcadeObject doubles), 0x004fbd70 (ret 0x10) takes four
    // pass-through arguments, 0x004fceb0 (ret 0x10) an axis and an angle,
    // 0x00444d80 (ret 4) attaches a modifier.
    void UnknownFunction4fc660(const Vector3* position);
    void UnknownFunction4fe850(Vector3* a, Vector3* b);
    void UnknownFunction4fbd70(const Vector3* a, const Vector3* b, int c, int d); // two axes by pointer
    void UnknownFunction4fceb0(Vector3 axis, float angle);
    void UnknownFunction444d80(GameObject* modifier);
    // Seen from D3DIMSoultreeModifier.cpp: 0x00444de0 and 0x00444f10 (both
    // ret 4) remove a modifier from the first (+0x264) or second (+0x26c)
    // modifier list.
    void UnknownFunction444de0(GameObject* modifier);
    void UnknownFunction444f10(GameObject* modifier);

    // Not reconstructed: 0x00440810, the drawing function 0x00440f30, slot
    // 12 and 0x00443de0 (see samples/render/D3DIMSoulTreeNearMisses.cpp).
    void UnknownFunction440810();                  // 0x00440810
    void UnknownFunction440d40(int lod);           // 0x00440d40 (ret 4): transforms the vertex groups
    void UnknownFunction440f30();                  // 0x00440f30: draws the current level of detail
    void UnknownFunction443de0(Vector3* center, Vector3* extents); // 0x00443de0 (ret 8): picks the level of detail
    void UnknownFunction4439c0(UnknownSoultreeCounters* counters); // 0x004439c0 (ret 4)
    void UnknownFunction4434b0();                  // 0x004434b0: bounding boxes of the subtree
    void UnknownFunction4435b0();                  // 0x004435b0: axes of the subtree
    void UnknownFunction443740(Vector3 center, Vector3 extents, int flag); // 0x00443740 (ret 0x1c)
    void UnknownFunction442f70(UnknownSoultreeSurface* surface);  // 0x00442f70 (ret 4): wireframe
    void UnknownFunction4433f0(int surface, UnknownSoultreeVertex** vertices, int* vertexCount,
                               unsigned short** indices, int* indexCount, int a6, int lod); // 0x004433f0
    int UnknownFunction440060();                   // 0x00440060: reads the levels of detail
    void UnknownFunction444440();                  // 0x00444440
    void UnknownFunction4444c0(int value);         // 0x004444c0
    void UnknownFunction4444e0();                  // 0x004444e0
    void UnknownFunction444a40(D3DIMSoultreeObject* source); // 0x00444a40
    int UnknownFunction444c70(int index, const char* name, int a); // 0x00444c70
    void UnknownFunction444d00(int lod);           // 0x00444d00
    void UnknownFunction444d40(int lod);           // 0x00444d40
    void UnknownFunction444d60(float value);       // 0x00444d60
    void UnknownFunction444e80();                  // 0x00444e80
    void UnknownFunction444eb0(GameObject* modifier); // 0x00444eb0
    void UnknownFunction444fb0();                  // 0x00444fb0
    int UnknownFunction444fe0();                   // 0x00444fe0
    int UnknownFunction445030(int lod);   // 0x00445030: surface count
    int UnknownFunction445060(SoultreeObject* node, int lod); // 0x00445060: node moves vertices
    void UnknownFunction4451e0(int index);         // 0x004451e0

    int field_0x1a4;
    int field_0x1a8;
    int field_0x1ac;
    int field_0x1b0;
    int field_0x1b4;
    int field_0x1b8;
    LightManager* field_0x1bc;                 // slot 9's `a`; lights the vertices
    char field_0x1c0[0x80];                    // name (0x7f characters)
    TextureMapManager* field_0x240;            // texture manager (first option dword)
    int field_0x244;                           // texture format, 0x22b
    int field_0x248[6];                        // texture options
    int field_0x260;
    int field_0x264;                           // first modifier list
    GameObject** field_0x268;
    int field_0x26c;                           // second modifier list
    GameObject** field_0x270;
    int field_0x274;                           // level-of-detail count
    int field_0x278;                           // lowest level of detail
    int field_0x27c;                           // current level of detail
    float* field_0x280;                        // "AutoLOD#%i" distances
    float field_0x284;
    int field_0x288;
    UnknownSoultreeLod* field_0x28c;
    SoultreeMaterial** field_0x290;            // materials
    int field_0x294;                           // material count
    UnknownSoultreeCounters field_0x298;       // slot 14 counts frames in +0x08 / +0x0c
    UnknownSoultreeCounters field_0x2a8;
    float field_0x2b8;                         // frame time (slot 10)
    Vector3 field_0x2bc;
    int field_0x2c8;
    int field_0x2cc;                           // load flag (slot 9)
    int field_0x2d0;
    int field_0x2d4;                           // 1 after construction
};
