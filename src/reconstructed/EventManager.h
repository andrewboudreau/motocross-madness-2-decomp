#pragma once

#include "GameObject.h"
#include "MatrixUtil.h"

class UnknownMessageTarget;
struct TrackGameViewOwner;
struct UnknownKrustyBikeView;

// 0x50-byte entry; EventManager keeps 11 at +0x50.
struct UnknownEventEntry {
    UnknownEventEntry();                           // 0x0045c830 (resets through 0x0045c840)

    unsigned char field_0x00[0x50];
};

// RTTI: EventManager : GameObject (vtable 0x0055259c; 0xd08 bytes, the size
// TrackGame slot 4 allocates). Its constructor 0x0045c9e0 writes the vtable.
// Only the members TrackGame and KrustyBikeCamera call are declared.
class EventManager : public GameObject {
public:
    explicit EventManager(int flags);              // 0x0045c9e0
    virtual ~EventManager();                       // 0x0045cae0 (deleting wrapper 0x0045cac0)
    virtual GameObject* UnknownVirtualSlot8(void* value); // 0x0045caf0

    // The first race-mode object of TrackGame+0x558..+0x568 present, its
    // +0x34 view and its +0x6c target.
    TrackGameViewOwner* UnknownFunction45d2b0();   // 0x0045d2b0
    UnknownKrustyBikeView* UnknownFunction45d2f0(); // 0x0045d2f0
    UnknownMessageTarget* UnknownFunction45d340(); // 0x0045d340
    void UnknownFunction45d270();                  // 0x0045d270: slot 5 on all three
    int UnknownFunction45d390();                   // 0x0045d390: whether any mode is present

    float field_0x2c;                              // "KeepAliveTimeout" (slot 8)
    float field_0x30;
    int field_0x34;
    int field_0x38;
    int field_0x3c;
    int field_0x40;
    int field_0x44;
    unsigned char field_0x48;
    int field_0x4c;
    UnknownEventEntry field_0x50[11];
    int field_0x3c0;
    Vector3 field_0x3c4;
    int field_0x3d0;
    int field_0x3d4;
    unsigned char field_0x3d8[0x440 - 0x3d8];
    int field_0x440;
    unsigned char field_0x444[0xd08 - 0x444];
};
