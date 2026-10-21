// Individually identified Character primary virtual defaults. The original
// translation unit and the unused argument types remain unproven.
// Raw primary/virtual-base RTTI and slot evidence: docs/CHARACTER_DEFAULTS.md.
#include "../krusty2/motion/D3DIMSoultreeCharacter.h"

// Primary slots 0, 2 and 10: complete 0x00464e90, one-byte return.
void Character::CharacterVirtualSlot0() {}
void Character::CharacterVirtualSlot2() {}
void Character::CharacterVirtualSlot10() {}

// Primary slots 3 and 9: complete 0x00464e80, return popping one argument.
void Character::CharacterVirtualSlot3(int) {}
void Character::CharacterVirtualSlot9(int) {}
