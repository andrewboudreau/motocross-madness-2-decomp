// Motnctrl.h -- the types of Motnctrl.cpp (the motion cache and its file readers), shared by
// src/krusty2/motion/Motnctrl.cpp and the near misses in samples/physics/motion/Motnctrl.cpp.
#ifndef KRUSTY2_MOTION_MOTNCTRL_H
#define KRUSTY2_MOTION_MOTNCTRL_H

#include "motion/D3DIMSoultreeCharacter.h"
#include "motion/MotionPose.h"

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

#endif
