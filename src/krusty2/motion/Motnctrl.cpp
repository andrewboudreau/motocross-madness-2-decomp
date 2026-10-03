// Motnctrl.cpp -- reconstruction of D:\aardvark\VC\krusty2\Motnctrl.cpp (motion control: the global
// motion manager, the .VUE/.MOT motion loaders and Character's motion playback).
//
// The retail __FILE__ string 'D:\aardvark\VC\krusty2\Motnctrl.cpp' is at 0x0056e034; its xrefs run
// from 0x004a5659 (the manager initialiser, line 9) to 0x004a9a87 (FreeMotion, line 0x5a7) with the
// line numbers rising with the address.  Character's shape comes from
// src/krusty2/motion/D3DIMSoultreeCharacter.h, which is included, not redeclared.  See
// src/krusty2/motion/README.md for the evidence summary.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "core/DebugAlloc.h"
#include "motion/D3DIMSoultreeCharacter.h"
#include "MotionPose.h"   // Math3D.h: the four per-TU Vec3 initialisers at 0x004a54f0..0x004a562c

extern "C" char* _strupr(char*);   // 0x00535d3d

// File and .slt readers shared with D3DIMSoultreeMotnctrl.cpp (local declarations; names tier 3).
class OwnerRegistry;
extern OwnerRegistry* g_pOwnerRegistry;                 // 0x00572b44
class ArchiveFile {                                      // 0x134 bytes, ctor takes the registry (0x00460d10, ret 4)
public:
    explicit ArchiveFile(OwnerRegistry* registry);
    ~ArchiveFile();                                      // 0x00460d60
    int Open(const char* name, const char* mode, int a); // 0x00460f50, ret 0xc; 0 on failure
    int Read(void* buffer, int size, int count);         // 0x00461640, ret 0xc
    int ReadLine(char* buffer, int size);                // 0x00461aa0, ret 8; 0 at the end of the file
    int Seek(int a, int b, int c);                       // 0x00461340, ret 0xc
    char pad_0x00[0x130];
    int field_0x130;                                     // +0x130 first argument of Seek in LoadVue (tier 3)
};
class SltFile {                                          // 0x5c4 bytes
public:
    void SelectSection(const char* name);                // 0x004b78f0, ret 4
    void GetString(const char* key, const char* def, char* out, int size);  // 0x004b7ec0, ret 0x10
    void GetInt(const char* key, int def, int* out);     // 0x004b7f10, ret 0xc
    int GetFloat(const char* key, float def, float* out);   // 0x004b7f40, ret 0xc; 0 when the key is missing
};

// One motion (0xcc bytes, allocated by MotionManager::Load at line 0x159).  Offsets tier 1 (Load,
// LoadMot, LoadVue, FreeMotion and the playback code); names tier 3.
struct Motion {
    char name[0x51];             // +0x00 "Name" key of the MOTION section (at most 0x50 chars)
    char fileName[0x51];         // +0x51 "VUE" key: the .VUE or .MOT file, relative to the content directory
    MotionPoseList* frames;      // +0xa4 frameCount frame records (0xc bytes each)
    int frameCount;              // +0xa8 "FrameCount", clamped to the frames actually read
    int posesPerFrame;           // +0xac pose records per frame
    int looping;                 // +0xb0 "Looping"
    int fps;                     // +0xb4 "FPS" (default 15)
    float frameTime;             // +0xb8 1 / fps
    float transitionTime;        // +0xbc "TransitionTime" (default frameCount / fps / 4, at most 0.5)
    int sorted;                  // +0xc0 set once every frame's poses are sorted (0x004a6ab0)
    int captured;                // +0xc4 set by CaptureMotion (0x004a6a60)
    int refCount;                // +0xc8 Character::LoadMotions adds, MotionManager::Release drops
};

