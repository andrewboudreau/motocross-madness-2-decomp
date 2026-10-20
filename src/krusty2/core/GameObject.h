// Shared declaration for the physics and game hierarchies.
// The primary vtable has 27 slots and the GameObject subobject is 0x2c bytes.
// Keeping one class definition preserves the same inherited methods and ABI in
// every translation unit, including virtual and secondary-base uses.
#pragma once
#include "../../reconstructed/GameObject.h"
