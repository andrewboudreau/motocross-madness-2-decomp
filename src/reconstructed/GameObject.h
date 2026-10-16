#pragma once

struct UnknownControlEvent;
struct UnknownInputEntry;

#include "BaseObject.h"

// 0x0065b548: the number of running GameObjectIterators (each start adds
// one, Stop or running out subtracts it); while nonzero, Release, the
// sibling release and the relinking calls return 0.
extern int g_ActiveGameObjectIterators;

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

    // Slot names come from the callers (tier 2), slot numbers and bodies are
    // retail (vtable 0x00552a2c):
    //  4/5   Disable/Enable: DisableByClassName and EnableByClassName call
    //        them; they clear/set the +0x25 bits 0-1, then call slot 6/7.
    //  6/7   OnDisable/OnEnable: the walks slots 4 and 5 end in.
    //  8     SetRenderContext: stores renderContext and returns this; callers
    //        pass the render target and append the result (Game::CreateObjects).
    //  10    Tick: Game::Tick (0x004685d0) calls the root's slot 10 with the
    //        clamped frame time and times it as "Tick" on the profile page.
    //  12/13/15 PrepareGeometry/RenderPre3D/RenderPost3D: Game::RenderFrame
    //        times the root's slots 12, 13 and 15 under those profile labels.
    //  16    SetPaused: Game::SetInputSuspended passes its flag; the body
    //        stores isPaused, and SoundGroup pauses its members.
    //  17/18 OnDeactivate/OnActivate: PCGame::OnDeactivate/OnActivate.
    //  20/21 OnChar/OnKeyDown: PCGame::OnChar/OnKeyDown.
    //  22/23 OnControlRelease/OnControlPress: Game slots 13/14.
    //  24    OnNetworkMessage: Game::OnNetworkMessage.
    //  25    AttachRenderContext: Game::AttachRenderTarget hands the root the
    //        render target; every child gets it, then renderContext is set.
    //  26    MarkPendingRelease: sets pendingRelease on the subtree.
    // Slots 9, 11, 14 and 19 keep placeholder names.
    virtual void Disable();
    virtual void Enable();
    virtual void OnDisable();
    virtual void OnEnable();
    virtual GameObject* SetRenderContext(void* value);
    virtual int UnknownVirtualSlot9(int value);
    virtual int Tick(float frameTime);
    virtual int UnknownVirtualSlot11(int value);
    virtual int PrepareGeometry();
    virtual int RenderPre3D();
    virtual int UnknownVirtualSlot14();
    virtual int RenderPost3D();
    virtual int SetPaused(int value);
    virtual int OnDeactivate();
    virtual int OnActivate();
    virtual int UnknownVirtualSlot19(int value);
    virtual int OnChar(int value);
    virtual int OnKeyDown(int value);
    // Input events from Game slots 13 and 14, passed down the children.
    virtual int OnControlRelease(UnknownControlEvent* event, UnknownInputEntry* entry);
    virtual int OnControlPress(UnknownControlEvent* event, UnknownInputEntry* entry);
    // Network messages (Game slot 17 forwards NetworkInterface 0x004aced0's).
    virtual int OnNetworkMessage(int type, void* data, int from, int to, int flags);
    virtual int AttachRenderContext(void* value);
    virtual void MarkPendingRelease();

    // 0x00469190: appends `child` (and its later siblings) to the children,
    // sets their parent and `child`'s +0x1c; returns `child`.
    int AppendChild(GameObject* child, int value);
    int AppendSibling(GameObject* child); // 0x00469130 (KrustyUI 0x004988a0)
    // 0x00469130 takes two arguments (ret 8); GUIManager.cpp passes -1 as the second.
    int AppendSibling(GameObject* child, int value);
    int InsertBefore(GameObject* parent, int value); // 0x00469260 (GUIManager.cpp)
    // 0x00469770: finds an object whose class list (+0x28) or RTTI name
    // matches `name`, among the children (mode 0), descendants (1),
    // siblings (2), parent (3) or ancestors (4). GUIManager.cpp passes 1,
    // "GroundFog".
    GameObject* FindByClassName(int mode, const char* name);
    // 0x004691f0: unlinks this object from its siblings and parent and
    // recomputes the root's flags; returns 1 (0 while the global is set).
    int Unlink();
    // 0x00469680: releases this object and every later sibling, back to front.
    int ReleaseSiblings();
    // 0x00469ce0: appends the RTTI class name of `object` to classNames.
    void AppendClassName(GameObject* object);
    // 0x004690c0: ORs `flags` into childFlags and propagates the result up
    // through every parent (each adds its own ownFlags).
    void AddFlags(unsigned int flags);
    // 0x00469100: recomputes childFlags from the children; returns it with
    // ownFlags added.
    unsigned int RecomputeFlags();
    // 0x00469c80: unlinks and releases every descendant marked with +0x25 bit 3.
    void ReleaseDetached();

