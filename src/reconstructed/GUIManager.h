#pragma once

#include <windows.h>

#include "ContainerList.h"
#include "ControlInterface.h"
#include "GameObject.h"
#include "RenderTarget.h"
#include "UIDialog.h"

// GUIManager.cpp (literal __FILE__ "D:\aardvark\VC\krusty2\GUIManager.cpp"
// at 0x0056c3cc). Strong inference, the TU is 0x00484dd0..0x004886df: its
// .CRT$XCU entries are 0x0056623c..0x0056624c (the four per-TU vector
// initializers and the HiResMeter global's initializer 0x00488340), its
// .data runs from "GroundFog" (0x0056c338) to the HiResMeter strings
// (0x0056c540), and InGameProcs.cpp's code starts at 0x004886e0. RTTI
// classes whose vtables point into it: GUICursor : GameCursor, GUIManager,
// UIDlgContainer, ToolTip, GUIInputDevice, GUIUser and HiResMeter (all
// GameObjects). Member names are provisional.

class BackgroundImage;
class DebugOverlay;
class GUIInputDevice;
class GUIManager;
class GUIUser;
class InputDevice;
class Palette8;
class PCTextureMap;
class SoundGroup;
class TextureMapManager;

// Stack iterator over an object's descendants whose class name matches
// (0x94 bytes; samples/physics/soultree_base declares the same helper).
class GameObjectIterator {
public:
    GameObjectIterator(GameObject* root, int mode, const char* filter); // 0x00469950
    ~GameObjectIterator();                                                // 0x00469a40
    GameObject* Next();                                                   // 0x00469a50

private:
    char field_0x00[0x94];
};

// IDirectDrawClipper-shaped interface at GUIManager+0x3dc (created through
// Display+0x190's method 4, handed to Display+0x19c's method 28).
struct UnknownGuiClipper {
    virtual long __stdcall UnknownMethod0();
    virtual long __stdcall UnknownMethod1();
    virtual long __stdcall UnknownMethod2();                          // Release
    virtual long __stdcall UnknownMethod3();
    virtual long __stdcall UnknownMethod4();
    virtual long __stdcall UnknownMethod5();
    virtual long __stdcall UnknownMethod6();
    virtual long __stdcall UnknownMethod7(void* clipList, unsigned long flags); // SetClipList
    virtual long __stdcall UnknownMethod8(unsigned long flags, void* window);   // SetHWnd
};

// The two methods 0x004868b0 calls on Display+0x190 and Display+0x19c
// (RenderInterfaces.h keeps their slots argument-less).
struct UnknownGuiDirectDraw {
    virtual long __stdcall UnknownMethod0();
    virtual long __stdcall UnknownMethod1();
    virtual long __stdcall UnknownMethod2();
    virtual long __stdcall UnknownMethod3();
    virtual long __stdcall UnknownMethod4(unsigned long flags, UnknownGuiClipper** clipper,
                                          void* outer);              // CreateClipper
};

struct UnknownGuiSurface {
    virtual long __stdcall UnknownMethod0();
    virtual long __stdcall UnknownMethod1();
    virtual long __stdcall UnknownMethod2();
    virtual long __stdcall UnknownMethod3();
    virtual long __stdcall UnknownMethod4();
    virtual long __stdcall UnknownMethod5();
    virtual long __stdcall UnknownMethod6();
    virtual long __stdcall UnknownMethod7();
    virtual long __stdcall UnknownMethod8();
    virtual long __stdcall UnknownMethod9();
    virtual long __stdcall UnknownMethod10();
    virtual long __stdcall UnknownMethod11();
    virtual long __stdcall UnknownMethod12();
    virtual long __stdcall UnknownMethod13();
    virtual long __stdcall UnknownMethod14();
    virtual long __stdcall UnknownMethod15();
    virtual long __stdcall UnknownMethod16();
    virtual long __stdcall UnknownMethod17();
    virtual long __stdcall UnknownMethod18();
    virtual long __stdcall UnknownMethod19();
    virtual long __stdcall UnknownMethod20();
    virtual long __stdcall UnknownMethod21();
    virtual long __stdcall UnknownMethod22();
    virtual long __stdcall UnknownMethod23();
    virtual long __stdcall UnknownMethod24();
    virtual long __stdcall UnknownMethod25();
    virtual long __stdcall UnknownMethod26();
    virtual long __stdcall UnknownMethod27();
    virtual long __stdcall UnknownMethod28(UnknownGuiClipper* clipper); // SetClipper
};

