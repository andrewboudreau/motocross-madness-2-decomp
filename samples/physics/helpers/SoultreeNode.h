// SoultreeNode.h -- PROVISIONAL declarations of SoultreeObject (soultree.cpp) helpers that
// ../common/SoultreeObject.h does not declare yet. The class owner (common/) should add these
// methods to SoultreeObject itself; until then they live on a derived stand-in so the
// SoultreeObject member functions can call them (same object address, non-virtual calls,
// __thiscall). Names are tier 3. Layout facts are tier 1/2 (see SoultreeObject.h).
#ifndef MCM2_PHYSICS_HELPERS_SOULTREENODE_H
#define MCM2_PHYSICS_HELPERS_SOULTREENODE_H

#include "../common/Math3D.h"

class SoultreeNode : public SoultreeObject {
public:
    // 0x004fdab0. Sets subtreeDirty (+0x18c) on this node and on every ancestor.
    void MarkSubtreeDirty();

    // 0x004fb880. Clears worldValid on this node and everything below it: recurses along
    // nextSibling first, then walks down firstChild while the flag was set.
    void InvalidateSiblingChain();

    // 0x004fd910. Makes 'child' the last child of this node, detaching it from its old parent.
    void AddChild(SoultreeNode* child);

    // 0x004fd960. Appends 'node' at the end of this node's sibling list (walks nextSibling).
    void AppendSibling(SoultreeNode* node);

    // 0x004fd990. Unlinks 'child' from this node's child list and clears its links. The
    // child's world matrix is invalidated first.
    void RemoveChild(SoultreeNode* child);

    // 0x004fceb0. thiscall, ret 0x10. Applies a rotation of 'angle' about 'axis' to the local
    // matrix (arguments: Vec3 by value, then a float). Only its call signature is known
    // (tier 2, from the pushes in 0x004fd1f0); the body has not been reconstructed.
    void Rotate(Vec3 axis, float angle);

    // 0x004fd1f0. thiscall, ret 0x1c. Rotates this node about the local-space point 'pivot'
    // so that the pivot stays where it was in the parent's space. Tier 3 name.
    void RotateAboutPoint(Vec3 pivot, Vec3 axis, float angle);

    // 0x004fdae0 is FindByName (declared in SoultreeObject.h).
};

#endif
