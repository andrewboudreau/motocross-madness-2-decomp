// CollisionCharacter.cpp -- see CollisionCharacter.h.
#include <stddef.h>
#include "CollisionCharacter.h"
#include "soultree/SoultreePhysicsCallees.h"
#include "collision/CollisionShapeTests.h"
#include "core/MemTag.h"

#define CC_FILE "D:\\aardvark\\VC\\krusty2\\CollisionCharacter.cpp"


typedef char cc_check_own[(offsetof(CollisionCharacter, field_0x260) == 0x260) ? 1 : -1];

// 0x0043c890 (shared out-of-line copy, inlined here): v / s computed as one reciprocal and three
// multiplies (same helper the Terrain code uses).
static inline Vec3 CharacterDivide(const Vec3& v, float s)
{
    float inv = 1.0f / s;
    return Vec3(inv * v.x, inv * v.y, inv * v.z);
}

CollisionCharacter::CollisionCharacter(int a) : GameObject(1), D3DIMSoultreeCharacter(a)
{
    collisionObject = 0;
}

CollisionCharacter::~CollisionCharacter()
{
}

// Slot 14 (0x00431cf0): tail call to the base implementation (0x00469360).
int CollisionCharacter::GameObjectVirtualSlot14()
{
    return GameObject::GameObjectVirtualSlot14();
}

// Slot 10 (0x00431b30, ret 4): per-frame update.  Tier 3 semantics:
//  * velocity = (node position - previous position) / dt, and the position is remembered;
//  * when the collision object has a contact record (QueryCollisions), the record's vector at +0
//    scaled by 1.005 is subtracted from the translation row (+0xf8..+0x100) of the shape's
//    matrix history (the hull's field_0xc8 matrix), for every hull of a model shape (type 1)
//    or for the single payload otherwise.
int CollisionCharacter::GameObjectVirtualSlot10(float dt)
{
    if (collisionObject)
        collisionObject->UpdatePlacement();

    Vec3 pos;
    sceneNode->GetPositionIn(0, &pos);
    nodeVelocity = CharacterDivide(pos - lastNodePosition, dt);
    lastNodePosition = pos;

    if (collisionObject && collisionObject->QueryCollisions()) {
        const Vec3* rec = (const Vec3*)collisionObject->contactRecord;
        field_0x248 = rec[2];
        field_0x254 = rec[1];
        if (collisionObject->shapeType == 1) {
            CollisionModelBody* model = (CollisionModelBody*)collisionObject->shape;
            for (int i = 0; i < model->elementCount; i++) {
                CollisionHullBody* hull = &model->elements[i];
                hull->bodyTransform._41 -= rec->x * 1.005f;
                hull->bodyTransform._42 -= rec->y * 1.005f;
                hull->bodyTransform._43 -= rec->z * 1.005f;
            }
        } else {
            CollisionHullBody* hull = (CollisionHullBody*)collisionObject->shape;
            hull->bodyTransform._41 -= rec->x * 1.005f;
            hull->bodyTransform._42 -= rec->y * 1.005f;
            hull->bodyTransform._43 -= rec->z * 1.005f;
        }
    }
    return GameObject::GameObjectVirtualSlot10(dt);
}

// 0x004319c0 (ret 0x1c).  Loads the character through the D3DIM base (slot 11 with a1, name,
// desc, a5, a6, a7), then creates the CollisionObject for it (new(0xb8, __FILE__, 27),
// ctor 0x00431e70 with 1) and gives it a shape: from the .col file colPath when one was
// supplied, otherwise from the scene node.  The character becomes the object's owner with
// tag 0x2711.  Returns the result of GameObject slot 8 (stores a1 in field_0x18 and returns
// the GameObject).  The 'Collision' allocation category is selected around the new.
GameObject* CollisionCharacter::Load(int a1, const char* name, const char* colPath,
                                     const SoultreeLoadDesc* desc, int a5, int a6, int a7)
{
    D3DIMSoultreeCharacter::D3DIMVirtualSlot11(a1, name, desc, a5, a6, a7);
    int prevTag = g_MemTagStack->Push("Collision");
    collisionObject = new(CC_FILE, 27) CollisionObject(1);
    collisionObject->Configure(a1, 1, 0, 1);
    collisionObject->ignoreVegetation = 1;
    // node + 0x140 is the scene node whose position slot 10 tracks (tier 3)
    sceneNode = *(SoultreeObject**)((char*)modelNode + 0x140);
    GameObject::Method_0x00469190((GraphicsTest*)collisionObject, -1);
    if (colPath && *colPath)
        collisionObject->LoadShape(modelNode, colPath);
    else
        collisionObject->SetModelShape(modelNode, 1, 1, 0, 0);
    collisionObject->Fn_00435fe0();
    collisionObject->SetOwner(this, 0x2711);
    g_MemTagStack->Pop(prevTag);
    return GameObject::GameObjectVirtualSlot8(a1);
}
