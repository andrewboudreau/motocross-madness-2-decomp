// Candidate kept in samples/: with D3DIMSoultreeMotnctrl.bindings.json, 23 of its 26 targets
// (targets.json) pass `tools/run_physics_samples.py --strict`; slots 2, 4 and 11 remain partial.
// See docs/PHYSICS_VALIDATION.md.
// D3DIMSoultreeMotnctrl.cpp -- D3DIMSoultreeCharacter methods (motion-control loading and playback).
//
// The retail __FILE__ string 'D:\aardvark\VC\krusty2\D3DIMSoultreeMotnctrl.cpp' is at 0x00568b70; it is
// used by the debug allocator calls in D3DIMSoultreeCharacter's slot 11 (0x00445680), destructor core
// (0x004459a0), slot 0 (0x00445a70) and slot 9 (0x00445ec0).  The class shape comes from
// src/krusty2/motion/D3DIMSoultreeCharacter.h, which is included, not redeclared.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "core/DebugAlloc.h"
#include "collision/CollisionObject.h"
#include "motion/D3DIMSoultreeCharacter.h"
#include "core/SoultreeObject.h"
#include "motion/MotionPose.h"

extern "C" char* _strupr(char*);   // 0x00535d3d (case-maps with 0x20; locale-aware CRT strupr)

// 0x00445460: writes a row for 'node', then one for each following sibling (nextSibling +0x144), then
// recurses into the first child (+0x140) of 'node' and of each sibling; returns the next free row index.
// Retail writes the first row with the clears up front and the sibling rows with the clears last.
int FillNodeNames(int index, NodeNameEntry* table, SoultreeObject* node)
{
    table[index].node = node;
    table[index].field_0x38 = 0;
    table[index].field_0x3c = 0;
    _strupr(node->name);
    int len = strlen(node->name);
    int count = len > 0x31 ? 0x31 : len;
    strncpy(table[index].name, node->name, count);
    table[index].name[count] = 0;
    index++;
    for (SoultreeObject* n = node->nextSibling; n; n = n->nextSibling) {
        table[index].node = n;
        _strupr(n->name);
        len = strlen(n->name);
        count = len > 0x31 ? 0x31 : len;
        strncpy(table[index].name, n->name, count);
        table[index].name[count] = 0;
        table[index].field_0x38 = 0;
        table[index].field_0x3c = 0;
        index++;
    }
    if (node->firstChild)
        index = FillNodeNames(index, table, node->firstChild);
    for (SoultreeObject* m = node->nextSibling; m; m = m->nextSibling) {
        if (m->firstChild)
            index = FillNodeNames(index, table, m->firstChild);
    }
    return index;
}

// slot 8 (0x00445e70, ret 4): copies the 0x4f-char name at +0x1a4 from the argument object, then runs
// Character's slot 8 (0x004a98b0, direct call).  The header types the argument as int; it is a pointer to
// another D3DIMSoultreeCharacter (tier 3).
void D3DIMSoultreeCharacter::CharacterVirtualSlot8(int a)
{
    D3DIMSoultreeCharacter* src = (D3DIMSoultreeCharacter*)a;
    int len = strlen(src->d3d_field_0x1a4);
    int n = len > 0x4f ? 0x4f : len;
    strncpy(d3d_field_0x1a4, src->d3d_field_0x1a4, n);
    d3d_field_0x1a4[n] = 0;
    Character::CharacterVirtualSlot8(a);
}

// slot 6 (0x00446480, ret 8): applies a pose record to its node.  'b' is an optional per-row enable
// mask indexed by the record's node index.  Argument types are tier 3 (the header uses int).
void D3DIMSoultreeCharacter::CharacterVirtualSlot6(int a, int b)
{
    const CharacterPose* pose = (const CharacterPose*)a;
    const int* mask = (const int*)b;
    if (mask && mask[pose->nodeIndex] == 0)
        return;
    SoultreeObject* node = nodeNames[pose->nodeIndex].node;
    if (pose->hasPose) {
        node->SetAxesPtr(&pose->axisZ, &pose->axisY, 0, 1);
        node->SetPosition(pose->position);
    } else {
        node->SetAxesIn(modelNode, &pose->axisZ, &pose->axisY, 0, 1);
        node->TranslateIn(modelNode, pose->position);
    }
}

