#pragma once

// MorphBastardModifier.h -- the reconstructed D:\aardvark\VC\krusty2\
// MorphBastardModifier.cpp (literal __FILE__ at 0x0056df3c, xrefs
// 0x004a3215..0x004a5447; code 0x004a3150..0x004a562b, after the debug
// allocator's 0x004a3120 helper and before Motnctrl.cpp's first dynamic
// initializer 0x004a5630).
//
// Confirmed (tier 1): RTTI .?AVMorphBastardModifier@@ (COL 0x0055d788),
// MorphBastardModifier : D3DIMSoultreeModifier : GraphicsTest : GameObject :
// BaseObject, all single non-virtual at mdisp 0; vtable 0x00555230 (28
// slots). It overrides slot 0 (deleting destructor 0x004a3180) and slot 27
// (0x004a4c60). The constructor 0x004a3150 (called from KrustyBike.cpp
// 0x004910de after operator new(0x58)) and the destructor 0x004a31a0 write
// the vptr. Member and method names are provisional (tier 3).
//
// What the code does (tier 3): 0x004a33b0 reads a parameter file
// ("RiderMorph.mbf") of "MorphBastard %i" sections. Each morph object names a
// node ("ObjectName") and targets ("MorphBastard %i Target %i") driven by a
// controller node's rotation about one axis ("XAxis"/"YAxis"/"ZAxis" give
// the target value); each target carries per-vertex deltas
// ("... Deltas" rows: vertex index and x/y/z). Slot 27 copies the mesh it
// is given once (0x004a5290) and blends the deltas into the copy.

#include "D3DIMSoultreeModifier.h"
#include "MatrixUtil.h"

// Transposes the rotation part of `m`.
inline void MorphBastardTransposeRotation(Matrix4& m)
{
    float t;
    t = m.m[0][1];
    m.m[0][1] = m.m[1][0];
    m.m[1][0] = t;
    t = m.m[0][2];
    m.m[0][2] = m.m[2][0];
    m.m[2][0] = t;
    t = m.m[1][2];
    m.m[1][2] = m.m[2][1];
    m.m[2][1] = t;
}

// Inverts a rotation + translation matrix in place.
inline void MorphBastardInvertRigid(Matrix4& m)
{
    MorphBastardTransposeRotation(m);
    Vector3 t(-(m.m[3][0] * m.m[0][0] + m.m[3][1] * m.m[1][0] + m.m[3][2] * m.m[2][0]),
              -(m.m[3][0] * m.m[0][1] + m.m[3][1] * m.m[1][1] + m.m[3][2] * m.m[2][1]),
              -(m.m[3][0] * m.m[0][2] + m.m[3][1] * m.m[1][2] + m.m[3][2] * m.m[2][2]));
    m.m[3][0] = t.x;
    m.m[3][1] = t.y;
    m.m[3][2] = t.z;
}

// The scene node calls used here (the model, the morph objects and the
// controllers). Tier 2 from the call sites; src/krusty2/core/SoultreeObject.h
// has the canonical view.
class MorphBastardNode {
public:
    void UnknownFunction4fca60(Matrix4* out);                    // 0x004fca60: local matrix
    void UnknownFunction4fca80(MorphBastardNode* frame, Matrix4* out); // 0x004fca80: matrix in frame
    MorphBastardNode* UnknownFunction4fdae0(const char* name);   // 0x004fdae0: named descendant
};

// One delta set (0x14 bytes, sorted by value with qsort 0x4a3bb0).
struct MorphBastardTarget {
    float field_0x00;                 // controller value of the target
    int field_0x04;                   // delta count
    int* field_0x08;                  // vertex indices (line 0xd9)
    Vector3* field_0x0c;              // deltas (line 0xda)
    float* field_0x10;                // last weight per delta (line 0xdb)
};

// One controller channel (0x6c bytes): a controller node and axis that
// drives a list of targets of its morph object.
struct MorphBastardChannel {
    int field_0x00;                   // axis 0..2
    Matrix4 field_0x04;               // inverse rest matrix of the controller
    MorphBastardNode* field_0x44;     // controller
    int* field_0x48;                  // target numbers; [0] is -1
    int field_0x4c;                   // their count
    MorphBastardTarget* field_0x50;   // one per target number (line 0xb2)
    float field_0x54;                 // smallest target value
    float field_0x58;                 // largest target value
    int field_0x5c;                   // -1
    float field_0x60;                 // current value per axis (0x004a3c80)
    float field_0x64;
    float field_0x68;
};

