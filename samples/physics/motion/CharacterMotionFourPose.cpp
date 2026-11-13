// Private complete four-motion pose blend, retail 0x004a9050..0x004a98a9 (thiscall, ret 0x1c,
// 2138 bytes, 677 instructions). KrustyBike::UpdateRiderPose (0x0048eea0) calls it on the rider and
// on the bike with (idx, next, cnt, other, poseParam, 1 - poseBlend, poseLeanBlend). The method name
// and the original translation unit are provisional; retail places it after 0x004a8c50 inside
// Motnctrl.cpp's extent (0x004a5630..0x004a9a9f).
//
// Decoded behaviour (confirmed from the instructions):
//   - Each of the four motions is sampled at the same phase: position = (frameCount - 1) * phase,
//     rounded to binary32 in memory; frameIndex = fistp(position - 0.5f) under the current x87
//     rounding mode (one int slot, the dead first parameter, receives all four conversions);
//     fraction = position - frameIndex; frame = &frames[frameIndex]; next = frame when
//     frameIndex + 1 == frameCount, else &frames[frameIndex + 1].
//   - The first motion becomes currentMotion (+0x18) and its sampled frame currentFrame (+0x1c).
//   - The node loop runs to currentMotion->posesPerFrame (+0xac), reloaded after every slot 4 call.
//   - Per node: motions a and b go through BlendFirst (clamped normalised lerp of the axes,
//     re-orthogonalised, lerped position, node index and flag from the first pose), c and d through
//     InterpolatePose (0x004a8470); then ab = InterpolatePose(a, b, 1 - weight),
//     cd = InterpolatePose(c, d, 1 - weight), result = InterpolatePose(ab, cd, 1 - lean), and
//     ApplyPose (vtable +0x10) receives (&result, 0).
//
// Code shape (not byte exact; linked with /QIfist /Op, config/game_build.json):
//   - Retail's ebp frame and bare `fistp dword ptr [eax]` come from an inline-asm helper, out of
//     reach here. The asm also keeps `position` and the helper argument in binary32 stack slots;
//     /Op gives the same rounding to every float assignment, FistpFloatToInt rounds the argument.
//   - Retail expands BlendFirst at both call sites. Plain `inline` gets the first expansion only
//     (VC6's inline budget refuses the second here, with any of the four pose-local counts, natural
//     operators or explicit call views); __forceinline expands both.
//   - Retail calls CharacterPose's generated constructor (0x004a8930) for BlendFirst's pose locals
//     three times in the first expansion and four times in the second, and only the second keeps the
//     weighted sums in its first pose local (stores to ebp-0x134..-0x114): strong inference that the
//     inline declares four poses and that the first expansion inlined the first constructor, so its
//     sums were dead. Here every constructor inlines and both sets of stores are dead (the sum calls
//     remain, as in retail).
#include "../../../src/krusty2/motion/Motnctrl.h"
#include "../../common/FistpFloatToInt.h"

void InterpolatePose(CharacterPose*, CharacterPose*, CharacterPose*, float);

// The clamped first-motion interpolation that 0x004a8c50 also expands (CharacterMotionBlend.cpp),
// written with the explicit out-of-line vector views retail calls (0x00421d00, 0x005015b0,
// 0x00421cb0, 0x005087b0, 0x00515600).
static __forceinline void BlendFirst(CharacterPose* a, CharacterPose* b, CharacterPose* out, float t)
{
    if (t < 0.0f) t = 0.0f;
    else if (t > 1.0f) t = 1.0f;
    float s = 1.0f - t;
    out->nodeIndex = a->nodeIndex;
    out->hasPose = a->hasPose;
    CharacterPose linear, unused1, unused2, unused3;
    Vec3 dz, sz, az, dy, sy, ay, dp, sp, ap;
    out->axisZ = Vec3Normalize(*Vec3AddCall(&az, &a->axisZ,
        Vec3ScaleCall(&sz, Vec3SubtractCall(&dz, &b->axisZ, &a->axisZ), t)));
    out->axisY = Vec3Normalize(*Vec3AddCall(&ay, &a->axisY,
        Vec3ScaleCall(&sy, Vec3SubtractCall(&dy, &b->axisY, &a->axisY), t)));
    out->position = *Vec3AddCall(&ap, &a->position,
        Vec3ScaleCall(&sp, Vec3SubtractCall(&dp, &b->position, &a->position), t));
    Vec3 axisX = CrossProductCall(out->axisZ, out->axisY);
    out->axisY = CrossProductCall(axisX, out->axisZ);
    // Unused weighted sums (retail computes them; nothing reads linear afterwards).
    Vec3 wa, wb, sum;
    linear.axisZ = *Vec3AddCall(&sum, Vec3ScaleCall(&wa, &a->axisZ, s), Vec3ScaleCall(&wb, &b->axisZ, t));
    linear.axisY = *Vec3AddCall(&sum, Vec3ScaleCall(&wa, &a->axisY, s), Vec3ScaleCall(&wb, &b->axisY, t));
    linear.position = *Vec3AddCall(&sum, Vec3ScaleCall(&wa, &a->position, s), Vec3ScaleCall(&wb, &b->position, t));
}

