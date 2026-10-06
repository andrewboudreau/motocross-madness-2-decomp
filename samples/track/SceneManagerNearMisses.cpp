// Near-miss SceneManager.cpp candidates, kept out of src/reconstructed until
// they match. See docs/SCENEMANAGER.md.
//
// Scene::UnknownFunction4ecd60 (0x004ecd60, 4543 bytes; 4534 of 4536 compared
// positions): the "StaticModels" reader. Everything but one store lines up,
// including the frame, the jump table and the EH states. The model name's
// terminator `field_0xb8->field_0x04[i].field_0x10[count] = 0` is
// `add edx,ebx; mov [edx+esi+0x10],0` in retail (element offset first) and
// `add edx,esi; mov [edx+ebx+0x10],0` here. Pointer and index spellings,
// the clamp forms, function-scope declarations, inline helpers and an
// element pointer local do not change it (helpers and locals make it worse).

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/reconstructed/AuralScape.h"
#include "../../src/reconstructed/LightEmitter.h"
#include "../../src/reconstructed/Net.h"
#include "../../src/reconstructed/PCAudio.h"
#include "../../src/reconstructed/SoultreeMaterial.h"
#include "../../src/reconstructed/TextureMap.h"
#include "../../src/reconstructed/Tgafile.h"
#include "../../src/reconstructed/SceneManager.h"

// The particle emitters a physics model may carry (0x004ecd60). Views of
// classes reconstructed in samples/physics/effects: GameObjects whose own
// slot 27 attaches them and returns the GameObject to add as a child.
class UnknownSceneParticleEmitter : public GameObject {
public:
    virtual GameObject* UnknownVirtualSlot27(void* owner, int a);
};
class UnknownSceneDustEmitter : public UnknownSceneParticleEmitter {
public:
    explicit UnknownSceneDustEmitter(int flags);       // 0x004b8a00

    unsigned char field_0x2c[0x80 - 0x2c];
};
class UnknownSceneChunkEmitter : public UnknownSceneParticleEmitter {
public:
    explicit UnknownSceneChunkEmitter(int flags);      // 0x004b8df0

    unsigned char field_0x2c[0x608 - 0x2c];
};
class UnknownSceneSprayEmitter : public UnknownSceneParticleEmitter {
public:
    explicit UnknownSceneSprayEmitter(int flags);      // 0x004b9310

    unsigned char field_0x2c[0x608 - 0x2c];
};

// The physics part of a "PhysicsObject" model (0x524 bytes, constructor
// 0x005037c0; src/krusty2/soultree's SoultreePhysicsObject family). Only the
// slots and fields 0x004ecd60 uses are declared: slot 37 attaches a particle
// emitter, slot 40 loads the model and returns the GameObject to add.
class UnknownScenePhysicsBody {
public:
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5();
    virtual void UnknownVirtualSlot6();
    virtual void UnknownVirtualSlot7();
    virtual void UnknownVirtualSlot8();
    virtual void UnknownVirtualSlot9();
    virtual void UnknownVirtualSlot10();
    virtual void UnknownVirtualSlot11();
    virtual void UnknownVirtualSlot12();
    virtual void UnknownVirtualSlot13();
    virtual void UnknownVirtualSlot14();
    virtual void UnknownVirtualSlot15();
    virtual void UnknownVirtualSlot16();
    virtual void UnknownVirtualSlot17();
    virtual void UnknownVirtualSlot18();
    virtual void UnknownVirtualSlot19();
    virtual void UnknownVirtualSlot20();
    virtual void UnknownVirtualSlot21();
    virtual void UnknownVirtualSlot22();
    virtual void UnknownVirtualSlot23();
    virtual void UnknownVirtualSlot24();
    virtual void UnknownVirtualSlot25();
    virtual void UnknownVirtualSlot26();
    virtual void UnknownVirtualSlot27();
    virtual void UnknownVirtualSlot28();
    virtual void UnknownVirtualSlot29();
    virtual void UnknownVirtualSlot30();
    virtual void UnknownVirtualSlot31();
    virtual void UnknownVirtualSlot32();
    virtual void UnknownVirtualSlot33();
    virtual void UnknownVirtualSlot34();
    virtual void UnknownVirtualSlot35();
    virtual void UnknownVirtualSlot36();
    virtual void UnknownVirtualSlot37(int type, GameObject* emitter, void* point, int a);
    virtual void UnknownVirtualSlot38();
    virtual void UnknownVirtualSlot39();
    virtual GameObject* UnknownVirtualSlot40(void* owner, const char* name, LightManager* lights, int a4,
                                             Vector3 position, Vector3 look, Vector3 up, int a8, int a9,
                                             float a10, int points, int emitters, Scene* scene, float a14,
                                             int a15, float a16, float a17, float radius, int shape,
                                             int minPoints, char a21, int a22);