// The fog object 0x00484f10 finds by name ("GroundFog") under Game+0x34.
// Only the members it reads are declared; its class is not established.
struct UnknownGroundFogCamera {
    unsigned char field_0x000[0x170];
    float field_0x170;                        // eye position
    float field_0x174;
    float field_0x178;
};

struct UnknownGroundFogOwner {
    unsigned char field_0x00[0x08];
    UnknownGroundFogCamera* field_0x08;
};

struct UnknownGroundFog {
    unsigned char field_0x00[0x18];
    UnknownGroundFogOwner* field_0x18;        // GameObject+0x18
    unsigned char field_0x1c[0x30 - 0x1c];
    float field_0x30;                         // fog plane height
    float field_0x34;                         // depth limit
    int field_0x38;                           // alpha limit
    unsigned char field_0x3c[0x40 - 0x3c];
    float field_0x40;                         // density
};

// A 0x20-byte lit vertex (D3DLVERTEX layout): position, colour, specular.
struct UnknownFogVertex {
    float x;
    float y;
    float z;
    unsigned long reserved;
    unsigned long color;
    unsigned long specular;
    float tu;
    float tv;
};

// The object whose first member 0x00484f10 caches the fog in (0 before
// the lookup, 1 when there is none). Its class is not established.
struct UnknownGroundFogShader {
    // 0x00484f10: fogs `count` vertices by their depth below the fog plane.
    void UnknownFunction484f10(UnknownFogVertex* vertices, int count);

    UnknownGroundFog* field_0x00;
};

// Cursor animation passed to GUICursor 0x00485150 (GUIUser+0x1c8).
struct UnknownCursorAnimation {
    int UnknownFunction4730b0();              // 0x004730b0: current frame
    unsigned char field_0x00[0x08];
    int field_0x08;
    int field_0x0c;
    int field_0x10;
    unsigned char field_0x14[0x20 - 0x14];
    int field_0x20;
};

// RTTI: GameCursor : GameObject (vtable 0x0055130c; constructor
// 0x0043ea00). Only what GUIManager.cpp uses is declared.
class GameCursor : public GameObject {
public:
    explicit GameCursor(int flags);           // 0x0043ea00
    virtual ~GameCursor();                    // 0x0043eb60 (deleting wrapper 0x0043ea50)
    virtual int UnknownVirtualSlot15();       // 0x0043ed40
    // 0x0043ea70: an input-driven cursor following the two bindings.
    GameObject* UnknownFunction43ea70(void* target, UnknownControlBinding* x, UnknownControlBinding* y,
                                      const char* image, TextureMapManager* textures,
                                      BackgroundImage* background, void* palette, Palette8* palette8);
    // 0x0043eaf0: a cursor without bindings.
    GameObject* UnknownFunction43eaf0(void* target, const char* image, TextureMapManager* textures,
                                      BackgroundImage* background, void* palette, Palette8* palette8);

    unsigned char field_0x2c[0x38 - 0x2c];
    int field_0x38;                           // current frame
    unsigned char field_0x3c[0x54 - 0x3c];
    int field_0x54;                           // position
    int field_0x58;
};

// RTTI: GUICursor : GameCursor (vtable 0x00553ed0; 0x60 bytes, the size
// 0x00487dd0 allocates).
class GUICursor : public GameCursor {
public:
    explicit GUICursor(int flags);            // 0x00485100
    virtual ~GUICursor();                     // 0x00485140 (deleting wrapper 0x00485120)
    virtual int UnknownVirtualSlot15();       // 0x00485170
    void UnknownFunction485150(UnknownCursorAnimation* animation); // 0x00485150

    UnknownCursorAnimation* field_0x5c;
};