// slot 3 (0x00446110, ret 4): refreshes every record of the list from the live nodes.  The records are
// first passed (last to first) to slot 4, then each is overwritten with its node's current pose.
void D3DIMSoultreeCharacter::CharacterVirtualSlot3(int a)
{
    MotionPoseList* list = (MotionPoseList*)a;
    for (int i = list->count - 1; i >= 0; i--)
        CharacterVirtualSlot4((int)&list->poses[i], 0);
    for (int j = 0; j < list->count; j++) {
        SoultreeObject* node = nodeNames[list->poses[j].nodeIndex].node;
        Vec3 position;
        Vec3 axisZ;
        Vec3 axisY;
        node->GetPosition(&position);
        node->GetAxes(&axisZ, &axisY);
        list->poses[j].hasPose = 1;
        list->poses[j].axisZ = axisZ;
        list->poses[j].axisY = axisY;
        list->poses[j].position = position;
    }
}

// slot 4 (0x00446210, ret 8): like slot 6 but honours the per-row field_0x3c / field_0x38 enables.
void D3DIMSoultreeCharacter::CharacterVirtualSlot4(int a, int b)
{
    const CharacterPose* pose = (const CharacterPose*)a;
    const int* mask = (const int*)b;
    if (mask && mask[pose->nodeIndex] == 0)
        return;
    SoultreeObject* node = nodeNames[pose->nodeIndex].node;
    int axesEnabled = nodeNames[pose->nodeIndex].field_0x3c;
    if (pose->hasPose) {
        if (axesEnabled)
            node->SetAxesPtr(&pose->axisZ, &pose->axisY, 0, 1);
        if (nodeNames[pose->nodeIndex].field_0x38)
            node->SetPositionVec3(pose->position);
    } else {
        if (axesEnabled)
            node->SetAxesIn(modelNode, &pose->axisZ, &pose->axisY, 0, 1);
        if (nodeNames[pose->nodeIndex].field_0x38)
            node->TranslateIn(modelNode, pose->position);
    }
}

// slot 5 (0x00446300, ret 8): mirrored variant of slot 4.  The target node comes from the mirror map
// (+0x18c, indexed by the record's node index) and the x components of both axes and the position are
// negated.  The per-row enables are still read from the unmapped row.
void D3DIMSoultreeCharacter::CharacterVirtualSlot5(int a, int b)
{
    const CharacterPose* pose = (const CharacterPose*)a;
    const int* mask = (const int*)b;
    if (mask && mask[pose->nodeIndex] == 0)
        return;
    NodeNameEntry* table = nodeNames;
    int mirrored = mirrorMap[pose->nodeIndex];
    SoultreeObject* node = table[mirrored].node;
    Vec3 axisZ;
    Vec3 axisY;
    axisZ.x = -pose->axisZ.x;
    axisZ.y = pose->axisZ.y;
    axisZ.z = pose->axisZ.z;
    axisY.x = -pose->axisY.x;
    axisY.y = pose->axisY.y;
    axisY.z = pose->axisY.z;
    if (pose->hasPose) {
        if (table[pose->nodeIndex].field_0x3c)
            node->SetAxesPtr(&axisZ, &axisY, 0, 1);
        if (nodeNames[pose->nodeIndex].field_0x38) {
            Vec3 position;
            position.x = -pose->position.x;
            position.y = pose->position.y;
            position.z = pose->position.z;
            node->TranslateIn(modelNode, position);
        }
    } else {
        if (table[pose->nodeIndex].field_0x3c)
            node->SetAxesIn(modelNode, &axisZ, &axisY, 0, 1);
        if (nodeNames[pose->nodeIndex].field_0x38) {
            Vec3 position;
            position.x = -pose->position.x;
            position.y = pose->position.y;
            position.z = pose->position.z;
            node->TranslateIn(modelNode, position);
        }
    }
}