    int field_0x04;
    void* field_0x08;                     // the scene node the collision points follow
    unsigned char field_0x0c[0x12c - 0x0c];
    void** field_0x12c;                   // collision points
    int field_0x130;                      // collision point count
    unsigned char field_0x134[0x21c - 0x134];
};
class UnknownScenePhysicsObject : public UnknownScenePhysicsBody, public D3DIMSoultreeObject {
public:
    UnknownScenePhysicsObject(int a, int b);         // 0x005037c0

    unsigned char field_0x4f4[0x524 - 0x4f4];
};

// CollisionObject (0xb8 bytes; ObjectPicker.h and src/krusty2/collision
// declare the same class). The model it collides for is at +0x60.
class UnknownSceneCollisionObject : public UnknownSoultreeQuadTreeObject, public GameObject {
public:
    explicit UnknownSceneCollisionObject(int flags);   // 0x00431e70
    void UnknownFunction4320f0(void* a, int b, int c, int d);  // 0x004320f0
    void UnknownFunction432720(D3DIMSoultreeObject* model, int a, int b, int c, int d); // 0x00432720
    void UnknownFunction432800(D3DIMSoultreeObject* model, const char* path); // 0x00432800: from a ".col" file
    void UnknownFunction435fe0();                      // 0x00435fe0

    unsigned char field_0x38[0x60 - 0x38];
    D3DIMSoultreeObject* field_0x60;
    unsigned char field_0x64[0xb8 - 0x64];
};

// 0x0043a330 (cdecl, CollisionPoint.cpp): adds a collision point at
// `position` to `points` (`*count` of `capacity`).
void* UnknownFunction43a330(int capacity, void** points, const Vector3* position, void* owner, float a4,
                            int* count, float friction, int a7);
// 0x004b5d00 (cdecl): the look and up vectors of a heading and pitch.
void UnknownFunction4b5d00(Vector3* look, Vector3* up, float heading, float pitch, int a);

// An "ObjectShape" (0x24 bytes) or "EmitterType<n>" (0x1c bytes) keyword.
struct UnknownSceneShapeKeyword {
    char name[0x20];
    int value;
};
struct UnknownSceneEmitterKeyword {
    char name[0x18];
    int value;
};

// The value of keyword `name` (0 when not listed).
static inline int FindShape(const char* name, const UnknownSceneShapeKeyword* table)
{
    for (int i = 0; i < 2; i++) {
        if (!_stricmp(name, table[i].name))
            return table[i].value;
    }
    return 0;
}
static inline int FindEmitter(const char* name, const UnknownSceneEmitterKeyword* table)
{
    for (int i = 0; i < 4; i++) {
        if (!_stricmp(name, table[i].name))
            return table[i].value;
    }
    return 0;
}

