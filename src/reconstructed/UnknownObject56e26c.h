#pragma once

#include "ControlInterface.h"

// The object behind the global pointer at 0x0056e26c, used throughout the
// camera and input code. It is polymorphic (virtual slot 22 is called). Its
// class is not established; only members that reconstructed functions touch
// are declared, at their observed offsets.

// Object returned by (+0x570)->0x0045d340; KrustyBikeCamera slot 58 hands it
// a message.
class UnknownMessageTarget;

class UnknownObject56e26cList {
public:
    UnknownMessageTarget* UnknownFunction45d340(); // 0x0045d340
};

struct UnknownKrustyBikeView;

struct UnknownObject56e26cViewOwner {
    unsigned char field_0x00[0x34];
    UnknownKrustyBikeView* field_0x34;
};

// Object embedded at +0x578; KrustyBikeCamera slot 52 reads its mode.
class UnknownObject56e26cMode {
public:
    int UnknownFunction524100(); // 0x00524100
};

struct UnknownObject56e26cSettings {
    unsigned char field_0x00[0x6c];
    int field_0x6c;      // freezes RenderTarget's frame index (0x004e8cc0)
};

class UnknownObject56e26c {
public:
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5();
    virtual void UnknownVirtualSlot6();
    virtual void UnknownVirtualSlot7();
    virtual void UnknownVirtualSlot8();
    virtual void UnknownVirtualSlot9();
    virtual void UnknownVirtualSlot10();
    virtual void UnknownVirtualSlot11();
    virtual void UnknownVirtualSlot12();
    virtual void UnknownVirtualSlot13();
    virtual void UnknownVirtualSlot14();
    virtual void UnknownVirtualSlot15();
    virtual void UnknownVirtualSlot16();
    virtual void UnknownVirtualSlot17();
    virtual void UnknownVirtualSlot18();
    virtual void UnknownVirtualSlot19();
    virtual void UnknownVirtualSlot20();
    virtual void UnknownVirtualSlot21();
    // Named boolean setting (JoystickDevice asks for "JoyDirectionFlipped").
    virtual bool UnknownVirtualSlot22(const char* name, int defaultValue);

    void UnknownFunction468880(); // 0x00468880 (PCCamera slot 27)
    // 0x00521970: copies string `id` into buffer (size bytes).
    void UnknownFunction521970(int id, char* buffer, int size);

    unsigned char field_0x0004[0x0c - 4];
    UnknownObject56e26cSettings* field_0x0c;
    int field_0x10;
    ControlInterface* field_0x14;
    int field_0x18;      // KrustyBikeCamera slot 42 tests > 1
    unsigned char field_0x001c[0x558 - 0x1c];
    // Objects whose +0x34 KrustyBikeCamera slot 10 takes as its view, by
    // field_0x2d74.
    UnknownObject56e26cViewOwner* field_0x558;
    UnknownObject56e26cViewOwner* field_0x55c;
    UnknownObject56e26cViewOwner* field_0x560;
    UnknownObject56e26cViewOwner* field_0x564;
    UnknownObject56e26cViewOwner* field_0x568;
    int field_0x56c;
    UnknownObject56e26cList* field_0x570;
    int field_0x574;
    UnknownObject56e26cMode field_0x578;
    unsigned char field_0x0579[0x2930 - 0x579];
    int field_0x2930;    // saved KrustyBikeCamera state (slots 61, 62)
    float field_0x2934;  // saved KrustyBikeCamera presets (slots 59, 60)
    float field_0x2938;
    float field_0x293c;
    float field_0x2940;
    unsigned char field_0x2944[0x2d74 - 0x2944];
    int field_0x2d74;    // selects KrustyBikeCamera's view (slot 10)
    unsigned char field_0x2d78[0x3430 - 0x2d78];
    int field_0x3430;    // blocks KrustyBikeCamera slot 23
};

extern UnknownObject56e26c* g_UnknownGlobal56e26c;
