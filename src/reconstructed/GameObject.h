#pragma once

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
    virtual int UnknownVirtualSlot10(int value);
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
    virtual int UnknownVirtualSlot22(int a, int b);
    virtual int UnknownVirtualSlot23(int a, int b);
    virtual int UnknownVirtualSlot24(int a, int b, int c, int d, int e);
    virtual int UnknownVirtualSlot25(void* value);
    virtual void UnknownVirtualSlot26();

    // 0x00469680: releases this object and every later sibling, back to front.
    int UnknownFunction469680();
    // 0x00469ce0: appends the RTTI class name of `object` to field_0x28.
    void UnknownFunction469ce0(GameObject* object);

protected:
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
