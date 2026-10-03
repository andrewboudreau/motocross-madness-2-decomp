// SoultreeHierarchy.cpp -- scene-graph node hierarchy/dirty-flag helpers of SoultreeObject
// (soultree.cpp, 0x004fb4f0..0x004fdb31). Class attribution tier 2 (see SoultreeObject.h).
#include "math/Math3D.h"

// 0x004fdab0.
void SoultreeObject::MarkSubtreeDirty()
{
    subtreeDirty = 1;
    for (SoultreeObject* p = parent; p; p = p->parent)
        p->subtreeDirty = 1;
}

// 0x004fb880.
void SoultreeObject::InvalidateSiblingChain()
{
    SoultreeObject* n = this;
    for (;;) {
        if (n->nextSibling)
            n->nextSibling->InvalidateSiblingChain();
        if (!n->worldValid)
            break;
        n->worldValid = 0;
        n = n->firstChild;
        if (!n)
            break;
    }
}

// 0x004fb850. Marks the ancestors' subtree flag, then invalidates this node and its
// descendants if the world matrix was valid. (The siblings of THIS node are not touched.)
void SoultreeObject::InvalidateWorldMatrix()
{
    MarkSubtreeDirty();
    if (worldValid) {
        worldValid = 0;
        if (firstChild)
            firstChild->InvalidateSiblingChain();
    }
}

// 0x004fd960.
void SoultreeObject::AppendSibling(SoultreeObject* node)
{
    SoultreeObject* last = this;
    if (last->nextSibling) {
        do {
            last = last->nextSibling;
        } while (last->nextSibling);
    }
    last->nextSibling = node;
    node->field_0x148 = (int)last;   // previous sibling (tier 3 role)
}

// 0x004fd910.
void SoultreeObject::AddChild(SoultreeObject* child)
{
    if (child->parent)
        parent->RemoveChild(child);
    child->parent = this;
    if (!firstChild)
        firstChild = child;
    else
        firstChild->AppendSibling(child);
}

// 0x004fd990. Unlinks 'child' from this node's child list. The list search has no null
// check (the child is assumed to be present). Each link is cleared only if it is set.
void SoultreeObject::RemoveChild(SoultreeObject* child)
{
    child->InvalidateWorldMatrix();
    SoultreeObject* n = firstChild;
    while (n != child)
        n = n->nextSibling;
    if (n == firstChild)
        firstChild = n->nextSibling;
    if (n->nextSibling)
        n->nextSibling->field_0x148 = n->field_0x148;
    if (n->field_0x148)
        ((SoultreeObject*)n->field_0x148)->nextSibling = n->nextSibling;
    if (n->parent)
        n->parent = 0;
    if (n->nextSibling)
        n->nextSibling = 0;
    if (n->field_0x148)
        n->field_0x148 = 0;
}

// 0x004fb4f0. Refreshes worldMatrix = localMatrix * parent->worldMatrix (row-vector
// convention), recursing up the parent chain first; a node without a parent copies its local
// matrix. The product is expanded inline (no MatrixMultiply call).
void SoultreeObject::UpdateWorldMatrix()
{
    if (worldValid)
        return;
    if (parent) {
        parent->UpdateWorldMatrix();
        const Matrix4& p = parent->worldMatrix;
        worldMatrix._11 = localMatrix._11 * p._11 + localMatrix._12 * p._21 + localMatrix._13 * p._31 + localMatrix._14 * p._41;
        worldMatrix._12 = localMatrix._11 * p._12 + localMatrix._12 * p._22 + localMatrix._13 * p._32 + localMatrix._14 * p._42;
        worldMatrix._13 = localMatrix._11 * p._13 + localMatrix._12 * p._23 + localMatrix._13 * p._33 + localMatrix._14 * p._43;
        worldMatrix._14 = localMatrix._11 * p._14 + localMatrix._12 * p._24 + localMatrix._13 * p._34 + localMatrix._14 * p._44;
        worldMatrix._21 = localMatrix._21 * p._11 + localMatrix._22 * p._21 + localMatrix._23 * p._31 + localMatrix._24 * p._41;
        worldMatrix._22 = localMatrix._21 * p._12 + localMatrix._22 * p._22 + localMatrix._23 * p._32 + localMatrix._24 * p._42;
        worldMatrix._23 = localMatrix._21 * p._13 + localMatrix._22 * p._23 + localMatrix._23 * p._33 + localMatrix._24 * p._43;
        worldMatrix._24 = localMatrix._21 * p._14 + localMatrix._22 * p._24 + localMatrix._23 * p._34 + localMatrix._24 * p._44;
        worldMatrix._31 = localMatrix._31 * p._11 + localMatrix._32 * p._21 + localMatrix._33 * p._31 + localMatrix._34 * p._41;
        worldMatrix._32 = localMatrix._31 * p._12 + localMatrix._32 * p._22 + localMatrix._33 * p._32 + localMatrix._34 * p._42;
        worldMatrix._33 = localMatrix._31 * p._13 + localMatrix._32 * p._23 + localMatrix._33 * p._33 + localMatrix._34 * p._43;
        worldMatrix._34 = localMatrix._31 * p._14 + localMatrix._32 * p._24 + localMatrix._33 * p._34 + localMatrix._34 * p._44;
        worldMatrix._41 = localMatrix._41 * p._11 + localMatrix._42 * p._21 + localMatrix._43 * p._31 + localMatrix._44 * p._41;
        worldMatrix._42 = localMatrix._41 * p._12 + localMatrix._42 * p._22 + localMatrix._43 * p._32 + localMatrix._44 * p._42;
        worldMatrix._43 = localMatrix._41 * p._13 + localMatrix._42 * p._23 + localMatrix._43 * p._33 + localMatrix._44 * p._43;
        worldMatrix._44 = localMatrix._41 * p._14 + localMatrix._42 * p._24 + localMatrix._43 * p._34 + localMatrix._44 * p._44;
    } else {
        worldMatrix = localMatrix;
    }
    worldValid = 1;
}
