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
// The order follows the spill of the element offset (ebx, stored at
// [esp+0x4c] around the physics branch and reloaded in the sound section,
// as in retail): builds without that spill (the physics branch, the
// collision block or the sound section removed) emit retail's
// `add edx, ebx`, and a small probe with the same statements does too.
// Declaration order, `count`/`length`/`i` types (int, long, unsigned,
// register), `'\0'`, `*(p + count)`, extra uses of `count` and the
// /G3-/G6, /GB, /Gy, /Gf, /Zp, /Op, /Ox, /vm* flags leave it.

//
// Scene::UnknownFunction4edfe0 (0x004edfe0, 5343 bytes): the "Animations"
// reader. Code, calls, EH states, the error tails and the frame size
// (0xcfc) line up except: (1) the frame slots of most scalars and of
// `list`/`sltPath` (VC6 orders the frame by use, not declaration; the
// arrays follow retail once `dot` reuses `automatic` and the random-set loop
// reuses `i`); (2) the "NumberOfMotions" test keeps the count in a register
// for the allocation (retail compares memory against esi and reloads it);
// (3) the "Cannot find %s" errors share one sprintf tail where retail keeps
// two (VUE/SLT and MCF/MotionToPlay/RandomSet); (4) the random-set loop end
// loads in a different order. 4372 of 5346 compared positions.
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

