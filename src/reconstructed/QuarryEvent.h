#pragma once

#include "GameObject.h"

class RenderTarget;

// Progress callback the events' initialisers take (EventManager passes the
// cdecl 0x0045cb20).
typedef void (*UnknownProgressCallback)(int* step);

// RTTI: BaseQuarryEvent : GameObject (vtable 0x0055766c; 0xa4 bytes, the
// size EventManager 0x0045cb70 allocates). Its constructor 0x004de2a0 writes
// the vtable; its code sits among QuarryStuntEvent.cpp's literals. TrackGame
// keeps these race-mode objects at +0x558..+0x568.
class BaseQuarryEvent : public GameObject {
public:
    explicit BaseQuarryEvent(int flags);      // 0x004de2a0
    // 0x004de3b0: GameObject's slot 8, then loads the event; on failure it
    // releases itself (slot 2) and returns 0, else returns this.
    BaseQuarryEvent* UnknownFunction4de3b0(RenderTarget* target, UnknownProgressCallback progress);

    unsigned char field_0x2c[0xa4 - 0x2c];
};

// RTTI: NationalRace : BaseQuarryEvent (vtable 0x00555354; 0xb0 bytes).
class NationalRace : public BaseQuarryEvent {
public:
    explicit NationalRace(int flags);         // 0x004aa7f0
    // 0x004aa850: the base initialiser, then 0x0048ad50 on +0xa4.
    NationalRace* UnknownFunction4aa850(RenderTarget* target, UnknownProgressCallback progress);

    unsigned char field_0xa4[0xb0 - 0xa4];
};