// The global motion cache (0x18 bytes, pointer at 0x0068512c).  Names tier 3.
class MotionManager {
public:
    MotionManager();
    void Release(Motion* motion);
    Motion* Load(Character* character, const char* section);
    int LoadMot(Character* character, const char* path, Motion* motion);
    int LoadVue(Character* character, const char* path, Motion* motion);
    Motion* Find(const char* name, int unused);
    int count;                   // +0x00 entries in motions
    int totalBytes;              // +0x04 Load adds sizeof(Motion)
    int field_0x08;              // +0x08 ctor 0
    int field_0x0c;              // +0x0c ctor 0
    int keepLoaded;              // +0x10 Release never frees a motion while set
    Motion** motions;            // +0x14 count entries (DebugMalloc/DebugRealloc, lines 0x18c/0x18e/0x12a)
};

void FreeMotion(Motion* motion);

// owner: Motnctrl.cpp (__FILE__ 0x56e034)
// $E initialiser at 0x004a5640 (jmp thunk 0x004a5630): line 9, result stored at 0x0068512c.
MotionManager* g_pMotionManager = new(__FILE__, 9) MotionManager();

// owner: bracket only (qsort callback of 0x004a6b10)
// 0x004a56b0: orders pose records by descending node index.
int ComparePoses(const void* a, const void* b)
{
    const CharacterPose* pa = (const CharacterPose*)a;
    const CharacterPose* pb = (const CharacterPose*)b;
    if (pa->nodeIndex > pb->nodeIndex)
        return -1;
    return pa->nodeIndex < pb->nodeIndex;
}

// owner: bracket only (between the manager initialiser and the manager methods)
// 0x004a56d0: advances a looping motion's frame cursor by the whole frames in *time and keeps the
// remainder in *time.  Returns 0 when the cursor wrapped past the last frame.
int AdvanceLooping(Motion* motion, MotionPoseList** cursor, float* time)
{
    int frame = (*cursor)->field_0x00;
    int next = frame + (int)(motion->fps * *time);
    *time -= (next - frame) * motion->frameTime;
    if (next >= motion->frameCount) {
        do
            next -= motion->frameCount;
        while (next >= motion->frameCount);
        *cursor = &motion->frames[next];
        return 0;
    }
    *cursor = &motion->frames[next];
    return 1;
}

// owner: bracket only
// 0x004a5750: non-looping variant; also returns the two frames to blend and the blend weight.
// At the end of the motion it stops on the last frame (weight 1) and returns 0.
int AdvanceClamped(Motion* motion, MotionPoseList** cursor, MotionPoseList** from, MotionPoseList** to,
                   float* time, float* weight)
{
    int frame = (*cursor)->field_0x00;
    int next = frame + (int)(motion->fps * *time);
    if (next >= motion->frameCount - 1) {
        *time -= (motion->frameCount - frame - 1) * motion->frameTime;
        *from = &motion->frames[motion->frameCount - 2];
        *cursor = *to = &motion->frames[motion->frameCount - 1];
        *weight = 1.0f;
        return 0;
    }
    *time -= (next - frame) * motion->frameTime;
    *from = &motion->frames[next];
    *to = &motion->frames[next + 1];
    *weight = *time / motion->frameTime;
    *cursor = *from;
    return 1;
}

// owner: Motnctrl.cpp (__FILE__ 0x56e034): the class's Release/Load reference it
// 0x004a5840
MotionManager::MotionManager()
{
    count = 0;
    motions = 0;
    totalBytes = 0;
    field_0x08 = 0;
    field_0x0c = 0;
    keepLoaded = 0;
}

// owner: Motnctrl.cpp (__FILE__ 0x56e034, line 0x12a)
// 0x004a5860: drops one reference; the last one frees the motion and removes it from the table.
void MotionManager::Release(Motion* motion)
{
    int index = 0;
    for (int i = 0; i < count; i++) {
        if (motions[i] == motion)
            index = i;
    }
    if (--motion->refCount > 0)
        return;
    if (keepLoaded)
        return;
    FreeMotion(motion);
    motions[index] = 0;
    for (int j = index; j < count - 1; j++)
        motions[j] = motions[j + 1];
    count--;
    motions = (Motion**)DebugRealloc(motions, count * 4, __FILE__, 0x12a);
}

