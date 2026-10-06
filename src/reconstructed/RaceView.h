#pragma once

#include "GameObject.h"
#include "MatrixUtil.h"
#include "OverlayRect.h"
#include "VCRfile.h"

// A queued text line (0x8c bytes, TrackOverlay.cpp). Callers build one on
// the stack (constructor 0x0051b200, no destructor) and hand it to
// TextQueueOverlay, which keeps heap copies (copy constructor 0x0051b250).
class UnknownMessage {
public:
    UnknownMessage(const char* text, float duration); // 0x0051b200
    UnknownMessage(const UnknownMessage& other);       // 0x0051b250

    char field_0x00[0x80];                    // text
    float field_0x80;                         // seconds to show it
    float field_0x84;                         // seconds shown so far
    UnknownMessage* field_0x88;               // next queued line
};

// RTTI: TextQueueOverlay : GameObject (vtable 0x00558760), 0x58 bytes
// (TrackOverlay.cpp): draws the head of a queue of UnknownMessage lines with
// GDI, then drops it after its duration. EventManager 0x0045d270 calls its
// slot 5. Member names are provisional.
class TextQueueOverlay : public GameObject {
public:
    explicit TextQueueOverlay(int flags);     // 0x0051b2a0
    virtual ~TextQueueOverlay();              // 0x0051b300 (deleting wrapper 0x0051b2e0)
    virtual int UnknownVirtualSlot10(float frameTime); // 0x0051b3f0
    virtual int UnknownVirtualSlot15();       // 0x0051b450

    // 0x0051b320: binds the render target and the text rectangle and creates
    // the font from the GUI's face name (Arial when it has none).
    TextQueueOverlay* UnknownFunction51b320(void* target, UnknownOverlayRect rect);
    void UnknownFunction51b540(UnknownMessage* message); // 0x0051b540: replaces the head
    void UnknownFunction51b5e0(UnknownMessage* message); // 0x0051b5e0: appends
    void UnknownFunction51b670();             // 0x0051b670: drops the head

    void* field_0x2c;                         // font (DeleteObject)
    UnknownOverlayRect field_0x30;            // text rectangle (+0x40 less one)
    UnknownOverlayRect field_0x40;            // shadow rectangle
    UnknownMessage* field_0x50;               // queue head
    unsigned int field_0x54;                  // text colour, 0xff00
};

struct UnknownEventRacer;
struct UnknownCameraBikeRider;

// 0x54-byte race status node (RaceStatus.cpp, DebugCalloc'd): one per
// racer, chained through +0x50; the racer points back at it from +0x744.
struct UnknownEventRacerPart {
    int field_0x00;                         // 1-based order of creation
    UnknownEventRacer* field_0x04;          // racer
    unsigned char field_0x08[0x0c - 0x08];
    float field_0x0c;
    unsigned char field_0x10[0x28 - 0x10];
    Vector3 field_0x28;                     // the racer's position when created
    int field_0x34;                         // the view's +0xc8
    unsigned char field_0x38[0x50 - 0x38];
    UnknownEventRacerPart* field_0x50;      // next
};

// A racer: a view's +0x38 (its own) and +0x3c (all, by racer slot), and the
// object EventManager ranks and updates from network messages. It reaches
// GameObject through a vbptr at +0x04 (EventManager 0x0045cdc0 calls slot 4
// through it), as KrustyBike's RTTI shows (virtual GameObject base, pdisp 4).
// The class is not established; only the vbase access path is modelled.
struct UnknownEventRacer : virtual public GameObject {
    virtual void UnknownVirtualSlot0();            // gives the racer its own vfptr at +0
    unsigned char field_0x008[0x0c - 0x08];
    Vector3 field_0x00c;                           // position (KrustyBikeCamera slot 10 adds 2 to y)
    unsigned char field_0x018[0x4a0 - 0x18];
    int field_0x4a0;                               // EventManager 0x0045eef0: counts as done when set
    unsigned char field_0x4a4[0x5c4 - 0x4a4];
    UnknownCameraBikeRider* field_0x5c4;           // as BikeCamera's bike +0x5c4 (TrackOverlay 0x005190e0)
    unsigned char field_0x5c8[0x5e0 - 0x5c8];
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
    float field_0x768;                             // a score (sorted with TrackOverlay's 0x005199f0)
    unsigned char field_0x76c[0x784 - 0x76c];
    int field_0x784;                               // finishing position (1-based)
    int field_0x788;
    unsigned char field_0x78c[0x790 - 0x78c];
    int field_0x790;
    unsigned char field_0x794[0x7a0 - 0x794];
    unsigned short field_0x7a0;
    unsigned char field_0x7a2[0x7a4 - 0x7a2];
    char field_0x7a4;                              // finished
    unsigned char field_0x7a5[0x11bc - 0x7a5];
    int field_0x11bc;                              // network player id
    char field_0x11c0;                             // AI racer's index in its messages
};

// Object at KrustyBikeCamera+0x3b8 (chosen by slot 10 from the global's
// +0x558..+0x568 objects); slot 48 tests two flags. TrackGame and
// EventManager use it as a GameObject (slot 5, the +0x25 flag bits), which
// fits KrustyBike's primary base chain; its class is not established.
struct UnknownKrustyBikeView : public GameObject {
    void UnknownFunction420590(int player);  // 0x00420590 (EventManager slot 24)
    // 0x00420b00 (near bikerace.cpp's literals): saves the replay to `path`
    // with `description`.
    void UnknownFunction420b00(char* path, char* description);
    // 0x004204e0: the next racer after `*iterator` (advancing it), or 0.
    UnknownEventRacer* UnknownFunction4204e0(int* iterator);
    // 0x004210f0: writes two positions relative to `reference`.
    void UnknownFunction4210f0(Vector3* a, Vector3* b, void* reference, int flags);

    unsigned char field_0x02c[0x38 - 0x2c];
    UnknownEventRacer* field_0x38;                // its own racer
    UnknownEventRacer** field_0x3c;               // all racers, by racer slot
    UnknownEventRacer** field_0x40;               // AI racers (TrackGame+0x2d94 of them)
    GameObject* field_0x44;
    unsigned char field_0x048[0x50 - 0x48];
    GameObject* field_0x50;
    unsigned char field_0x054[0x60 - 0x54];
    GameObject* field_0x60;
    unsigned char field_0x064[0xc8 - 0x64];
    int field_0x0c8;
    unsigned char field_0x0cc[0x158 - 0xcc];
    int field_0x158;                     // entries in +0x3c
    unsigned char field_0x15c[0x18a - 0x15c];
    bool field_0x18a;                    // racing (EventManager 0x0045eef0)
    unsigned char field_0x18b[0x18e - 0x18b];
    bool field_0x18e;                    // slot 10: view available
    unsigned char field_0x18f[0x1a8 - 0x18f];
    UnknownVcrFile* field_0x1a8;         // ghost recording (EventManager 0x0045cdc0)
    unsigned char field_0x1ac[0x3f8 - 0x1ac];
    bool field_0x3f8;
    bool field_0x3f9;
};