// One morph object (0x4c bytes). The empty constructor (from Matrix4's)
// leaves only the count-down of the inlined construction loop at 0x004a34d5.
struct MorphBastardObject {
    MorphBastardObject() {}
    int field_0x00;                   // channel count
    MorphBastardChannel* field_0x04;  // channels (realloc'd, lines 0x8f/0x98)
    MorphBastardNode* field_0x08;     // the morphed node
    Matrix4 field_0x0c;               // its inverse rest matrix
};

// 32-byte vertex (position, normal, texture coordinates) with an empty
// constructor, like D3DVERTEX under D3D_OVERLOADS.
struct MorphBastardVertex {
    MorphBastardVertex() {}
    float x, y, z;
    float nx, ny, nz;
    float tu, tv;
};

struct MorphBastardFace {
    unsigned short field_0x00[3];
};

// A vertex group of a mesh (0x14 bytes); the pointers index the mesh arrays.
struct MorphBastardMeshGroup {
    MorphBastardNode* field_0x00;     // the node the group belongs to
    int field_0x04;                   // vertex count
    MorphBastardVertex* field_0x08;
    MorphBastardVertex* field_0x0c;
    Vector3* field_0x10;
};

struct MorphBastardMeshPair {
    int field_0x00;
    int field_0x04;
};

// The mesh slot 27 receives and returns (0x38 bytes).
struct UnknownSoultreeMesh {
    int field_0x00;                   // group count
    MorphBastardMeshGroup* field_0x04;
    int field_0x08;                   // vertex count
    int field_0x0c;                   // face count
    MorphBastardVertex* field_0x10;
    MorphBastardVertex* field_0x14;
    Vector3* field_0x18;
    MorphBastardFace* field_0x1c;
    MorphBastardMeshPair field_0x20;  // copied as one 8-byte value
    int* field_0x28;
    int field_0x2c;
    int field_0x30;
    int field_0x34;
};

// The D3DIMSoultreeObject passed to slot 27, seen from this file: only its
// current level of detail at +0x27c (src/krusty2/bvh/BoundingBoxTreeBuild.h).
struct MorphBastardSoultreeView {
    unsigned char field_0x000[0x27c];
    int field_0x27c;                  // nonzero: draw the mesh unchanged
};

class MorphBastardModifier : public D3DIMSoultreeModifier {
public:
    explicit MorphBastardModifier(int flags);        // 0x004a3150 (ret 4)
    virtual ~MorphBastardModifier();                 // 0x004a31a0 (deleting wrapper 0x004a3180)
    // 0x004a4c60 (ret 0xc)
    virtual void UnknownVirtualSlot27(D3DIMSoultreeObject* object, UnknownSoultreeMesh* mesh,
                                      UnknownSoultreeMesh** out);

    // 0x004a33b0 (ret 0xc): slot 8 of the base, then loads `path` with the
    // nodes of `model`.
    MorphBastardModifier* UnknownFunction4a33b0(void* value, const char* path, MorphBastardNode* model);
    // 0x004a3c80 (ret 0x44)
    void UnknownFunction4a3c80(MorphBastardChannel* channel, Matrix4 inverse);
    // 0x004a4bb0 (ret 4): updates every channel of `object`.
    void UnknownFunction4a4bb0(MorphBastardObject* object);
    // 0x00464e80 (ret 4): the shared empty stub, called with the object index.
    void UnknownFunction464e80(int index);
    // 0x004a5290 (ret 4): makes field_0x40 a copy of `mesh`.
    void UnknownFunction4a5290(UnknownSoultreeMesh* mesh);

    UnknownSoultreeMesh* field_0x40;
    int field_0x44;                   // morph object count
    MorphBastardObject* field_0x48;   // line 0x57
    Vector3* field_0x4c;              // summed deltas per vertex (lines 0x21d..0x21f)
    Vector3* field_0x50;              // smallest delta per vertex
    Vector3* field_0x54;              // largest delta per vertex
};

// 0x004a3bb0: qsort order of MorphBastardTarget by value.
int UnknownFunction4a3bb0(const void* a, const void* b);
// 0x004a3be0: the angle between two unit vectors (0 at 1, pi/2 at 0, else acos).
float UnknownFunction4a3be0(Vector3 a, Vector3 b);
