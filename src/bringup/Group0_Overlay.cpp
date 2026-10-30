// Group0_Overlay.cpp -- bring-up body for Overlay slot 10. Behavior follows
// the retail disassembly; byte matching was not attempted.
#include "Overlay.h"

// 0x00499ae0 (ret 4): forwards to GameObject::Tick (0x004693d0) and returns
// its result.
int Overlay::Tick(float frameTime)
{
    return GameObject::Tick(frameTime);
}
