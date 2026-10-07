// SoultreeObject.h -- scene-graph node (soultree.cpp) as seen by the physics code.
//
// Shared declaration included by math/Math3D.h.
//
// Class identity: RTTI .?AVSoultreeObject@@ (tier 1), bases QuadTreeObject (+0)
// and GameObject (+12). The constructor at 0x004fb2b0 writes both vptrs
// (0x00557c18 at +0, 0x00557c40 at +12) and initializes the transform fields
// below (tier 1 for offsets). The non-virtual helpers are attributed to this class
// because they all begin with UpdateWorldMatrix (0x004fb4f0), which reads the
// +0x138 / +0x13c fields that constructor initializes, and they sit next to the
// soultree.cpp __FILE__ references (tier 2). Their bodies are in
// soultree/soultree.cpp (the retail soultree.cpp, 0x004fb2b0..0x004fefd8).
//
// Two views of one class:
//  * Physics code sees a FLAT view: no virtual functions, the two vptrs and the
//    base-class bodies are padding, so member offsets are right without pulling
//    GameObject/QuadTreeObject into every Math3D.h user. Do not derive from it.
//  * soultree.cpp, the unit that defines the class, defines
//    SOULTREE_OBJECT_WITH_BASES before including Math3D.h. It then sees the real
//    bases (QuadTreeObject + GameObject, from collision/CollisionObject.h) and the
//    virtual functions, so the constructor, destructor and vtables are emitted as
//    in retail. Member offsets are identical in both views.
//
// It is the single declaration of the node for every physics area (the former
// stand-ins helpers/SoultreeNode.h and the SoultreeNode class of
// soultree_base/SoultreePhysicsCallees.h are merged here).
#ifndef MCM2_PHYSICS_COMMON_SOULTREEOBJECT_H
#define MCM2_PHYSICS_COMMON_SOULTREEOBJECT_H

class UnknownParameterBlock;
class UnknownParameterStream;

#ifdef SOULTREE_OBJECT_WITH_BASES
#include "collision/CollisionObject.h"   // QuadTreeObject, GameObject

// 0x00461640 (thiscall, ret 0xc): Read(dst, elementSize, count). The stream class is
// named CollisionFileStream / UnknownTextureStream elsewhere; tier 3 stand-in here.
class SoultreeFileStream {
public:
    int Read(void* dst, int size, int count);
};
// src/reconstructed/Parameterblocks.h declares the full class; only the members
// soultree.cpp calls are listed (same mangled names).
class UnknownParameterBlock {
public:
    UnknownParameterBlock();                                                     // 0x004b7130
    ~UnknownParameterBlock();                                                    // 0x004b7190
    void UnknownFunction4b77a0(UnknownParameterStream* stream, int offset, int index); // 0x004b77a0
    int UnknownFunction4b7f70(const char* name);                                 // 0x004b7f70
    int UnknownFunction4b8010(char* raw);                                        // 0x004b8010
    int UnknownFunction4b81c0(int index, float* out);                            // 0x004b81c0
    int UnknownFunction4b8200(int index, char* out);                             // 0x004b8200
    char field_0x00[0x5c4];        // operator new(0x5c4) at 0x004fdb83 (tier 1)
};
#endif

#ifdef SOULTREE_OBJECT_WITH_BASES
class SoultreeObject : public QuadTreeObject, public GameObject {
#else
class SoultreeObject {
#endif
public:
    // 0x004fb2b0, thiscall, ret 4 (tier 1). SoultreePhysicsBaseObject slot 2 passes 1.
    // The argument's meaning is unknown.
    explicit SoultreeObject(int a);

    // ---- transform helpers (all __thiscall, callee pops) --------------------

    // 0x004fb4f0. Recomputes worldMatrix = localMatrix * parent->worldMatrix
    // (recursing up the parent chain first) unless worldValid is set.
    void UpdateWorldMatrix();

    // 0x004fb850. Clears worldValid on this node and invalidates the children.
    void InvalidateWorldMatrix();

    // 0x004fd5c0. Local direction -> world: returns v * R(world) (3x3, no translation).
    Vec3 LocalToWorldDirection(const Vec3& v);

