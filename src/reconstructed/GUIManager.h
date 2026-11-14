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
struct GuiDirectDrawClipper {
    virtual long __stdcall QueryInterface();
    virtual long __stdcall AddRef();
    virtual long __stdcall Release();                          // Release
    virtual long __stdcall GetClipList();
    virtual long __stdcall GetHWnd();
    virtual long __stdcall Initialize();
    virtual long __stdcall IsClipListChanged();
    virtual long __stdcall SetClipList(void* clipList, unsigned long flags); // SetClipList
    virtual long __stdcall SetHWnd(unsigned long flags, void* window);   // SetHWnd
};

// The two methods 0x004868b0 calls on Display+0x190 and Display+0x19c
// (RenderInterfaces.h keeps their slots argument-less).
struct GuiDirectDraw {
    virtual long __stdcall QueryInterface();
    virtual long __stdcall AddRef();
    virtual long __stdcall Release();
    virtual long __stdcall Compact();
    virtual long __stdcall CreateClipper(unsigned long flags, GuiDirectDrawClipper** clipper,
                                          void* outer);              // CreateClipper
};

struct GuiDirectDrawSurface {
    virtual long __stdcall QueryInterface();
    virtual long __stdcall AddRef();
    virtual long __stdcall Release();
    virtual long __stdcall AddAttachedSurface();
    virtual long __stdcall AddOverlayDirtyRect();
    virtual long __stdcall Blt();
    virtual long __stdcall BltBatch();
    virtual long __stdcall BltFast();
    virtual long __stdcall DeleteAttachedSurface();
    virtual long __stdcall EnumAttachedSurfaces();
    virtual long __stdcall EnumOverlayZOrders();
    virtual long __stdcall Flip();
    virtual long __stdcall GetAttachedSurface();
    virtual long __stdcall GetBltStatus();
    virtual long __stdcall GetCaps();
    virtual long __stdcall GetClipper();
    virtual long __stdcall GetColorKey();
    virtual long __stdcall GetDC();
    virtual long __stdcall GetFlipStatus();
    virtual long __stdcall GetOverlayPosition();
    virtual long __stdcall GetPalette();
    virtual long __stdcall GetPixelFormat();
    virtual long __stdcall GetSurfaceDesc();
    virtual long __stdcall Initialize();
    virtual long __stdcall IsLost();
    virtual long __stdcall Lock();
    virtual long __stdcall ReleaseDC();
    virtual long __stdcall Restore();
    virtual long __stdcall SetClipper(GuiDirectDrawClipper* clipper); // SetClipper
};

// The fog object 0x00484f10 finds by name ("GroundFog") under Game+0x34.
// Only the members it reads are declared; its class is not established.
struct GroundFogCameraView {
    unsigned char field_0x000[0x170];
    float eyeX;                               // +0x170 eye position
    float eyeY;                               // +0x174
    float eyeZ;                               // +0x178
};

struct GroundFogRenderTargetView {
    unsigned char field_0x00[0x08];
    GroundFogCameraView* currentCamera;    // +0x08
};

struct GroundFogView {
    unsigned char field_0x00[0x18];
    GroundFogRenderTargetView* renderContext;     // +0x18 GameObject+0x18
    unsigned char field_0x1c[0x30 - 0x1c];
    float planeHeight;                        // +0x30: fog plane height (compared with each vertex's y)
    float depthLimit;                         // +0x34
    int alphaLimit;                           // +0x38
    unsigned char field_0x3c[0x40 - 0x3c];
    float density;                            // +0x40
};

// A 0x20-byte lit vertex (D3DLVERTEX layout): position, colour, specular.
struct GroundFogVertex {
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
struct GroundFogCacheOwner {
    // 0x00484f10: fogs `count` vertices by their depth below the fog plane.
    void FogVertices(GroundFogVertex* vertices, int count);

