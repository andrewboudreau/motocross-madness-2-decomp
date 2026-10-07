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
    // Network messages (Game slot 17 forwards NetworkInterface 0x004aced0's).
    virtual int UnknownVirtualSlot24(int type, void* data, int from, int to, int flags);
    virtual int UnknownVirtualSlot25(void* value);
    virtual void UnknownVirtualSlot26();

    // 0x00469190: appends `child` (and its later siblings) to the children;
    // not reconstructed.
    int UnknownFunction469190(GameObject* child, int value);
    int UnknownFunction469130(GameObject* child); // 0x00469130 (KrustyUI 0x004988a0)
    // 0x00469130 takes two arguments (ret 8); GUIManager.cpp passes -1 as the second.
    int UnknownFunction469130(GameObject* child, int value);
    int UnknownFunction469260(GameObject* parent, int value); // 0x00469260 (GUIManager.cpp)
    // 0x00469770: finds a descendant by name (GUIManager.cpp passes 1, "GroundFog").
    GameObject* UnknownFunction469770(int mode, const char* name);
    // 0x004691f0: unlinks this object from its siblings and parent and
    // recomputes the root's flags; returns 1 (0 while the global is set).
    int UnknownFunction4691f0();
    // 0x00469680: releases this object and every later sibling, back to front.
    int UnknownFunction469680();
    // 0x00469ce0: appends the RTTI class name of `object` to field_0x28.
    void UnknownFunction469ce0(GameObject* object);
    // 0x004690c0: ORs `flags` into field_0x20 and propagates the result up
    // through every parent (each adds its own field_0x1C).
    void UnknownFunction4690c0(unsigned int flags);
    // 0x00469100: recomputes field_0x20 from the children; returns it with
    // field_0x1C added.
    unsigned int UnknownFunction469100();
    // 0x00469c80: unlinks and releases every descendant marked with +0x25 bit 3.
    void UnknownFunction469c80();

protected:
    // 0x00468dd0 / 0x00468f10: TrackGame turns the "RaceSound" child on and
    // off through these.
    void UnknownFunction468dd0(const char* name);
    void UnknownFunction468f10(const char* name);

    friend class GameObjectIterator; // gameobj.cpp walks the links and names
    friend class Game;      // reads the +0x25 bits of its DebugOverlay
    friend class TrackGame; // reads the +0x25 bits of its UI objects (0x00521860)
    friend class EventManager; // reads a race-mode object's +0x25 bit 0 (0x0045eef0)
    friend class Sound;     // reads its SoundGroup's +0x25 bit 2 (0x004bc6b0)
    friend class NameOverlay; // reads its racer's +0x25 bit 0 (0x005190e0)
    friend class StatsOverlay; // reads its racers' +0x25 bit 0 (0x00519a20)
    friend class Wrecker;   // clears its rigid body's +0x25 bit 0 (0x005327c0)
    friend class RaceSound; // turns the "SoundGroup" children off (0x004e3430)
    friend class GUIManager; // reads a dialog's +0x25 bit 0 (0x00485bd0)
    friend class GUIUser;   // reads a control's +0x25 bit 3 and +0x28 (0x00487730, 0x00487800)
    friend class UICtlContainer; // walks its children back to front (0x0047b3f0)
    friend class UnknownGameUiControl; // walks its siblings (0x004727c0)
    friend class UIDialog;  // reads its control container's first child (0x0046f120)
    // RaceStatus.cpp 0x004e5ca0 reads a racer's +0x25 bit 0.
    friend int UnknownFunction4e5ca0(struct UnknownEventRacerPart** list);
    friend int UnknownFunction4e62d0(int keepRacing); // and +0x25 bit 0 of the view's racers

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
