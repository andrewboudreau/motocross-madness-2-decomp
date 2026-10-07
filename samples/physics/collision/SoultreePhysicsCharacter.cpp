// SoultreePhysicsCharacter small virtuals (assigned targets).  TU ownership tier 2:
// SoulTreePhysics.cpp (__FILE__ at 0x00503b13/0x00503f87).
#include <string.h>
#include "soultree/SoultreePhysicsCharacter.h"
#include "soultree/SoultreePhysicsCallees.h"
#include "soultree/SoultreePhysicsObject.h"

// SoultreePhysicsBaseObject::field_0x128 is the CollisionObject that slot 2 creates
// (`new` of 0xb8 bytes, ctor 0x00431e70, tier 1).

// The scene node at field_0x08 is the D3DIM object (SoultreePhysicsCharacter copies
// d3d_field_0x1a0 into it).  0x004444c0 (thiscall, `ret 4`) is one of that object's methods.
// SoultreeObject (common/SoultreeObject.h) does not declare it, so it is
// reached through this view (tier 3 name).
class SoultreeD3DNode {
public:
    void Fn_4444c0(int a);
};

// 0x134-byte file object: ctor 0x00460d10 takes one global, the destructor 0x00460d60
// fcloses the FILE* at +0x14 (tier 2).  Names are tier 3.
class SoultreeFile {
public:
    SoultreeFile(int a);            // 0x00460d10
    ~SoultreeFile();                // 0x00460d60
    char field_0x00[0x134];
};

// Object at +0x574 of the global at 0x0056e26c.  0x004e9cd0 (thiscall, ret 0x10) sits
// between the ResourceManager.cpp and SceneManager.cpp __FILE__ references.  It gets the
// file object, the name, the mode "rb" and an output buffer; on a zero return the caller
// clears the output path (tier 2 data flow, tier 3 names).
class SoultreeFileFinder {
public:
    int Fn_4e9cd0(SoultreeFile* file, const char* name, const char* mode, char* outPath);
};
struct SoultreeGlobals {
    char field_0x000[0x574];
    SoultreeFileFinder* field_0x574;
};
extern SoultreeGlobals* g_0056e26c;
extern int g_00572b44;          // passed by value to the SoultreeFile constructor

// slot 1 (0x005040c0): base slot 1 then clears four flag bytes.
void SoultreePhysicsCharacter::UnknownVirtualSlot1(float value)
{
    SoultreePhysicsBaseObject::UnknownVirtualSlot1(value);
    field_0x430 = 0;
    field_0x431 = 0;
    field_0x432 = 0;
    field_0x433 = 0;
}

// slot 8 (0x005041c0)
void SoultreePhysicsCharacter::UnknownVirtualSlot8()
{
    rotationPivot = localCenterOfMass = poseNode->WorldToLocalPoint(centerOfMass);
}

// slot 42 (0x00504470)
int SoultreePhysicsCharacter::UnknownVirtualSlot42()
{
    if (field_0x433 >= 0 && field_0x430) {
        return 1;
    }
    return 0;
}

// slot 33 (0x005040f0), retail `ret 0x18`
int SoultreePhysicsCharacter::UnknownVirtualSlot33(const Vec3* a1, const Vec3* a2,
                                                   const Vec3* a3, const Vec3* a4,
                                                   int a5, float a6)
{
    UnknownVirtualSlot1(a6);
    bodyForward = *a2;
    bodyUp = *a3;
    sceneNode->SetPosition(a1->x, a1->y, a1->z);
    sceneNode->GetPosition(&position);
    UnknownVirtualSlot36();
    if (centerNode) {
        centerNode->GetPositionIn(0, &centerOfMass);
    } else {
        sceneNode->GetPositionIn(0, &centerOfMass);
    }
    Method_0x004a8b00();
    collisionObject->Fn_00435fe0();
    respawnPending = 0;
    justReset = 1;
    attachmentResetPending = 1;
    return 0;
}

// slot 41 (0x00504360)
void SoultreePhysicsCharacter::UnknownVirtualSlot41()
{
    poseNode->GetAxesIn(0, &savedForward, &savedUp);
    modelNode->SetAxesIn(0, &savedForward, &savedUp, 1, 0);
    Method_0x004a8b00();
    field_0x430 = 0;
    UnknownVirtualSlot34();
    OrientationAnglesFromVectors(bodyForward, bodyUp, &bodyYaw, &bodyPitch, &bodyRoll, &bodySinRoll,
              &bodyCosRoll, &bodyCosPitch, &bodySinPitch);
    savedForward = bodyForward;
    savedUp = bodyUp;
    savedYaw = bodyYaw;
    savedPitch = bodyPitch;
    savedRoll = bodyRoll;
    savedSinRoll = bodySinRoll;
    savedCosRoll = bodyCosRoll;
    savedCosPitch = bodyCosPitch;
    savedSinPitch = bodySinPitch;
}