// slot 10 (0x00446520): formats a "Position/Orientation Data" line per node into a local buffer
// (format string at 0x00568bbc).  The buffer is not read afterwards in retail (debug output stripped).
void D3DIMSoultreeCharacter::CharacterVirtualSlot10()
{
    for (int i = 0; i < nodeCount; i++) {
        char text[0x100];
        Vec3 position;
        Vec3 look;
        Vec3 up;
        nodeNames[i].node->UpdateWorldMatrix();
        nodeNames[i].node->GetPositionIn(0, &position);
        nodeNames[i].node->GetAxesIn(0, &look, &up);
        sprintf(text, "Position/Orientation Data %s XPos:%f YPos:%f ZPos:%f UpX:%f UpY:%f UpZ:%f LookX:%f LookY:%f LookZ:%f\n",
                nodeNames[i].node->name, position.x, position.y, position.z, up.x, up.y, up.z,
                look.x, look.y, look.z);
    }
}

// slot 2 (0x00445fc0): captures the current pose of every node but the root into
// poseList (+0x190; poses allocated on first use, line 0x12d), in each node's parent space.
void D3DIMSoultreeCharacter::CharacterVirtualSlot2()
{
    int count = nodeCount;
    poseList.count = count - 1;
    if (poseList.poses == 0)
        poseList.poses = (CharacterPose*)DebugMalloc(count * 0x2c - 1, __FILE__, 0x12d);
    for (int i = 0; i < nodeCount - 1; i++) {
        poseList.poses[i].nodeIndex = i + 1;
        poseList.poses[i].hasPose = 1;
        SoultreeObject* node = nodeNames[i + 1].node;
        SoultreeObject* parent = node->parent;
        Vec3 position;
        Vec3 axisZ;
        Vec3 axisY;
        node->GetPositionIn(parent, &position);
        node->GetAxesIn(parent, &axisZ, &axisY);
        poseList.poses[i].position = position;
        poseList.poses[i].axisZ = axisZ;
        poseList.poses[i].axisY = axisY;
    }
    Method_0x004a6b10(&poseList);
}

// Global registry (pointer at 0x00572b44): a list of entries keyed by an owner pointer at entry+0x10.
// 0x004e93f0 finds the entry whose owner is the argument; 0x004e9010 sets an entry's owner.  Names tier 3.
class OwnerEntry : public BaseObject {
public:
    char pad_0x08[8];
    D3DIMSoultreeCharacter* owner;                       // +0x10 tested and set by slot 11
};
class OwnerRegistry {
public:
    OwnerEntry* FindByName(const char* name, int a);    // 0x004e9360, ret 8
    void Load(const char* name, const char* path);      // 0x004e9430, ret 8
    OwnerEntry* FindByOwner(void* owner);               // 0x004e93f0, ret 4
    void SetOwner(OwnerEntry* entry, void* owner);      // 0x004e9010, ret 8
};
extern OwnerRegistry* g_pOwnerRegistry;                 // 0x00572b44

// Destructor core 0x004459a0 (retail passes the GameObject virtual base as 'this'; the deleting
// destructor 0x00446680 is already matched in samples/physics/hierarchy).  Frees the 0x18-byte copied
// descriptor at +0x208 (debug free, line 0xc4) and the node table at +0x188, drops the registry entry
// owned by this object, then runs Character's destructor.
D3DIMSoultreeCharacter::~D3DIMSoultreeCharacter()
{
    if (*(void**)(d3d_field_0x1a4 + 0x64))
        DebugFree(*(void**)(d3d_field_0x1a4 + 0x64), __FILE__, 0xc4);
    if (nodeNames)
        operator delete(nodeNames);
    OwnerEntry* entry = g_pOwnerRegistry->FindByOwner(this);
    if (entry)
        g_pOwnerRegistry->SetOwner(entry, 0);
}

// GameObject slot 4 override (0x00446640, reached through vtordisp thunk 0x004466c0): runs GameObject's
// slot 4, then clears the four dwords at +0x1f4..+0x200 (ctor 0x004455b0 also zeroes them).
void D3DIMSoultreeCharacter::GameObjectVirtualSlot4()
{
    GameObject::GameObjectVirtualSlot4();
    *(int*)(d3d_field_0x1a4 + 0x50) = 0;
    *(int*)(d3d_field_0x1a4 + 0x54) = 0;
    *(int*)(d3d_field_0x1a4 + 0x58) = 0;
    *(int*)(d3d_field_0x1a4 + 0x5c) = 0;
}