// A dialog as GUIManager sees it: UIDialog (0x7f58 bytes) with its own
// slots 27 and 28 and the members read here. Never constructed as such.
class UnknownGuiDialog : public UIDialog {
public:
    // Slot 27 (0x6c): creates the dialog's controls.
    virtual int UnknownVirtualSlot27(void* target, int a, int b, int flags, SoundGroup* sound,
                                     TextureMapManager* textures, char* directory,
                                     UnknownGuiDialog* parent, BackgroundImage* background,
                                     char* font, int fontSize, GUIManager* gui, GUIUser* user, int c);
    virtual void UnknownVirtualSlot28(int value); // slot 28 (0x70)

    void UnknownFunction46ea60(int value);    // 0x0046ea60
    int UnknownFunction46e9a0(int value);     // 0x0046e9a0
    void UnknownFunction46ffd0(int value);    // 0x0046ffd0
    void UnknownFunction470070(int a, int b, CameraRect* rect); // 0x00470070

    UnknownGuiDialog* field_0x2c;             // parent dialog
    unsigned char field_0x30[0x34 - 0x30];
    GUIUser* field_0x34;
    unsigned char field_0x38[0xc8 - 0x38];
    int field_0xc8;
    unsigned char field_0xcc[0xdc - 0xcc];
    int field_0xdc;                           // font height
    char field_0xe0[0x108 - 0xe0];            // font face
    int field_0x108;                          // bold
    unsigned char field_0x10c[0x144 - 0x10c];
    int field_0x144;                          // flags from 0x00485a70
    int field_0x148;                          // takes the input first (0x00485df0)
    unsigned char field_0x14c[0x160 - 0x14c];
    RECT field_0x160;                         // screen area
    unsigned char field_0x170[0x7f3c - 0x170];
    GameObject* field_0x7f3c;                 // child dialogs are added here
};

// A dialog control as GUIUser and ToolTip see it. Never constructed as such.
class UnknownGuiControl : public GameObject {
public:
    virtual void UnknownVirtualSlot27();
    virtual void UnknownVirtualSlot28();
    virtual void UnknownVirtualSlot29();
    virtual void UnknownVirtualSlot30();
    virtual int UnknownVirtualSlot31();       // takes the focus; 0 refuses it
    virtual void UnknownVirtualSlot32(UnknownGuiControl* next);
    virtual void UnknownVirtualSlot33(UnknownGuiControl* next);

    unsigned char field_0x2c[0x3c - 0x2c];
    CameraRect field_0x3c;                    // area
    unsigned char field_0x4c[0x5c - 0x4c];
    int field_0x5c;                           // control type (11: edit field)
    unsigned char field_0x60[0xb8 - 0x60];
    UnknownGuiDialog* field_0xb8;
    GUIManager* field_0xbc;
    unsigned char field_0xc0[0xf0 - 0xc0];
    char* field_0xf0;                         // tool tip text
    unsigned char field_0xf4[0x130 - 0xf4];
    char field_0x130[0x158 - 0x130];          // font face
    int field_0x158;                          // bold
    unsigned char field_0x15c[0x160 - 0x15c];
    int field_0x160;                          // font height
    unsigned char field_0x164[0x210 - 0x164];
    char* field_0x210;                        // accepted characters
    RECT field_0x214;
};

// RTTI: UIDlgContainer : GameObject (vtable 0x00553fb0; 0x2c bytes, the
// size 0x004853b0 allocates; inline constructor).
class UIDlgContainer : public GameObject {
public:
    UIDlgContainer() : GameObject(1) {}
    virtual ~UIDlgContainer();                // 0x00488110 (deleting wrapper 0x004880f0)
};

// RTTI: ToolTip : GameObject (vtable 0x00554020; 0x68 bytes, the size
// 0x00487680 allocates).
class ToolTip : public GameObject {
public:
    explicit ToolTip(int flags);              // 0x00486990
    virtual ~ToolTip();                       // 0x00486a30 (deleting wrapper 0x004869f0)
    virtual int UnknownVirtualSlot10(float frameTime); // 0x00486ab0
    virtual int UnknownVirtualSlot15();       // 0x00486db0

