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
    float eyeX;                               // +0x170 eye position
    float eyeY;                               // +0x174
    float eyeZ;                               // +0x178
};

struct UnknownGroundFogOwner {
    unsigned char field_0x00[0x08];
    UnknownGroundFogCamera* currentCamera;    // +0x08
};

struct UnknownGroundFog {
    unsigned char field_0x00[0x18];
    UnknownGroundFogOwner* renderContext;     // +0x18 GameObject+0x18
    unsigned char field_0x1c[0x30 - 0x1c];
    float planeHeight;                        // +0x30: fog plane height (compared with each vertex's y)
    float depthLimit;                         // +0x34
    int alphaLimit;                           // +0x38
    unsigned char field_0x3c[0x40 - 0x3c];
    float density;                            // +0x40
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
    void FogVertices(UnknownFogVertex* vertices, int count);

    UnknownGroundFog* groundFog;              // +0x00: FindByClassName(1, "GroundFog"), or 1 when none
};

// Cursor animation passed to GUICursor 0x00485150 (GUIUser+0x1c8). Its
// AdvancePastSounds is UIAnim's (0x004730b0) and dialogs cast a control's
// "MouseAnim" UIAnim to it, so the members follow GameUi.h's UIAnim.
struct UnknownCursorAnimation {
    int AdvancePastSounds();              // 0x004730b0: current frame
    unsigned char field_0x00[0x08];
    int currentFrame;                         // +0x08 UIAnim::currentFrame
    int playForwards;                         // +0x0c UIAnim::playForwards
    int frameCount;                           // +0x10 UIAnim::frameCount
    unsigned char field_0x14[0x20 - 0x14];
    int lastStepTime;                         // +0x20 UIAnim::lastStepTime
};

// RTTI: GUICursor : GameCursor (vtable 0x00553ed0; 0x60 bytes, the size
// 0x00487dd0 allocates).
class GUICursor : public GameCursor {
public:
    explicit GUICursor(int flags);            // 0x00485100
    virtual ~GUICursor();                     // 0x00485140 (deleting wrapper 0x00485120)
    virtual int RenderPost3D();               // 0x00485170
    void SetAnimation(UnknownCursorAnimation* animation); // 0x00485150

    UnknownCursorAnimation* cursorAnimation;  // +0x5c: SetAnimation; advanced by slot 15
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
    RECT caretRect;                           // +0x214  UIEditBox +0x214 caretRect (GameUi.h); EnableImeInput places the IME composition window at its top left
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
    virtual int Tick(float frameTime);                 // 0x00486ab0
    virtual int RenderPost3D();               // 0x00486db0

    ToolTip* Create(void* target, GUIManager* gui); // 0x00486a10
    void ShowControlToolTip(UnknownGuiControl* control); // 0x00486b10: control's text at the pointer or below it
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
    virtual int Tick(float frameTime);                 // 0x00487320
    virtual int AttachRenderContext(void* context);      // 0x004873b0

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

#include "GUIManagerCore.h"

#include "HiResMeter.h"