    // 0x004fd660. Local point -> world: returns p * worldMatrix (with translation).
    Vec3 LocalToWorldPoint(const Vec3& p);

    // 0x004fd710. World direction -> local: returns v * transpose(R(world)), i.e.
    // the inverse rotation for an orthonormal world matrix. Copies worldMatrix to a
    // local and transposes it inline before the multiply.
    Vec3 WorldToLocalDirection(const Vec3& v);

    // 0x004fd7f0. World point -> local: returns p * inverse(worldMatrix) where the
    // inverse is the rigid-transform inverse (transposed rotation, translation
    // -(t * R^T)).
    Vec3 WorldToLocalPoint(const Vec3& p);

    // 0x004fc630 / 0x004fc660. Write the local translation (localMatrix row 3,
    // +0xe8..+0xf0) and invalidate the world matrix.
    void SetPosition(float x, float y, float z);
    void SetPosition(const Vec3& p);

    // 0x004fc970, thiscall, ret 4: writes the local translation (+0xe8..+0xf0) to *out
    // and returns out in eax (tier 1). Declared with an explicit out pointer because the
    // callers (0x00501c20, 0x005040f0) pass the destination member's address directly,
    // which a by-value return does not reproduce under VC6 (tier 2).
    Vec3* GetPosition(Vec3* out);

    // 0x004fc9a0. Position of this node expressed in 'frame' space: the local
    // translation when frame == this, else the world translation, transformed by
    // frame->WorldToLocalPoint when frame is non-null.
    void GetPositionIn(SoultreeObject* frame, Vec3* out);

    // 0x004fc540. World matrix rows 2 (+0x118) and 1 (+0x108) expressed in 'frame'
    // space via frame->WorldToLocalDirection; (0,0,1) and (0,1,0) when frame == this.
    // Either output may be null. Semantic axis names are tier 3.
    void GetAxesIn(SoultreeObject* frame, Vec3* axisZ, Vec3* axisY);

    // 0x004fdae0. Depth-first search by name (compares the string at +0x38),
    // visiting nextSibling (+0x144) before firstChild (+0x140). Returns 0 if absent.
    SoultreeObject* FindByName(const char* name);

    // ---- frame-relative matrix/position accessors (added later; append-only) ----
    // 'frame' conventions: frame == this -> local space, frame == 0 -> world space,
    // otherwise the space of that node.

    // 0x004fca80. *out = this node's matrix in frame space: identity when frame ==
    // this, the world matrix when frame == 0, else world re-expressed in frame.
    // PhysicsRigidBody passes frame = 0 and feeds the result to QuatFromMatrix.
    void GetMatrixIn(SoultreeObject* frame, Matrix4* out);

    // 0x004fb8c0. Sets this node's matrix so that it equals *m in frame space.
    // frame == this copies *m to localMatrix (0x004fca30). Otherwise *m is
    // converted in place, so m is not const.
    void SetMatrixIn(SoultreeObject* frame, Matrix4* m);

    // 0x004fc890. Despite the name (kept for the existing callers) this TRANSLATES: it
    // converts the offset *p from 'frame' space (directions through
    // frame->LocalToWorldDirection and parent->WorldToLocalDirection; frame == 0 through
    // parent->WorldToLocalPoint when there is a parent) and adds it with Translate (0x004fc850).
    void SetPositionIn(SoultreeObject* frame, const Vec3* p);

    // 0x004fc050, 1171 bytes, ret 0x14. The inverse of GetAxesIn. It builds a
    // rotation whose row 2 is *axisZ and row 1 is *axisY, given in 'frame' space,
    // and stores it into localMatrix. The local translation (+0xe8..+0xf0) is saved
    // first and restored afterwards, then InvalidateWorldMatrix (0x004fb850) runs.
    // With orthogonalize != 0 it forms r = axisZ x axisY. keepZ != 0 recomputes
    // axisY = axisZ x r; otherwise axisZ = r x axisY. It then normalizes both
    // (0x005087b0, Vec3Normalize), and row 0 is the cross of the other two
    // (0x00515600). frame == this stores the rotation as it is; otherwise it goes
    // through the frame's and the parent's matrices (0x0042a1a0). Tier 2 for data
    // flow. The names and the role of the last argument are tier 3.
    void SetAxesIn(SoultreeObject* frame, const Vec3* axisZ, const Vec3* axisY,
                   int orthogonalize, int keepZ);

