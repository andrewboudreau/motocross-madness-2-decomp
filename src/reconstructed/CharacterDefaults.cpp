// Individually identified Character primary virtual defaults. The original
// translation unit and the unused argument types remain unproven.
// Raw primary/virtual-base RTTI and slot evidence: docs/CHARACTER_DEFAULTS.md.
#include "../krusty2/motion/D3DIMSoultreeCharacter.h"

// Primary slots 0, 2 and 10: complete 0x00464e90, one-byte return.
void Character::BuildModel() {}
void Character::CaptureRestPose() {}
void Character::DumpNodeTransforms() {}

// Primary slots 3 and 9: complete 0x00464e80, return popping one argument.
void Character::CaptureFramePoses(int) {}
void Character::CopyModelFrom(int) {}
