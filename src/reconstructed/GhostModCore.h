#pragma once

// Shared complete declaration; GhostMod.h documents its provenance.
#include "D3DIMSoultreeModifierCore.h"

class GhostMod1 : public D3DIMSoultreeModifier {
public:
    explicit GhostMod1(int flags);             // 0x0047bb20 (ret 4)
    virtual ~GhostMod1();                      // 0x0047bc60 (deleting wrapper 0x0047bb50)
    virtual void UnknownVirtualSlot27(D3DIMSoultreeObject* object, UnknownSoultreeMesh* mesh,
                                      UnknownSoultreeMesh** out); // 0x0047bb70
    // 0x0047bbf0 (ret 4)
    void StartFade(float duration);
    virtual int Tick(float frameTime);                 // 0x0047bc20

    float timeRemaining;                       // time remaining
    float totalDuration;                       // duration
};