    // ---- hierarchy helpers (formerly helpers/SoultreeNode.h; names tier 3) ----

    // 0x004fdab0. Sets subtreeDirty (+0x18c) on this node and on every ancestor.
    void MarkSubtreeDirty();

    // 0x004fb880. Clears worldValid on this node and everything below it: recurses along
    // nextSibling first, then walks down firstChild while the flag was set.
    void InvalidateSiblingChain();

    // 0x004fd910. Makes 'child' the last child of this node, detaching it from its old parent.
    void AddChild(SoultreeObject* child);

    // 0x004fd960. Appends 'node' at the end of this node's sibling list (walks nextSibling).
    void AppendSibling(SoultreeObject* node);

    // 0x004fd990. Unlinks 'child' from this node's child list and clears its links. The
    // child's world matrix is invalidated first.
    void RemoveChild(SoultreeObject* child);

    // 0x004fceb0. thiscall, ret 0x10: localMatrix = localMatrix * R(axis, angle) through
    // MatrixMultiply (0x0042a1a0), keeping the local translation; the axis is normalized
    // first (same inline axis-angle matrix as RotateAbout 0x004fcce0 and SetRotation 0x004fd090).
    void Rotate(Vec3 axis, float angle);

    // 0x004fd1f0. thiscall, ret 0x1c. Rotates this node about the local-space point 'pivot'
    // so that the pivot stays where it was in the parent's space. Tier 3 name.
    void RotateAboutPoint(Vec3 pivot, Vec3 axis, float angle);

    // ---- added in wave 4 (motion): methods called by SteeringControl / D3DIMSoultreeMotnctrl ----

    // 0x004fbd70, thiscall, ret 0x10: SetAxesIn-style setter taking the two axes by pointer
    // (axisZ, axisY, orthogonalize, keepZ); the pose/steering callers pass them straight through
    // (tier 2 for the signature, role tier 3).
    void SetAxesPtr(const Vec3* axisZ, const Vec3* axisY, int orthogonalize, int keepZ);

    // 0x004fcce0, thiscall, ret 0x10: Rotate with the axis passed as three loose floats
    // (SteeringControl's call shape).
    void RotateAbout(float x, float y, float z, float angle);

    // 0x004fc690, thiscall, ret 0x10: for frame == this it adds 'delta' to the local translation
    // +0xe8..+0xf0 and invalidates the world matrix; otherwise it SETS the position to 'delta'
    // given in 'frame' space (frame->LocalToWorldPoint, then parent->WorldToLocalPoint; frame == 0
    // stores it unchanged) through SetPosition (tier 2).
    void TranslateIn(SoultreeObject* frame, Vec3 delta);

    // 0x004fc4f0, thiscall, ret 8: local rotation rows 2 and 1 (the frame-less form of GetAxesIn
    // at 0x004fc540). Names tier 3.
    void GetAxes(Vec3* axisZ, Vec3* axisY);

    // 0x004fc630 again, spelled with the position by value: the pose callers in D3DIMSoultreeMotnctrl slots 4
    // and 6 copy a Vec3 to the argument area dword by dword (tier 2). The float overload above is the same
    // function; this declaration exists only so those callers keep that call shape.
    void SetPositionVec3(Vec3 p);

    // 0x004fedb0, thiscall, plain ret (soultree.cpp, __FILE__ line 0x49b): appends this node to the global node
    // array (0x00689ec4, count 0x00689ec8, capacity 0x00689ecc) unless it is already listed. Name tier 3.
    void RegisterNode();

    // 0x004fee30, thiscall, plain ret: removes this node from the same global array (name tier 3).
    void UnregisterNode();

    // 0x004fda30, thiscall: 1 + the counts of nextSibling and firstChild, i.e. the nodes of
    // this node's subtree plus those of its later siblings.
    int CountNodes();

    // ---- callees of SoultreePhysicsBaseObject (formerly SoultreePhysicsCallees.h) ----