// 0x004ecd60: see the declaration. A model whose ".seg" name is the
// stadium's gets no collision object; its ".col" file is used when the
// archive has one.
int Scene::UnknownFunction4ecd60(LightManager* lights, int a2, int a3, int a4, void (*progress)(int),
                                 int interval)
{
    int models;
    int next;
    int stadium;
    int points;
    int emitters;
    int minPoints;
    int physics;
    int collision;
    int useLighting;
    int oneShot;
    int force2D;
    unsigned long flags;
    float radius;
    float heading;
    float pitch;
    float friction;
    float oneShotDistance;
    float randomTriggerPercent;
    Vector3 position;
    Vector3 look;
    Vector3 up;
    UnknownSound3DParameters params;
    char model[0x44];
    char key[0x40];
    char text[0x204];
    char slt[0x108];
    char sound[0x104];
    char sltPath[0x104];
    char message[0x204];
    char path[0x104];
    char colPath[0x104];

    models = 0;
    field_0xb8 = 0;
    int found = field_0xdc.UnknownFunction4b78f0("StaticModels");
    if (field_0x2c->field_0x38c && found) {
        if (!field_0xdc.UnknownFunction4b7f10("NumberOfStaticModels", -1, &models)) {
            sprintf(text, "\nNumberOfStaticModels cannot be found under [%s] in %s.\n\n", "StaticModels",
                    field_0x7c8);
            UnknownFunction464e80(text);
            return 0;
        }
        if (models > 0) {
    UnknownTextureStream* stream =
        new(__FILE__, 1749) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    field_0xb8 = new(__FILE__, 1751) UnknownSceneBuffer;
    field_0xb8->field_0x00 = models;
    field_0xb8->field_0x04 = new(__FILE__, 1753) UnknownSceneCaster[field_0xb8->field_0x00];
    for (int i = 0; i < field_0xb8->field_0x00; i = next) {
        if (i % interval == interval - 1 && progress)
            progress(0);
        next = i + 1;
        sprintf(model, "Model%d", next);
        field_0xdc.UnknownFunction4b78f0(model);
        if (!field_0xdc.UnknownFunction4b7ec0("SLT", "", slt, 0x104)) {
            sprintf(message, "\nCannot find %s for [%s] in %s.\n\n", slt, model, field_0x7c8);
            UnknownFunction464e80(message);
            return 0;
        }
        strcpy(path, slt);
        stadium = 0;
        strcpy(strrchr(path, '.'), ".seg");
        if (!_stricmp(field_0x6bc, path))
            stadium = 1;
        sprintf(sltPath, "%s\\%s", "Res", slt);
        strcpy(strrchr(slt, '.'), ".col");
        sprintf(path, "%s\\%s", "Res", slt);
        if (!g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9cd0(stream, path, "rb", (int)colPath)) {
            strcpy(strrchr(slt, '.'), "");
            colPath[0] = 0;
        }
        UnknownFunction4eb160(&position, model, "Position", "0.0,0.0,0.0", 0, 0);
        if (field_0xdc.UnknownFunction4b7f40("Heading", -999.0f, &heading)) {
            field_0xdc.UnknownFunction4b7f40("Pitch", 0, &pitch);
            UnknownFunction4b5d00(&look, &up, heading * 0.01745329f, pitch * 0.01745329f, 0);
        } else {
            UnknownFunction4eb160(&look, model, "LookVector", "0.0,0.0,1.0", 0, 0);
            UnknownFunction4eb160(&up, model, "UpVector", "0.0,1.0,0.0", 0, 0);
        }
        field_0xb8->field_0x04[i].field_0x0c = 0;
        field_0xdc.UnknownFunction4b7f10("PhysicsObject", 0, &physics);
        if (physics) {
            field_0xdc.UnknownFunction4b7f10("NumberOfCollisionPoints", 0, &points);
            field_0xdc.UnknownFunction4b7f10("NumberOfEmitters", 0, &emitters);
            field_0xdc.UnknownFunction4b7f10("MinCollisionPointsToRestOn", 1, &minPoints);
            field_0xdc.UnknownFunction4b7f40("Radius", 1.0f, &radius);
            field_0xdc.UnknownFunction4b7ec0("ObjectShape", "", text, 0x80);
            UnknownSceneShapeKeyword shapes[2] = {{"Box", 0}, {"Sphere", 1}};
            int shape = FindShape(text, shapes);
            if (minPoints >= points - 1)
                minPoints = points - 1;
            UnknownScenePhysicsObject* object = new(__FILE__, 1845) UnknownScenePhysicsObject(1, 1);
            UnknownFunction469190(object->UnknownVirtualSlot40(field_0x18, sltPath, lights, a4, position, look, up,
                                                               0, a2, 5.0f, points, emitters, this,
                                                               0.033333f, 10, 0.025f, 0.05f, radius, shape,
                                                               minPoints, field_0x88, 1),
                                  -1);
            field_0xb8->field_0x04[i].field_0x08 = object;
            field_0xb8->field_0x04[i].field_0x04 = (ShadowCaster*)(D3DIMSoultreeObject*)object;
            field_0xb8->field_0x04[i].physics = 1;
            if (points > 0) {
                for (int p = 0; p < points; p++) {
                    sprintf(key, "CollisionPoint%d", p + 1);
                    UnknownFunction4eb160(&position, model, key, "0.0,0.0,0.0", 0, 0);
                    sprintf(key, "FrictionCoefficient%d", p + 1);
                    field_0xdc.UnknownFunction4b7f40(key, 0.6f, &friction);
                    UnknownFunction43a330(points, object->field_0x12c, &position,
                                          object->UnknownScenePhysicsBody::field_0x08, 0, &object->field_0x130,
                                          friction, (int)this);
                }
            if (a2) {
                int dust = 0;
                int chunk = 0;
                int spray = 0;
                int nextEmitter;
                for (int e = 0; e < emitters; e = nextEmitter) {
                    nextEmitter = e + 1;
                    sprintf(key, "EmitterType%d", nextEmitter);
                    field_0xdc.UnknownFunction4b7ec0(key, "", text, 0x17);
                    UnknownSceneEmitterKeyword types[4] = {
                        {"DustEmitter", 1}, {"DirtChunkEmitter", 2}, {"DirtSprayEmitter", 3},
                        {"ExhaustEmitter", 4},
                    };
                    switch (FindEmitter(text, types)) {
                    case 1:
                        if (dust) {
                            sprintf(message, "\nOnly ONE %s for [%s] allowed.\n\n", text, model);
                            UnknownFunction464e80(message);
                        } else {
                            UnknownSceneDustEmitter* emitter = new(__FILE__, 1917) UnknownSceneDustEmitter(1);
                            D3DIMSoultreeObject* node = object;
                            node->UnknownFunction469190(emitter->UnknownVirtualSlot27(field_0x18, a2), -1);
                            object->UnknownVirtualSlot37(1, emitter, *object->field_0x12c, 0);
                            dust = 1;
                        }
                        break;
                    case 2:
                        if (chunk) {
                            sprintf(message, "\nOnly ONE %s for [%s] allowed.\n\n", text, model);
                            UnknownFunction464e80(message);
                        } else {
                            UnknownSceneChunkEmitter* emitter = new(__FILE__, 1927) UnknownSceneChunkEmitter(1);
                            D3DIMSoultreeObject* node = object;
                            node->UnknownFunction469190(emitter->UnknownVirtualSlot27(field_0x18, a2), -1);
                            object->UnknownVirtualSlot37(2, emitter, *object->field_0x12c, 0);
                            chunk = 1;
                        }
                        break;
                    case 3:
                        if (spray) {
                            sprintf(message, "\nOnly ONE %s for [%s] allowed.\n\n", text, model);
                            UnknownFunction464e80(message);
                        } else {
                            UnknownSceneSprayEmitter* emitter = new(__FILE__, 1937) UnknownSceneSprayEmitter(1);
                            D3DIMSoultreeObject* node = object;
                            node->UnknownFunction469190(emitter->UnknownVirtualSlot27(field_0x18, a2), -1);
                            object->UnknownVirtualSlot37(3, emitter, *object->field_0x12c, 0);
                            spray = 1;
                        }
                        break;
                    case 4:
                        break;
                    default:
                        sprintf(message, "\nUnknown EmitterType (%s) for [%s].\n\n", text, model);
                        UnknownFunction464e80(message);
                        break;
                    }
                }
            }
            }
        } else {
            field_0xb8->field_0x04[i].field_0x08 = 0;
            D3DIMSoultreeObject* object = new(__FILE__, 1956) D3DIMSoultreeObject(1);
            field_0xb8->field_0x04[i].field_0x04 = (ShadowCaster*)object;
            UnknownFunction469190(object->UnknownVirtualSlot9(field_0x18, sltPath, (int)lights, a4, 1), -1);
            int length = strcspn(sltPath, ".");
            int count = length < 0x14 ? length : 0x14;
            strncpy(field_0xb8->field_0x04[i].field_0x10, sltPath, count);
            field_0xb8->field_0x04[i].field_0x10[count] = 0;
            ((D3DIMSoultreeObject*)field_0xb8->field_0x04[i].field_0x04)->UnknownFunction4fc630(position);
            ((D3DIMSoultreeObject*)field_0xb8->field_0x04[i].field_0x04)->UnknownFunction4fbd70(&look, &up, 1, 0);
            field_0xdc.UnknownFunction4b7f10("Collision", 1, &collision);
            field_0xb8->field_0x04[i].field_0x0c = 0;
            if (collision && !stadium
                && ((D3DIMSoultreeObject*)field_0xb8->field_0x04[i].field_0x04)->UnknownFunction4fda30() == 1) {
                g_MemTagStack->Push("Collision");
                field_0xb8->field_0x04[i].field_0x0c = new(__FILE__, 1974) UnknownSceneCollisionObject(1);
                field_0xb8->field_0x04[i].field_0x0c->UnknownFunction4320f0(field_0x18, 1, 0, 1);
                field_0xb8->field_0x04[i].field_0x0c->field_0x60 =
                    (D3DIMSoultreeObject*)field_0xb8->field_0x04[i].field_0x04;
                if (!colPath[0])
                    field_0xb8->field_0x04[i].field_0x0c->UnknownFunction432720(
                        (D3DIMSoultreeObject*)field_0xb8->field_0x04[i].field_0x04, 0, 0, 0, 0);
                else
                    field_0xb8->field_0x04[i].field_0x0c->UnknownFunction432800(
                        (D3DIMSoultreeObject*)field_0xb8->field_0x04[i].field_0x04, colPath);
                field_0xb8->field_0x04[i].field_0x0c->UnknownFunction435fe0();
                ((D3DIMSoultreeObject*)field_0xb8->field_0x04[i].field_0x04)
                    ->UnknownFunction469190(field_0xb8->field_0x04[i].field_0x0c, -1);
                g_MemTagStack->Push("Scene");
            }
            field_0xb8->field_0x04[i].physics = 0;
        }
        field_0xdc.UnknownFunction4b7f10("UseLighting", 1, &useLighting);
        field_0xb8->field_0x04[i].useLighting = useLighting;
        ((D3DIMSoultreeObject*)field_0xb8->field_0x04[i].field_0x04)
            ->UnknownFunction4444c0(field_0xb8->field_0x04[i].useLighting);
        if (field_0xc0) {
            field_0xdc.UnknownFunction4b7ec0("SoundResourceFile", "", sound, 0x103);
            if (strcmp(sound, "") && _stricmp(sound, "NONE")) {
                UnknownFunction4ef9c0(model, sound, 1, &params, &flags, &oneShotDistance, &randomTriggerPercent,
                                      &oneShot, &force2D);
                SoultreeSoundEmitter* emitter =
                    (new(__FILE__, 2015)
                         SoultreeSoundEmitter((AuralScape*)field_0xc0, field_0xc4, 1, field_0x25_bit0))
                        ->UnknownFunction4029f0(field_0x18, sound, "",
                                                (UnknownVehiclePart*)field_0xb8->field_0x04[i].field_0x04,
                                                params, flags, oneShotDistance, randomTriggerPercent, 0,
                                                force2D);
                UnknownFunction469190(emitter, -1);
                if (!emitter) {
                    sprintf(text, "\nScene::SoundEmitter(%s) not created\n", sound);
                    UnknownFunction464e80(text);
                }
            }
        } else {
            sprintf(text, "\nScene:  No AuralScape, so NO Static Model SOUNDS!\n");
            UnknownFunction464e80(text);
        }
    }
    delete stream;
    UnknownFunction464e80("");
    return 1;
        }
        sprintf(text, "\nNumber of models<1 in %s.\n\n", "StaticModels", field_0x7c8);
        UnknownFunction464e80(text);
        return 0;
    }
    sprintf(text, "\nCannot find data for [%s] in %s.\n\n", "StaticModels", field_0x7c8);
    UnknownFunction464e80(text);
    return 0;
}