// Retail's inline-asm fistp helper gives this function an EBP frame; the
// pragma keeps the frame pointer for it alone (docs/GAME_BUILD.md#frame-pointer-units).
#pragma optimize("y", off)
void Character::ApplyFourMotionBlendAtPhase(int aHandle, int bHandle, int cHandle, int dHandle,
                                  float phase, float weight, float lean)
{
    Motion* a = reinterpret_cast<Motion*>(aHandle);
    Motion* b = reinterpret_cast<Motion*>(bHandle);
    Motion* c = reinterpret_cast<Motion*>(cHandle);
    Motion* d = reinterpret_cast<Motion*>(dHandle);

    int frameIndex = a->frameCount - 1;
    float position = frameIndex * phase;
    FistpFloatToInt(&frameIndex, position - 0.5f);
    currentMotion = a;
    currentFrame = &a->frames[frameIndex];
    float aWeight = position - frameIndex;
    MotionPoseList* aFrame = &a->frames[frameIndex];
    MotionPoseList* aNext = frameIndex + 1 == a->frameCount
        ? aFrame : &a->frames[frameIndex + 1];

    position = (b->frameCount - 1) * phase;
    FistpFloatToInt(&frameIndex, position - 0.5f);
    float bWeight = position - frameIndex;
    MotionPoseList* bFrame = &b->frames[frameIndex];
    MotionPoseList* bNext = frameIndex + 1 == b->frameCount
        ? bFrame : &b->frames[frameIndex + 1];

    position = (c->frameCount - 1) * phase;
    FistpFloatToInt(&frameIndex, position - 0.5f);
    float cWeight = position - frameIndex;
    MotionPoseList* cFrame = &c->frames[frameIndex];
    MotionPoseList* cNext = frameIndex + 1 == c->frameCount
        ? cFrame : &c->frames[frameIndex + 1];

    position = (d->frameCount - 1) * phase;
    FistpFloatToInt(&frameIndex, position - 0.5f);
    float dWeight = position - frameIndex;
    MotionPoseList* dFrame = &d->frames[frameIndex];
    MotionPoseList* dNext = frameIndex + 1 == d->frameCount
        ? dFrame : &d->frames[frameIndex + 1];

    float pairWeight = 1.0f - weight;
    float leanWeight = 1.0f - lean;
    for (int i = 0; i < currentMotion->posesPerFrame; ++i) {
        CharacterPose aPose, bPose, cPose, dPose, abPose, cdPose, result;
        BlendFirst(&aFrame->poses[i], &aNext->poses[i], &aPose, aWeight);
        BlendFirst(&bFrame->poses[i], &bNext->poses[i], &bPose, bWeight);
        InterpolatePose(&cFrame->poses[i], &cNext->poses[i], &cPose, cWeight);
        InterpolatePose(&dFrame->poses[i], &dNext->poses[i], &dPose, dWeight);
        InterpolatePose(&aPose, &bPose, &abPose, pairWeight);
        InterpolatePose(&cPose, &dPose, &cdPose, pairWeight);
        InterpolatePose(&abPose, &cdPose, &result, leanWeight);
        ApplyPose((int)&result, 0);
    }
}
#pragma optimize("", on)