    // 0x004fe850. thiscall, ret 8: *center / *extents = subtreeBoundsA / subtreeBoundsB, after
    // UpdateSubtreeBounds when subtreeDirty is set (SoultreePhysicsBaseObject slot 2 derives the
    // box inertia from the extents).
    void GetSubtreeBounds(Vec3* center, Vec3* extents);

    // 0x004fbd10, thiscall, ret 0x20: SetAxesPtr with the two axes as loose floats. The
    // seventh and eighth arguments are copied through unchanged into SetAxesPtr's int
    // 'orthogonalize' and 'keepZ' (mov/push, no conversion), so both are ints (tier 1). The
    // caller (SoultreePhysicsBaseObject slot 2) passes 0,0,1, 0,1,0, 0, 1.
    void SetAxes(float zx, float zy, float zz, float yx, float yy, float yz,
                     int orthogonalize, int keepZ);

    // ---- soultree.cpp members added with the unit's promotion (names tier 3) ----

    // 0x004fc740, ret 8: places this node so that its position is *p given in 'frame'
    // space: frame == this adds *p to the local translation; otherwise *p goes through
    // frame->LocalToWorldPoint (frame != 0) and parent->WorldToLocalPoint, then SetPosition.
    void SetPositionInFrame(SoultreeObject* frame, const Vec3* p);

    // 0x004fc850, ret 4: adds *d to the local translation and invalidates the world matrix.
    void Translate(const Vec3* d);

    // 0x004fca30, ret 4: localMatrix = *m, then InvalidateWorldMatrix.
    void SetLocalMatrix(const Matrix4* m);

    // 0x004fca60, ret 4: *out = localMatrix.
    void GetLocalMatrix(Matrix4* out);

    // 0x004fcc70: resets the local and world matrices of this node, its later siblings and
    // its descendants to identity (0x004a1410) and marks the world matrices stale.
    void ResetMatrices();

    // 0x004fda60, ret 8: appends every descendant to out[] (children first, level by level
    // per parent), advancing *count.
    void CollectDescendants(int* count, SoultreeObject** out);

    // 0x004fd090, ret 0x10: replaces the local rotation by the rotation about (x, y, z) by
    // 'angle' (radians), keeping the local translation.
    void SetRotation(float x, float y, float z, float angle);

    // 0x004fdb40 / 0x004fdb50: set / clear field_0x14c.
    void SetFlag14c();
    void ClearFlag14c();

    // 0x004fdb60, ret 8, EH: creates the parameter block field_0x1a0 (__FILE__ line 0x346),
    // opens it on 'stream' (0x004b77a0, index 1), loads through slot 3 and deletes the block.
    // The stream type is the one src/reconstructed/Parameterblocks.h names.
    void LoadFromParameters(UnknownParameterStream* stream, int offset);

    // 0x004fe0a0, ret 8: *a / *b = localBoundsA / localBoundsB, after slot 5 when
    // localBoundsValid is 0.
    void GetLocalBounds(Vec3* a, Vec3* b);

    // 0x004fe0f0: refreshes subtreeBoundsA/B (center and half extents) from this node's
    // local bounds (when field_0x150 is set) and every child's 0x004fe2e0.
    void UpdateSubtreeBounds();

    // 0x004fe2e0, ret 0x10: grows the box (*have, *lo, *hi) by this node's subtree, expressed
    // through 'frame' (a world matrix). Signature from the caller 0x004fe0f0 (tier 2).
    void AccumulateBounds(int* have, Vec3* lo, Vec3* hi, const Matrix4* frame);

    // 0x004fe8a0, ret 8: world-space box (lo, hi) of the subtree bounds: the center goes through
    // the world matrix and the half extents through |R(world)| (tier 2).
    void GetWorldBounds(Vec3* lo, Vec3* hi);

    // 0x004fecd0: inserts this node in the global quadtree (0x0068aba4) or updates its range,
    // using GetWorldBounds; the quadtree code is kept in field_0x198 (15 = none).
    void UpdateQuadtreeCell();

    // 0x004fed70: removes this node from the quadtree when field_0x194 is set and resets
    // field_0x194 / field_0x198.
    void RemoveFromQuadtree();