// owner: Motnctrl.cpp (__FILE__ 0x56e034, lines 0x159, 0x18c, 0x18e)
// 0x004a5900: returns the motion described by the character's .slt section 'section', loading it
// (.VUE or .MOT) when no motion of that name is cached.
Motion* MotionManager::Load(Character* character, const char* section)
{
    char drive[4];
    char path[0x104];
    char name[0x104];
    char ext[0x100];
    char fileName[0x104];
    char fname[0x100];
    char dir[0x100];
    character->sltFile->SelectSection(section);
    character->sltFile->GetString("Name", "NONE", name, -1);
    Motion* found = Find(name, 0);
    if (found)
        return found;
    Motion* motion = new(__FILE__, 0x159) Motion;
    motion->captured = 0;
    motion->sorted = 0;
    motion->refCount = 0;
    totalBytes += sizeof(Motion);
    int len = strlen(name);
    int n = len > 0x50 ? 0x50 : len;
    strncpy(motion->name, name, n);
    motion->name[n] = 0;
    character->sltFile->GetString("VUE", "NONE", fileName, -1);
    len = strlen(fileName);
    n = len > 0x50 ? 0x50 : len;
    strncpy(motion->fileName, fileName, n);
    motion->fileName[n] = 0;
    strcpy(path, "");
    strcat(path, character->contentDirectory);
    strcat(path, motion->fileName);
    character->sltFile->GetInt("FrameCount", 0, &motion->frameCount);
    character->sltFile->GetInt("Looping", 0, &motion->looping);
    character->sltFile->GetInt("FPS", 15, &motion->fps);
    motion->frameTime = 1.0f / motion->fps;
    if (!character->sltFile->GetFloat("TransitionTime", 0.0f, &motion->transitionTime)) {
        motion->transitionTime = (float)motion->frameCount / motion->fps * 0.25f;
        if (motion->transitionTime > 0.5f)
            motion->transitionTime = 0.5f;
    }
    motion->frames = 0;
    _splitpath(path, drive, dir, fname, ext);
    int ok;
    if (_stricmp(ext, ".VUE") == 0)
        ok = LoadVue(character, path, motion);
    else if (_stricmp(ext, ".MOT") == 0)
        ok = LoadMot(character, path, motion);
    else
        ok = 0;
    if (ok) {
        if (motions)
            motions = (Motion**)DebugRealloc(motions, (count + 1) * 4, __FILE__, 0x18c);
        else
            motions = (Motion**)DebugMalloc((count + 1) * 4, __FILE__, 0x18e);
        motions[count] = motion;
        count++;
        return motions[count - 1];
    }
    delete motion;
    return 0;
}

// owner: Motnctrl.cpp (__FILE__ 0x56e034, lines 0x19e, 0x1b2, 0x1b6)
// 0x004a5c30: binary .MOT file: frame count, poses per frame, then per pose a node index byte and
// the two axes and the position (0x24 bytes).
int MotionManager::LoadMot(Character* character, const char* path, Motion* motion)
{
    ArchiveFile* file = new(__FILE__, 0x19e) ArchiveFile(g_pOwnerRegistry);
    if (!file->Open(path, "rb", 0)) {
        char message[0x80];
        sprintf(message, "Unable to open %s\n", path);
        return 0;
    }
    int frameCount;
    motion->sorted = 1;
    motion->captured = 1;
    file->Read(&frameCount, 4, 1);
    file->Read(&motion->posesPerFrame, 4, 1);
    if (frameCount < motion->frameCount)
        motion->frameCount = frameCount;
    motion->frames = (MotionPoseList*)DebugMalloc(motion->frameCount * sizeof(MotionPoseList), __FILE__, 0x1b2);
    for (int i = 0; i < motion->frameCount; i++) {
        MotionPoseList* frame = &motion->frames[i];
        frame->poses = (CharacterPose*)DebugMalloc(motion->posesPerFrame * sizeof(CharacterPose), __FILE__, 0x1b6);
        frame->field_0x00 = i;
        frame->count = motion->posesPerFrame;
        for (int j = 0; j < motion->posesPerFrame; j++) {
            CharacterPose* pose = &frame->poses[j];
            file->Read(pose, 1, 1);
            file->Read(&pose->axisZ, 0x24, 1);
            pose->hasPose = 1;
        }
    }
    delete file;
    return 1;
}

