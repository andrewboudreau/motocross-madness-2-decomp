#pragma once

#include <windows.h>

#include "ContainerList.h"
#include "ControlInterface.h"
#include "GameCursor.h"
#include "GameObject.h"
#include "GameObjectIterator.h"
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

// IDirectDrawClipper-shaped interface at GUIManager+0x3dc (created through
// Display+0x190's method 4, handed to Display+0x19c's method 28).
struct UnknownGuiClipper {
    virtual long __stdcall UnknownMethod0();
    virtual long __stdcall UnknownMethod1();
    virtual long __stdcall Release();                          // Release
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
    virtual long __stdcall Release();
    virtual long __stdcall UnknownMethod3();
    virtual long __stdcall UnknownMethod4(unsigned long flags, UnknownGuiClipper** clipper,
                                          void* outer);              // CreateClipper
};

struct UnknownGuiSurface {
    virtual long __stdcall UnknownMethod0();
    virtual long __stdcall UnknownMethod1();
    virtual long __stdcall Release();
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
    int AdvancePastSounds();              // 0x004730b0: current frame
    unsigned char field_0x00[0x08];
    int field_0x08;
    int field_0x0c;
    int field_0x10;
    unsigned char field_0x14[0x20 - 0x14];
    int field_0x20;
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
    CameraRect area;                          // +0x3c: area
    unsigned char field_0x4c[0x5c - 0x4c];
    int controlType;                          // +0x5c: control type (11: edit field)
    unsigned char field_0x60[0xb8 - 0x60];
    UIDialog* ownerDialog;            // +0xb8
    GUIManager* ownerGui;                     // +0xbc
    unsigned char field_0xc0[0xf0 - 0xc0];
    char* toolTipText;                        // +0xf0: tool tip text
    unsigned char field_0xf4[0x130 - 0xf4];
    char fontFace[0x158 - 0x130];             // +0x130
    int bold;                                 // +0x158: bold
    unsigned char field_0x15c[0x160 - 0x15c];
    int fontHeight;                           // +0x160
    unsigned char field_0x164[0x210 - 0x164];
    char* acceptedCharacters;                 // +0x210: accepted characters
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
    void ShowText(const char* text, int* position, float time);

    GUIManager* tipGui;                       // +0x2c
    PCTextureMap* textTexture;                // +0x30: the rendered text
    int backgroundRegion;                     // +0x34: background region, -1 when none
    int restoreFrames;                        // +0x38: frames left to restore
    CameraRect screenArea;                    // +0x3c
    RECT textArea;                            // +0x4c: text area
    int enabled;                              // +0x5c: enabled
    float showDelay;                          // +0x60: seconds left before showing
    int shown;                                // +0x64
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
    GUIInputDevice* Bind(void* target, InputDevice* device, float minX, float maxX,
                         float minY, float maxY);
    void Rebind();             // 0x00487150: rebinds at the current position

    UnknownControlBinding bindingX;           // +0x2c: x
    UnknownControlBinding bindingY;           // +0x68: y
    POINT pointerPosition;                    // +0xa4: position
    InputDevice* inputDevice;                 // +0xac
    float rangeMinX;                          // +0xb0: minimum x
    float rangeMaxX;                          // +0xb4: maximum x
    float rangeMinY;                          // +0xb8: minimum y
    float rangeMaxY;                          // +0xbc: maximum y
    GUIUser* ownerUser;                       // +0xc0: owning user
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
    void CreateToolTip(void* target, GUIManager* gui); // 0x00487680: new tool tip
    void UnknownFunction487710();             // 0x00487710
    int UnknownFunction487730(UnknownGuiControl* control, UnknownGuiControl** previous, int update);
    int UnknownFunction487790(UnknownGuiControl* control, UnknownGuiControl** previous, int update);
    int UnknownFunction487800(UnknownGuiControl* control, UnknownGuiControl** previous);
    int UnknownFunction487870(UnknownGuiControl* control, UnknownGuiControl** previous);
    void EnableImeInput(int enable);   // 0x00487990: input method editor on/off
    int UnknownFunction487bf0(UnknownGuiControl** previous);
    int AcceptDevice(GUIInputDevice* device); // 0x00487c30: accepts a device
    void AcceptKeyboardAndJoysticks();             // 0x00487d00: accepts the keyboard and joysticks
    void ForgetDevices();             // 0x00487d60: forgets every device
    void CreateCursor(const char* image, int visible); // 0x00487dd0: creates the cursor
    void UnknownFunction487fb0(UnknownCursorAnimation* animation);
    void UnknownFunction488010(const char* image, int redraw);
    void ReleaseCursor();             // 0x004880c0: releases the cursor
    int SetPointerDevice(GUIInputDevice* device); // 0x00488120: sets the pointer device
    int UnknownFunction488160(InputDevice* device);
    int UnknownFunction4881d0(UnknownControlEvent* event);
    GUIInputDevice* UnknownFunction488240(InputDevice* device);
    GUIInputDevice* UnknownFunction4882a0(UnknownControlEvent* event);
    GUIInputDevice* UnknownFunction488310(int index);

