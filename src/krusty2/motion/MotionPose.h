// Pose records used by D3DIMSoultreeMotnctrl.cpp.
#ifndef KRUSTY2_MOTION_MOTIONPOSE_H
#define KRUSTY2_MOTION_MOTIONPOSE_H

#include "../../../samples/physics/common/Math3D.h"

// One 0x2c-byte pose record (names tier 3): the node it targets (row index into the character's
// node table), a flag selecting between the two apply paths and the node pose.
struct CharacterPose {
    unsigned char nodeIndex;  // +0x00 row in the name table (<< 6 in the address arithmetic)
    int hasPose;              // +0x04 slot 3 stores 1 after capturing; slot 6 tests it
    Vec3 axisZ;               // +0x08 passed as axisZ to SetAxesPtr/SetAxesIn
    Vec3 axisY;               // +0x14 passed as axisY
    Vec3 position;            // +0x20 passed to SetPosition/TranslateIn
};

// Argument of slot 3: an array of pose records (names tier 3).
struct MotionPoseList {
    int field_0x00;
    CharacterPose* poses;        // +0x04 array of 0x2c-byte records
    int count;                // +0x08 element count
};
#endif
