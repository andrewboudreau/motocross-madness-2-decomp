// CollisionCharacter.cpp -- see CollisionCharacter.h.
#include <stddef.h>
#include "CollisionCharacter.h"
#include "../soultree_base/SoultreePhysicsCallees.h"
#include "../collision/CollisionShapeTests.h"
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
    field_0x210 = 0;
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
//  * when the collision object has a contact record (Fn_00438e70), the record's vector at +0
//    scaled by 1.005 is subtracted from the translation row (+0xf8..+0x100) of the shape's
//    matrix history (the hull's field_0xc8 matrix), for every hull of a model shape (type 1)
//    or for the single payload otherwise.
int CollisionCharacter::GameObjectVirtualSlot10(float dt)
{
    if (field_0x210)
        field_0x210->Fn_00435fb0();

    Vec3 pos;
    field_0x214->GetPositionIn(0, &pos);
    field_0x224 = CharacterDivide(pos - field_0x218, dt);
    field_0x218 = pos;

    if (field_0x210 && field_0x210->Fn_00438e70()) {
        const Vec3* rec = (const Vec3*)field_0x210->field_0x5c;
        field_0x248 = rec[2];
        field_0x254 = rec[1];
        if (field_0x210->field_0x50 == 1) {
            CollisionModelBody* model = (CollisionModelBody*)field_0x210->field_0x54;
            for (int i = 0; i < model->elementCount; i++) {
                CollisionHullBody* hull = &model->elements[i];
                hull->field_0xc8._41 -= rec->x * 1.005f;
                hull->field_0xc8._42 -= rec->y * 1.005f;
                hull->field_0xc8._43 -= rec->z * 1.005f;
            }
        } else {
            CollisionHullBody* hull = (CollisionHullBody*)field_0x210->field_0x54;
            hull->field_0xc8._41 -= rec->x * 1.005f;
            hull->field_0xc8._42 -= rec->y * 1.005f;
            hull->field_0xc8._43 -= rec->z * 1.005f;
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
    field_0x210 = new(CC_FILE, 27) CollisionObject(1);
    field_0x210->Fn_004320f0(a1, 1, 0, 1);
    field_0x210->field_0x68 = 1;
    // node + 0x140 is the scene node whose position slot 10 tracks (tier 3)
    field_0x214 = *(SoultreeObject**)((char*)d3d_field_0x1a0 + 0x140);
    GameObject::Method_0x00469190((GraphicsTest*)field_0x210, -1);
    if (colPath && *colPath)
        field_0x210->Fn_00432800(d3d_field_0x1a0, colPath);
    else
        field_0x210->Fn_004324b0(d3d_field_0x1a0, 1, 1, 0, 0);
    field_0x210->Fn_00435fe0();
    field_0x210->SetOwner(this, 0x2711);
    g_MemTagStack->Pop(prevTag);
    return GameObject::GameObjectVirtualSlot8(a1);
}
