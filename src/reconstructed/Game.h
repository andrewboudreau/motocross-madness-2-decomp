#pragma once

class ControlInterface;
struct UnknownControlEvent;
struct UnknownInputEntry;

// Object at Game+0x0c. Game slot 9 calls its slot 3; RenderTarget 0x004e8cc0
// reads +0x6c.
struct UnknownObject56e26cSettings {
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    unsigned char field_0x04[0x6c - 4];
    int field_0x6c;      // freezes RenderTarget's frame index (0x004e8cc0)
};

// Interface at Game+0x2f4; most Game slots forward to it. Released (slot 2)
// on shutdown. The class is not established.
class UnknownGameInterface {
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
    virtual void UnknownVirtualSlot16(int value);
    virtual void UnknownVirtualSlot17();
    virtual void UnknownVirtualSlot18();
    virtual int UnknownVirtualSlot19(int value);
    virtual void UnknownVirtualSlot20();
    virtual void UnknownVirtualSlot21();
    virtual int UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry);
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry);
    virtual int UnknownVirtualSlot24(int a, int b, int c, int d, int e);
    virtual void UnknownVirtualSlot25(int value);
};

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
    virtual void UnknownVirtualSlot2();       // 0x00467af0, not reconstructed
    virtual int UnknownVirtualSlot3();        // 0x00468c90 (shared "return 1" body)
    virtual int UnknownVirtualSlot4();        // 0x00468c90
    virtual int UnknownVirtualSlot5();        // 0x00467ae0
    virtual int UnknownVirtualSlot6();        // 0x00467ae0
    virtual void UnknownVirtualSlot7() = 0;
    virtual void UnknownVirtualSlot8();       // 0x00467eb0, not reconstructed
    virtual int UnknownVirtualSlot9();        // 0x004685c0
    virtual void UnknownVirtualSlot10();      // 0x004685d0, not reconstructed
    virtual int UnknownVirtualSlot11(int value);                                          // 0x004688a0
    virtual int UnknownVirtualSlot12(int value);                                          // 0x004688e0
    virtual int UnknownVirtualSlot13(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x00468900
    virtual int UnknownVirtualSlot14(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x00468930
    virtual void UnknownVirtualSlot15();      // 0x00468a30, not reconstructed
    virtual void UnknownVirtualSlot16();      // 0x00468ae0, not reconstructed
    virtual int UnknownVirtualSlot17(int a, int b, int c, int d, int e);                  // 0x00468ba0
    virtual void UnknownVirtualSlot18();      // 0x00468bd0, not reconstructed
    virtual void UnknownVirtualSlot19() = 0;
    virtual void UnknownVirtualSlot20() = 0;
    virtual void UnknownVirtualSlot21() = 0;
    // Named setting (JoystickDevice asks for "JoyDirectionFlipped").
    virtual int UnknownVirtualSlot22(const char* name, int defaultValue) = 0;
    virtual void UnknownVirtualSlot23() = 0;
    virtual void UnknownVirtualSlot24() = 0;
    virtual void UnknownVirtualSlot25() = 0;
    virtual void UnknownVirtualSlot26() = 0;
    virtual void UnknownVirtualSlot27() = 0;
    virtual void UnknownVirtualSlot28() = 0;
    virtual void UnknownVirtualSlot29() = 0;
    virtual int UnknownVirtualSlot30(int id, char* text);                                 // 0x00468c60
    virtual int UnknownVirtualSlot31() = 0;
    virtual void UnknownVirtualSlot32() = 0;
    virtual int UnknownVirtualSlot33();       // 0x00467e80
    virtual void UnknownVirtualSlot34() = 0;

    void UnknownFunction468880();             // 0x00468880 (PCCamera slot 27)

    int field_0x04;
    void* field_0x08;
    UnknownObject56e26cSettings* field_0x0c;
    int field_0x10;
    ControlInterface* field_0x14;
    int field_0x18;                           // 1 initially; KrustyBikeCamera slot 42 tests > 1
    unsigned char field_0x001c[0x34 - 0x1c];
    int field_0x34;
    void* field_0x38;
    int field_0x3c;
    char field_0x40[0x1c4 - 0x40];            // empty string initially
    int field_0x1c4;
    int field_0x1c8;
    unsigned char field_0x01cc[0x2d0 - 0x1cc];
    int field_0x2d0;
    unsigned char field_0x2d4;                // flag bits (bit 2: KeyboardDevice 0x0048a240)
    unsigned char field_0x2d5_bit0 : 1;       // slots 11 and 13
    unsigned char field_0x2d5_bits : 7;
    unsigned char field_0x02d6[2];
    int field_0x2d8;
    int field_0x2dc;
    float field_0x2e0;
    float field_0x2e4;
    float field_0x2e8;
    float field_0x2ec;
    float field_0x2f0;                        // frame time (KeyboardDevice 0x0048a0c0)
    UnknownGameInterface* field_0x2f4;
};