// GameObject slot 5 override (0x00446620, thunk 0x004466d0): forwards to GameObject's slot 5.
void D3DIMSoultreeCharacter::GameObjectVirtualSlot5()
{
    GameObject::GameObjectVirtualSlot5();
}

// Constructor 0x004455b0 (ret 8: one argument plus the hidden most-derived flag).  Builds the GameObject
// virtual base with 1, runs Character's constructor (0x004a6780, which receives the argument) and clears
// the D3DIM members plus Character's name table pointer.
D3DIMSoultreeCharacter::D3DIMSoultreeCharacter(int a) : GameObject(1), Character(a)
{
    modelNode = 0;
    d3d_field_0x1a4[0] = 0;
    *(int*)(d3d_field_0x1a4 + 0x50) = 0;
    *(int*)(d3d_field_0x1a4 + 0x54) = 0;
    *(int*)(d3d_field_0x1a4 + 0x58) = 0;
    *(int*)(d3d_field_0x1a4 + 0x5c) = 0;
    *(int*)(d3d_field_0x1a4 + 0x64) = 0;
    nodeNames = 0;
}

// The scene-node model that Character owns at +0x1a0: retail class D3DIMSoultreeObject (RTTI, vtable 0x005513ec, QuadTreeObject +0
// and GameObject +0xc, 0x2d8 bytes).  samples/physics/collision/SoultreePhysicsObject.h declares it, but its slot 7
// (0x00444560, `ret 4`) takes the object to copy from and the header declares no argument, so this TU keeps a local
// stand-in with the same layout and vtable shape (tier 3).
class SceneItem;
class ModelObject : public QuadTreeObject, public GameObject {
public:
    explicit ModelObject(int a);                 // 0x0043f160, ret 4
    virtual void ModelSlot2();                   // 0x0043f950
    virtual void ModelSlot3();                   // 0x0043fe40
    virtual void ModelSlot4();                   // 0x004440a0
    virtual void ModelSlot5();                   // 0x00444140
    virtual void ModelSlot6();                   // 0x004450c0
    virtual void CopyFrom(ModelObject* source);  // 0x00444560, ret 4: copies the node tree of 'source'
    virtual void ModelSlot8();                   // 0x00444b60
    // 0x0043f4b0, ret 0x14: loads the model; slot 0 passes (field_0x18, name, descriptor words, 1).
    virtual void Load(void* a1, const char* a2, int a3, int a4, int a5);
    char pad_0x38[0x240 - 0x38];
    int field_0x240;                             // +0x240 copied by CopyFrom, passed to SceneItem::Init
    int field_0x244;                             // +0x244 copied by CopyFrom, passed to SceneItem::Init
    char field_0x248[0x274 - 0x248];             // +0x248 passed by address to SceneItem::Init
    int field_0x274;                             // +0x274 moved from the temporary model in slot 0
    char pad_0x278[0x280 - 0x278];
    int field_0x280;                             // +0x280 moved (and cleared in the source) in slot 0
    int pad_0x284;
    int field_0x288;                             // +0x288 moved in slot 0
    int field_0x28c;                             // +0x28c moved (and cleared in the source) in slot 0
    SceneItem** items;                           // +0x290 array of field_0x294 SceneItem pointers
    int itemCount;                               // +0x294
    char pad_0x298[0x2d8 - 0x298];
};

// 0x00500xxx-range helper object (0xd0 bytes, GameObject base, ctor 0x004fefe0 ret 4): one per entry of
// ModelObject::items (tier 3 name).
class SceneItem : public GameObject {
public:
    explicit SceneItem(int a);                                        // 0x004fefe0
    void Init(void* a, void* b, int c, int d);                        // 0x004ff0b0, ret 0x10
    void CopyFrom(SceneItem* other);                                  // 0x005000f0, ret 4
    char pad_0x2c[0xd0 - 0x2c];
};