    GUIInputDevice* pointerDevice;            // +0x2c: pointer device
    GUICursor* userCursor;                    // +0x30
    int field_0x34;                           // pointer position
    int field_0x38;
    int field_0x3c[32];
    GUIManager* userGui;                      // +0xbc
    ToolTip* userToolTip;                     // +0xc0
    int field_0xc4;
    char cursorImage[0x80];                   // +0xc8: cursor image ("cursor.tga")
    char waitImage[0x80];                     // +0x148: wait image ("wait.tga")
    UnknownCursorAnimation* cursorAnimation;  // +0x1c8
    UnknownGuiControl* field_0x1cc;
    UnknownGuiControl* field_0x1d0;
    UnknownGuiControl* focusControl;          // +0x1d4: focus
    UnknownGuiControl* field_0x1d8;
    int field_0x1dc;
    ContainerList<GUIInputDevice*> acceptedDevices; // +0x1e0: accepted devices
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
    GUIManager* SetUp(void* target, Palette8* palette, TextureMapManager* textures,
                      BackgroundImage* background, int startSound, SoundGroup* sound,
                      const char* font, short fontSize, const char* cursor,
                      int callback);
    // 0x00485a70: shows `dialog`.
    UIDialog* ShowDialog(UIDialog* dialog, int a, int flags, int b,
                                            UIDialog* parent, int c, int d, int wait);
    void UnknownFunction485bd0(UIDialog* dialog, int a, int wait);
    int OpenDialogResource(const char* resource); // 0x00485c80: opens a dialog resource
    void CloseDialogResource();             // 0x00485d50: closes it
    void UnknownFunction485d70(const char* directory); // 0x00485d70
    UIDialog* FindInputDialog();      // 0x00485df0: the dialog taking input
    Sound* FindSectionObject(const char* name); // 0x00485ec0: the opened dialog's section object
    int UnknownFunction485ee0(int value);     // 0x00485ee0: sets +0x2c, returns the old value
    void CreateBackground();                  // 0x00485ef0: creates the background
    void ReleaseBackground();             // 0x00485fc0: releases it
    void GrabBackground(int dim);      // 0x004860a0: grabs the screen as the background
    void ReleaseBackgroundGrab();             // 0x004860f0: releases the grab
    void UnknownFunction486150(Palette8* palette); // 0x00486150
    // 0x00486170: copies the screen (or `rect`) into a new texture, halving
    // its brightness when `dim`.
    PCTextureMap* CopyScreenToTexture(int dim, CameraRect* rect);
    int UnknownFunction4864f0();              // 0x004864f0
    void RedrawFrame();             // 0x00486500: redraws a frame
    GUIUser* GetUser(int index);              // 0x00486540: user `index` (0 past 3)
    void UnknownFunction486560(const char* image); // 0x00486560
    void UnknownFunction486590(const char* image, int visible); // 0x00486590
    void UnknownFunction4865e0(const char* image, int redraw); // 0x004865e0
    void ShowCursors(int show);               // 0x00486630: each user's cursor slot 5 (show) or 4
    void ReleaseCursors();             // 0x00486680: releases the cursors
    void UnknownFunction4866c0(const char* font); // 0x004866c0
    // 0x00486740: a `width` x `height` texture filled with `color`.
    PCTextureMap* CreateFilledTexture(int width, int height, unsigned long color);
    // 0x004868b0: creates (CreateClipper) and attaches (SetClipper, SetHWnd
    // with the game window) or detaches the primary surface's clipper.
    int EnableWindowClipper(int enable);
    void UnknownFunction464e90();             // 0x00464e90 (shared empty body)

    int field_0x2c;
    UIDialog* openedDialog;           // +0x30: dialog opened by 0x00485c80
    Palette8* guiPalette;                     // +0x34
    Palette8* field_0x38;
    BackgroundImage* guiBackground;           // +0x3c
    BackgroundImage* redrawBackground;        // +0x40: redrawn when +0x1f4 runs out
    SoundGroup* guiSoundGroup;                // +0x44
    int ownsBackground;                       // +0x48: owns +0x3c
    int ownsSoundGroup;                       // +0x4c: owns +0x44
    char dialogFontName[0x80];                // +0x50: dialog font
    short dialogFontSize;                     // +0xd0: dialog font size
    int field_0xd4;
    TextureMapManager* guiTextures;           // +0xd8
    PCTextureMap* screenGrab;                 // +0xdc: screen grab
    int drawGrabFirst;                        // +0xe0: draw the grab before the children
    int grabRegion;                           // +0xe4
    int field_0xe8;
    int field_0xec;                           // from TrackGame+0xc40 (KrustyUI 0x004988a0)
    char dialogDirectory[0x100];              // +0xf0: dialog directory
    int field_0x1f0;
    int waitFrames;                           // +0x1f4: frames to wait
    int field_0x1f8;
    void* toolTipFont;                        // +0x1fc: tool tip font (HFONT)
    char cursorImage[0x80];                   // +0x200: cursor image
    char waitCursorImage[0x80];               // +0x280: wait cursor image
    unsigned char field_0x300[0x304 - 0x300];
    int field_0x304;
    GUIInputDevice* mouseDevice;              // +0x308: mouse
    GUIInputDevice* keyboardDevice;           // +0x30c: keyboard
    GUIInputDevice* joystickDevices[8];       // +0x310: joysticks
    GUIUser* users[4];                        // +0x330
    int userCount;                            // +0x340: users
    GameObject* dialogContainer;              // +0x344: the UIDlgContainer
    void* languageModule;                     // +0x348: "uilang.dll" module
    int field_0x34c;
    char toolTipFontFace[0x80];               // +0x350: tool tip font face
    int field_0x3d0;
    int toolTipBold;                          // +0x3d4: bold
    int toolTipItalic;                        // +0x3d8: italic
    UnknownGuiClipper* windowClipper;         // +0x3dc
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