    ToolTip* UnknownFunction486a10(void* target, GUIManager* gui); // 0x00486a10
    void UnknownFunction486b10(UnknownGuiControl* control);        // 0x00486b10
    // 0x00486b80: shows `text` near `position` for `time` seconds.
    void UnknownFunction486b80(const char* text, int* position, float time);

    GUIManager* field_0x2c;
    PCTextureMap* field_0x30;                 // the rendered text
    int field_0x34;                           // background region, -1 when none
    int field_0x38;                           // frames left to restore
    CameraRect field_0x3c;                    // screen area
    RECT field_0x4c;                          // text area
    int field_0x5c;                           // enabled
    float field_0x60;                         // seconds left before showing
    int field_0x64;                           // shown
};

// RTTI: GUIInputDevice : GameObject (vtable 0x00554090; 0xc4 bytes).
// Binds a pointer position to an input device through two control
// bindings.
class GUIInputDevice : public GameObject {
public:
    GUIInputDevice();                         // 0x00486e80
    virtual ~GUIInputDevice();                // 0x00486f20 (deleting wrapper 0x00486f00)
    virtual int UnknownVirtualSlot10(float frameTime); // 0x00487320
    virtual int UnknownVirtualSlot25(void* value);     // 0x004873b0

    // 0x00486fb0: binds to `device` within [minX, maxX] x [minY, maxY]
    // (all 0: the target's size).
    GUIInputDevice* UnknownFunction486fb0(void* target, InputDevice* device, float minX, float maxX,
                                          float minY, float maxY);
    void UnknownFunction487150();             // 0x00487150: rebinds at the current position

    UnknownControlBinding field_0x2c;         // x
    UnknownControlBinding field_0x68;         // y
    POINT field_0xa4;                         // position
    InputDevice* field_0xac;
    float field_0xb0;                         // minimum x
    float field_0xb4;                         // maximum x
    float field_0xb8;                         // minimum y
    float field_0xbc;                         // maximum y
    GUIUser* field_0xc0;                      // owning user
};

// RTTI: GUIUser : GameObject (vtable 0x00554100; 0x1f4 bytes). One of
// GUIManager's four users (KrustyUI.h's UnknownKrustyUIGuiLayer is the
// same object): a cursor, a tool tip and the input devices it accepts.
class GUIUser : public GameObject {
public:
    GUIUser();                                // 0x004873d0
    virtual ~GUIUser();                       // 0x00487540 (deleting wrapper 0x00487520)
    virtual int UnknownVirtualSlot10(float frameTime); // 0x004875a0
    virtual int UnknownVirtualSlot15();       // 0x00487620

    void UnknownFunction4875c0(int force);    // 0x004875c0
    GUIUser* UnknownFunction487650(void* target, GUIManager* gui); // 0x00487650
    void UnknownFunction487680(void* target, GUIManager* gui); // 0x00487680: new tool tip
    void UnknownFunction487710();             // 0x00487710
    int UnknownFunction487730(UnknownGuiControl* control, UnknownGuiControl** previous, int update);
    int UnknownFunction487790(UnknownGuiControl* control, UnknownGuiControl** previous, int update);
    int UnknownFunction487800(UnknownGuiControl* control, UnknownGuiControl** previous);
    int UnknownFunction487870(UnknownGuiControl* control, UnknownGuiControl** previous);
    void UnknownFunction487990(int enable);   // 0x00487990: input method editor on/off
    int UnknownFunction487bf0(UnknownGuiControl** previous);
    int UnknownFunction487c30(GUIInputDevice* device); // 0x00487c30: accepts a device
    void UnknownFunction487d00();             // 0x00487d00: accepts the keyboard and joysticks
    void UnknownFunction487d60();             // 0x00487d60: forgets every device
    void UnknownFunction487dd0(const char* image, int visible); // 0x00487dd0: creates the cursor
    void UnknownFunction487fb0(UnknownCursorAnimation* animation);
    void UnknownFunction488010(const char* image, int redraw);
    void UnknownFunction4880c0();             // 0x004880c0: releases the cursor
    int UnknownFunction488120(GUIInputDevice* device); // 0x00488120: sets the pointer device
    int UnknownFunction488160(InputDevice* device);
    int UnknownFunction4881d0(UnknownControlEvent* event);
    GUIInputDevice* UnknownFunction488240(InputDevice* device);
    GUIInputDevice* UnknownFunction4882a0(UnknownControlEvent* event);
    GUIInputDevice* UnknownFunction488310(int index);

