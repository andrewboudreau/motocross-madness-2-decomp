// Private complete two-motion pose blend, retail 0x004a8c50..0x004a9042.
#include "../../../src/krusty2/motion/Motnctrl.h"
#include "CharacterMotionFloatToInt.h"

void InterpolatePose(CharacterPose*, CharacterPose*, CharacterPose*, float);


// Retail's first interpolation expands through these out-of-line value operations.
static void BlendFirst(CharacterPose* a, CharacterPose* b, CharacterPose* out, float t)
{
    if (t < 0.0f) t = 0.0f;
    else if (t > 1.0f) t = 1.0f;
    float s = 1.0f - t;
    out->nodeIndex = a->nodeIndex;
    out->hasPose = a->hasPose;
    Vec3 dz, sz, az, dy, sy, ay, dp, sp, ap;
    out->axisZ = Vec3Normalize(*Vec3AddCall(&az, &a->axisZ,
        Vec3ScaleCall(&sz, Vec3SubtractCall(&dz, &b->axisZ, &a->axisZ), t)));
    out->axisY = Vec3Normalize(*Vec3AddCall(&ay, &a->axisY,
        Vec3ScaleCall(&sy, Vec3SubtractCall(&dy, &b->axisY, &a->axisY), t)));
    out->position = *Vec3AddCall(&ap, &a->position,
        Vec3ScaleCall(&sp, Vec3SubtractCall(&dp, &b->position, &a->position), t));
    Vec3 axisX = CrossProductCall(out->axisZ, out->axisY);
    out->axisY = CrossProductCall(axisX, out->axisZ);
    Vec3 wa, wb, sum;
    Vec3AddCall(&sum, Vec3ScaleCall(&wa, &a->axisZ, s), Vec3ScaleCall(&wb, &b->axisZ, t));
    Vec3AddCall(&sum, Vec3ScaleCall(&wa, &a->axisY, s), Vec3ScaleCall(&wb, &b->axisY, t));
    Vec3AddCall(&sum, Vec3ScaleCall(&wa, &a->position, s), Vec3ScaleCall(&wb, &b->position, t));
}

void Character::Method_0x004a8c50(int firstHandle, int secondHandle, float phase, float weight)
{
    Motion* first = reinterpret_cast<Motion*>(firstHandle);
    Motion* second = reinterpret_cast<Motion*>(secondHandle);
    int frameIndex = first->frameCount - 1;
    float position = frameIndex * phase;
    float shifted = position - 0.5f;
    FloatToInt(&frameIndex, shifted);
    currentMotion = first;
    currentFrame = &first->frames[frameIndex];
    float firstWeight = position - frameIndex;
    MotionPoseList* firstFrame = &first->frames[frameIndex];
    MotionPoseList* firstNext = frameIndex + 1 == first->frameCount
        ? firstFrame : &first->frames[frameIndex + 1];

    position = (second->frameCount - 1) * phase;
    shifted = position - 0.5f;
    // The second retail conversion reuses the first conversion's destination.
    FloatToInt(&frameIndex, shifted);
    float secondWeight = position - frameIndex;
    MotionPoseList* secondFrame = &second->frames[frameIndex];
    MotionPoseList* secondNext = frameIndex + 1 == second->frameCount
        ? secondFrame : &second->frames[frameIndex + 1];

    float finalWeight = 1.0f - weight;
    for (int i = 0; i < currentFrame->count; ++i) {
        CharacterPose firstPose, secondPose, result;
        BlendFirst(&firstFrame->poses[i], &firstNext->poses[i], &firstPose, firstWeight);
        InterpolatePose(&secondFrame->poses[i], &secondNext->poses[i], &secondPose, secondWeight);
        InterpolatePose(&firstPose, &secondPose, &result, finalWeight);
        CharacterVirtualSlot4((int)&result, 0);
    }
}
