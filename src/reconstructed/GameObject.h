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
    virtual void UnknownVirtualSlot21();
    virtual int UnknownVirtualSlot22(int a, int b);
    virtual int UnknownVirtualSlot23(int a, int b);
    virtual void UnknownVirtualSlot24();
    virtual void UnknownVirtualSlot25();
    virtual void UnknownVirtualSlot26();

    void UnknownFunction469680(); // 0x00469680, called on the first child by Release

protected:
    // Offsets and widths are evidenced by the reconstructed methods; names are
    // placeholders. Children are reached through field_0x10 and chained through
    // their field_0x0C (traversal by slots 6, 7, 12-18 and 26).
    GameObject* field_0x08;    // previous sibling
    GameObject* field_0x0C;    // next sibling
    GameObject* field_0x10;    // first child
    GameObject* field_0x14;    // parent
    void* field_0x18;          // set by slot 8; Camera's owner object
    int field_0x1C;
    unsigned int field_0x20;   // flag word gating slots 12-18
    unsigned char field_0x24;
    unsigned char field_0x25;  // bit 0/1 cleared by slot 4; bit 2 set by slot 16;
                               // bit 3 skips traversal
};
