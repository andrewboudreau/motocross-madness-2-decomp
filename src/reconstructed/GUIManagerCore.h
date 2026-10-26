#pragma once

#include "GameObject.h"
#include "ContainerList.h"
#include "UIDialog.h"

class GUIManager;
class GUIInputDevice;
class GUICursor;
class ToolTip;
class UnknownGuiControl;
class InputDevice;
class Palette8;
class BackgroundImage;
class TextureMapManager;
class PCTextureMap;
class UnknownCursorAnimation;
struct UnknownGuiClipper;

// RTTI: GUIUser : GameObject (vtable 0x00554100; 0x1f4 bytes). One of
// GUIManager's four users: a cursor, a tool tip and the input devices it accepts.
class GUIUser : public GameObject {
public:
    GUIUser();                                // 0x004873d0
    virtual ~GUIUser();                       // 0x00487540 (deleting wrapper 0x00487520)
    virtual int Tick(float frameTime);                 // 0x004875a0
    virtual int RenderPost3D();               // 0x00487620

    void UpdateFocus(int force);              // 0x004875c0: applies the pending focus (+0x1d0) or hover (+0x1cc)
    GUIUser* Create(void* target, GUIManager* gui); // 0x00487650: base create, GUI and tool tip
    void CreateToolTip(void* target, GUIManager* gui); // 0x00487680: new tool tip
    void ReleaseToolTip();                    // 0x00487710: releases the tool tip
    // 0x00487730 / 0x00487790: make `control` the pending hover (focus also
    // sets hover), unless the pending one is a drop-down list ("UIDDL");
    // `update` applies it at once through 0x004875c0.
    int RequestHover(UnknownGuiControl* control, UnknownGuiControl** previous, int update);
    int RequestFocus(UnknownGuiControl* control, UnknownGuiControl** previous, int update);
    int SetHoverControl(UnknownGuiControl* control, UnknownGuiControl** previous); // 0x00487800: +0x1d8
    int MoveFocus(UnknownGuiControl* control, UnknownGuiControl** previous); // 0x00487870: moves the focus
    void EnableImeInput(int enable);   // 0x00487990: input method editor on/off
    int ClearFocus(UnknownGuiControl** previous); // 0x00487bf0: drops the focus, returning it
    int AcceptDevice(GUIInputDevice* device); // 0x00487c30: accepts a device
    void AcceptKeyboardAndJoysticks();             // 0x00487d00: accepts the keyboard and joysticks
    void ForgetDevices();             // 0x00487d60: forgets every device
    void CreateCursor(const char* image, int visible); // 0x00487dd0: creates the cursor
    void SetCursorAnimation(UnknownCursorAnimation* animation); // 0x00487fb0: default when null
    void RecreateCursor(const char* image, int redraw); // 0x00488010: recreates the cursor
    void ReleaseCursor();             // 0x004880c0: releases the cursor
    int SetPointerDevice(GUIInputDevice* device); // 0x00488120: sets the pointer device
    int UsesDevice(InputDevice* device);      // 0x00488160: pointer or accepted device (1 without a pointer)
    int UsesEventDevice(UnknownControlEvent* event); // 0x004881d0: UsesDevice for the event's device
    GUIInputDevice* FindDevice(InputDevice* device); // 0x00488240: its GUIInputDevice
    GUIInputDevice* FindEventDevice(UnknownControlEvent* event); // 0x004882a0: FindDevice for the event's device
    GUIInputDevice* GetAcceptedDevice(int index);

    GUIInputDevice* pointerDevice;            // +0x2c: pointer device
    GUICursor* userCursor;                    // +0x30
    int pointerX;                             // +0x34: pointer position, copied from the pointer device (0x00487620)
    int pointerY;                             // +0x38
    int controlDown[32];                      // +0x3c: UIDialog slot 23 sets a control's flag, slot 22 clears it
    GUIManager* userGui;                      // +0xbc
    ToolTip* userToolTip;                     // +0xc0
    int field_0xc4;
    char cursorImage[0x80];                   // +0xc8: cursor image ("cursor.tga")
    char waitImage[0x80];                     // +0x148: wait image ("wait.tga")
    UnknownCursorAnimation* cursorAnimation;  // +0x1c8
    UnknownGuiControl* pendingHoverControl;   // +0x1cc: set by 0x00487730/0x00487790; UpdateFocus passes it to SetHoverControl
    UnknownGuiControl* pendingFocusControl;   // +0x1d0: set by 0x00487790; UpdateFocus passes it to MoveFocus
    UnknownGuiControl* focusControl;          // +0x1d4: focus
    UnknownGuiControl* hoverControl;          // +0x1d8: set by SetHoverControl and MoveFocus
    int field_0x1dc;
    ContainerList<GUIInputDevice*> acceptedDevices; // +0x1e0: accepted devices
};

// RTTI: GUIManager : GameObject (vtable 0x00553f40; 0x3e0 bytes). KrustyUI
// keeps one at +0x2c. The compatibility names in KrustyUI.h alias this type.
class GUIManager : public GameObject {
public:
    explicit GUIManager(int flags);           // 0x00485190
    virtual ~GUIManager();                    // 0x00485320 (deleting wrapper 0x00485300)
    virtual int Tick(float frameTime);                 // 0x00485870
    virtual int RenderPre3D();                // 0x004858d0
    virtual int RenderPost3D();               // 0x00485970
    virtual int OnControlRelease(UnknownControlEvent* event, UnknownInputEntry* entry);     // 0x00485830
    virtual int OnControlPress(UnknownControlEvent* event, UnknownInputEntry* entry);       // 0x004857f0

