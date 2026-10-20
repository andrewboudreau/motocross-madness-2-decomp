// Near misses of D:\aardvark\VC\krusty2\Motnctrl.cpp (0x004a5630..0x004a9a9f).  The strict exact
// functions of the unit and its types live in src/krusty2/motion/Motnctrl.cpp / Motnctrl.h; this file
// keeps only the reconstructions that do not match yet (scores in targets.json): the text .VUE
// loader, the slot 7 pose helpers and slot 7 itself.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "core/DebugAlloc.h"
#include "motion/Motnctrl.h"

extern "C" char* _strupr(char*);   // 0x00535d3d

// owner: Motnctrl.cpp (__FILE__ 0x56e034, lines 0x1c9, 0x1ec, 0x1f9, 0x22a)
// 0x004a5e40: text .VUE file.  A first pass counts the TRANSFORM lines of the first frame (and those
// naming a node of this character); the second pass reads one frame per line group, swapping y and z.
// Near miss (505/1090): retail keeps the file in ebx and poses (sharing the zero constant) in ebp, the
// candidate swaps ebx/ebp and esi/edi.  poses initialised first and the frame index stored before the
// count move it from 498; declaration/assignment orders do not change the permutation (targets.json).
int MotionManager::LoadVue(Character* character, const char* path, Motion* motion)
{
    char line[0x100];
    ArchiveFile* file = new(__FILE__, 0x1c9) ArchiveFile(g_pOwnerRegistry);
    if (!file->Open(path, "r", 0)) {
        char message[0x80];
        sprintf(message, "Unable to open %s\n", path);
        return 0;
    }
    int poses = 0;
    int transforms = 0;
    int frame = 0;
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
        list->frameIndex = frame;
        list->count = poses;
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
// The squared length through its own inline (DotProduct nested one level deeper) and the scaling in
// an else block set VC6's inline budget like retail in 0x004a7fd0: in the axisY path DotProduct is
// called and operator* expanded (calling the constructor) for the first two unit vectors, the outer
// DotProduct and ClampFloat are expanded; in the axisZ path all of them are called.
inline float PoseLengthSquared(const Vec3& v)
{
    return DotProduct(v, v);
}

inline Vec3 UnitVector(const Vec3& v)
{
    float lengthSquared = PoseLengthSquared(v);
    if (lengthSquared == 1.0f)
        return v;
    else {
        float inverseLength = FastInvSqrt(lengthSquared);
        return v * inverseLength;
    }
}

// 0x0042de90 (BoundingBoxTreeBuild.cpp): rotation of 'angle' radians about (x, y, z), row-vector
// convention, no translation.  Declared here with this TU's Matrix4.
void AxisAngleMatrix(Matrix4* m, float x, float y, float z, float angle);

// owner: bracket only
// 0x004a7dc0 (cdecl, called twice by slot 7 when the angle is nonzero): *dst = *src rotated by
// 'angle' about 'axis'.  The pose's axes and position form a matrix (row 0 the cross of axisY and
// axisZ), which is multiplied by the rotation; the position is copied unchanged.
// Partial (targets.json); 0..15 dead initialised int locals at the top of the function change nothing.
// Swept (tools/sweep_variants.py): every declaration order of pose/rotation/position/axisX/axisZ
// (hoisted or not) leaves 456 masked; normalising axisZ first gives 448; initialising from
// Vec3Normalize(src->...), via copies, named or scoped temporaries, and `Vec3 position =
// src->position` are all worse (55..193). The position block written as chained
// assignments or interleaved per component gives 205; storing pose._44 before the position copies leaves 476, as do
// an assigned or direct-initialised axisX, a const Vec3& of src->position and a float copy of its x
// (the fld of position.x still schedules above the axisX copies).
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
// Near miss (1097/1121): the call sequence and frame match; only the axisZ cross product (laid out
// first) loads b's component first in each product where retail loads a's (the axisY one matches).
// No gain from a local cross helper with or without parentheses or with swapped operands, a named
// cross local, swapped comparison operands, or swapped DotProduct arguments (registers swap, 1099
// strict but esi/edi exchanged, not kept).  Also no gain: a const Vec3& alias of a->axisZ or
// b->axisZ (in the cross or in the test, 1069..1097), a Vec3 copy (188..198), a pointer alias
// (1097), an inline PoseAxisDiffers test by reference or by value (23..130), or reordered or
// swapped component comparisons in the axisZ test (1004..1006).
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
// Retail also constructs two poses that are never used (two calls of CharacterPose's generated
// constructor, 0x004a8930, and their 0x58 bytes of frame); with them VC6 calls both
// CrossProducts and the unused sums' operators out of line as retail does (inline budget).
void InterpolatePose(CharacterPose* a, CharacterPose* b, CharacterPose* out, float t)
{
    ClampFloat(0.0f, 1.0f, &t);
    float s = 1.0f - t;
    out->nodeIndex = a->nodeIndex;
    out->hasPose = a->hasPose;
    CharacterPose unused1, unused2;
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

// The cubic ease of the blend weight, as in src/krusty2/motion/Motnctrl.cpp (AdvanceMotion): the
// float copy through a folded double expression reproduces retail's FPU shape (slot 7 at
// 0x004a7711: fld w; fld st0; fadd st0,st0; fsubr 3.0f; fmul st1; fmul st1; fstp; fstp st0) while
// the clamp before it keeps reloading the weight from memory.
inline float SmoothStep(float x)
{
    float t = (float)(x * 1.0);
    return (3.0f - (t + t)) * t * t;
}

// Slot 7's expansion of the pose blend (the same inline that AdvanceMotion, strict exact in
// src/krusty2/motion/Motnctrl.cpp, expands with the first term's operators inlined).  In slot 7
// retail calls the out-of-line copies of every operator (0x421d00, 0x5015b0, 0x421cb0, 0x515600),
// which the plain operators reproduce once the inline budget runs out, and also calls
// CharacterPose's generated default constructor (0x004a8930, `mov eax,ecx`) for pose temporaries
// that are never used otherwise: two in the first expansion and four in the second.  With four
// unused poses VC6 calls it three times and twice; six or two unused poses move both counts
// together (five/four, one/zero), so the retail split is not a count of locals.
inline void BlendPose7(CharacterPose* a, CharacterPose* b, CharacterPose* out, float t)
{
    ClampFloat(0.0f, 1.0f, &t);
    float s = 1.0f - t;
    out->nodeIndex = a->nodeIndex;
    out->hasPose = a->hasPose;
    CharacterPose unused1, unused2, unused3, unused4;
    out->axisZ = Vec3Normalize(a->axisZ + (b->axisZ - a->axisZ) * t);
    out->axisY = Vec3Normalize(a->axisY + (b->axisY - a->axisY) * t);
    out->position = a->position + (b->position - a->position) * t;
    Vec3 axisX = CrossProduct(out->axisZ, out->axisY);
    out->axisY = CrossProduct(axisX, out->axisZ);
    Vec3 unused;
    unused = a->axisZ * s + b->axisZ * t;
    unused = a->axisY * s + b->axisY * t;
    unused = a->position * s + b->position * t;
}

// 0x004a5750, strict exact in src/krusty2/motion/Motnctrl.cpp: non-looping advance returning the two
// frames to blend and the blend weight.
int AdvanceClamped(Motion* motion, MotionPoseList** cursor, MotionPoseList** from, MotionPoseList** to,
                   float* time, float* weight);

// owner: bracket only (Character method contiguous with its Motnctrl.cpp methods)
// 0x004a70c0 (ret 0xc; Character slot 7, inherited by D3DIMSoultreeCharacter): per-frame playback of
// a non-looping motion with sub-frame interpolation.  The current motion is advanced by AdvanceClamped;
// every pose is interpolated between the two frames around the cursor (or, past the last frame,
// extrapolated by the rotation and translation of the last frame pair) and applied through slot 4
// (mirror == 0) or slot 5.  While a blend is active the blend-from motion is advanced the same way and
// the two poses are interpolated with a smoothstep weight over blendDuration; nodes flagged in the
// name table keep the blend-from pose.  Past the last frame a looping motion is restarted through
// BlendToMotion, a non-looping one sets motionStopped.  Returns the frame number, or 0.
// Near miss, 3330 vs 3327 bytes: the call sequence and the clamp/ease shape are retail's, but the
// inline budget is spent differently (see BlendPose7; in the first extrapolation loop retail inlines
// only operator- for the frame delta, calling Vec3::Vec3 0x404e60 for its result, and calls the
// operator* / operator+ COMDATs that VC6 inlines here) and the frame is 0x284 instead of 0x2dc.
// Lead (session 14, not kept: strict 127 < 152 while the tail is still off): the ctor-call split
// follows from the unused-pose counts once the two blend expansions are separate inlines.  With
// 3 unused poses in the first expansion and 6 (or 4..5 plus 3..6 dead CharacterPose locals at the
// top of this function) in the second, VC6 calls the ctor 2 and 4 times and the frame is 0x2dc as
// in retail; the first extrapolation then matches retail's call sequence (PoseRotation,
// RotatePose, Vec3::Vec3, operator*, operator*, operator+) when it is an inline helper taking
// (a, b, out, flag, t, fps) with the named delta.  Left: the second extrapolation, which retail
// expands with operator-/*/*/+ calls; the same helper there is called out of line, written
// directly it inlines everything, and nested position helpers move the first one's budget.
// (One identical-body expansion per loop always gives counts n and n - 1, whatever is added.)
int Character::CharacterVirtualSlot7(float time, int mirror, int mask)
{
    MotionPoseList* from;
    MotionPoseList* to;
    float weight;
    int finished;
    int ok;

    if (motionStopped)
        return 0;
    chr_field_0x10 += time;
    finished = 0;
    if (AdvanceClamped(currentMotion, &currentFrame, &from, &to, &chr_field_0x10, &weight)) {
        ok = 1;
    } else {
        ok = 0;
        finished = 1;
    }
    if (blendActive == 0) {
        lastFrame = currentFrame;
        lastFrameTime = chr_field_0x10;
        if (ok) {
            for (int i = 0; i < from->count; i++) {
                CharacterPose pose;
                BlendPose7(&from->poses[i], &to->poses[i], &pose, weight);
                if (mirror == 0)
                    CharacterVirtualSlot4((int)&pose, mask);
                else
                    CharacterVirtualSlot5((int)&pose, mask);
            }
        } else {
            for (int i = 0; i < from->count; i++) {
                CharacterPose* a = &from->poses[i];
                int flag = nodeNames[a->nodeIndex].applyTranslation;
                float t = chr_field_0x10;
                int fps = currentMotion->fps;
                CharacterPose* b = &to->poses[i];
                Vec3 axis;
                float angle;
                CharacterPose out;
                PoseRotation(a, b, &axis, &angle);
                float ffps = (float)fps;
                out.hasPose = a->hasPose;
                angle = ffps * angle * t;
                out.nodeIndex = a->nodeIndex;
                if (angle != 0.0f) {
                    RotatePose(b, &out, axis, angle);
                } else {
                    out.axisZ = b->axisZ;
                    out.axisY = b->axisY;
                }
                if (flag) {
                    Vec3 delta = b->position - a->position;
                    out.position = b->position + delta * ffps * t;
                } else {
                    out.position = b->position;
                }
                if (mirror == 0)
                    CharacterVirtualSlot4((int)&out, mask);
                else
                    CharacterVirtualSlot5((int)&out, mask);
            }
        }
    } else {
        MotionPoseList* from2;
        MotionPoseList* to2;
        float weight2;

        blendFromTime += time;
        lastFrameTime += time;
        float r = blendFromTime / blendDuration;
        float w;
        if (r < 0.0f)
            w = 0.0f;
        else if (r >= 1.0f)
            w = 1.0f;
        else
            w = SmoothStep(r);
        int ok2 = AdvanceClamped(blendFromMotion, &lastFrame, &from2, &to2, &lastFrameTime, &weight2) != 0;
        for (int j = 0; j < from->count; j++) {
            CharacterPose pose2;
            CharacterPose out;
            CharacterPose result;
            BlendPose7(&from->poses[j], &to->poses[j], &out, weight);
            if (ok2) {
                InterpolatePose(&from2->poses[j], &to2->poses[j], &pose2, weight2);
            } else {
                CharacterPose* a = &from2->poses[j];
                int flag = nodeNames[a->nodeIndex].applyTranslation;
                float t = lastFrameTime;
                int fps = blendFromMotion->fps;
                CharacterPose* b = &to2->poses[j];
                Vec3 axis;
                float angle;
                PoseRotation(a, b, &axis, &angle);
                float ffps = (float)fps;
                pose2.hasPose = a->hasPose;
                angle = angle * t * ffps;
                pose2.nodeIndex = a->nodeIndex;
                if (angle != 0.0f) {
                    RotatePose(b, &pose2, axis, angle);
                } else {
                    pose2.axisZ = b->axisZ;
                    pose2.axisY = b->axisY;
                }
                if (flag) {
                    pose2.position = b->position + (b->position - a->position) * ffps * t;
                } else {
                    pose2.position = b->position;
                }
            }
            InterpolatePose(&pose2, &out, &result, w);
            if (nodeNames[pose2.nodeIndex].applyTranslation)
                result = pose2;
            else
                InterpolatePose(&pose2, &out, &result, w);
            if (mirror == 0)
                CharacterVirtualSlot4((int)&result, mask);
            else
                CharacterVirtualSlot5((int)&result, mask);
        }
        if (blendFromTime >= blendDuration) {
            blendActive = 0;
            lastFrameTime = 0.0f;
        }
    }
    if (finished) {
        if (currentMotion->looping) {
            BlendToMotion(currentMotion);
            return 0;
        }
        motionStopped = 1;
        chr_field_0x10 = 0.0f;
        lastFrameTime = 0.0f;
        return 0;
    }
    return currentFrame->frameIndex;
}