// owner: Motnctrl.cpp (__FILE__ 0x56e034, lines 0x1c9, 0x1ec, 0x1f9, 0x22a)
// 0x004a5e40: text .VUE file.  A first pass counts the TRANSFORM lines of the first frame (and those
// naming a node of this character); the second pass reads one frame per line group, swapping y and z.
int MotionManager::LoadVue(Character* character, const char* path, Motion* motion)
{
    char line[0x100];
    ArchiveFile* file = new(__FILE__, 0x1c9) ArchiveFile(g_pOwnerRegistry);
    if (!file->Open(path, "r", 0)) {
        char message[0x80];
        sprintf(message, "Unable to open %s\n", path);
        return 0;
    }
    int transforms = 0;
    int frame = 0;
    int poses = 0;
    while (file->ReadLine(line, 0xff)) {
        if (frame > 1)
            break;
        _strupr(line);
        char* token = strtok(line, " ");
        if (_stricmp(token, "FRAME") == 0) {
            frame++;
        } else if (_stricmp(token, "TRANSFORM") == 0) {
            transforms++;
            if (character->FindNode(strtok(0, " \"")) != 0xff)
                poses++;
        }
    }
    motion->sorted = 0;
    motion->captured = 0;
    motion->frames = (MotionPoseList*)DebugMalloc(motion->frameCount * sizeof(MotionPoseList), __FILE__, 0x1ec);
    motion->posesPerFrame = poses;
    frame = 0;
    file->Seek(file->field_0x130, 0, 1);
    while (file->ReadLine(line, 0xff) && frame < motion->frameCount) {
        _strupr(line);
        strtok(line, " ");
        MotionPoseList* list = &motion->frames[frame];
        list->poses = (CharacterPose*)DebugMalloc(poses * sizeof(CharacterPose), __FILE__, 0x1f9);
        list->count = poses;
        list->field_0x00 = frame;
        int k = 0;
        for (int t = 0; t < transforms; t++) {
            file->ReadLine(line, 0xff);
            strtok(line, " ");
            int node = character->FindNode(strtok(0, " \""));
            if (node != 0xff) {
                list->poses[k].nodeIndex = node;
                list->poses[k].hasPose = 0;
                strtok(0, " ");
                strtok(0, " ");
                strtok(0, " ");
                list->poses[k].axisZ.x = atof(strtok(0, " "));
                list->poses[k].axisZ.z = atof(strtok(0, " "));
                list->poses[k].axisZ.y = atof(strtok(0, " "));
                list->poses[k].axisY.x = atof(strtok(0, " "));
                list->poses[k].axisY.z = atof(strtok(0, " "));
                list->poses[k].axisY.y = atof(strtok(0, " "));
                list->poses[k].position.x = atof(strtok(0, " "));
                list->poses[k].position.z = atof(strtok(0, " "));
                list->poses[k].position.y = atof(strtok(0, " "));
                k++;
            }
        }
        frame++;
    }
    delete file;
    if (frame < motion->frameCount) {
        motion->frameCount = frame;
        motion->frames = (MotionPoseList*)DebugRealloc(motion->frames, frame * sizeof(MotionPoseList), __FILE__, 0x22a);
    }
    return 1;
}

// owner: bracket only (MotionManager method between Motnctrl.cpp xrefs)
// 0x004a6290: case-insensitive name lookup; the second argument is unused.
Motion* MotionManager::Find(const char* name, int unused)
{
    for (int i = 0; i < count; i++) {
        if (_stricmp(motions[i]->name, name) == 0)
            return motions[i];
    }
    return 0;
}

// owner: Motnctrl.cpp (__FILE__ 0x56e034, line 0x26b)
// 0x004a62d0 (LoadVUT, tier 3): reads the .VUT node table once: per line a node name, its display
// name and an optional flag ('T' enables the position).
void Character::Method_0x004a62d0()
{
    char line[0x100];
    if (vutLoaded)
        return;
    ArchiveFile* file = new(__FILE__, 0x26b) ArchiveFile(g_pOwnerRegistry);
    int len = strlen(contentDirectory);
    int n = len > 0xff ? 0xff : len;
    strncpy(line, contentDirectory, n);
    line[n] = 0;
    strcat(line, vutFilename);
    if (!file->Open(line, "r", 0))
        return;
    while (file->ReadLine(line, 0xff)) {
        _strupr(line);
        int node = FindNode(strtok(line, "=,"));
        if (node != 0xff) {
            char* name = strtok(0, "=, \n");
            int nameLen = strlen(name);
            int count = nameLen > 0x31 ? 0x31 : nameLen;
            strncpy(nodeNames[node].name, name, count);
            nodeNames[node].name[count] = 0;
            nodeNames[node].field_0x38 = 0;
            nodeNames[node].field_0x3c = 1;
            char* flag = strtok(0, "=, \n");
            if (flag) {
                _strupr(flag);
                if (*flag == 'T')
                    nodeNames[node].field_0x38 = 1;
            }
        }
    }
    delete file;
    vutLoaded = 1;
}