    // 0x004853b0: sets the GUI up (KrustyUI 0x004988a0 passes its target,
    // Display+0x64 palette, the textures, 0, 0, 0, "Arial", 15, "ui\\cursor.tga"
    // and a callback).
    GUIManager* SetUp(void* target, Palette8* palette, TextureMapManager* textures,
                      BackgroundImage* background, int startSound, SoundGroup* sound,
                      const char* font, short fontSize, const char* cursor,
                      int callback);
    // 0x00485a70: shows `dialog`.
    UIDialog* ShowDialog(UIDialog* dialog, int menu, int flags, int area,
                         UIDialog* parent, int initParam, int name, int wait);
    // 0x00485bd0: adds a shown dialog to its container.
    void AttachDialog(UIDialog* dialog, int initParam, int wait);
    int OpenDialogResource(const char* resource); // 0x00485c80: opens a dialog resource
    void CloseDialogResource();             // 0x00485d50: closes it
    void SetDialogDirectory(const char* directory); // 0x00485d70
    UIDialog* FindInputDialog();      // 0x00485df0: the dialog taking input
    Sound* FindSectionObject(const char* name); // 0x00485ec0: the opened dialog's section object
    int SwapModalDialog(int value);     // 0x00485ee0: sets +0x2c, returns the old value
    void CreateBackground();                  // 0x00485ef0: creates the background
    void ReleaseBackground();             // 0x00485fc0: releases it
    void GrabBackground(int dim);      // 0x004860a0: grabs the screen as the background
    void ReleaseBackgroundGrab();             // 0x004860f0: releases the grab
    void SetGuiPalette(Palette8* palette); // 0x00486150
    // 0x00486170: copies the screen (or `rect`) into a new texture, halving
    // its brightness when `dim`.
    PCTextureMap* CopyScreenToTexture(int dim, CameraRect* rect);
    int UnknownFunction4864f0();              // 0x004864f0
    void RedrawFrame();             // 0x00486500: redraws a frame
    GUIUser* GetUser(int index);              // 0x00486540: user `index` (0 past 3)
    void SetWaitCursorImage(const char* image); // 0x00486560
    void CreateCursors(const char* image, int visible); // 0x00486590
    void RecreateCursors(const char* image, int redraw); // 0x004865e0
    void ShowCursors(int show);               // 0x00486630: each user's cursor slot 5 (show) or 4
    void ReleaseCursors();             // 0x00486680: releases the cursors
    void SetToolTipFont(const char* font); // 0x004866c0
    // 0x00486740: a `width` x `height` texture filled with `color`.
    PCTextureMap* CreateFilledTexture(int width, int height, unsigned long color);
    // 0x004868b0: creates (CreateClipper) and attaches (SetClipper, SetHWnd
    // with the game window) or detaches the primary surface's clipper.
    int EnableWindowClipper(int enable);
    void UnknownFunction464e90();             // 0x00464e90 (shared empty body)

    int modalDialog;                          // +0x2c the newest modal UIDialog (SwapModalDialog)
    UIDialog* openedDialog;           // +0x30: dialog opened by 0x00485c80
    Palette8* guiPalette;                     // +0x34
    Palette8* setupPalette;                   // +0x38: SetUp's palette
    BackgroundImage* guiBackground;           // +0x3c
    BackgroundImage* redrawBackground;        // +0x40: redrawn when +0x1f4 runs out
    SoundGroup* guiSoundGroup;                // +0x44
    int ownsBackground;                       // +0x48: owns +0x3c
    int ownsSoundGroup;                       // +0x4c: owns +0x44
    char dialogFontName[0x80];                // +0x50: dialog font
    short dialogFontSize;                     // +0xd0: dialog font size
    int setupCallback;                        // +0xd4: SetUp's last argument
    TextureMapManager* guiTextures;           // +0xd8
    PCTextureMap* screenGrab;                 // +0xdc: screen grab
    int drawGrabFirst;                        // +0xe0: draw the grab before the children
    int grabRegion;                           // +0xe4
    int grabCopyFrames;                       // +0xe8: the screen grab's full-copy frames left (BackgroundImage::DrawImage `frames`)
    int uiAnimations;                         // +0xec ChkUIAnims (OptionProcs.cpp); from TrackGame+0xc40 (KrustyUI 0x004988a0); handed to each dialog's animatesControls
    char dialogDirectory[0x100];              // +0xf0: dialog directory
    int paletteChanged;                       // +0x1f0: set by 0x00486150; slot 10 clears it
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
    int soundVolume;                          // +0x34c the UI sounds' volume: (uiVolume - 100) * 25 - 200, or -10200 with UI sounds off (OptionProcs.cpp)
    char toolTipFontFace[0x80];               // +0x350: tool tip font face
    int fontSizeOffset;                       // +0x3d0 a float added to the dialog and control font heights (0x0046a920); string 0xfed (KrustyUI)
    int toolTipBold;                          // +0x3d4: bold
    int toolTipItalic;                        // +0x3d8: italic
    UnknownGuiClipper* windowClipper;         // +0x3dc
};