// 0x004edfe0: see the declaration. Reads "Animations": an entry per
// "Animation<n>", either a key-framed character (MCF file, motions and a
// motion sequence) or a procedural car (SLT and VUE files), each with an
// optional sound emitter; then the "RandomSet<n>" sections.
int Scene::UnknownFunction4edfe0(LightManager* lights, int a3, int a4, void (*progress)(int), int interval)
{
    int automatic;
    float fps;
    int animations;
    int i;
    int frontWheelsTurn;
    float lagDistance;
    int next;
    int tires;
    int useLighting;
    float randomTriggerPercent;
    float oneShotDistance;
    int exclude;
    unsigned long flags;
    float heading;
    float pitch;
    int force2D;
    char name[0x40];
    Vector3 up;
    Vector3 look;
    int oneShot;
    char motion[0x14];
    char number[0x18];
    char setName[0x44];
    char key[0x44];
    char message[0x204];
    char file[0x108];
    char vue[0x40];
    char text[0x204];
    UnknownSound3DParameters params;
    char colPath[0x104];
    char sound[0x184];
    char sequence[0x80];
    char attach[0x84];
    char list[0x84];
    char sltPath[0x104];
    char vuePath[0x104];

    animations = 0;
    int found = field_0xdc.UnknownFunction4b78f0("Animations");
    if (field_0x2c->field_0x38c && found) {
        if (!field_0xdc.UnknownFunction4b7f10("NumberOfAnimations", -1, &animations)) {
            sprintf(text, "\nNumberOfAnimations cannot be found under [%s] in %s.\n\n", "Animations",
                    field_0x7c8);
            UnknownFunction464e80(text);
            return 0;
        }
        if (animations > 0) {
    field_0xb4 = new(__FILE__, 2178) UnknownSceneTable;
    field_0xb4->field_0x00 = animations;
    UnknownFunction4de580("Animations", "NumberOfAnimations", animations);
    field_0xb4->field_0x04 = new(__FILE__, 2181) UnknownSceneEntry[animations];
    for (i = 0; i < field_0xb4->field_0x00; i = next) {
        if (i % interval == interval - 1 && progress)
            progress(0);
        field_0xb4->field_0x04[i].field_0x00_bit0 = 0;
        next = i + 1;
        sprintf(name, "Animation%d", next);
        field_0xdc.UnknownFunction4b78f0(name);
        if (!field_0xdc.UnknownFunction4b7ec0("AnimationType", "", text, 0x40)) {
            sprintf(message, "\nScene: AnimationType not specified for anim#%d.  Using KeyFramed.\n", i);
            UnknownFunction464e80(message);
            field_0xb4->field_0x04[i].field_0x00_bit3 = 1;
        } else if (!_strnicmp(text, "Procedural", 3)) {
            field_0xb4->field_0x04[i].field_0x00_bit3 = 0;
            if (!field_0xdc.UnknownFunction4b7ec0("VUE", "", vue, 0x40)) {
                sprintf(message, "\nCannot find %s for [%s] in %s.\n\n", "VUE", name, field_0x7c8);
                UnknownFunction464e80(message);
                return 0;
            }
            sprintf(text, "%s\\%s", "Res", vue);
            UnknownTextureStream* stream =
                new(__FILE__, 2216) UnknownTextureStream((int)g_UnknownResourceManager572b44);
            if (!g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9cd0(stream, text, "rb", (int)vuePath)) {
                sprintf(sound, "No procedural vue file found in resources.  Aborting.\n");
                delete stream;
                return 0;
            }
            delete stream;
            UnknownFunction4de580(name, "VUE", vuePath);
            if (!field_0xdc.UnknownFunction4b7f40("FPS", 30.0f, &fps))
                UnknownFunction4de580(name, "FPS(Default)", fps);
            else
                UnknownFunction4de580(name, "FPS", fps);
            if (!field_0xdc.UnknownFunction4b7f40("LagDistance", 0, &lagDistance))
                UnknownFunction4de580(name, "LagDistance(Default)", lagDistance);
            else
                UnknownFunction4de580(name, "LagDistance", lagDistance);
            if (!field_0xdc.UnknownFunction4b7f10("FrontWheelsTurn", 1, &frontWheelsTurn)) {
                UnknownFunction4de580(name, "FrontWheelsTurn(Default)", frontWheelsTurn);
                if (lagDistance == 0)
                    frontWheelsTurn = 1;
                else
                    frontWheelsTurn = 0;
            } else {
                UnknownFunction4de580(name, "FrontWheelsTurn", frontWheelsTurn);
            }
            if (!field_0xdc.UnknownFunction4b7f10("NumberOfTires", 4, &tires))
                UnknownFunction4de580(name, "NumberOfTires(Default)", tires);
            else
                UnknownFunction4de580(name, "NumberOfTires", tires);
        } else {
            field_0xb4->field_0x04[i].field_0x00_bit3 = 1;
        }
        field_0xb4->field_0x04[i].field_0x00_bit5 = 0;
        if (field_0xb4->field_0x04[i].field_0x00_bit3 && !field_0xdc.UnknownFunction4b7ec0("MCF", "", file, 0x104)) {
            sprintf(message, "\nCannot find %s for [%s] in %s.\n\n", "MCF", name, field_0x7c8);
            UnknownFunction464e80(message);
            return 0;
        }
        if (!field_0xb4->field_0x04[i].field_0x00_bit3 && !field_0xdc.UnknownFunction4b7ec0("SLT", "", file, 0x104)) {
            sprintf(message, "\nCannot find %s for [%s] in %s.\n\n", "SLT", name, field_0x7c8);
            UnknownFunction464e80(message);
            return 0;
        }
        sprintf(sltPath, "%s\\%s", "Res", file);
        if (field_0xb4->field_0x04[i].field_0x00_bit3)
            UnknownFunction4f0ec0(file);
        int length = strcspn(file, ".");
        int count = length < 0x14 ? length : 0x14;
        strncpy(field_0xb4->field_0x04[i].field_0x2c, file, count);
        field_0xb4->field_0x04[i].field_0x2c[count] = 0;
        length = strlen(file);
        int size = length > 0x103 ? 0x103 : length;
        strncpy(colPath, file, size);
        colPath[size] = 0;
        strcpy(strrchr(colPath, '.'), ".col");
        UnknownTextureStream* stream =
            new(__FILE__, 2281) UnknownTextureStream((int)g_UnknownResourceManager572b44);
        sprintf(text, "%s\\%s", "Res", colPath);
        if (!g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9cd0(stream, text, "rb", (int)colPath))
            colPath[0] = 0;
        delete stream;
        if (!UnknownFunction4eb160(&field_0xb4->field_0x04[i].field_0x0c, name, "Position", "0.0,0.0,0.0", 0, 0))
            UnknownFunction4eb160(&field_0xb4->field_0x04[i].field_0x0c, name, "Offset", "0.0,0.0,0.0", 0, 0);
        if (field_0xb4->field_0x04[i].field_0x00_bit3) {
            UnknownSceneObject* character = new(__FILE__, 2300) UnknownSceneObject(1);
            field_0xb4->field_0x04[i].field_0x04 = character;
            if (!UnknownFunction469190(character->UnknownFunction4319c0(field_0x18, sltPath, colPath, lights, a4, 1, 1),
                                       -1)) {
                sprintf(message, "\nScene: Cannot create character from %s.\n\n", file);
                UnknownFunction464e80(message);
                return 0;
            }
        }
        if (!field_0xb4->field_0x04[i].field_0x00_bit3) {
            UnknownSceneAnimatedObject* car = new(__FILE__, 2305) UnknownSceneAnimatedObject(1);
            field_0xb4->field_0x04[i].field_0x08 = car;
            if (!UnknownFunction469190(car->UnknownVirtualSlot27(field_0x18, sltPath, colPath, lights, a4, 0, vuePath,
                                                                 &field_0xb4->field_0x04[i].field_0x0c, fps,
                                                                 frontWheelsTurn, lagDistance, 1.0f, tires, 0, 0, 0),
                                       -1)) {
                sprintf(message, "\nScene: Cannot create procedural model from %s.\n\n", file);
                UnknownFunction464e80(message);
                return 0;
            }
        }
        g_MemTagStack->Push("Scene");
        field_0xdc.UnknownFunction4b7f10("UseLighting", 1, &useLighting);
        int axes = 0;
        if (field_0xdc.UnknownFunction4b7f40("Heading", -999.0f, &heading)) {
            field_0xdc.UnknownFunction4b7f40("Pitch", 0, &pitch);
            UnknownFunction4b5d00(&look, &up, heading * 0.01745329f, pitch * 0.01745329f, 0);
            axes = 1;
        } else if (UnknownFunction4eb160(&look, name, "LookVector", "0.0,0.0,1.0", 0, 0)) {
            UnknownFunction4eb160(&up, name, "UpVector", "0.0,1.0,0.0", 0, 0);
            axes = 1;
        }
        field_0xdc.UnknownFunction4b7f10("ExcludeFromCameraIteration", 0, &exclude);
        field_0xb4->field_0x04[i].field_0x00_bit4 = exclude;
        if (field_0xb4->field_0x04[i].field_0x00_bit3) {
            UnknownFunction4de580(name, "MCF", file);
            field_0xb4->field_0x04[i].field_0x04->field_0x1a0->UnknownFunction4fc660(&field_0xb4->field_0x04[i].field_0x0c);
            if (axes)
                field_0xb4->field_0x04[i].field_0x04->field_0x1a0->UnknownFunction4fbd70(&look, &up, 1, 0);
            field_0xb4->field_0x04[i].field_0x04->field_0x1a0->UnknownFunction4444c0(useLighting);
            field_0xdc.UnknownFunction4b7f10("UseBlendedKeys", 1, &automatic);
            field_0xb4->field_0x04[i].field_0x00_bit1 = automatic;
            field_0xdc.UnknownFunction4b7f10("NumberOfMotions", 0, &field_0xb4->field_0x04[i].field_0x1c);
            field_0xb4->field_0x04[i].field_0x20 = 0;
            field_0xb4->field_0x04[i].field_0x28 = 0;
            if (field_0xb4->field_0x04[i].field_0x1c > 0) {
                field_0xb4->field_0x04[i].field_0x20 = new(__FILE__, 2357) void*[field_0xb4->field_0x04[i].field_0x1c];
                for (int m = 0; m < field_0xb4->field_0x04[i].field_0x1c; m++) {
                    sprintf(key, "MotionToPlay%d", m + 1);
                    if (!field_0xdc.UnknownFunction4b7ec0(key, "", motion, 0x10)) {
                        sprintf(message, "\nCannot find %s for [%s] in %s.\n\n", key, name, field_0x7c8);
                        UnknownFunction464e80(message);
                        return 0;
                    }
                    UnknownFunction4de580(name, key, motion);
                    field_0xb4->field_0x04[i].field_0x20[m] =
                        field_0xb4->field_0x04[i].field_0x04->UnknownFunction4a6b30(motion, 1);
                }
                if (field_0xdc.UnknownFunction4b7ec0("MotionSequence", "", sequence, 0x40)) {
                    char* token = strtok(sequence, ",");
                    int j = 0;
                    field_0xdc.UnknownFunction4b7ec0("NumberInSequence", "", number, 0x10);
                    field_0xb4->field_0x04[i].field_0x25 = atoi(number);
                    if (field_0xb4->field_0x04[i].field_0x25) {
                        field_0xb4->field_0x04[i].field_0x28 =
                            new(__FILE__, 2379) char[field_0xb4->field_0x04[i].field_0x25];
                        for (; token && j < field_0xb4->field_0x04[i].field_0x25; j++) {
                            field_0xb4->field_0x04[i].field_0x28[j] = atoi(token);
                            token = strtok(0, ",");
                        }
                        field_0xdc.UnknownFunction4b7f10("AutomaticSequence", 1, &automatic);
                        field_0xb4->field_0x04[i].field_0x00_bit2 = automatic;
                        field_0xb4->field_0x04[i].field_0x26 = field_0xb4->field_0x04[i].field_0x28[0];
                        if (field_0xb4->field_0x04[i].field_0x26 >= field_0xb4->field_0x04[i].field_0x25)
                            field_0xb4->field_0x04[i].field_0x26 = 0;
                        field_0xb4->field_0x04[i].field_0x24 = field_0xb4->field_0x04[i].field_0x26;
                        UnknownFunction4eb040(i, field_0xb4->field_0x04[i].field_0x26, 0, 0);
                    } else {
                        field_0xb4->field_0x04[i].field_0x26 = 0;
                        sprintf(message, "\n%s for [%s] in %s is ZERO!\n\n", "NumberInSequence", name, field_0x7c8);
                        UnknownFunction464e80(message);
                        return 0;
                    }
                } else {
                    field_0xb4->field_0x04[i].field_0x24 = 0;
                    field_0xb4->field_0x04[i].field_0x25 = 0;
                    sprintf(message, "\n%s for [%s] in %s is ZERO!\n\n", "MotionSequence", name, field_0x7c8);
                    UnknownFunction464e80(message);
                    return 0;
                }
            }
        } else {
            UnknownFunction4de580(name, "SLT", sltPath);
            field_0xb4->field_0x04[i].field_0x1c = 1;
            field_0xb4->field_0x04[i].field_0x08->field_0x34->UnknownFunction4444c0(useLighting);
            if (axes)
                field_0xb4->field_0x04[i].field_0x08->field_0x34->UnknownFunction4fbd70(&look, &up, 1, 0);
        }
        if (field_0xb4->field_0x04[i].field_0x1c > 0) {
            if (field_0xc0) {
                field_0xdc.UnknownFunction4b7ec0("SoundResourceFile", "", sound, 0x103);
                if (strcmp(sound, "") && _stricmp(sound, "NONE")) {
                    UnknownFunction4ef9c0(name, sound, 0, &params, &flags, &oneShotDistance, &randomTriggerPercent,
                                          &oneShot, &force2D);
                    field_0xdc.UnknownFunction4b7ec0("SoundAttachToObject", "", attach, 0x40);
                    UnknownSceneLodObject* node;
                    if (field_0xb4->field_0x04[i].field_0x00_bit3)
                        node = field_0xb4->field_0x04[i].field_0x04->field_0x1a0;
                    else
                        node = field_0xb4->field_0x04[i].field_0x08->field_0x34;
                    SoultreeSoundEmitter* emitter =
                        (new(__FILE__, 2433)
                             SoultreeSoundEmitter((AuralScape*)field_0xc0, field_0xc4, 1, field_0x25_bit0))
                            ->UnknownFunction4029f0(field_0x18, sound, attach, (UnknownVehiclePart*)node, params,
                                                    flags, oneShotDistance, randomTriggerPercent, 0, force2D);
                    if (!emitter) {
                        sprintf(text, "\nScene::SoundEmitter(%s) not created\n", sound);
                        UnknownFunction464e80(text);
                    } else {
                        UnknownFunction469190(emitter, -1);
                    }
                }
            } else {
                sprintf(text, "\nScene:  No AuralScape, so NO Animation SOUNDS!\n");
                UnknownFunction464e80(text);
            }
        }
    }
    field_0xdc.UnknownFunction4b78f0("Animations");
    field_0xdc.UnknownFunction4b7f10("NumberOfRandomSets", 0, &field_0xb4->field_0x08);
    if (field_0xb4->field_0x08 > 0) {
        field_0xb4->field_0x0c = new(__FILE__, 2466) UnknownSceneEntry2[field_0xb4->field_0x08];
        for (i = 0; i < field_0xb4->field_0x08; i = next) {
            field_0xb4->field_0x0c[i].field_0x08 = 0;
            field_0xb4->field_0x0c[i].field_0x0c = 0;
            next = i + 1;
            sprintf(setName, "RandomSet%d", next);
            if (!field_0xdc.UnknownFunction4b78f0(setName)) {
                sprintf(message, "\nCannot find [%s] in %s.\n\n", setName, field_0x7c8);
                UnknownFunction464e80(message);
                return 0;
            }
            if (!field_0xdc.UnknownFunction4b7ec0("RandomSet", "", list, 0x80)) {
                sprintf(message, "\nCannot find %s for [%s] in %s.\n\n", "RandomSet", setName, field_0x7c8);
                UnknownFunction464e80(message);
                return 0;
            }
            char* token = strtok(list, ",");
            int j = 0;
            field_0xb4->field_0x0c[i].field_0x04 = 0;
            field_0xdc.UnknownFunction4b7ec0("NumberInSequence", 0, (char*)&field_0xb4->field_0x0c[i].field_0x00, -1);
            if (field_0xb4->field_0x0c[i].field_0x00) {
                field_0xb4->field_0x0c[i].field_0x08 = new(__FILE__, 2491) signed char[field_0xb4->field_0x0c[i].field_0x00];
                field_0xb4->field_0x0c[i].field_0x0c = new(__FILE__, 2492) signed char[field_0xb4->field_0x0c[i].field_0x00];
                for (; token && j < field_0xb4->field_0x0c[i].field_0x00; j++) {
                    automatic = strcspn(token, ".");
                    if (!automatic || automatic == (int)strlen(token)) {
                        sprintf(message, "\nInvalid Animation or Motion #%d in [%s] in %s.\n\n", j, setName,
                                field_0x7c8);
                        UnknownFunction464e80(message);
                        return 0;
                    }
                    token[automatic] = 0;
                    field_0xb4->field_0x0c[i].field_0x08[j] = atoi(token) - 1;
                    field_0xb4->field_0x04[field_0xb4->field_0x0c[i].field_0x08[j]].field_0x00_bit0 = 1;
                    if (field_0xb4->field_0x04[i].field_0x00_bit3)
                        field_0xb4->field_0x04[field_0xb4->field_0x0c[i].field_0x08[j]].field_0x04->UnknownVirtualSlot4();
                    else
                        field_0xb4->field_0x04[field_0xb4->field_0x0c[i].field_0x08[j]].field_0x08->UnknownVirtualSlot4();
                    field_0xb4->field_0x0c[i].field_0x0c[j] = atoi(token + automatic + 1) - 1;
                    token = strtok(0, ",");
                }
                field_0xb4->field_0x0c[i].field_0x04 = field_0xb4->field_0x04;
            }
        }
    } else {
        field_0xb4->field_0x0c = 0;
    }
    UnknownFunction464e80("");
    return 1;
        }
        sprintf(text, "\nNumber of animations<1 in %s.\n\n", "Animations", field_0x7c8);
        UnknownFunction464e80(text);
        return 0;
    }
    sprintf(text, "\nCannot find data for [%s] in %s.\n\n", "Animations", field_0x7c8);
    UnknownFunction464e80(text);
    return 0;
}
