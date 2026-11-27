// Motion.h -- one motion of the MotionManager cache (Motnctrl.cpp). Split from Motnctrl.h
// so readers of a character's current motion (wrecker.cpp) need not include Math3D.h,
// whose per-TU vector constants would add dynamic initializers to their unit.
#ifndef KRUSTY2_MOTION_MOTION_H
#define KRUSTY2_MOTION_MOTION_H

struct MotionPoseList;  // D3DIMSoultreeCharacter.h

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

#endif
