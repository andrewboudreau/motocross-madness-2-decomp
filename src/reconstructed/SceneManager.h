#pragma once

#include <stdio.h>

#include "ContainerList.h"
#include "GameObject.h"
#include "MatrixUtil.h"
#include "Parameterblocks.h"
#include "TrackGame.h"

class Sound;
class SoundGroup;
struct UnknownSound3DParameters;

// The object a scene entry holds. Tier 2: 0x004eb040 calls 0x004a8b40
// (D3DIMSoultreeCharacter::SetMotion in src/krusty2/motion) on it, whose
// +0x1a4 buffer 0x004eaec0 compares as the name. Only that is declared here.
struct UnknownSceneObject {
    unsigned char field_0x000[0x1a4];
    char field_0x1a4[0x6c];
};

// 0x44-byte element of UnknownSceneTable::field_0x04. The flag byte at +0
// is read and written as single bits (0x004ea880 tests bit 3, 0x004eafd0
// sets bit 2).
struct UnknownSceneEntry {
    unsigned char field_0x00_bit0 : 1;
    unsigned char field_0x00_bit1 : 1;
    unsigned char field_0x00_bit2 : 1;
    unsigned char field_0x00_bit3 : 1;
    unsigned char field_0x01[3];
    UnknownSceneObject* field_0x04;
    void* field_0x08;
    unsigned char field_0x0c[0x1c - 0x0c];
    int field_0x1c;
    void* field_0x20;
    int field_0x24;
    void* field_0x28;
    unsigned char field_0x2c[0x44 - 0x2c];
};

// 0x10-byte element of UnknownSceneTable::field_0x0c.
struct UnknownSceneEntry2 {
    signed char field_0x00;
    unsigned char field_0x01[3];
    int field_0x04;
    void* field_0x08;
    void* field_0x0c;
};

// Owned by Scene+0xb4; freed with its arrays by the destructor.
struct UnknownSceneTable {
    int field_0x00;                       // entry count
    UnknownSceneEntry* field_0x04;
    int field_0x08;                       // entry2 count
    UnknownSceneEntry2* field_0x0c;
};

class ShadowCaster;
class ProjectedShadow;

// 0x28-byte element of UnknownSceneBuffer::field_0x04: a shadow caster
// (0x004edf20 attaches a D3DIMSoultreeShadow to each).
struct UnknownSceneCaster {
    unsigned char field_0x00;
    unsigned char field_0x01[3];
    ShadowCaster* field_0x04;
    unsigned char field_0x08[0x28 - 0x08];
};

// Owned by Scene+0xb8: the shadow casters.
struct UnknownSceneBuffer {
    int field_0x00;                       // caster count
    UnknownSceneCaster* field_0x04;
};

// Minimal view of RTTI class D3DIMSoultreeShadow (0x48 bytes), declared in
// full in src/krusty2/shadow/D3DIMSoultreeShadow.h, whose header tree cannot
// be mixed with this one. Only what Scene 0x004edf20 calls is declared.
class D3DIMSoultreeShadow : public GameObject {
public:
    explicit D3DIMSoultreeShadow(int flags);                       // 0x00446840
    D3DIMSoultreeShadow* Attach(int host, ShadowCaster* caster, ProjectedShadow* shadow); // 0x004468b0

    unsigned char field_0x2c[0x48 - 0x2c];
};

// What 0x004f00e0 counts: a record with a texture name at +0x2c, built by
// its two callers (0x004f0390, 0x004f0ec0) on the stack. Only the name is
// known; its extent is provisional.
struct UnknownSceneTextureInfo {
    unsigned char field_0x00[0x2c];
    char field_0x2c[0x40];
};

// Scene+0xb0, allocated by 0x004ebdb0 from the "Fog" section.
struct UnknownSceneFog {
    unsigned long color;                  // "ColorRGB", default 0x8080c0
    float visibility;                     // "Visibility", default 1.0
    float haziness;                       // "Haziness", default 0.5
};

// The 0x18-byte resource manager 0x004f0d20 keeps (unused) on its stack.
// Tier 2: the same retail class as UnknownResourceManager. Its constructor
// 0x004e8e80 zeroes the six fields; its destructor 0x004e8ea0 frees both
// arrays (ResourceManager.cpp lines 135 and 139). A separate view because
// declaring the destructor and fields on the shared class flips one register
// choice in TrackGame 0x00520ab0 (see docs/SCENEMANAGER.md).
class UnknownSceneResourceManager {
public:
    UnknownSceneResourceManager();       // 0x004e8e80
    ~UnknownSceneResourceManager();      // 0x004e8ea0

    int field_0x00;
    int field_0x04;
    int field_0x08;                       // field_0x14 count
    int field_0x0c;                       // field_0x10 count
    void** field_0x10;
    void** field_0x14;
};

// Name/value pair scanned by 0x004e9980.
struct UnknownSceneKeyword {
    const char* name;
    int value;
};

