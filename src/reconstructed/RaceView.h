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

struct UnknownEventRacerPart {
    unsigned char field_0x00[0x0c];
    float field_0x0c;
};

// A racer: a view's +0x38 (its own) and +0x3c (all, by racer slot), and the
// object EventManager ranks and updates from network messages. The class is
// not established.
struct UnknownEventRacer {
    unsigned char field_0x000[0x10];
    float field_0x010;                             // KrustyBikeCamera slot 10 adds 2 for the height
    unsigned char field_0x014[0x4a0 - 0x14];
    int field_0x4a0;                               // EventManager 0x0045eef0: counts as done when set
    unsigned char field_0x4a4[0x5e0 - 0x4a4];
    char field_0x5e0[0x744 - 0x5e0];               // name
    UnknownEventRacerPart* field_0x744;
    int field_0x748;                               // time stamp; 0x7ffffffe until finished
    unsigned char field_0x74c[0x750 - 0x74c];
    int field_0x750;
    float field_0x754;                             // FLT_MAX until finished
    int field_0x758;
    float field_0x75c;
    float field_0x760;                             // best of +0x75c
    float field_0x764;                             // sum of +0x75c
    int field_0x768;
    unsigned char field_0x76c[0x784 - 0x76c];
    int field_0x784;                               // finishing position (1-based)
    int field_0x788;
    unsigned char field_0x78c[0x790 - 0x78c];
    int field_0x790;
    unsigned char field_0x794[0x7a0 - 0x794];
    unsigned short field_0x7a0;
    unsigned char field_0x7a2[0x7a4 - 0x7a2];
    char field_0x7a4;                              // finished
    unsigned char field_0x7a5[0x11c0 - 0x7a5];
    char field_0x11c0;                             // AI racer's index in its messages
};

// Object at KrustyBikeCamera+0x3b8 (chosen by slot 10 from the global's
// +0x558..+0x568 objects); slot 48 tests two flags. TrackGame and
// EventManager use it as a GameObject (slot 5, the +0x25 flag bits), which
// fits KrustyBike's primary base chain; its class is not established.
struct UnknownKrustyBikeView : public GameObject {
    void UnknownFunction420590(int player);  // 0x00420590 (EventManager slot 24)
    // 0x004204e0: the next racer after `*iterator` (advancing it), or 0.
    UnknownEventRacer* UnknownFunction4204e0(int* iterator);
    // 0x004210f0: writes two positions relative to `reference`.
    void UnknownFunction4210f0(Vector3* a, Vector3* b, void* reference, int flags);

    unsigned char field_0x02c[0x38 - 0x2c];
    UnknownEventRacer* field_0x38;                // its own racer
    UnknownEventRacer** field_0x3c;               // all racers, by racer slot
    UnknownEventRacer** field_0x40;               // AI racers (TrackGame+0x2d94 of them)
    unsigned char field_0x044[0x18a - 0x44];
    bool field_0x18a;                    // racing (EventManager 0x0045eef0)
    unsigned char field_0x18b[0x18e - 0x18b];
    bool field_0x18e;                    // slot 10: view available
    unsigned char field_0x18f[0x3f8 - 0x18f];
    bool field_0x3f8;
    bool field_0x3f9;
};
