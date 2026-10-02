#pragma once

#include "GameObject.h"

class UnknownMessageTarget;

// RTTI: EventManager : GameObject (vtable 0x0055259c; 0xd08 bytes, the size
// TrackGame slot 4 allocates). Its constructor 0x0045c9e0 writes the vtable.
// Only the members TrackGame and KrustyBikeCamera call are declared.
class EventManager : public GameObject {
public:
    explicit EventManager(int flags);              // 0x0045c9e0
    GameObject* UnknownFunction45d2b0();           // 0x0045d2b0
    GameObject* UnknownFunction45d2f0();           // 0x0045d2f0
    UnknownMessageTarget* UnknownFunction45d340(); // 0x0045d340

    unsigned char field_0x2c[0xd08 - 0x2c];
};