protected:
    // 0x00468dd0 / 0x00468f10: TrackGame turns the "RaceSound" child on and
    // off through these.
    void DisableByClassName(const char* name);
    void EnableByClassName(const char* name);

    friend class GameObjectIterator; // gameobj.cpp walks the links and names
    friend class Game;      // reads the +0x25 bits of its DebugOverlay
    friend class TrackGame; // reads the +0x25 bits of its UI objects (0x00521860)
    friend class EventManager; // reads a race-mode object's +0x25 bit 0 (0x0045eef0)
    friend class Sound;     // reads its SoundGroup's +0x25 bit 2 (0x004bc6b0)
    friend class NameOverlay; // reads its racer's +0x25 bit 0 (0x005190e0)
    friend class StatsOverlay; // reads its racers' +0x25 bit 0 (0x00519a20)
    friend class RadarOverlay; // reads its racers' +0x25 bit 0 (0x0051bed0)
    friend class Wrecker;   // clears its rigid body's +0x25 bit 0 (0x005327c0)
    friend class RaceSound; // turns the "SoundGroup" children off (0x004e3430)
    friend class GUIManager; // reads a dialog's +0x25 bit 0 (0x00485bd0)
    friend class GUIUser;   // reads a control's +0x25 bit 3 and +0x28 (0x00487730, 0x00487800)
    friend class UICtlContainer; // walks its children back to front (0x0047b3f0)
    friend class UIControl; // walks its siblings (0x004727c0)
    friend class UIDialog;  // reads its control container's first child (0x0046f120)
    // RaceStatus.cpp 0x004e5ca0 reads a racer's +0x25 bit 0.
    friend int PruneStatusList(struct UnknownEventRacerPart** list);  // 0x004e5ca0
    friend int RankByScore(int keepRacing); // and +0x25 bit 0 of the view's racers (0x004e62d0)

    // Offsets and widths are evidenced by the reconstructed methods; names are
    // placeholders. Children are reached through firstChild and chained through
    // their nextSibling (traversal by slots 6, 7, 12-18 and 26).
    GameObject* prevSibling;                  // +0x08 previous sibling
    GameObject* nextSibling;                  // +0x0c next sibling
    GameObject* firstChild;                   // +0x10 first child
    GameObject* parent;                       // +0x14 parent
    void* renderContext;                      // +0x18 set by SetRenderContext and AttachRenderContext (slots 8, 25); the render target the object draws with (cast to RenderTarget/PCRenderTarget by its users)
    int ownFlags;                             // +0x1c flags this object adds to its parent's childFlags (0x004690c0, 0x00469100)
    unsigned int childFlags;                  // +0x20 OR of the children's ownFlags and childFlags; gates slots 12-18 (0x00469100)
    unsigned char field_0x24;
    // Byte +0x25 is a bitfield: the constructor's single merged store of all
    // four low bits is VC6's bitfield code shape. Bit 0 enables the gated child
    // walks, bit 3 excludes a child from every walk; bit 2 is set by slot 16
    // and also excludes a child from slots 9 and 10.
    unsigned char isEnabled : 1;              // +0x25 bit 0: cleared by Disable (slot 4), set by Enable (slot 5)
    unsigned char field_0x25_bit1 : 1;        // +0x25 bit 1: set and cleared with bit 0; gates OnNetworkMessage (slot 24)
    unsigned char isPaused : 1;               // +0x25 bit 2: stored by SetPaused (slot 16)
    unsigned char pendingRelease : 1;         // +0x25 bit 3: set by MarkPendingRelease (slot 26); ReleaseDetached releases the marked children
    char* classNames;                         // +0x28 DebugMalloc'd string, starts empty; see 0x00469ce0
};
