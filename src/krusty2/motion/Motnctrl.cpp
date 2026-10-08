// Motnctrl.cpp -- reconstruction of D:\aardvark\VC\krusty2\Motnctrl.cpp (motion control: the global
// motion manager, the .VUE/.MOT motion loaders and Character's motion playback).
//
// The retail __FILE__ string 'D:\aardvark\VC\krusty2\Motnctrl.cpp' is at 0x0056e034; its xrefs run
// from 0x004a5659 (the manager initialiser, line 9) to 0x004a9a87 (FreeMotion, line 0x5a7) with the
// line numbers rising with the address.  Character's shape comes from
// src/krusty2/motion/D3DIMSoultreeCharacter.h, which is included, not redeclared.  See
// src/krusty2/motion/README.md for the evidence summary.
//
// The unit is 0x004a5630..0x004a9a9f: the manager initialiser (.CRT$XCU 192, line 9) first, the
// Math3D.h vector set (.CRT$XCU 188-191, 0x004a8940..0x004a8a7b, globals 0x00685120..0x00685158)
// mid-file and FreeMotion (line 0x5a7) last.  Every function here is strict exact with all
// relocations resolved (Motnctrl.bindings.json).  Not here: the text .VUE loader 0x004a5e40,
// the slot 7 helpers 0x004a7dc0, 0x004a7fd0, 0x004a8440 (an out-of-line copy of an inline) and
// 0x004a8470, and slot 7 0x004a70c0 itself (near misses, samples/physics/motion/Motnctrl.cpp),
// and 0x004a8bf0, 0x004a8c50 and 0x004a9050 (they round with direct fistp instructions where the
// rest of the unit calls __ftol: __asm, not reproducible here).
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "core/DebugAlloc.h"
#include "motion/D3DIMSoultreeCharacter.h"
#include "motion/MotionPose.h"   // Math3D.h: the four per-TU Vec3 initialisers (.CRT$XCU 188-191)
#include "motion/Motnctrl.h"

extern "C" char* _strupr(char*);   // 0x00535d3d

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
void Character::LoadVut()
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
void Character::LoadMir()
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
    lastFrameTime = 0.0f;
    lastFrame = 0;
    blendFromTime = 0.0f;
    blendActive = 0;
    strcpy(sltPath, "");
    strcpy(vutFilename, "");
    strcpy(mirFilename, "");
    strcpy(contentDirectory, "");
    poseList.poses = 0;
    poseList.field_0x00 = 0;
    lastFrame = 0;
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
    ApplyRestPose();
}

// owner: Motnctrl.cpp (__FILE__ 0x56e034, lines 0x337, 0x343)
// Destructor core 0x004a69d0 (retail passes the GameObject virtual base as 'this').
Character::~Character()
{
    currentMotion = 0;
    currentFrame = 0;
    if (poseList.poses)
        DebugFree(poseList.poses, __FILE__, 0x337);
    if (motions) {
        for (int i = 0; i < motionCount; i++)
            g_pMotionManager->Release(motions[i]);
        DebugFree(motions, __FILE__, 0x343);
    }
}

// owner: bracket only
// 0x004a6a60: stores the current node pose of every frame of 'motion' (slot 3 per frame).
void Character::CaptureMotion(Motion* motion)
{
    ApplyRestPose();
    motion->captured = 1;
    for (int i = 0; i < motion->frameCount; i++)
        CharacterVirtualSlot3((int)&motion->frames[i]);
}

// owner: bracket only
// 0x004a6ab0: sorts the pose records of every frame of 'motion'.
void Character::SortMotion(Motion* motion)
{
    for (int i = 0; i < motion->frameCount; i++)
        SortPoseList(&motion->frames[i]);
    motion->sorted = 1;
}

// owner: bracket only
// 0x004a6b10: sorts a pose list by descending node index.
void Character::SortPoseList(void* poseList)
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

// Inline helper of the playback methods: clamps *value to [low, high].  The out-of-line copy VC6
// emits for it is 0x004a8440 (exact in samples/physics/motion/Motnctrl.cpp, where PoseRotation
// calls it once).
inline void ClampFloat(float low, float high, float* value)
{
    if (*value < low) {
        *value = low;
        return;
    }
    if (*value > high)
        *value = high;
}

// The cubic ease 3x^2 - 2x^3 of the blend weight.  Retail (0x004a6cd3, and slot 7 at 0x004a7711)
// loads the memory-homed weight once and keeps a copy on the FPU stack (fld st0; fadd st0,st0;
// fsubr 3.0f; fmul st1; fmul st1; fstp w; fstp st0).  A float spelling of the ease reloads the
// weight for every use, and any double-typed use of the weight makes VC6 keep it on the stack
// through the preceding clamp as well (retail reloads it there).  The float copy made through a
// double expression that folds away gives both: VC6 drops the `* 1.0` but keeps the conversion
// nodes, so the copy is a register temporary distinct from the weight in memory.
inline float SmoothStep(float x)
{
    float t = (float)(x * 1.0);
    return (3.0f - (t + t)) * t * t;
}

