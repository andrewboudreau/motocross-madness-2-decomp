// SoultreePhysicsObject and D3DIMSoultreeObject -- class shapes for the SoulTreePhysics.cpp
// members of SoultreePhysicsObject (ctor 0x005037c0, dtor 0x005038d0, GameObject slot 10
// 0x00503c50) and the slot 40 loader 0x00503970, all defined in SoulTreePhysics.cpp.
//
// Evidence (tier 1, analysis/rtti_classes.json and analysis/vtables.json):
//  * SoultreePhysicsObject : SoultreePhysicsBaseObject (+0), D3DIMSoultreeObject (+540).
//    COL/vtable records: 0x00557f54 @0 (41 slots, slot 40 = 0x00503970 introduced here),
//    0x00557f28 @540 (10 slots), 0x00557eb8 @552 (27, D3DIMSoultreeObject's plain GameObject:
//    slot 0 = 0x00503890, slot 10 = 0x00503c50), 0x00557e48 @1272 (27, the
//    SoultreePhysicsBaseObject virtual GameObject; slot 0 is the vtordisp thunk 0x00504260
//    `sub ecx,[ecx-4]; sub ecx,0x2d0; jmp 0x00503890`, slot 10 the thunk 0x00504270 ->
//    0x00503c50, so a vtordisp dword sits at 0x4f4).
//  * D3DIMSoultreeObject : SoultreeObject : QuadTreeObject (+0), GameObject (+12), all
//    non-virtual (pdisp -1).  So SoultreePhysicsObject holds TWO GameObjects: the virtual
//    one from SoultreePhysicsBaseObject and a plain one at 0x21c + 12 = 0x228.
//  * VC6 gives the destructor and the GameObject slot 10 override of SoultreePhysicsObject a
//    `this` of that plain GameObject (+0x228): the deleting destructor 0x00503890 does
//    `lea esi,[ecx-0x228]`, the core 0x005038d0 restores the vptrs through esi-0x228 and
//    the vbase-vtable thunks adjust by 0x2d0 = 0x4f8 - 0x228.  The same holds for
//    D3DIMSoultreeObject itself (dtor 0x0043f2b0: `mov [edi-0xc], 0x005513ec`).  That is
//    what the two GameObject bases below produce.
//  * D3DIMSoultreeObject primary vtable 0x005513ec: QuadTreeObject's two slots, then slots
//    2..9 (slot 9 = 0x0043f4b0, `ret 0x14`).  Its GameObject vtable 0x0055137c overrides
//    slots 0 (0x0043f280), 4 (0x00444540), 5 (0x00444520), 10 (0x00443490),
//    12 (0x00443aa0), 14 (0x00444030) and 25 (0x004fec30, SoultreeObject's); the
//    SoultreePhysicsObject vtable 0x00557eb8 (@552) keeps 4, 5, 12, 14 and 25.
//
// D3DIMSoultreeObject (and SoultreeObject, its base) are the single declarations of
// src/reconstructed/D3DIMSoulTree.h and core/SoultreeObject.h; the former folded
// physics view of D3DIMSoultreeObject is merged into them.
#ifndef SOULTREE_PHYSICS_OBJECT_H
#define SOULTREE_PHYSICS_OBJECT_H

#include "../math/Vec3.h"
#include "../motion/D3DIMSoultreeCharacter.h"
#include "SoultreePhysicsBaseObjectCore.h"
#include "../../reconstructed/D3DIMSoulTree.h"

class SoultreePhysicsObject : public SoultreePhysicsBaseObject, public D3DIMSoultreeObject {
public:
    // 0x005037c0 (thiscall, ret 8 = flags + the hidden most-derived flag): GameObject(1) when
    // most-derived, SoultreePhysicsBaseObject(flags), D3DIMSoultreeObject(flags), then
    // sceneNode = this (the D3DIMSoultreeObject subobject at +0x21c).
    explicit SoultreePhysicsObject(int flags);
    virtual ~SoultreePhysicsObject();             // core 0x005038d0, deleting 0x00503890
    virtual int Tick(float dt);                    // 0x00503c50 (vbase thunk 0x00504270)
    // Slot 40 (0x00503970, `ret 0x70` = 28 argument dwords, tier 1).  Grouping (tier 2):
    //  * renderContext, modelPath, lights and textures (a1..a4) go to D3DIMSoultreeObject::LoadModel
    //    (slot 9) as (a1, a2, a3, a4, 1); modelPath is also the name the ".col" path is built from, lights
    //    is tested at +0x25.
    //  * a5..a7 are three Vec3 by value (struct-copy shape) and a8..a22 are forwarded
    //    unchanged to slot 2 as its a6..a20, so their types and names are slot 2's.  Slot 2
    //    gets (a1, 1, a5, a6, a7, a8..a22).
    //  * The SceneManager caller 0x004ed3bf passes the result to GameObject::AppendChild.
    //  * Returns the D3DIMSoultreeObject GameObject (+0x228), not the virtual one: the tail
    //    0x00503c16 null-checks `this`, then this+0x21c, then adds 12.
    virtual GameObject* LoadPhysicsModel(int renderContext, const char* modelPath,
                                             const LightManager* lights, const TextureContext* textures,
                                             Vec3 startPosition, Vec3 startForward,
                                             Vec3 startUp, void* terrainProbe,
                                             void* linkedObject, float weight,
                                             int contactCapacity, int attachmentSlots,
                                             Scene* trackObject, float fixedStep,
                                             int maxSteps, float drag, float elasticity,
                                             float radius, int shapeMode, int restThreshold,
                                             unsigned char probeMask, int createCollision);
};

#endif