// RTTI: Scene : GameObject : BaseObject (vtable 0x0055786c), 0x8cc bytes
// (operator new(0x8cc) at 0x004e9a30 and 0x004e9aea). Scene overrides
// slots 0, 5, 7, 10, 14, 22 and 23. Field names are provisional.
class Scene : public GameObject {
public:
    explicit Scene(int flags);                       // 0x004ea660
    virtual ~Scene();                                // 0x004ea880 (deleting 0x004ea7c0)

    virtual void UnknownVirtualSlot5();              // 0x004eaa60
    virtual void UnknownVirtualSlot7();              // 0x004eaab0
    virtual int UnknownVirtualSlot10(float frameTime); // 0x004eab00, not reconstructed
    virtual int UnknownVirtualSlot14();              // 0x004f0040
    virtual int UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry);
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry);

    // 0x004ea7e0: stores the owner and settings, adds the sound group child.
    Scene* UnknownFunction4ea7e0(UnknownTrackGameObject574* manager, int value, int unused,
                                 char flag);
    // 0x004eaec0: the entry object named like `path`'s file name.
    UnknownSceneObject* UnknownFunction4eaec0(const char* path, int* index);
    void UnknownFunction4eafd0(int index);
    void UnknownFunction4eff30(int level);           // 0x004eff30: detail level (QuarryStuntEvent.cpp)
    void UnknownFunction4eb000(float time);
    int UnknownFunction4eb040(int index, float time, int a, int b); // not reconstructed
    // 0x004eb160/0x004eb300/0x004eb480: read "x,y,z", "r,g,b" and a Y/T/1
    // flag list from `block` (default: field_0xdc). Parameter names provisional.
    int UnknownFunction4eb160(Vector3* out, const char* section, const char* key,
                              const char* def, int useDefault, UnknownParameterBlock* block);
    int UnknownFunction4eb300(unsigned long* out, const char* section, const char* key,
                              const char* def, int unused, UnknownParameterBlock* block);
    int UnknownFunction4eb480(char* out, int count, const char* section, const char* key,
                              const char* def, int unused, UnknownParameterBlock* block);
    int UnknownFunction4ebdb0();                     // reads the "Fog" section
    int UnknownFunction4eca20();                     // opens the "ResourceFiles" archives
    void UnknownFunction4edf20(ProjectedShadow* shadow, int flags); // shadows for the casters
    void UnknownFunction4ecc10();                    // reads the "Stadium" section
    void UnknownFunction4ef9c0(const char* section, char* file, int is3D,
                               UnknownSound3DParameters* params, unsigned long* flags,
                               float* oneShotDistance, float* randomTriggerPercent,
                               int* oneShot, int* force2D);
    // 0x004de580 is a shared empty `ret 0xc` body (also BaseQuarryEvent and
    // NationalRace slot 34) that the readers call directly with the value
    // they read: a string or, in 0x004ebdb0, a float. The two overloads
    // here are provisional.
    void UnknownFunction4de580(const char* section, const char* key, const char* value);
    void UnknownFunction4de580(const char* section, const char* key, float value);
    // 0x00464e80 is likewise a shared empty `ret 4` body (FollowCamera slot 52
    // among others), called directly with the formatted "Cannot find data" text.
    void UnknownFunction464e80(const char* message);
    int UnknownFunction4f0d20(int* count);           // loads the scene file
    int UnknownFunction4f0ec0(char* path);           // model path -> its SLT file
    int UnknownFunction4f1130(unsigned long* info, char* a, char* b, int c); // not reconstructed

    UnknownTrackGameObject574* field_0x2c;
    char field_0x30[0x40];
    Vector3 field_0x70;
    Vector3 field_0x7c;
    char field_0x88;
    unsigned char field_0x89[0xa4 - 0x89];
    void* field_0xa4;
    int field_0xa8;
    void* field_0xac;
    UnknownSceneFog* field_0xb0;
    UnknownSceneTable* field_0xb4;
    UnknownSceneBuffer* field_0xb8;
    int field_0xbc;
    int field_0xc0;
    SoundGroup* field_0xc4;
    ContainerList<Sound*> field_0xc8;
    UnknownParameterBlock field_0xdc;
    unsigned char field_0x6a0_bit0 : 1;              // "Stadium" section present
    unsigned char field_0x6a1[3];
    float field_0x6a4;                               // Stadium "Top"
    float field_0x6a8;                               // Stadium "Bottom"
    float field_0x6ac;                               // Stadium "Scale"
    Vector3 field_0x6b0;                             // Stadium "Position"
    char field_0x6bc[0x100];
    int field_0x7bc;                                 // toggled by slot 22
    int field_0x7c0;
    FILE* field_0x7c4;
    char field_0x7c8[0x104];
};

int UnknownFunction4f0310(UnknownResourceManager* manager, const char* name, const char* path);
int UnknownFunction4e9980(int* out, const char* name, const UnknownSceneKeyword* table);
// 0x004f00e0: counts the texture by width (256, 128, 64 or 32) in counts[0..3];
// "PROCEDURAL" textures count as 64.
int UnknownFunction4f00e0(UnknownSceneTextureInfo* texture, int* counts);