// owner: Motnctrl.cpp (__FILE__ 0x56e034, lines 0x29d, 0x2ab)
// 0x004a6500 (LoadMIR, tier 3): reads the .MIR file of node pairs into the symmetric mirror map.
void Character::Method_0x004a6500()
{
    char line[0x100];
    ArchiveFile* file = new(__FILE__, 0x29d) ArchiveFile(g_pOwnerRegistry);
    int len = strlen(contentDirectory);
    int n = len > 0xff ? 0xff : len;
    strncpy(line, contentDirectory, n);
    line[n] = 0;
    strcat(line, mirFilename);
    if (!file->Open(line, "r", 0)) {
        delete file;
        return;
    }
    mirrorMap = (int*)operator new(nodeCount << 2, __FILE__, 0x2ab);
    while (file->ReadLine(line, 0xff)) {
        _strupr(line);
        int a = FindNode(strtok(line, "=,"));
        int b = FindNode(strtok(0, "=, \n"));
        mirrorMap[a] = b;
        mirrorMap[b] = a;
    }
    delete file;
}

// owner: Motnctrl.cpp (__FILE__ 0x56e034, line 0x2d2)
// 0x004a66c0: loads the .slt sections "MOTION 1".."MOTION n" through the manager.
int Character::LoadMotions()
{
    char section[0x50];
    currentMotion = 0;
    if (motionCount == 0)
        return 0;
    if (keepMotions)
        motions = (Motion**)DebugMalloc(motionCount * 4, __FILE__, 0x2d2);
    for (int i = 0; i < motionCount; i++) {
        sprintf(section, "MOTION %i", i + 1);
        Motion* motion = g_pMotionManager->Load(this, section);
        if (motion) {
            motion->refCount++;
            if (keepMotions)
                motions[i] = motion;
            SortMotion(motion);
            CaptureMotion(motion);
        }
    }
    return 1;
}

// owner: bracket only (Character method contiguous with its Motnctrl.cpp methods)
// 0x004a6780 (ret 8: the argument plus the hidden most-derived flag).
Character::Character(int a)
    : GameObject(a)
{
    chr_field_0x0c = 1;
    motionCount = 0;
    currentMotion = 0;
    currentFrame = 0;
    chr_field_0x10 = 0.0f;
    blendFromMotion = 0;
    chr_field_0x28 = 0;
    chr_field_0x24 = 0;
    blendFromTime = 0.0f;
    blendActive = 0;
    strcpy(sltPath, "");
    strcpy(vutFilename, "");
    strcpy(mirFilename, "");
    strcpy(contentDirectory, "");
    poseList.poses = 0;
    poseList.field_0x00 = 0;
    chr_field_0x24 = 0;
    vutLoaded = 0;
    motions = 0;
    blendDuration = 0.0f;
    chr_field_0x38 = 0;
    mirrorMap = 0;
    keepMotions = 1;
}

// owner: bracket only
// 0x004a6910: GameObject slot 8's body on the virtual base; the second argument is unused.
void Character::Method_0x004a6910(int a, int b)
{
    GameObject::GameObjectVirtualSlot8(a);
}

