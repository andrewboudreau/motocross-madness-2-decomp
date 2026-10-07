// Near misses of D:\aardvark\VC\krusty2\Motnctrl.cpp (0x004a5630..0x004a9a9f).  The strict exact
// functions of the unit and its types live in src/krusty2/motion/Motnctrl.cpp / Motnctrl.h; this file
// keeps only the reconstructions that do not match yet (scores in targets.json): the text .VUE
// loader and the slot 7 pose helpers.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "core/DebugAlloc.h"
#include "motion/Motnctrl.h"

extern "C" char* _strupr(char*);   // 0x00535d3d

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
// Retail stores the inverse length to a stack temp before scaling; the named local reproduces it.
inline Vec3 UnitVector(const Vec3& v)
{
    float lengthSquared = DotProduct(v, v);
    if (lengthSquared == 1.0f)
        return v;
    float inverseLength = FastInvSqrt(lengthSquared);
    return v * inverseLength;
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

// The cubic ease 3x^2 - 2x^3 of the blend weight.  Retail (0x004a6cd3, and slot 7 at 0x004a7711)
// loads the weight once and keeps it on the FPU stack (fld st0; fadd st0,st0; fsubr 3.0f;
// fmul st1; fmul st1); every float spelling tried reloads it from memory instead, and a double
// parameter keeps the weight on the stack through the preceding clamp too (retail reloads there).
inline float SmoothStep(float x)
{
    return (3.0f - (x + x)) * x * x;
}

// The inlined form of InterpolatePose above as AdvanceMotion expands it: VC6 inlines the operators
// of the first term and calls the out-of-line copies (0x421d00, 0x5015b0, 0x421cb0, 0x515600) for
// the rest, which the call views reproduce.  The three weighted sums at the end are never used.
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

// 0x004a56d0, strict exact in src/krusty2/motion/Motnctrl.cpp: advances *cursor by *time within the
// looping motion and returns 0 when the motion ran out.
int AdvanceLooping(Motion* motion, MotionPoseList** cursor, float* time);

// owner: bracket only (Character method contiguous with its Motnctrl.cpp methods)
// 0x004a6bb0 (ret 0xc; callers 0x49945f, 0x499627, 0x4997b1, 0x4ead18, 0x4eae78, 0x4eb108):
// per-frame playback.  Advances the current motion by dt (a finished non-looping motion sets
// chr_field_0x0c and returns 0), then applies every pose of the current frame through slot 4
// (mirror == 0) or slot 5.  While a blend is active each pose is interpolated from the frame
// applied last with a smoothstep weight over blendDuration.  Returns the frame number.
// 1295 vs 1293 bytes: identical up to the ease (see SmoothStep), which shifts the rest.
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
