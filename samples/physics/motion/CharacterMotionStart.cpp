// Complete nonexact motion-start candidate, retail 0x004a8bf0..0x004a8c41.
// Requires authentic VC6 /QIfist for the conversion helper below. Original
// translation-unit ownership and the blend method at 0x004a8c50 remain unknown.
#include "motion/Motnctrl.h"

#include "CharacterMotionFloatToInt.h"

// Retail's inline-asm fistp helper gives this function an EBP frame; the
// pragma keeps the frame pointer for it alone (docs/GAME_BUILD.md#frame-pointer-units).
#pragma optimize("y", off)
void Character::ApplyMotionAtPhase(int handle, float phase)
{
    Motion* motion = reinterpret_cast<Motion*>(handle);
    phase = (motion->frameCount - 1) * phase - 0.5f;
    // Retail reuses the count-minus-one temporary as its FISTP destination.
    // An unmasked invalid conversion leaves that previous integer untouched.
    int frame = motion->frameCount - 1;
    FloatToInt(&frame, phase);
    currentMotion = motion;
    currentFrame = &motion->frames[frame];
    ApplyPoseList(currentFrame, 0);
}
#pragma optimize("", on)
