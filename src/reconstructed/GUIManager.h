#pragma once

#include <windows.h>

#include "ContainerList.h"
#include "ControlInterface.h"
#include "GameCursor.h"
#include "GameObject.h"
#include "GameObjectIterator.h"
#include "GroundFogableObject.h"
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

// The fog object 0x00484f10 finds under Game+0x34 by its class name
// "GroundFog" (FindByClassName matches the class list at +0x28; there is
// no RTTI for it). A GameObject (+0x18 is the render target); only the
// members FogVertices reads are declared.
struct GroundFog {
    unsigned char field_0x00[0x18];
    RenderTarget* renderContext;              // +0x18 GameObject+0x18
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

// RTTI: GUICursor : GameCursor (vtable 0x00553ed0; 0x60 bytes, the size
// 0x00487dd0 allocates).
class GUICursor : public GameCursor {
public:
    explicit GUICursor(int flags);            // 0x00485100
    virtual ~GUICursor();                     // 0x00485140 (deleting wrapper 0x00485120)
    virtual int RenderPost3D();               // 0x00485170
    void SetAnimation(UIAnim* animation); // 0x00485150

    UIAnim* cursorAnimation;  // +0x5c: SetAnimation; advanced by slot 15
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
    void ShowControlToolTip(UIControl* control); // 0x00486b10: control's text at the pointer or below it
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