    // 0x004fefb0 is the out-of-line copy of an inline 3x3 transpose (cdecl, Matrix4*).

#ifdef SOULTREE_OBJECT_WITH_BASES
    // ---- virtual functions (soultree.cpp only; see the header comment) ----
    // GameObject slot 0 (secondary vtable 0x00557c40): scalar deleting dtor 0x004fb400,
    // body 0x004fb430. 'this' is the GameObject subobject (+12).
    virtual ~SoultreeObject();
    // Primary vtable 0x00557c18 (QuadTreeObject's): slots 0/1 inherited, 2..8 introduced.
    virtual void UnknownVirtualSlot2(SoultreeFileStream* stream);      // 0x004fdc00, ret 4: loads a hierarchy
    virtual void UnknownVirtualSlot3();                                // 0x004fddc0: loads from parameterBlock
    virtual void UnknownVirtualSlot4(SoultreeObject** out);            // 0x004fe020, ret 4: *out = new node
    virtual void UnknownVirtualSlot5();                                // 0x00464e90 (shared empty body)
    virtual void UnknownVirtualSlot6();                                // 0x004fec70: identity local matrices
    virtual void UnknownVirtualSlot7(SoultreeObject* src);             // 0x004feb10, ret 4: clone src's tree into this
    virtual void UnknownVirtualSlot8(SoultreeObject* src, SoultreeObject* dst); // 0x004feb30, ret 8
    // GameObject slot 25 override 0x004fec30: forwards to every node of the sibling chain at
    // +0x140 (GameObject view +0x134) before GameObject's own slot 25 (0x00469720).
    virtual int GameObjectVirtualSlot25(int a);
#endif

    // ---- layout (offsets tier 1 from the constructor/helpers; names tier 3) ----
#ifndef SOULTREE_OBJECT_WITH_BASES
    char pad_0x000[0x38];          // QuadTreeObject vptr (+0), GameObject vptr (+12)...
#endif
    char name[0x80];               // 0x038, compared by FindByName (size unknown)
    Matrix4 localMatrix;           // 0x0b8, identity after construction
    Matrix4 worldMatrix;           // 0x0f8, local * parent world
    int worldValid;                // 0x138, 0 = worldMatrix stale
    SoultreeObject* parent;        // 0x13c
    SoultreeObject* firstChild;    // 0x140
    SoultreeObject* nextSibling;   // 0x144
    SoultreeObject* prevSibling;   // 0x148, previous sibling (AppendSibling/RemoveChild link it)
    // 0x14c..0x188 are zeroed by the constructor. The stream load (slot 2,
    // 0x004fdc00) reads 0x14c, 0x154..0x18b in this order; the clone (slot 8)
    // copies 0x14c and clears 0x154 (tier 1 offsets, tier 3 roles).
    int field_0x14c;               // 0x14c, set/cleared by 0x004fdb40 / 0x004fdb50
    int field_0x150;               // 0x150
    int localBoundsValid;          // 0x154, 0 makes 0x004fe0a0 call slot 5 first
    Vec3 localBoundsA;             // 0x158, returned by 0x004fe0a0
    Vec3 localBoundsB;             // 0x164
    int field_0x170;               // 0x170
    Vec3 subtreeBoundsA;           // 0x174, returned by 0x004fe850 (refreshed when subtreeDirty)
    Vec3 subtreeBoundsB;           // 0x180
    int subtreeDirty;              // 0x18c, set on self and all ancestors by 0x004fdab0
    int field_0x190;               // 0x190, 1 after construction
    int inQuadtree;                // 0x194, nonzero while inserted in the quadtree (0x004fed70 tests and clears it)
    int quadtreeCell;              // 0x198, quadtree cell code (15 = none) passed to Remove/UpdateRange
    int loadedNodeCount;           // 0x19c, node count of a loaded hierarchy (slot 2 reads it, slot 3 stores the "Object Hierarchy" count)
    UnknownParameterBlock* parameterBlock; // 0x1a0, parameter block used by 0x004fdb60/slot 3;
                                   // sizeof 0x1a4 from the operator new in
                                   // SoultreePhysicsBaseObject slot 2 (tier 1)
};

#endif
