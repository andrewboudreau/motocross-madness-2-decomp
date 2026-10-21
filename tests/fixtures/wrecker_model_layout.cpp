#include <stddef.h>
#include "../../src/reconstructed/Wrecker.h"
#include "../../src/krusty2/motion/Motnctrl.h"

// The local views carry identical storage at the explicit call boundaries.
typedef char vector_sizes[sizeof(Vector3) == sizeof(Vec3) && sizeof(Vec3) == 12 ? 1 : -1];
typedef char vector_x[offsetof(Vector3, x) == offsetof(Vec3, x) ? 1 : -1];
typedef char vector_y[offsetof(Vector3, y) == offsetof(Vec3, y) ? 1 : -1];
typedef char vector_z[offsetof(Vector3, z) == offsetof(Vec3, z) ? 1 : -1];
typedef char matrix_sizes[sizeof(UnknownWreckerMatrix) == sizeof(Matrix4) && sizeof(Matrix4) == 64 ? 1 : -1];
typedef char matrix_position[offsetof(UnknownWreckerMatrix, _41) == offsetof(Matrix4, _41) ? 1 : -1];
typedef char frame_size[sizeof(UnknownWreckerFrame) == 64 ? 1 : -1];
typedef char frame_position[offsetof(UnknownWreckerFrame, position) == offsetof(Matrix4, _41) ? 1 : -1];
typedef char skeleton_child[offsetof(SoultreeObject, firstChild) == 0x140 ? 1 : -1];
typedef char skeleton_dirty[offsetof(SoultreeObject, subtreeDirty) == 0x18c ? 1 : -1];
typedef char character_motion[offsetof(Character, currentMotion) == 0x18 ? 1 : -1];
typedef char character_frame[offsetof(Character, currentFrame) == 0x1c ? 1 : -1];
typedef char character_blend[offsetof(Character, blendActive) == 0x34 ? 1 : -1];
typedef char character_pose[offsetof(Character, poseList) == 0x190 ? 1 : -1];
typedef char character_model[offsetof(D3DIMSoultreeCharacter, modelNode) == 0x1a0 ? 1 : -1];
typedef char motion_count[offsetof(UnknownWreckerMotion, frameCount) == offsetof(Motion, frameCount) ? 1 : -1];
typedef char motion_fps[offsetof(UnknownWreckerMotion, fps) == offsetof(Motion, fps) ? 1 : -1];
typedef char pose_frame[offsetof(MotionPoseList, frameIndex) == 0 ? 1 : -1];
typedef char pose_size[sizeof(MotionPoseList) == 12 ? 1 : -1];
typedef char wrecker_size[sizeof(Wrecker) == 0x53c ? 1 : -1];