// slot 40 (0x00503de0, `ret 0x6c`).  Loads the character through the D3DIM base and runs the
// physics setup (slot 2).  Then it builds "<name without extension>.col"; if that file is
// found it becomes the collision shape, otherwise the shape comes from the node.
GameObject* SoultreePhysicsCharacter::UnknownVirtualSlot40(int a1, int a2, const char* a3,
                                                           const SoultreeLoadDesc* a4, int a5,
                                                           Vec3 a6, Vec3 a7,
                                                           Vec3 a8, void* a9, void* a10,
                                                           float a11, int a12, int a13,
                                                           SoultreeSlot1f0* a14, float a15,
                                                           int a16, float a17, float a18,
                                                           int a19, unsigned char a20, int a21)
{
    char name[0x104];
    char colPath[0x104];

    D3DIMSoultreeCharacter::D3DIMVirtualSlot11(a1, a3, a4, a5, 1, 1);
    poseNode = sceneNode = modelNode;
    SoultreePhysicsBaseObject::UnknownVirtualSlot2(a1, a2, a6, a7, a8, a9, a10, a11, a12, a13,
                                                   a14, a15, a16, a17, a18, 0.0f, 0, a19, a20,
                                                   a21);
    if (a4 && (a4->field_0x25 & 1)) {
        ((SoultreeD3DNode*)sceneNode)->Fn_4444c0(1);
    }

    int len = strlen(a3);
    int n = len > 0x103 ? 0x103 : len;
    strncpy(name, a3, n);
    name[n] = 0;
    strcpy(strrchr(name, '.'), ".col");

    SoultreeFile* file = new(__FILE__, 0x8cd) SoultreeFile(g_00572b44);
    if (!g_0056e26c->field_0x574->Fn_4e9cd0(file, name, "rb", colPath)) {
        colPath[0] = 0;
    }
    delete file;

    if (collisionObject) {
        if (colPath[0]) {
            collisionObject->Fn_00432800(sceneNode, colPath);
        } else {
            collisionObject->Fn_004324b0(sceneNode, 1, 0, 0, 0);
        }
        collisionObject->ownerType = 0;
        collisionObject->ownerObject = this;
        collisionObject->Fn_00435fe0();
        GameObject* child = collisionObject;
        GameObject::Method_0x00469190(child, -1);
    }
    return this;
}

// SoultreePhysicsObject slot 40 (0x00503970, `ret 0x70`).  The same loader for the D3DIM
// object base.  The differences are slot 9 instead of slot 11, line 0x84e instead of 0x8cd,
// and shape setup 0x00432720 instead of 0x004324b0.  SoultreePhysicsObject has two
// GameObjects, so field_0x08 and the GameObject call are qualified.
GameObject* SoultreePhysicsObject::UnknownVirtualSlot40(int a1, const char* a2,
                                                        const SoultreeLoadDesc* a3, int a4,
                                                        Vec3 a5, Vec3 a6,
                                                        Vec3 a7, void* a8, void* a9,
                                                        float a10, int a11, int a12,
                                                        SoultreeSlot1f0* a13, float a14,
                                                        int a15, float a16, float a17,
                                                        float a18, int a19, int a20,
                                                        unsigned char a21, int a22)
{
    char name[0x104];
    char colPath[0x104];

    D3DIMSoultreeObject::D3DIMObjectVirtualSlot9(a1, a2, a3, a4, 1);
    SoultreePhysicsBaseObject::UnknownVirtualSlot2(a1, 1, a5, a6, a7, a8, a9, a10, a11, a12,
                                                   a13, a14, a15, a16, a17, a18, a19, a20,
                                                   a21, a22);
    if (a3 && (a3->field_0x25 & 1)) {
        ((SoultreeD3DNode*)SoultreePhysicsBaseObject::sceneNode)->Fn_4444c0(1);
    }

    int len = strlen(a2);
    int n = len > 0x103 ? 0x103 : len;
    strncpy(name, a2, n);
    name[n] = 0;
    strcpy(strrchr(name, '.'), ".col");

    SoultreeFile* file = new(__FILE__, 0x84e) SoultreeFile(g_00572b44);
    if (!g_0056e26c->field_0x574->Fn_4e9cd0(file, name, "rb", colPath)) {
        colPath[0] = 0;
    }
    delete file;

    if (collisionObject) {
        if (colPath[0]) {
            collisionObject->Fn_00432800(SoultreePhysicsBaseObject::sceneNode, colPath);
        } else {
            collisionObject->Fn_00432720(SoultreePhysicsBaseObject::sceneNode, 1, 0, 0, 0);
        }
        collisionObject->ownerType = 0;
        collisionObject->ownerObject = this;
        collisionObject->Fn_00435fe0();
        GameObject* child = collisionObject;
        D3DIMSoultreeObject::Method_0x00469190(child, -1);
    }
    return static_cast<D3DIMSoultreeObject*>(this);
}