// slot 9 (0x00445ec0, ret 4): clones the argument character's node model, makes it a child of this object's
// GameObject, then rebuilds the node name table (line 0x11b) and re-registers the model's nodes.
void D3DIMSoultreeCharacter::CharacterVirtualSlot9(int a)
{
    SoultreeObject* source = ((D3DIMSoultreeCharacter*)a)->modelNode;
    ModelObject* model = new(__FILE__, 0x116) ModelObject(1);
    modelNode = (SoultreeObject*)model;
    ((ModelObject*)modelNode)->CopyFrom((ModelObject*)source);
    GameObject* child = (ModelObject*)modelNode;
    Method_0x00469190(child, -1);
    nodeCount = modelNode->CountNodes();
    nodeNames = (NodeNameEntry*)operator new(nodeCount << 6, __FILE__, 0x11b);
    FillNodeNames(0, nodeNames, modelNode);
    Method_0x004a6500();
    Method_0x004a62d0();
    modelNode->RegisterNode();
}

// slot 0 (0x00445a70): builds this character's node model from scratch: a model named "Base Character Frame"
// is loaded, a second model is loaded under the character's combined name, its items are cloned onto the first
// and its node tree is attached below it, then the node name table is rebuilt (line 0x100).
void D3DIMSoultreeCharacter::CharacterVirtualSlot0()
{
    char name[0x104];
    int len = strlen(contentDirectory);
    int n = len > 0x103 ? 0x103 : len;
    strncpy(name, contentDirectory, n);
    name[n] = 0;
    strcat(name, d3d_field_0x1a4);

    ModelObject* frame = new(__FILE__, 0xd6) ModelObject(statusFlags & 1);
    modelNode = (SoultreeObject*)frame;
    frame->Load(field_0x18, "", *(int*)(d3d_field_0x1a4 + 0x60), *(int*)(d3d_field_0x1a4 + 0x64), 1);
    GameObject* frameChild = (ModelObject*)modelNode;
    Method_0x00469190(frameChild, -1);
    modelNode->RegisterNode();
    len = strlen("Base Character Frame");
    n = len > 0x7f ? 0x7f : len;
    strncpy(modelNode->name, "Base Character Frame", n);
    modelNode->name[n] = 0;

    ModelObject* loaded = new(__FILE__, 0xdf) ModelObject(statusFlags & 1);
    loaded->Load(field_0x18, name, *(int*)(d3d_field_0x1a4 + 0x60), *(int*)(d3d_field_0x1a4 + 0x64),
                 *(int*)(d3d_field_0x1a4 + 0x68));
    ((SoultreeObject*)loaded)->UnregisterNode();
    OwnerEntry* owner = g_pOwnerRegistry->FindByOwner(loaded);
    if (owner)
        g_pOwnerRegistry->SetOwner(owner, 0);
    ((ModelObject*)modelNode)->itemCount = loaded->itemCount;
    ((ModelObject*)modelNode)->items =
        (SceneItem**)DebugMalloc(((ModelObject*)modelNode)->itemCount << 2, __FILE__, 0xeb);
    for (int i = 0; i < ((ModelObject*)modelNode)->itemCount; i++) {
        ((ModelObject*)modelNode)->items[i] = new(__FILE__, 0xed) SceneItem(1);
        ((ModelObject*)modelNode)->items[i]->Init(field_0x18, ((ModelObject*)modelNode)->field_0x248,
                                                  ((ModelObject*)modelNode)->field_0x240,
                                                  ((ModelObject*)modelNode)->field_0x244);
        Method_0x00469190(((ModelObject*)modelNode)->items[i], -1);
        ((ModelObject*)modelNode)->items[i]->CopyFrom(loaded->items[i]);
        loaded->items[i]->BaseObjectVirtualSlot2();
    }
    ((ModelObject*)modelNode)->field_0x28c = loaded->field_0x28c;
    loaded->field_0x28c = 0;
    modelNode->AddChild((SoultreeObject*)loaded);
    ((ModelObject*)modelNode)->field_0x274 = loaded->field_0x274;
    ((ModelObject*)modelNode)->field_0x280 = loaded->field_0x280;
    ((ModelObject*)modelNode)->field_0x288 = loaded->field_0x288;
    loaded->field_0x280 = 0;
    nodeCount = modelNode->CountNodes();
    nodeNames = (NodeNameEntry*)operator new(nodeCount << 6, __FILE__, 0x100);
    FillNodeNames(0, nodeNames, modelNode);
    Method_0x004a6500();
    Method_0x004a62d0();
}

