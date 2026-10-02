#pragma once

struct UnknownControlEvent;
struct UnknownInputEntry;

#include "BaseObject.h"

// Global read by GameObject::Release (0x0065b548); nonzero blocks release.
extern int g_UnknownGlobal65b548;

// RTTI: GameObject : BaseObject. GameObject introduces primary slots 4-26.
// Only signatures needed by reconstructed derived classes are refined; the rest
// keep placeholder void() declarations so slot numbering matches the retail
// vtable. Reconstructed bodies are in GameObject.cpp.
class GameObject : public BaseObject {
public:
    // 0x00468ca0 (gameobj.cpp); only the low bit of the argument is used.
    explicit GameObject(int flags);
    virtual ~GameObject(); // destructor core 0x00468d60
    virtual int Release(); // slot 2 override, 0x004696c0

    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5();
    virtual void UnknownVirtualSlot6();
    virtual void UnknownVirtualSlot7();
    virtual GameObject* UnknownVirtualSlot8(void* value);
    virtual int UnknownVirtualSlot9(int value);
    virtual int UnknownVirtualSlot10(float frameTime);
    virtual int UnknownVirtualSlot11(int value);
    virtual int UnknownVirtualSlot12();
    virtual int UnknownVirtualSlot13();
    virtual int UnknownVirtualSlot14();
    virtual int UnknownVirtualSlot15();
    virtual int UnknownVirtualSlot16(int value);
    virtual int UnknownVirtualSlot17();
    virtual int UnknownVirtualSlot18();
    virtual int UnknownVirtualSlot19(int value);
    virtual int UnknownVirtualSlot20(int value);
    virtual int UnknownVirtualSlot21(int value);
    // Input events from Game slots 13 and 14, passed down the children.
    virtual int UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry);
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry);
    virtual int UnknownVirtualSlot24(int type, void* data, int c, int d, int e);
    virtual int UnknownVirtualSlot25(void* value);
    virtual void UnknownVirtualSlot26();

    // 0x00469190: appends `child` (and its later siblings) to the children;
    // not reconstructed.
    int UnknownFunction469190(GameObject* child, int value);
    // 0x00469680: releases this object and every later sibling, back to front.
    int UnknownFunction469680();
    // 0x00469ce0: appends the RTTI class name of `object` to field_0x28.
    void UnknownFunction469ce0(GameObject* object);

protected:
    // 0x00468dd0 / 0x00468f10: TrackGame turns the "RaceSound" child on and
    // off through these.
    void UnknownFunction468dd0(const char* name);
    void UnknownFunction468f10(const char* name);

    friend class Game;      // reads the +0x25 bits of its DebugOverlay
    friend class TrackGame; // reads the +0x25 bits of its UI objects (0x00521860)
    friend class EventManager; // reads a race-mode object's +0x25 bit 0 (0x0045eef0)

    // Offsets and widths are evidenced by the reconstructed methods; names are
    // placeholders. Children are reached through field_0x10 and chained through
    // their field_0x0C (traversal by slots 6, 7, 12-18 and 26).
    GameObject* field_0x08;    // previous sibling
    GameObject* field_0x0C;    // next sibling
    GameObject* field_0x10;    // first child
    GameObject* field_0x14;    // parent
    void* field_0x18;          // set by slots 8 and 25; Camera's owner object
    int field_0x1C;
    unsigned int field_0x20;   // flag word gating slots 12-18
    unsigned char field_0x24;
    // Byte +0x25 is a bitfield: the constructor's single merged store of all
    // four low bits is VC6's bitfield code shape. Bit 0 enables the gated child
    // walks, bit 3 excludes a child from every walk; bit 2 is set by slot 16
    // and also excludes a child from slots 9 and 10.
    unsigned char field_0x25_bit0 : 1;
    unsigned char field_0x25_bit1 : 1;
    unsigned char field_0x25_bit2 : 1;
    unsigned char field_0x25_bit3 : 1;
    char* field_0x28;          // DebugMalloc'd string, starts empty; see 0x00469ce0
};
