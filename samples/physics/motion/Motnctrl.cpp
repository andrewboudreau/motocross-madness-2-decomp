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
    UnknownTextureStream* file = new(__FILE__, 0x1c9) UnknownTextureStream(g_ResourceManager);
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
    file->Seek(file->sliceOffset, 0, 1);
    while (file->ReadLine(line, 0xff) && frame < motion->frameCount) {
        _strupr(line);
        strtok(line, " ");
        MotionPoseList* list = &motion->frames[frame];
        list->poses = (CharacterPose*)DebugMalloc(poses * sizeof(CharacterPose), __FILE__, 0x1f9);
        list->frameIndex = frame;
        list->count = poses;
        int kept = 0;
        for (int transform = 0; transform < transforms; transform++) {
            file->ReadLine(line, 0xff);
            strtok(line, " ");
            int node = character->FindNode(strtok(0, " \""));
            if (node != 0xff) {
                list->poses[kept].nodeIndex = node;
                list->poses[kept].hasPose = 0;
                strtok(0, " ");
                strtok(0, " ");
                strtok(0, " ");
                list->poses[kept].axisZ.x = atof(strtok(0, " "));
                list->poses[kept].axisZ.z = atof(strtok(0, " "));
                list->poses[kept].axisZ.y = atof(strtok(0, " "));
                list->poses[kept].axisY.x = atof(strtok(0, " "));
                list->poses[kept].axisY.z = atof(strtok(0, " "));
                list->poses[kept].axisY.y = atof(strtok(0, " "));
                list->poses[kept].position.x = atof(strtok(0, " "));
                list->poses[kept].position.z = atof(strtok(0, " "));
                list->poses[kept].position.y = atof(strtok(0, " "));
                kept++;
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
// convention, no translation. Its provider uses the identical 64-byte tree matrix.
struct TreeMatrix4;
void AxisAngleMatrix(TreeMatrix4* m, float x, float y, float z, float angle);

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
// (the fld of position.x still schedules above the axisX copies).  Frame diagnosis (session 14):
// the two Vec3Normalize return temporaries share axisX's slot at +0 here where retail gives
// them a dedicated slot at +0x64; hoisting the pose locals does not separate them.  The one
// remaining code difference is the fld of src->position.x scheduled right after the cross
// product call (retail: four moves later); every order of the three axis rows and the position
// block, and the position copied first or as chained assignments, keep it (452..476).
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
    AxisAngleMatrix(reinterpret_cast<TreeMatrix4*>(&rotation), axis.x, axis.y, axis.z, angle);
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
// swapped component comparisons in the axisZ test (1004..1006).  Retail holds a in edi and b in
// esi.  Nor from a PoseAxisRotation inline for either path or both (326..334 differing lines),
// a cross wrapper (173), the axisZ test negated as `!(... == ... && ...)` (inert) or with
// swapped operands (the test changes, the cross does not).
// Retail holds a in edi and b in esi in both paths, yet loads b first in the axisY cross
// and a first in the axisZ one, so the fixed indirect-word order of VC6_PATTERN_CORPUS
// section 6 is not decided by the pointer pair alone. Also inert: `else if` / `else`
// chaining and `*&` operands; swapped axisZ compares with or without swapped dot
// arguments give 1089-1093.
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
// Two unused pose locals reproduce the inline budget of this candidate. The complete retail
// body has no CharacterPose constructor calls; their original declaration is unproven.
inline float InterpolatedLengthSquared(const Vec3& v)
{
    return Vec3DotCall(&v, &v);
}
inline Vec3 InterpolatedUnitVector(const Vec3& v)
{
    float lengthSquared = InterpolatedLengthSquared(v);
    if (lengthSquared == 1.0f)
        return v;
    else {
        float inverseLength = FastInvSqrt(lengthSquared);
        return v * inverseLength;
    }
}

void InterpolatePose(CharacterPose* a, CharacterPose* b, CharacterPose* out, float t)
{
    ClampFloat(0.0f, 1.0f, &t);
    float s = 1.0f - t;
    out->nodeIndex = a->nodeIndex;
    out->hasPose = a->hasPose;
    CharacterPose unused1, unused2;
    out->axisZ = InterpolatedUnitVector(a->axisZ + (b->axisZ - a->axisZ) * t);
    out->axisY = InterpolatedUnitVector(a->axisY + (b->axisY - a->axisY) * t);
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

// Slot 7's expansions of the pose blend (the same inline that AdvanceMotion, strict exact in
// src/krusty2/motion/Motnctrl.cpp, expands with the first term's operators inlined).  In slot 7
// retail calls the out-of-line copies of every operator (0x421d00, 0x5015b0, 0x421cb0, 0x515600),
// which the plain operators reproduce once the inline budget runs out, and also calls
// CharacterPose's generated default constructor (0x004a8930, `mov eax,ecx`) for pose temporaries
// that are never used otherwise: twice in the first expansion and four times in the second.  One
// inline expanded in both loops always gives counts n and n - 1, so the playback loop and the
// blend loop use two inlines with the same body and different numbers of unused poses (strong
// inference from the constructor calls; the local counts themselves are not observable).  Three and
// five give retail's 2/4 split and the best alignment (aligned instructions 342 -> 675 together with
// the extrapolation helper below); the earlier single inline with four gave 3/2.
inline void BlendPose7(CharacterPose* a, CharacterPose* b, CharacterPose* out, float t)
{
    ClampFloat(0.0f, 1.0f, &t);
    float s = 1.0f - t;
    out->nodeIndex = a->nodeIndex;
    out->hasPose = a->hasPose;
    CharacterPose unused1, unused2, unused3;
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

// The blend loop's copy of BlendPose7 (see above).
inline void BlendPose7B(CharacterPose* a, CharacterPose* b, CharacterPose* out, float t)
{
    ClampFloat(0.0f, 1.0f, &t);
    float s = 1.0f - t;
    out->nodeIndex = a->nodeIndex;
    out->hasPose = a->hasPose;
    CharacterPose unused1, unused2, unused3, unused4, unused5;
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

// Pose b extrapolated past the last frame pair (a, b) by t seconds at fps: the rotation from a to b
// scaled to the elapsed frames, and (when the node applies translation) the frame delta likewise.
// As an inline helper of the playback loop VC6 calls PoseRotation, RotatePose, Vec3::Vec3 for the
// delta and the operator*/operator+ COMDATs in retail's order (one more unused pose here also
// matches their placement, aligned 705, but that local would be padding).  The blend loop writes
// the same steps out: retail expands them there with operator- called too, and the helper would be
// called out of line once the budget is spent.
inline void ExtrapolatePose7(CharacterPose* a, CharacterPose* b, CharacterPose* out, int flag, float t, int fps)
{
    Vec3 axis;
    float angle;
    PoseRotation(a, b, &axis, &angle);
    float ffps = (float)fps;
    out->hasPose = a->hasPose;
    // Retail keeps FILD's integer value in x87 for rotation, while the
    // translation operators below receive the separately rounded float fps.
    angle = (double)fps * angle * t;
    out->nodeIndex = a->nodeIndex;
    if (angle != 0.0f) {
        RotatePose(b, out, axis, angle);
    } else {
        out->axisZ = b->axisZ;
        out->axisY = b->axisY;
    }
    if (flag) {
        Vec3 delta = b->position - a->position;
        Vec3 step = delta * ffps * t;
        out->position = b->position + step;
    } else {
        out->position = b->position;
    }
}

// owner: bracket only (Character method contiguous with its Motnctrl.cpp methods)
// 0x004a70c0 (ret 0xc; Character slot 7, inherited by D3DIMSoultreeCharacter): per-frame playback of
// a non-looping motion with sub-frame interpolation.  The current motion is advanced by AdvanceClamped;
// every pose is interpolated between the two frames around the cursor (or, past the last frame,
// extrapolated by the rotation and translation of the last frame pair) and applied through slot 4
// (mirror == 0) or slot 5.  While a blend is active the blend-from motion is advanced the same way and
// the two poses are interpolated with a smoothstep weight over blendDuration; nodes flagged in the
// name table keep the blend-from pose.  Past the last frame a looping motion is restarted through
// BlendToMotion, a non-looping one sets motionStopped.  Returns the frame number, or 0.
// Nonexact functional candidate; complete emitted extent 3333 bytes versus retail's 3327.
// See docs/MOTION_PLAYBACK.md for native differential coverage and dependency boundaries.
// With the two blend inlines
// (BlendPose7/BlendPose7B) the constructor calls split 2/4 as in retail, and the playback loop's
// extrapolation through ExtrapolatePose7 calls Vec3::Vec3 and the operator COMDATs in retail's
// order.  Left: this pointer in edi (retail esi) in the prologue, with &currentFrame in esi
// (retail edi) and motionStopped compared in memory (retail loads it into eax first; `!= 0`,
// `== 1`, a named copy and the time-add spellings do not change either), and the blend loop's
// unused sums and cross/normalise calls. Its extrapolation uses the explicit vector-call
// declarations to retain retail's rounding between subtraction, two scales and addition.
int Character::CharacterVirtualSlot7(float time, int mirror, int mask)
{
    MotionPoseList* from;
    MotionPoseList* to;
    float weight;
    int finished;
    int ok;

    if (motionStopped)
        return 0;
    finished = 0;
    chr_field_0x10 += time;
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
                CharacterPose* fromPose = &from->poses[i];
                int flag = nodeNames[fromPose->nodeIndex].applyTranslation;
                float frameTime = chr_field_0x10;
                int fps = currentMotion->fps;
                CharacterPose* toPose = &to->poses[i];
                CharacterPose out;
                ExtrapolatePose7(fromPose, toPose, &out, flag, frameTime, fps);
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
        float blendProgress = blendFromTime / blendDuration;
        float blendWeight;
        if (blendProgress < 0.0f)
            blendWeight = 0.0f;
        else if (blendProgress >= 1.0f)
            blendWeight = 1.0f;
        else
            blendWeight = SmoothStep(blendProgress);
        int ok2 = AdvanceClamped(blendFromMotion, &lastFrame, &from2, &to2, &lastFrameTime, &weight2) != 0;
        for (int j = 0; j < from->count; j++) {
            CharacterPose pose2;
            CharacterPose out;
            CharacterPose result;
            BlendPose7B(&from->poses[j], &to->poses[j], &out, weight);
            if (ok2) {
                InterpolatePose(&from2->poses[j], &to2->poses[j], &pose2, weight2);
            } else {
                CharacterPose* fromPose = &from2->poses[j];
                int flag = nodeNames[fromPose->nodeIndex].applyTranslation;
                float frameTime = lastFrameTime;
                int fps = blendFromMotion->fps;
                CharacterPose* toPose = &to2->poses[j];
                Vec3 axis;
                float angle;
                PoseRotation(fromPose, toPose, &axis, &angle);
                float ffps = (float)fps;
                pose2.hasPose = fromPose->hasPose;
                angle = (angle * frameTime) * ffps;
                pose2.nodeIndex = fromPose->nodeIndex;
                if (angle != 0.0f) {
                    RotatePose(toPose, &pose2, axis, angle);
                } else {
                    pose2.axisZ = toPose->axisZ;
                    pose2.axisY = toPose->axisY;
                }
                if (flag) {
                    // Retail calls each vector operator separately. Preserve the float
                    // result of each call before the next operation: an inlined chain
                    // retains x87 precision and changes the applied translation.
                    Vec3 delta, velocity, step;
                    Vec3SubtractCall(&delta, &toPose->position, &fromPose->position);
                    Vec3ScaleCall(&velocity, &delta, ffps);
                    Vec3ScaleCall(&step, &velocity, frameTime);
                    Vec3AddCall(&pose2.position, &toPose->position, &step);
                } else {
                    pose2.position = toPose->position;
                }
            }
            InterpolatePose(&pose2, &out, &result, blendWeight);
            if (nodeNames[pose2.nodeIndex].applyTranslation)
                result = pose2;
            else
                InterpolatePose(&pose2, &out, &result, blendWeight);
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