// owner: bracket only (Character slot 1, contiguous with the Motnctrl.cpp methods)
// slot 1 (0x004a6930): reads the "General info" keys, builds the node tables and loads the motions.
void Character::CharacterVirtualSlot1()
{
    sltFile->SelectSection("General info");
    sltFile->GetString("ContentDirectory", "", contentDirectory, -1);
    sltFile->GetString("VUTFilename", "\\", vutFilename, -1);
    sltFile->GetString("MIRFilename", "\\", mirFilename, -1);
    sltFile->GetInt("NumberOfMotions", 0, &motionCount);
    CharacterVirtualSlot0();
    CharacterVirtualSlot2();
    LoadMotions();
    Method_0x004a8b00();
}

// owner: Motnctrl.cpp (__FILE__ 0x56e034, lines 0x337, 0x343)
// Destructor core 0x004a69d0 (retail passes the GameObject virtual base as 'this').
Character::~Character()
{
    currentMotion = 0;
    currentFrame = 0;
    if (poseList.poses)
        operator delete(poseList.poses, __FILE__, 0x337);
    if (motions) {
        for (int i = 0; i < motionCount; i++)
            g_pMotionManager->Release(motions[i]);
        operator delete(motions, __FILE__, 0x343);
    }
}

// owner: bracket only
// 0x004a6a60: stores the current node pose of every frame of 'motion' (slot 3 per frame).
void Character::CaptureMotion(Motion* motion)
{
    Method_0x004a8b00();
    motion->captured = 1;
    for (int i = 0; i < motion->frameCount; i++)
        CharacterVirtualSlot3((int)&motion->frames[i]);
}

// owner: bracket only
// 0x004a6ab0: sorts the pose records of every frame of 'motion'.
void Character::SortMotion(Motion* motion)
{
    for (int i = 0; i < motion->frameCount; i++)
        Method_0x004a6b10(&motion->frames[i]);
    motion->sorted = 1;
}

// owner: bracket only
// 0x004a6b10: sorts a pose list by descending node index.
void Character::Method_0x004a6b10(void* poseList)
{
    MotionPoseList* list = (MotionPoseList*)poseList;
    qsort(list->poses, list->count, sizeof(CharacterPose), ComparePoses);
}

// owner: bracket only
// 0x004a6b30: looks a motion up by name in the global manager.
Motion* Character::FindMotion(const char* name, int a)
{
    return g_pMotionManager->Find(name, a);
}

// owner: bracket only
// 0x004a6b50: row of the node called 'name' (case-insensitive), 0xff when there is none.
unsigned char Character::FindNode(const char* name)
{
    for (int i = 0; i != nodeCount; i++) {
        if (_stricmp(nodeNames[i].name, name) == 0)
            return (unsigned char)i;
    }
    return 0xff;
}

// owner: bracket only
// slots 4, 5 and 6 (0x004a6ba0, one body after identical-COMDAT folding): empty in Character.
void Character::CharacterVirtualSlot4(int a, int b)
{
}

void Character::CharacterVirtualSlot5(int a, int b)
{
}

void Character::CharacterVirtualSlot6(int a, int b)
{
}

#include <math.h>
#include "math/FastMath.h"

// owner: bracket only
// 0x004a8440 (cdecl): clamps *value to [low, high].  An inline helper: 0x004a7fd0 inlines it once
// and calls this out-of-line copy once.
inline void ClampFloat(float low, float high, float* value)
{
    if (*value < low) {
        *value = low;
        return;
    }
    if (*value > high)
        *value = high;
}

// Unit vector through the inline Vec3 helpers (DotProduct, operator*).  0x004a7fd0 expands it six
// times; VC6 calls the out-of-line copies of DotProduct (0x0040ae30), operator* (0x005015b0) and
// Vec3::Vec3 (0x00404e60) at some of the sites.
inline Vec3 UnitVector(const Vec3& v)
{
    float lengthSquared = DotProduct(v, v);
    if (lengthSquared == 1.0f)
        return v;
    return v * FastInvSqrt(lengthSquared);
}

// 0x0042de90 (BoundingBoxTreeBuild.cpp): rotation of 'angle' radians about (x, y, z), row-vector
// convention, no translation.  Declared here with this TU's Matrix4.
void AxisAngleMatrix(Matrix4* m, float x, float y, float z, float angle);

