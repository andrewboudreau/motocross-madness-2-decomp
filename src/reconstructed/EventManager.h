#pragma once

#include "GameObject.h"
#include "MatrixUtil.h"
#include "RaceView.h"

class UnknownMessageTarget;
struct TrackGameViewOwner;
struct UnknownKrustyBikeView;

// 0x50-byte entry; EventManager keeps 11 at +0x50.
class Camera;

// 8-byte record sorted with 0x0045e930.
struct UnknownEventRanking {
    UnknownEventRacer* racer;
    float value;
};

// 16-byte record sorted with 0x0045d3d0.
struct UnknownEventStanding {
    int field_0x00;
    float field_0x04;
    float field_0x08;
    UnknownEventRacer* racer;                      // its +0x5e0 name breaks ties
};

// cdecl 0x0045d3d0: qsort order for standings (higher +0x00 first, then
// lower +0x04 and +0x08, then the racer's name).
int UnknownFunction45d3d0(const void* a, const void* b);

// cdecl 0x0045e930: qsort order for rankings (higher value first, then
// higher +0x7a0, +0x790 and +0x744->+0x0c).
int UnknownFunction45e930(const void* a, const void* b);

// cdecl 0x0045cb20: progress callback that 0x0045cb70 and 0x0045cdc0 pass
// by address (near miss: samples/game/EventManagerNearMisses.cpp).
void UnknownFunction45cb20(int* step);

// cdecl 0x0045fbb0: qsort order for unsigned values.
int UnknownFunction45fbb0(const void* a, const void* b);

// Objects in EventManager's +0x424 list; slot 10 calls their slot 7.
class UnknownEventListener {
public:
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5();
    virtual void UnknownVirtualSlot6();
    virtual void UnknownVirtualSlot7(float frameTime, int a, int b);
};

// cdecl 0x004aef40 (near Net.cpp's literals); EventManager slot 10 calls it
// once when +0x3c is set.
void UnknownFunction4aef40();

struct UnknownEventEntry {
    UnknownEventEntry();                           // 0x0045c830 (resets through 0x0045c840)
    void UnknownFunction45c840();                  // 0x0045c840: reset

    int field_0x00;                                // network player id
    unsigned char field_0x04[0x30 - 0x04];
    char field_0x30;                               // counted in TrackGame+0x3424 (else Game+0x18)
    unsigned char field_0x31[0x50 - 0x31];
};

// RTTI: EventManager : GameObject (vtable 0x0055259c; 0xd08 bytes, the size
// TrackGame slot 4 allocates). Its constructor 0x0045c9e0 writes the vtable.
// Only the members TrackGame and KrustyBikeCamera call are declared.
class EventManager : public GameObject {
public:
    explicit EventManager(int flags);              // 0x0045c9e0
    virtual ~EventManager();                       // 0x0045cae0 (deleting wrapper 0x0045cac0)
    virtual GameObject* UnknownVirtualSlot8(void* value); // 0x0045caf0
    virtual int UnknownVirtualSlot10(float frameTime);     // 0x0045f200: per-frame update
    // 0x0045f3a0: while UI interaction is blocked (TrackGame+0x3430), control 1,
    // 0x1c or 0x39 (keyboard) or any joystick press resumes it.
    virtual int UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry);
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x0045f440
    // 0x0045f490: network messages from `player`.
    virtual int UnknownVirtualSlot24(int type, void* data, int player, int d, int e);

    // The first race-mode object of TrackGame+0x558..+0x568 present, its
    // +0x34 view and its +0x6c target.
    TrackGameViewOwner* UnknownFunction45d2b0();   // 0x0045d2b0
    UnknownKrustyBikeView* UnknownFunction45d2f0(); // 0x0045d2f0
    UnknownMessageTarget* UnknownFunction45d340(); // 0x0045d340
    void UnknownFunction45d270();                  // 0x0045d270: slot 5 on all three
    int UnknownFunction45d390();                   // 0x0045d390: whether any mode is present
    void UnknownFunction45e520();                  // 0x0045e520: resets the entries
    // 0x0045e550: once every remote racer is ready, ends the network wait.
    void UnknownFunction45e550(float frameTime);
    // 0x0045f180: adds the championship points for `racer`'s position.
    void UnknownFunction45f180(UnknownEventRacer* racer, int* points);
    // 0x0045e600: starts the end-of-race block (or finishes at once), and
    // once it has run its course clears it and moves to the results.
    void UnknownFunction45e600();
    void UnknownFunction45cdc0(int value);         // 0x0045cdc0
    int UnknownFunction45d480();                   // 0x0045d480
    void UnknownFunction45e710(int menu);          // 0x0045e710
    void UnknownFunction45e9d0();                  // 0x0045e9d0
    void UnknownFunction45eef0(float frameTime);   // 0x0045eef0
    void UnknownFunction45f9a0();                  // 0x0045f9a0
    int UnknownFunction45cb70();                   // 0x0045cb70
    void UnknownFunction45fbd0(int player);        // 0x0045fbd0

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
    Camera* field_0x3d4;                           // panned while UI interaction is blocked
    Vector3 field_0x3d8;
    Vector3 field_0x3e4;
    unsigned char field_0x3f0[0x414 - 0x3f0];
    Vector3 field_0x414;                           // pan speed (per 7 seconds)
    int field_0x420;
    UnknownEventListener* field_0x424[7];
    int field_0x440;                               // armed by slot 23, consumed by slot 22
    unsigned char field_0x444[0xd08 - 0x444];
};