// Helpers of slot 11 (all names tier 3).
void BusyCursor();                                       // 0x004bfa80, cdecl: two cursor calls through function pointers
class ArchiveFile {                                      // 0x134 bytes, ctor takes the registry (0x00460d10, ret 4)
public:
    explicit ArchiveFile(OwnerRegistry* registry);
    ~ArchiveFile();                                      // 0x00460d60: frees the buffer at +0x14
    void Open(const char* name, const char* mode, int a);   // 0x00460f50, ret 0xc
    void SetBufferSize(int size);                        // 0x00461310, ret 4
    char pad_0x00[0x134];
};
class SltFile {                                          // 0x5c4 bytes, ctor 0x004b7130
public:
    SltFile();
    ~SltFile();                                          // 0x004b7190
    void Attach(ArchiveFile* file, int a, int b);        // 0x004b77a0, ret 0xc
    void SelectSection(const char* name);                // 0x004b78f0, ret 4
    void ReadString(const char* key, char* out, int n);  // 0x004b7b30, ret 0xc
    char pad_0x00[0x5c4];
};
struct LoadDescCopy {                                    // 0x18-byte block copied by slot 11 (no member is read in this TU)
    int field_0x00[6];
};

// slot 11 (0x00445680, ret 0x18): loads a character from the resource 'a2'.  The file name part is looked up
// in the owner registry; a cached entry with an owner is cloned through slot 8, otherwise the .slt file is read
// and the new object becomes the entry's owner.  Returns the GameObject subobject, or 0 when the lookup fails
// (the object has then released itself).
GameObject* D3DIMSoultreeCharacter::D3DIMVirtualSlot11(int a1, const char* a2, const SoultreeLoadDesc* a3, int a4,
                                                       int a5, int a6)
{
    char drive[4];
    char fname[0x100];
    char ext[0x100];
    char dir[0x100];
    BusyCursor();
    Method_0x004a6910(a1, a6);
    *(const SoultreeLoadDesc**)(d3d_field_0x1a4 + 0x60) = a3;
    *(int*)(d3d_field_0x1a4 + 0x68) = a5;
    if (a4) {
        LoadDescCopy* copy = (LoadDescCopy*)DebugMalloc(0x18, __FILE__, 0x64);
        *(LoadDescCopy**)(d3d_field_0x1a4 + 0x64) = copy;
        *copy = *(LoadDescCopy*)a4;
    }
    ArchiveFile* file = new(__FILE__, 0x68) ArchiveFile(g_pOwnerRegistry);
    _splitpath(a2, drive, dir, fname, ext);
    strcat(fname, ext);
    OwnerEntry* entry = g_pOwnerRegistry->FindByName(fname, 0);
    if (!entry) {
        g_pOwnerRegistry->Load(fname, a2);
        entry = g_pOwnerRegistry->FindByName(fname, 0);
        if (!entry) {
            BaseObjectVirtualSlot2();
            delete file;
            return 0;
        }
    }
    if (entry->owner) {
        entry->BaseObjectVirtualSlot1();
        CharacterVirtualSlot8((int)entry->owner);
        BusyCursor();
        delete file;
        return this;
    }
    file->Open(fname, "rb", 0);
    file->SetBufferSize(0x4000);
    int len = strlen(a2);
    int n = len > 0x4f ? 0x4f : len;
    strncpy(sltPath, a2, n);
    sltPath[n] = 0;
    SltFile* slt = new(__FILE__, 0xa5) SltFile();
    sltFile = slt;
    slt->Attach(file, 0, 1);
    slt->SelectSection("General info");
    slt->ReadString("SLTFile", d3d_field_0x1a4, -1);
    CharacterVirtualSlot1();
    delete sltFile;
    g_pOwnerRegistry->SetOwner(entry, this);
    delete file;
    BusyCursor();
    return this;
}