// owner: bracket only
// 0x004a7dc0 (cdecl, called twice by slot 7 when the angle is nonzero): *dst = *src rotated by
// 'angle' about 'axis'.  The pose's axes and position form a matrix (row 0 the cross of axisY and
// axisZ), which is multiplied by the rotation; the position is copied unchanged.
void RotatePose(CharacterPose* src, CharacterPose* dst, Vec3 axis, float angle)
{
    Matrix4 pose;
    Matrix4 rotation;

    dst->nodeIndex = src->nodeIndex;
    dst->hasPose = src->hasPose;
    Vec3 axisZ = src->axisZ;
    Vec3 axisY = src->axisY;
    axisY = Vec3Normalize(axisY);
    axisZ = Vec3Normalize(axisZ);
    Vec3 axisX = CrossProductCall(axisY, axisZ);
    pose._11 = axisX.x;
    pose._12 = axisX.y;
    pose._13 = axisX.z;
    pose._14 = 0.0f;
    pose._21 = axisY.x;
    pose._22 = axisY.y;
    pose._23 = axisY.z;
    pose._24 = 0.0f;
    pose._31 = axisZ.x;
    pose._32 = axisZ.y;
    pose._33 = axisZ.z;
    pose._34 = 0.0f;
    Vec3 position;
    pose._41 = src->position.x;
    pose._42 = src->position.y;
    pose._43 = src->position.z;
    position.x = pose._41;
    position.y = pose._42;
    position.z = pose._43;
    pose._44 = 1.0f;
    AxisAngleMatrix(&rotation, axis.x, axis.y, axis.z, angle);
    MatrixMultiply(&pose, pose, rotation);
    dst->axisZ.x = pose._31;
    dst->axisZ.y = pose._32;
    dst->axisZ.z = pose._33;
    dst->axisY.x = pose._21;
    dst->axisY.y = pose._22;
    dst->axisY.z = pose._23;
    dst->position.x = position.x;
    dst->position.y = position.y;
    dst->position.z = position.z;
}

// owner: bracket only
// 0x004a7fd0 (cdecl, called twice by slot 7): the rotation taking pose a's axes to pose b's, as
// an axis and an angle.  It rotates axisY onto axisY when they differ, otherwise axisZ onto axisZ;
// identical axes give the axis (0, 0, 1) and the angle 0.
void PoseRotation(CharacterPose* a, CharacterPose* b, Vec3* axis, float* angle)
{
    if (a->axisY.x != b->axisY.x || a->axisY.y != b->axisY.y || a->axisY.z != b->axisY.z) {
        *axis = UnitVector(CrossProduct(a->axisY, b->axisY));
        *angle = DotProduct(UnitVector(a->axisY), UnitVector(b->axisY));
        ClampFloat(-1.0f, 1.0f, angle);
        *angle = (float)acos(*angle);
        return;
    }
    if (a->axisZ.x != b->axisZ.x || a->axisZ.y != b->axisZ.y || a->axisZ.z != b->axisZ.z) {
        *axis = UnitVector(CrossProduct(a->axisZ, b->axisZ));
        *angle = DotProduct(UnitVector(a->axisZ), UnitVector(b->axisZ));
        ClampFloat(-1.0f, 1.0f, angle);
        *angle = (float)acos(*angle);
        return;
    }
    *axis = Vec3(0.0f, 0.0f, 1.0f);
    *angle = 0.0f;
}

// owner: bracket only
// 0x004a8470 (cdecl, called by slot 7 and 0x004a9050): *out = the blend of poses a and b at t,
// clamped to [0, 1]: the axes and the position are interpolated linearly (the axes renormalised),
// then axisY is made orthogonal to axisZ again.  The three weighted sums at the end are
// computed into a temporary and never used (retail calls operator* and operator+ for them).
void InterpolatePose(CharacterPose* a, CharacterPose* b, CharacterPose* out, float t)
{
    ClampFloat(0.0f, 1.0f, &t);
    float s = 1.0f - t;
    out->nodeIndex = a->nodeIndex;
    out->hasPose = a->hasPose;
    out->axisZ = UnitVector(a->axisZ + (b->axisZ - a->axisZ) * t);
    out->axisY = UnitVector(a->axisY + (b->axisY - a->axisY) * t);
    out->position = a->position + (b->position - a->position) * t;
    Vec3 axisX = CrossProduct(out->axisZ, out->axisY);
    out->axisY = CrossProduct(axisX, out->axisZ);
    Vec3 unused;
    unused = a->axisZ * s + b->axisZ * t;
    unused = a->axisY * s + b->axisY * t;
    unused = a->position * s + b->position * t;
}

