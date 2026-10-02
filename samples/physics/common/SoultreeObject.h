// SoultreeObject.h -- scene-graph node (soultree.cpp) as seen by the physics code.
//
// Owner: opus_rigidbody (samples/physics/common). Included by Math3D.h.
//
// Class identity: RTTI .?AVSoultreeObject@@ (tier 1), bases QuadTreeObject (+0)
// and GameObject (+12). The constructor at 0x004fb2b0 writes both vptrs
// (0x00557c18 at +0, 0x00557c40 at +12) and initializes the transform fields
// below (tier 1 for offsets). The non-virtual helpers are attributed to this class
// because they all begin with UpdateWorldMatrix (0x004fb4f0), which reads the
// +0x138 / +0x13c fields that constructor initializes, and they sit next to the
// soultree.cpp __FILE__ references (tier 2).
//
// This declaration is PROVISIONAL and deliberately minimal: it has no virtual
// functions and represents the two vptrs and the base-class bodies as padding, so
// that member offsets are right for the physics code. Do not derive from it.
//
// It is the single declaration of the node for every physics area (the former
// stand-ins helpers/SoultreeNode.h and the SoultreeNode class of
// soultree_base/SoultreePhysicsCallees.h are merged here).
#ifndef MCM2_PHYSICS_COMMON_SOULTREEOBJECT_H
#define MCM2_PHYSICS_COMMON_SOULTREEOBJECT_H

class SoultreeObject {
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

    // 0x004fc890. Sets this node's position so that it equals *p in frame space.
    // frame == 0 converts through parent->WorldToLocalPoint when there is a parent.
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

    // 0x004fceb0. thiscall, ret 0x10. Applies a rotation of 'angle' about 'axis' to the local
    // matrix (arguments: Vec3 by value, then a float). Only its call signature is known
    // (tier 2, from the pushes in 0x004fd1f0); the body has not been reconstructed.
    void Rotate(Vec3 axis, float angle);

    // 0x004fd1f0. thiscall, ret 0x1c. Rotates this node about the local-space point 'pivot'
    // so that the pivot stays where it was in the parent's space. Tier 3 name.
    void RotateAboutPoint(Vec3 pivot, Vec3 axis, float angle);

    // ---- added in wave 4 (motion): methods called by SteeringControl / D3DIMSoultreeMotnctrl ----

    // 0x004fbd70, thiscall, ret 0x10: SetAxesIn-style setter taking the two axes by pointer
    // (axisZ, axisY, orthogonalize, keepZ); the pose/steering callers pass them straight through
    // (tier 2 for the signature, role tier 3).
    void SetAxesPtr(const Vec3* axisZ, const Vec3* axisY, int orthogonalize, int keepZ);

    // 0x004fcce0, thiscall, ret 0x10: rotate the local matrix about the axis (x, y, z) by 'angle'
    // (tier 2 signature; the axis is passed as three loose floats by SteeringControl).
    void RotateAbout(float x, float y, float z, float angle);

    // 0x004fc690, thiscall, ret 0x10: adds 'delta' (given in 'frame' space) to the node position;
    // for frame == this it adds to the local translation +0xe8..+0xf0 and invalidates the world
    // matrix (tier 2).
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

    // 0x004fda30, thiscall: node count of the subtree (tier 3 role; signature from the callers).
    int CountNodes();

    // ---- callees of SoultreePhysicsBaseObject (formerly SoultreePhysicsCallees.h) ----

    // 0x004fe850. thiscall, two Vec3 out pointers (center, extents); a bounds query
    // (tier 3 role; SoultreePhysicsBaseObject slot 2 derives the box inertia from extents).
    void Fn_004fe850(Vec3* center, Vec3* extents);

    // 0x004fbd10. thiscall, 7 floats and an int (tier 1 argument shape); the caller
    // (SoultreePhysicsBaseObject slot 2) passes 0,0,1, 0,1,0, 0.0f, 1 (roles tier 3).
    void Fn_004fbd10(float a, float b, float c, float d, float e, float f, float g, int h);

    // ---- layout (offsets tier 1 from the constructor/helpers; names tier 3) ----
    char pad_0x000[0x38];          // QuadTreeObject vptr (+0), GameObject vptr (+12)...
    char name[0x80];               // 0x038, compared by FindByName (size unknown)
    Matrix4 localMatrix;           // 0x0b8, identity after construction
    Matrix4 worldMatrix;           // 0x0f8, local * parent world
    int worldValid;                // 0x138, 0 = worldMatrix stale
    SoultreeObject* parent;        // 0x13c
    SoultreeObject* firstChild;    // 0x140
    SoultreeObject* nextSibling;   // 0x144
    int field_0x148;               // 0x148
    char pad_0x14c[0x40];          // 0x14c..0x18b zeroed by the constructor
    int subtreeDirty;              // 0x18c, set on self and all ancestors by 0x004fdab0
    int field_0x190;               // 0x190, 1 after construction
    int field_0x194;               // 0x194
    int field_0x198;               // 0x198, 0xf after construction
    int field_0x19c;               // 0x19c
    int field_0x1a0;               // 0x1a0; sizeof 0x1a4 from the operator new in
                                   // SoultreePhysicsBaseObject slot 2 (tier 1)
};

#endif
