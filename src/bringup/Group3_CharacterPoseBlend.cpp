// Group 3 link bring-up (best-guess-unreviewed): 0x004a9050, the Character four-pose
// blend that KrustyBike::UpdateRiderPose (Group3_KrustyBike.cpp) calls on the rider and
// on the bike. Moved out of Group3_KrustyBike.cpp so that its one float-to-int
// conversion can be compiled the way retail's is: the game build compiles this unit
// with /QIfist (config/game_build.json bringup_flags), and Group3_KrustyBike.cpp keeps
// the retail-truncating `__ftol` casts of 0x00492ad0 (15) and 0x00493660 (3), which the
// flag would turn into roundings. This unit has no other float-to-int conversion.
//
// Retail's fistp helper at 0x004a9092 (`fstp [t]; fld [t]; mov eax,[out];
// fistp dword ptr [eax]`) converts `position - 0.5`, floor(position) except at exact
// ties under the default round-to-nearest-even mode. FistpFloatToInt is that conversion
// in C++ (a float store and reload, then FISTP through /QIfist).
// Range: position is (frameCount - 1) * phase and the index addresses the motion's frame
// array, so a value outside [0, frameCount) is already an out-of-bounds read in retail.
// The phase passed is KrustyBike::poseParam, which the remote update clamps to 0..1 and
// UpdateRiderPose's local path sets within 0.25..0.75 (inference from those writers in
// Group3_KrustyBike.cpp; not every writer was checked), so the index stays in range.
// Matching (VC6 SP3 vc6_o2_mt /QIfist, masked against retail's 677 instructions): 0.278.
// Retail inlines the motion sampling, so G3SampleMotion is `static inline` (out of
// line, as before the split, 0.103; inline with the old X87FloatToInt emulation 0.269).
// Marking G3BlendFirst inline changes nothing (VC6 keeps it out of line).
#include "motion/D3DIMSoultreeCharacter.h"
#include "motion/MotionPose.h"
#include "../../samples/common/FistpFloatToInt.h"

// ---- 0x004a9050, Character four-pose blend (thiscall ret 0x1c) ----
// Defined as D3DIMSoultreeCharacter.h's Character::Method_0x004a9050 so that
// UpdateRiderPose calls it as retail does (thiscall on the rider and on the bike).
// Per node: motions a and b are sampled with the clamped-phase blend that 0x004a8c50 uses
// for its first motion (the inline axis interpolation, re-orthogonalised), c and d with
// InterpolatePose; then (a,b) and (c,d) are mixed by 1 - weight and the two results by
// 1 - lean, and the pose goes to CharacterVirtualSlot4 (vtable +0x10).  Motion layout:
// +0xa4 frames (0xc-byte {index, poses, count}), +0xa8 frameCount, +0xac poses per frame.
void InterpolatePose(CharacterPose* a, CharacterPose* b, CharacterPose* out, float t);

struct G3MotionView {
    char pad_0x00[0xa4];
    MotionPoseList* frames;   // +0xa4
    int frameCount;           // +0xa8
    int posesPerFrame;        // +0xac
};

static inline void G3SampleMotion(G3MotionView* m, float phase, MotionPoseList** frame,
                           MotionPoseList** next, float* fraction)
{
    float position = (float)(m->frameCount - 1) * phase;
    // Retail's fistp helper (0x004a9092): floor(position) except at exact ties.
    int index;
    FistpFloatToInt(&index, position - 0.5f);
    *fraction = position - (float)index;
    *frame = &m->frames[index];
    *next = index + 1 == m->frameCount ? *frame : &m->frames[index + 1];
}

static void G3BlendFirst(CharacterPose* a, CharacterPose* b, CharacterPose* out, float t)
{
    if (t < 0.0f) t = 0.0f;
    else if (t > 1.0f) t = 1.0f;
    out->nodeIndex = a->nodeIndex;
    out->hasPose = a->hasPose;
    Vec3 d, s, r;
    out->axisZ = Vec3Normalize(*Vec3AddCall(&r, &a->axisZ,
        Vec3ScaleCall(&s, Vec3SubtractCall(&d, &b->axisZ, &a->axisZ), t)));
    out->axisY = Vec3Normalize(*Vec3AddCall(&r, &a->axisY,
        Vec3ScaleCall(&s, Vec3SubtractCall(&d, &b->axisY, &a->axisY), t)));
    out->position = *Vec3AddCall(&r, &a->position,
        Vec3ScaleCall(&s, Vec3SubtractCall(&d, &b->position, &a->position), t));
    Vec3 axisX = CrossProductCall(out->axisZ, out->axisY);
    out->axisY = CrossProductCall(axisX, out->axisZ);
}

void Character::Method_0x004a9050(int a, int b, int c, int d, float phase, float weight,
                                  float lean)
{
    G3MotionView* ma = reinterpret_cast<G3MotionView*>(a);
    G3MotionView* mb = reinterpret_cast<G3MotionView*>(b);
    G3MotionView* mc = reinterpret_cast<G3MotionView*>(c);
    G3MotionView* md = reinterpret_cast<G3MotionView*>(d);
    MotionPoseList *fa, *na, *fb, *nb, *fc, *nc, *fd, *nd;
    float ta, tb, tc, td;
    G3SampleMotion(ma, phase, &fa, &na, &ta);
    this->currentMotion = reinterpret_cast<Motion*>(ma);
    this->currentFrame = fa;
    G3SampleMotion(mb, phase, &fb, &nb, &tb);
    G3SampleMotion(mc, phase, &fc, &nc, &tc);
    G3SampleMotion(md, phase, &fd, &nd, &td);
    float pairWeight = 1.0f - weight;
    float leanWeight = 1.0f - lean;
    for (int i = 0; i < reinterpret_cast<G3MotionView*>(this->currentMotion)->posesPerFrame; ++i) {
        CharacterPose pa, pb, pc, pd, ab, cd, result;
        G3BlendFirst(&fa->poses[i], &na->poses[i], &pa, ta);
        G3BlendFirst(&fb->poses[i], &nb->poses[i], &pb, tb);
        InterpolatePose(&fc->poses[i], &nc->poses[i], &pc, tc);
        InterpolatePose(&fd->poses[i], &nd->poses[i], &pd, td);
        InterpolatePose(&pa, &pb, &ab, pairWeight);
        InterpolatePose(&pc, &pd, &cd, pairWeight);
        InterpolatePose(&ab, &cd, &result, leanWeight);
        this->CharacterVirtualSlot4((int)&result, 0);
    }
}