// owner: bracket only
// 0x004a8a80: passes every record of 'list' to slot 4.
void Character::ApplyPoseListSlot4(MotionPoseList* list, int mask)
{
    for (int i = 0; i < list->count; i++)
        CharacterVirtualSlot4((int)&list->poses[i], mask);
}

// owner: bracket only
// 0x004a8ac0: passes every record of 'list' to slot 6.
void Character::ApplyPoseListSlot6(MotionPoseList* list, int mask)
{
    for (int i = 0; i < list->count; i++)
        CharacterVirtualSlot6((int)&list->poses[i], mask);
}

// owner: bracket only
// 0x004a8b00: applies the rest pose list.
void Character::Method_0x004a8b00()
{
    ApplyPoseListSlot6(&poseList, 0);
}

// owner: bracket only
// 0x004a8b10: switches to the motion called 'name'.
void Character::SetMotionByName(const char* name)
{
    SetMotion(g_pMotionManager->Find(name, 1));
}

// owner: bracket only
// 0x004a8b40: switches to 'motion' at its first frame without blending.
void Character::SetMotion(Motion* motion)
{
    chr_field_0x0c = 0;
    currentMotion = motion;
    currentFrame = motion->frames;
    blendActive = 0;
}

// owner: bracket only
// 0x004a8b60: blends to 'motion' over its own transition time.
void Character::BlendToMotion(Motion* motion)
{
    BlendToMotion(motion, motion->transitionTime);
}

// owner: bracket only
// 0x004a8b80: blends from the current motion to 'motion' over 'time' when blending is enabled and
// both motions have the same number of poses per frame; otherwise switches directly.
void Character::BlendToMotion(Motion* motion, float time)
{
    if (time != 0.0f && chr_field_0x24 &&
        motion->frames->count == currentMotion->frames->count) {
        Motion* from = currentMotion;
        blendFromTime = chr_field_0x10;
        blendDuration = time;
        blendFromMotion = from;
        SetMotion(motion);
        blendActive = 1;
    } else {
        SetMotion(motion);
    }
}

// owner: Motnctrl.cpp (__FILE__ 0x56e034, line 0x585)
// slot 8 (0x004a98b0): copies the motion state, the file names, the node model (slot 9) and the rest
// pose list of another character.
void Character::CharacterVirtualSlot8(int a)
{
    Character* source = (Character*)a;
    chr_field_0x0c = source->chr_field_0x0c;
    chr_field_0x10 = source->chr_field_0x10;
    motionCount = source->motionCount;
    currentMotion = source->currentMotion;
    currentFrame = source->currentFrame;
    chr_field_0x24 = source->chr_field_0x24;
    blendFromTime = source->blendFromTime;
    blendDuration = source->blendDuration;
    blendActive = source->blendActive;
    chr_field_0x38 = source->chr_field_0x38;
    strcpy(sltPath, source->sltPath);
    strcpy(vutFilename, source->vutFilename);
    strcpy(mirFilename, source->mirFilename);
    strcpy(contentDirectory, source->contentDirectory);
    CharacterVirtualSlot9(a);
    poseList = source->poseList;
    poseList.poses = (CharacterPose*)DebugMalloc(poseList.count * sizeof(CharacterPose), __FILE__, 0x585);
    for (int i = 0; i < poseList.count; i++)
        poseList.poses[i] = source->poseList.poses[i];
    vutLoaded = 1;
    Method_0x004a8b00();
}

// owner: Motnctrl.cpp (__FILE__ 0x56e034, lines 0x5a5, 0x5a7)
// 0x004a9a30: frees a motion's frames and the motion.
void FreeMotion(Motion* motion)
{
    if (motion->frames) {
        for (int i = 0; i < motion->frameCount; i++)
            operator delete(motion->frames[i].poses, __FILE__, 0x5a5);
        operator delete(motion->frames, __FILE__, 0x5a7);
    }
    delete motion;
}