// The inlined form of InterpolatePose (0x004a8470) as AdvanceMotion expands it: VC6 inlines the
// operators of the first term (calling Vec3::Vec3 0x404e60 for their results) and calls the
// out-of-line copies (0x421d00, 0x5015b0, 0x421cb0, 0x515600) for the rest, which the call views
// of Math3D.h reproduce.  The three weighted sums at the end are never used.
inline void BlendPose(CharacterPose* a, CharacterPose* b, CharacterPose* out, float t)
{
    ClampFloat(0.0f, 1.0f, &t);
    float s = 1.0f - t;
    out->nodeIndex = a->nodeIndex;
    out->hasPose = a->hasPose;
    out->axisZ = Vec3Normalize(a->axisZ + (b->axisZ - a->axisZ) * t);
    Vec3 deltaY, scaledY, sumY;
    out->axisY = Vec3Normalize(*Vec3AddCall(&sumY, &a->axisY,
                                            Vec3ScaleCall(&scaledY, Vec3SubtractCall(&deltaY, &b->axisY, &a->axisY), t)));
    Vec3 deltaP, scaledP, sumP;
    out->position = *Vec3AddCall(&sumP, &a->position,
                                 Vec3ScaleCall(&scaledP, Vec3SubtractCall(&deltaP, &b->position, &a->position), t));
    Vec3 axisX = CrossProductCall(out->axisZ, out->axisY);
    out->axisY = CrossProductCall(axisX, out->axisZ);
    Vec3 weightedZA, weightedZB, blendedZ;
    Vec3AddCall(&blendedZ, Vec3ScaleCall(&weightedZA, &a->axisZ, s), Vec3ScaleCall(&weightedZB, &b->axisZ, t));
    Vec3 weightedYA, weightedYB, blendedY;
    Vec3AddCall(&blendedY, Vec3ScaleCall(&weightedYA, &a->axisY, s), Vec3ScaleCall(&weightedYB, &b->axisY, t));
    Vec3 weightedPA, weightedPB, blendedPosition;
    Vec3AddCall(&blendedPosition, Vec3ScaleCall(&weightedPA, &a->position, s), Vec3ScaleCall(&weightedPB, &b->position, t));
}

// owner: bracket only (Character method contiguous with its Motnctrl.cpp methods)
// 0x004a6bb0 (ret 0xc; callers 0x49945f, 0x499627, 0x4997b1, 0x4ead18, 0x4eae78, 0x4eb108):
// per-frame playback.  Advances the current motion by dt (a finished non-looping motion sets
// chr_field_0x0c and returns 0), then applies every pose of the current frame through slot 4
// (mirror == 0) or slot 5.  While a blend is active each pose is interpolated from the frame
// applied last with a smoothstep weight over blendDuration.  Returns the frame number.
int Character::AdvanceMotion(float dt, int mirror, int mask)
{
    if (chr_field_0x0c)
        return 0;
    chr_field_0x10 += dt;
    if (!AdvanceLooping(currentMotion, &currentFrame, &chr_field_0x10) && currentMotion->looping == 0) {
        chr_field_0x10 = 0.0f;
        chr_field_0x0c = 1;
        return 0;
    }
    if (blendActive == 0) {
        lastFrame = currentFrame;
        lastFrameTime = chr_field_0x10;
        for (int i = 0; i < currentFrame->count; i++) {
            if (mirror == 0)
                CharacterVirtualSlot4((int)&currentFrame->poses[i], mask);
            else
                CharacterVirtualSlot5((int)&currentFrame->poses[i], mask);
        }
    } else {
        blendFromTime += dt;
        float w = blendFromTime / blendDuration;
        if (w < 0.0f)
            w = 0.0f;
        else if (w >= 1.0f)
            w = 1.0f;
        else
            w = SmoothStep(w);
        for (int j = 0; j < currentFrame->count; j++) {
            CharacterPose pose;
            BlendPose(&lastFrame->poses[j], &currentFrame->poses[j], &pose, w);
            if (mirror == 0)
                CharacterVirtualSlot4((int)&pose, mask);
            else
                CharacterVirtualSlot5((int)&pose, mask);
        }
        if (blendFromTime >= blendDuration)
            blendActive = 0;
    }
    return currentFrame->field_0x00;
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
void Character::ApplyRestPose()
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
    if (time != 0.0f && lastFrame &&
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
    lastFrame = source->lastFrame;
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
    ApplyRestPose();
}

// owner: Motnctrl.cpp (__FILE__ 0x56e034, lines 0x5a5, 0x5a7)
// 0x004a9a30: frees a motion's frames and the motion.
void FreeMotion(Motion* motion)
{
    if (motion->frames) {
        for (int i = 0; i < motion->frameCount; i++)
            DebugFree(motion->frames[i].poses, __FILE__, 0x5a5);
        DebugFree(motion->frames, __FILE__, 0x5a7);
    }
    delete motion;
}
