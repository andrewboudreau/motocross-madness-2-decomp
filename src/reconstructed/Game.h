#pragma once

#include "Display.h"
#include "GameObject.h"
#include "RenderTarget.h"

class ControlInterface;
class DebugOverlay;
class SoundInterface;
class TextureMapManager;
struct UnknownControlEvent;
struct UnknownInputEntry;

// Network object at Game+0x08 (0x128 bytes; its code is near Net.cpp's
// literals). The class is not established.
class UnknownNetObject {
public:
    UnknownNetObject();                       // 0x004ab480
    ~UnknownNetObject();                      // 0x004ab570
    long UnknownFunction4ab6b0(int value);    // 0x004ab6b0

    unsigned char field_0x00[0x128];
};

// Global at 0x0056c470: a GameObject (Game's initialiser adds it as a child
// of the root and calls its slot 5), with a flag byte at +0x2c.
class UnknownGlobal56c470 : public GameObject {
public:
    void UnknownFunction4691f0();             // 0x004691f0

    unsigned char field_0x2c;
};
extern UnknownGlobal56c470* g_UnknownGlobal56c470;

// Static object at 0x0065b478 (its code is near FontTextureManager.cpp's
// literals).
class UnknownStatic65b478 {
public:
    void UnknownFunction4677c0();             // 0x004677c0
};
extern UnknownStatic65b478 g_UnknownStatic65b478;

// cdecl 0x00460ad0: fills math lookup tables (called by the constructor).
void UnknownFunction460ad0();

// cdecl 0x004bfa80: the current time stamp.
unsigned int UnknownFunction4bfa80();

// cdecl 0x0052d0d0, called last on shutdown.
void UnknownFunction52d0d0();