    GUIInputDevice* field_0x2c;               // pointer device
    GUICursor* field_0x30;
    int field_0x34;                           // pointer position
    int field_0x38;
    int field_0x3c[32];
    GUIManager* field_0xbc;
    ToolTip* field_0xc0;
    int field_0xc4;
    char field_0xc8[0x80];                    // cursor image ("cursor.tga")
    char field_0x148[0x80];                   // wait image ("wait.tga")
    UnknownCursorAnimation* field_0x1c8;
    UnknownGuiControl* field_0x1cc;
    UnknownGuiControl* field_0x1d0;
    UnknownGuiControl* field_0x1d4;           // focus
    UnknownGuiControl* field_0x1d8;
    int field_0x1dc;
    ContainerList<GUIInputDevice*> field_0x1e0; // accepted devices
};

// RTTI: GUIManager : GameObject (vtable 0x00553f40; 0x3e0 bytes). KrustyUI
// keeps one at +0x2c; KrustyUI.h declares the same object as
// UnknownKrustyUIGui (its constructor is 0x00485190).
class GUIManager : public GameObject {
public:
    explicit GUIManager(int flags);           // 0x00485190
    virtual ~GUIManager();                    // 0x00485320 (deleting wrapper 0x00485300)
    virtual int UnknownVirtualSlot10(float frameTime); // 0x00485870
    virtual int UnknownVirtualSlot13();       // 0x004858d0
    virtual int UnknownVirtualSlot15();       // 0x00485970
    virtual int UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x00485830
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x004857f0

    // 0x004853b0: sets the GUI up (KrustyUI 0x004988a0 passes its target,
    // Display+0x64, the textures, 0, 0, 0, "Arial", 15, "ui\\cursor.tga"
    // and a callback).
    GUIManager* UnknownFunction4853b0(void* target, Palette8* palette, TextureMapManager* textures,
                                      BackgroundImage* background, int startSound, SoundGroup* sound,
                                      const char* font, int fontSize, const char* cursor,
                                      int callback);
    // 0x00485a70: shows `dialog`.
    UnknownGuiDialog* UnknownFunction485a70(UnknownGuiDialog* dialog, int a, int flags, int b,
                                            UnknownGuiDialog* parent, int c, int d, int wait);
    void UnknownFunction485bd0(UnknownGuiDialog* dialog, int a, int wait);
    int UnknownFunction485c80(const char* resource); // 0x00485c80: opens a dialog resource
    void UnknownFunction485d50();             // 0x00485d50: closes it
    void UnknownFunction485d70(const char* directory); // 0x00485d70
    UnknownGuiDialog* UnknownFunction485df0(); // 0x00485df0: the dialog taking input
    int UnknownFunction485ec0(int value);     // 0x00485ec0
    int UnknownFunction485ee0(int value);     // 0x00485ee0: sets +0x2c, returns the old value
    void UnknownFunction485ef0();             // 0x00485ef0: creates the background
    void UnknownFunction485fc0();             // 0x00485fc0: releases it
    void UnknownFunction4860a0(int dim);      // 0x004860a0: grabs the screen as the background
    void UnknownFunction4860f0();             // 0x004860f0: releases the grab
    void UnknownFunction486150(Palette8* palette); // 0x00486150
    // 0x00486170: copies the screen (or `rect`) into a new texture, halving
    // its brightness when `dim`.
    PCTextureMap* UnknownFunction486170(int dim, CameraRect* rect);
    int UnknownFunction4864f0();              // 0x004864f0
    void UnknownFunction486500();             // 0x00486500: redraws a frame
    GUIUser* UnknownFunction486540(int index); // 0x00486540: user `index` (0 past 3)
    void UnknownFunction486560(const char* image); // 0x00486560
    void UnknownFunction486590(const char* image, int visible); // 0x00486590
    void UnknownFunction4865e0(const char* image, int redraw); // 0x004865e0
    void UnknownFunction486630(int show);     // 0x00486630
    void UnknownFunction486680();             // 0x00486680: releases the cursors
    void UnknownFunction4866c0(const char* font); // 0x004866c0
    // 0x00486740: a `width` x `height` texture filled with `color`.
    PCTextureMap* UnknownFunction486740(int width, int height, unsigned long color);
    int UnknownFunction4868b0(int enable);    // 0x004868b0: window clipper on/off
    void UnknownFunction464e90();             // 0x00464e90 (shared empty body)

