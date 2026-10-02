#pragma once

#include "GameObject.h"
#include "MatrixUtil.h"

// Stack object built by KrustyBikeCamera slot 58 (constructor 0x0051b200,
// no destructor call) and handed to UnknownMessageTarget::0x0051b540.
class UnknownMessage {
public:
    UnknownMessage(const char* text, float duration);

    unsigned char field_0x00[0x8c];
};

// Also a GameObject: EventManager 0x0045d270 calls its slot 5.
class UnknownMessageTarget : public GameObject {
public:
    void UnknownFunction51b540(UnknownMessage* message); // 0x0051b540
};

struct UnknownKrustyBikeViewPart {
    unsigned char field_0x00[0x10];
    float field_0x10;                    // slot 10 adds 2 for the height
};

// Object at KrustyBikeCamera+0x3b8 (chosen by slot 10 from the global's
// +0x558..+0x568 objects); slot 48 tests two flags. TrackGame and
// EventManager use it as a GameObject (slot 5, the +0x25 flag bits), which
// fits KrustyBike's primary base chain; its class is not established.
struct UnknownKrustyBikeView : public GameObject {
    // 0x004210f0: writes two positions relative to `reference`.
    void UnknownFunction4210f0(Vector3* a, Vector3* b, void* reference, int flags);

    unsigned char field_0x02c[0x38 - 0x2c];
    UnknownKrustyBikeViewPart* field_0x38;
    unsigned char field_0x03c[0x18e - 0x3c];
    bool field_0x18e;                    // slot 10: view available
    unsigned char field_0x18f[0x3f8 - 0x18f];
    bool field_0x3f8;
    bool field_0x3f9;
};