// RTTI: Game (root; PCGame : Game and TrackGame : PCGame derive from it).
// Its functions pass the literal __FILE__ "D:\\aardvark\\VC\\krusty2\\Game.cpp".
// The global object at 0x0056e26c is a Game: Game's own slots 13 and 14
// handle ControlInterface's input events, and the members the camera and
// input code read (+0x0c, +0x14 the ControlInterface, the +0x2d4 bits,
// +0x2f0) are the ones Game's constructor initialises. Slot signatures not
// yet reconstructed are placeholders; names are provisional.
class Game {
public:
    Game();                                   // 0x00467990
    virtual ~Game();                          // 0x00468a10 (deleting wrapper 0x00467ac0)
    virtual int UnknownVirtualSlot1();        // 0x00467ae0 (shared "return 1" body)
    virtual int UnknownVirtualSlot2();        // 0x00467af0: creates the controls
    virtual int UnknownVirtualSlot3();        // 0x00468c90 (shared "return 1" body)
    virtual int UnknownVirtualSlot4();        // 0x00468c90
    virtual int UnknownVirtualSlot5();        // 0x00467ae0
    virtual int UnknownVirtualSlot6();        // 0x00467ae0
    virtual void UnknownVirtualSlot7() = 0;
    virtual int UnknownVirtualSlot8();        // 0x00467eb0: renders a frame
    virtual int UnknownVirtualSlot9();        // 0x004685c0
    virtual void UnknownVirtualSlot10();      // 0x004685d0, not reconstructed (rdtsc)
    virtual int UnknownVirtualSlot11(int value);                                          // 0x004688a0
    virtual int UnknownVirtualSlot12(int value);                                          // 0x004688e0
    virtual int UnknownVirtualSlot13(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x00468900
    virtual int UnknownVirtualSlot14(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x00468930
    virtual int UnknownVirtualSlot15();       // 0x00468a30: shutdown
    virtual int UnknownVirtualSlot16(int value); // 0x00468ae0: network object
    virtual int UnknownVirtualSlot17(int a, int b, int c, int d, int e);                  // 0x00468ba0
    virtual int UnknownVirtualSlot18(const char* name, char* path); // 0x00468bd0
    virtual void UnknownVirtualSlot19() = 0;
    virtual int UnknownVirtualSlot20(const char* name, int defaultValue) = 0; // "VideoMemoryMB"
    virtual float UnknownVirtualSlot21(const char* name, float defaultValue) = 0;
    // Named setting (JoystickDevice asks for "JoyDirectionFlipped").
    virtual int UnknownVirtualSlot22(const char* name, int defaultValue) = 0;
    virtual int UnknownVirtualSlot23(const char* name, const char* defaultValue, char* buffer,
                                     unsigned long* size) = 0;
    virtual int UnknownVirtualSlot24(const char* name, void* data, unsigned long* size) = 0;
    virtual int UnknownVirtualSlot25(const char* name, int value) = 0;
    virtual int UnknownVirtualSlot26(const char* name, int value) = 0;
    virtual int UnknownVirtualSlot27(const char* name, int value) = 0;
    virtual int UnknownVirtualSlot28(const char* name, const char* value) = 0;
    virtual int UnknownVirtualSlot29(const char* name, const void* data, unsigned long size) = 0;
    virtual int UnknownVirtualSlot30(int id, char* text);                                 // 0x00468c60
    virtual RenderTarget* UnknownVirtualSlot31() = 0;
    virtual int UnknownVirtualSlot32() = 0;
    virtual int UnknownVirtualSlot33();       // 0x00467e80
    virtual void UnknownVirtualSlot34() = 0;

    // 0x00467b70: creates the root objects, sound, textures and (with the
    // "DebugOverlay" registry flag) the debug overlay.
    int UnknownFunction467b70(int value);
    void UnknownFunction468880();             // 0x00468880 (PCCamera slot 27)

    SoundInterface* field_0x04;               // a PCSoundInterface (initialiser)
    UnknownNetObject* field_0x08;
    UnknownDisplay* field_0x0c;
    RenderTarget* field_0x10;                 // a PCRenderTarget (PCGame slot 31)
    ControlInterface* field_0x14;
    int field_0x18;                           // 1 initially; KrustyBikeCamera slot 42 tests > 1
    TextureMapManager* field_0x1c;
    int field_0x20;
    int field_0x24;
    int field_0x28;
    int field_0x2c;                           // 0x115c after initialisation
    int field_0x30;
    GameObject* field_0x34;                   // second root object (initialiser 0x00467b70)
    DebugOverlay* field_0x38;                 // with "DebugOverlay" set
    TextureMapManager* field_0x3c;
    char field_0x40[0x1c4 - 0x40];            // empty string initially
    int field_0x1c4;
    int field_0x1c8;
    char field_0x1cc[0x2d0 - 0x1cc];          // directory for slot 18
    int field_0x2d0;
    unsigned char field_0x2d4_bit0 : 1;       // "AllowFreezeCamera"; slot 14 key handling
    unsigned char field_0x2d4_bit1 : 1;
    unsigned char field_0x2d4_bit2 : 1;       // registry "TestKey" (debug input, overlay)
    unsigned char field_0x2d4_bit3 : 1;
    unsigned char field_0x2d4_bit4 : 1;
    unsigned char field_0x2d4_bit5 : 1;
    unsigned char field_0x2d4_bit6 : 1;
    unsigned char field_0x2d4_bit7 : 1;
    unsigned char field_0x2d5_bit0 : 1;       // slots 11 and 13
    unsigned char field_0x2d5_bit1 : 1;       // set on shutdown
    unsigned char field_0x2d5_bit2 : 1;       // set by the constructor
    unsigned char field_0x2d5_bit3 : 1;       // set by the constructor
    unsigned char field_0x2d5_bits : 4;
    unsigned char field_0x02d6[2];
    unsigned int field_0x2d8;                 // time stamps (0x00468880)
    unsigned int field_0x2dc;
    float field_0x2e0;
    float field_0x2e4;
    float field_0x2e8;
    float field_0x2ec;
    float field_0x2f0;                        // frame time (KeyboardDevice 0x0048a0c0)
    GameObject* field_0x2f4;                  // root object; most slots forward to it
};