    int field_0x2c;
    UnknownGuiDialog* field_0x30;             // dialog opened by 0x00485c80
    Palette8* field_0x34;
    Palette8* field_0x38;
    BackgroundImage* field_0x3c;
    BackgroundImage* field_0x40;              // redrawn when +0x1f4 runs out
    SoundGroup* field_0x44;
    int field_0x48;                           // owns +0x3c
    int field_0x4c;                           // owns +0x44
    char field_0x50[0x80];                    // dialog font
    short field_0xd0;                         // dialog font size
    int field_0xd4;
    TextureMapManager* field_0xd8;
    PCTextureMap* field_0xdc;                 // screen grab
    int field_0xe0;                           // draw the grab before the children
    int field_0xe4;                           // its background region
    int field_0xe8;
    int field_0xec;                           // from TrackGame+0xc40 (KrustyUI 0x004988a0)
    char field_0xf0[0x100];                   // dialog directory
    int field_0x1f0;
    int field_0x1f4;                          // frames to wait
    int field_0x1f8;
    void* field_0x1fc;                        // tool tip font (HFONT)
    char field_0x200[0x80];                   // cursor image
    char field_0x280[0x80];                   // wait cursor image
    unsigned char field_0x300[0x304 - 0x300];
    int field_0x304;
    GUIInputDevice* field_0x308;              // mouse
    GUIInputDevice* field_0x30c;              // keyboard
    GUIInputDevice* field_0x310[8];           // joysticks
    GUIUser* field_0x330[4];
    int field_0x340;                          // users
    GameObject* field_0x344;                  // the UIDlgContainer
    void* field_0x348;                        // "uilang.dll" module
    int field_0x34c;
    char field_0x350[0x80];                   // tool tip font face
    int field_0x3d0;
    int field_0x3d4;                          // bold
    int field_0x3d8;                          // italic
    UnknownGuiClipper* field_0x3dc;
};

// RTTI: HiResMeter : GameObject (vtable 0x00554170). A global instance at
// 0x0067b468 (initializer 0x00488340) prints timer rows on a DebugOverlay
// page.
struct UnknownHiResMeterEntry {
    float field_0x00;
    float field_0x04;
    float field_0x08;                         // calls
    float field_0x0c;                         // total time
    float field_0x10;                         // calls this tick
    float field_0x14;                         // time this tick
    float field_0x18;                         // time over all ticks
    float field_0x1c;                         // ticks
    float field_0x20;                         // peak
    char field_0x24[0x24];                    // name
};

class HiResMeter : public GameObject {
public:
    explicit HiResMeter(int flags);           // 0x00488380
    virtual ~HiResMeter();                    // 0x00488430 (deleting wrapper 0x00488410)
    // 0x00488440: calibrates the time stamp counter (rdtsc; not
    // reconstructed).
    virtual GameObject* UnknownVirtualSlot8(void* value);
    virtual int UnknownVirtualSlot10(float frameTime); // 0x004885c0

    void UnknownFunction488510();             // 0x00488510
    void UnknownFunction488540();             // 0x00488540
    void UnknownFunction488580();             // 0x00488580

    unsigned char field_0x2c;                 // the counter is usable
    float field_0x30;                         // counts per millisecond
    int field_0x34;                           // entries in use
    int field_0x38;
    int field_0x3c;                           // overlay page, -1 until assigned
    UnknownHiResMeterEntry field_0x40[50];
};
