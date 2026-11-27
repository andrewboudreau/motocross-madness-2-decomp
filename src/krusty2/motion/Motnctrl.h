// Motnctrl.h -- the types of Motnctrl.cpp (the motion cache and its file readers), shared by
// src/krusty2/motion/Motnctrl.cpp and the near misses in samples/physics/motion/Motnctrl.cpp.
#ifndef KRUSTY2_MOTION_MOTNCTRL_H
#define KRUSTY2_MOTION_MOTNCTRL_H

#include "math/Math3D.h"
#include "motion/D3DIMSoultreeCharacter.h"
#include "motion/MotionPose.h"

// Shared archive stream and section/key reader; original class names are provisional.
#include "../../reconstructed/FileStream.h"
extern ResourceManager* g_ResourceManager; // 0x00572b44
#include "../../reconstructed/Parameterblocks.h"

#include "motion/Motion.h"

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