    GroundFogView* groundFog;              // +0x00: FindByClassName(1, "GroundFog"), or 1 when none
};

// Cursor animation passed to GUICursor 0x00485150 (GUIUser+0x1c8). Its
// AdvancePastSounds is UIAnim's (0x004730b0) and dialogs cast a control's
// "MouseAnim" UIAnim to it, so the members follow GameUi.h's UIAnim.
struct CursorUIAnimView {
    int AdvancePastSounds();              // 0x004730b0: current frame
    unsigned char field_0x00[0x08];
    int currentFrame;                         // +0x08 UIAnim::currentFrame
    int playForwards;                         // +0x0c UIAnim::playForwards
    int frameCount;                           // +0x10 UIAnim::frameCount
    int passesLeft;                           // +0x14 UIAnim::passesLeft
    int passCountdown;                        // +0x18 UIAnim::passCountdown
    int frameDelay;                           // +0x1c UIAnim::frameDelay
    int lastStepTime;                         // +0x20 UIAnim::lastStepTime
};

// RTTI: GUICursor : GameCursor (vtable 0x00553ed0; 0x60 bytes, the size
// 0x00487dd0 allocates).
class GUICursor : public GameCursor {
public:
    explicit GUICursor(int flags);            // 0x00485100
    virtual ~GUICursor();                     // 0x00485140 (deleting wrapper 0x00485120)
    virtual int RenderPost3D();               // 0x00485170
    void SetAnimation(CursorUIAnimView* animation); // 0x00485150

    CursorUIAnimView* cursorAnimation;  // +0x5c: SetAnimation; advanced by slot 15
};

// A dialog control as GUIUser and ToolTip see it. Never constructed as such.
class GuiControlView : public GameObject {
public:
    virtual void AdvanceImages();
    virtual void ContainsPoint();
    virtual void SetState();
    virtual void IsInteractive();
    virtual int EnterHover();       // takes the focus; 0 refuses it
    virtual void LeaveHover(GuiControlView* next);
    virtual void KillFocus(GuiControlView* next);

    unsigned char screenArea[0x3c - 0x2c];  // +0x2c UIControl screenArea (bytes kept)
    CameraRect area;                          // +0x3c: area
    unsigned char visibleArea[0x5c - 0x4c];  // +0x4c UIControl visibleArea (bytes kept)
    int controlType;                          // +0x5c: control type (11: edit field)
    unsigned char field_0x60[0xb8 - 0x60];
    UIDialog* ownerDialog;            // +0xb8
    GUIManager* ownerGui;                     // +0xbc
    char* controlText;                        // +0xc0 UIControl controlText ("Text")
    unsigned int textColor;                   // +0xc4 UIControl textColor
    int dropColor;                            // +0xc8 UIControl dropColor
    unsigned int currentTextColor;            // +0xcc UIControl currentTextColor
    int controlTextLength;                    // +0xd0 UIControl controlTextLength
    int textDrop;                             // +0xd4 UIControl textDrop
    int textAlign;                            // +0xd8 UIControl textAlign
    int textExtent[2];                        // +0xdc UIControl textExtent
    char* textLines;                          // +0xe4 UIControl textLines
    int textLeftMargin;                       // +0xe8 UIControl textLeftMargin
    int textTopMargin;                        // +0xec UIControl textTopMargin
    char* toolTipText;                        // +0xf0: tool tip text
    char controlName[0x32];                   // +0xf4 UIControl controlName (the .dtm section name)
    unsigned char field_0x126[0x128 - 0x126];
    int fontHandle;                           // +0x128 UIControl fontHandle
    int ownsFont;                             // +0x12c UIControl ownsFont
    char fontFace[0x158 - 0x130];             // +0x130
    int bold;                                 // +0x158: bold
    unsigned char italic[0x160 - 0x15c];     // +0x15c: UIControl::italic (bytes kept)
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
    void ShowControlToolTip(GuiControlView* control); // 0x00486b10: control's text at the pointer or below it
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

    ControlBinding bindingX;           // +0x2c: x
    ControlBinding bindingY;           // +0x68: y
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
