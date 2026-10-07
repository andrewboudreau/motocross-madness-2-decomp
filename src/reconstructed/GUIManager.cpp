// GUIManager.cpp -- reconstruction of D:\aardvark\VC\krusty2\GUIManager.cpp.
// See GUIManager.h for the TU extent and the classes.

#include <windows.h>
#include <imm.h>
#include <stdlib.h>
#include <string.h>

#include "GUIManager.h"

#include "BackgroundImage.h"
#include "DebugAlloc.h"
#include "DebugOverlay.h"
#include "JoystickDevice.h"
#include "MatrixUtil.h"
#include "MouseDevice.h"
#include "PCAudio.h"
#include "PCTextureMap.h"
#include "Palette8.h"
#include "Tgafile.h"
#include "TrackGame.h"

// The four vector constants that open many retail files: 0x0067b438,
// 0x0067b448, 0x0067b458 and 0x0067b428, built by 0x00484dd0..0x00484f0b.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// Milliseconds of the previous game tick (0x0065b534; HiResMeter prints it).
extern int g_UnknownGlobal65b534;

// cdecl 0x00460b50: table-driven square root (FollowCamera.h).
float UnknownFunction460b50(float value);

#define SoundSystem() ((PCSoundInterface*)g_UnknownGlobal56e26c->field_0x04)

// 0x00484f10
void UnknownGroundFogShader::UnknownFunction484f10(UnknownFogVertex* vertices, int count) {
    UnknownGroundFog* fog;
    UnknownGroundFogCamera* camera;
    float eyeX;
    float eyeY;
    float eyeZ;
    float density;
    float depth;
    float dx;
    float dy;
    float dz;
    float distance;
    float scale;
    int depthLimit;
    int alphaLimit;
    int steps;
    int alpha;
    int i;

    if (field_0x00 == (UnknownGroundFog*)1)
        return;
    if (!field_0x00) {
        field_0x00 = (UnknownGroundFog*)g_UnknownGlobal56e26c->field_0x34->UnknownFunction469770(1, "GroundFog");
        if (!field_0x00) {
            field_0x00 = (UnknownGroundFog*)1;
            return;
        }
    }
    fog = field_0x00;
    camera = fog->field_0x18->field_0x08;
    alphaLimit = fog->field_0x38;
    eyeX = camera->field_0x170;
    eyeY = camera->field_0x174;
    eyeZ = camera->field_0x178;
    depthLimit = (int)fog->field_0x34;
    density = fog->field_0x40;
    for (i = 0; i < count; i++) {
        if (vertices[i].y >= field_0x00->field_0x30 && field_0x00->field_0x30 <= eyeY) {
            vertices[i].specular = 0xff000000;
            continue;
        }
        if (field_0x00->field_0x30 > eyeY && vertices[i].y > eyeY)
            depth = field_0x00->field_0x30 - eyeY;
        else
            depth = field_0x00->field_0x30 - vertices[i].y;
        dx = eyeX - vertices[i].x;
        dy = eyeY - vertices[i].y;
        dz = eyeZ - vertices[i].z;
        if (dy < 0.0f)
            dy = -dy;
        distance = UnknownFunction460b50(dx * dx + dy * dy + dz * dz);
        steps = (int)depth;
        if (steps > depthLimit)
            steps = depthLimit;
        if (dy != 0.0f) {
            scale = distance * depth / dy;
            if (scale > distance)
                scale = distance;
            alpha = (int)(steps * scale * density);
            if (alpha > alphaLimit)
                alpha = alphaLimit;
        } else {
            alpha = alphaLimit;
        }
        vertices[i].specular = (255 << 24) | (alpha << 16) | (alpha << 8) | alpha;
        vertices[i].color = (255 << 24) | ((255 - alpha) << 16) | ((255 - alpha) << 8) | (255 - alpha);
    }
}

// 0x00485100
GUICursor::GUICursor(int flags) : GameCursor(flags) {
    field_0x5c = 0;
}

// 0x00485140
GUICursor::~GUICursor() {
}

// 0x00485150
void GUICursor::UnknownFunction485150(UnknownCursorAnimation* animation) {
    field_0x5c = animation;
    if (!animation)
        field_0x38 = 0;
}

// 0x00485170
int GUICursor::UnknownVirtualSlot15() {
    if (field_0x5c)
        field_0x38 = field_0x5c->AdvancePastSounds();
    return GameCursor::UnknownVirtualSlot15();
}

// 0x00485190
GUIManager::GUIManager(int flags) : GameObject(flags) {
    int i;

    openedDialog = 0;
    ownsSoundGroup = 0;
    field_0xd4 = 0;
    guiTextures = 0;
    ownsBackground = 0;
    screenGrab = 0;
    grabRegion = -1;
    field_0xe8 = 0;
    field_0xec = 1;
    dialogDirectory[0] = 0;
    waitFrames = 0;
    field_0x38 = 0;
    guiPalette = 0;
    guiSoundGroup = 0;
    guiBackground = 0;
    toolTipFont = 0;
    languageModule = 0;
    windowClipper = 0;
    field_0x1f0 = 0;
    field_0x1f8 = 0;
    redrawBackground = 0;
    field_0x304 = 0;
    cursorImage[0] = 0;
    waitCursorImage[0] = 0;
    strcpy(waitCursorImage, "wait.tga");
    strcpy(cursorImage, "cursor.tga");
    mouseDevice = 0;
    keyboardDevice = 0;
    for (i = 0; i < 8; i++)
        joystickDevices[i] = 0;
    userCount = 0;
    for (i = 0; i < 4; i++)
        users[i] = 0;
    dialogContainer = 0;
    field_0x2c = 0;
    field_0x34c = 0;
    strcpy(toolTipFontFace, "");
    field_0x3d0 = 0;
    toolTipBold = 0;
    toolTipItalic = 0;
    drawGrabFirst = 0;
}

// 0x00485320
GUIManager::~GUIManager() {
    ReleaseCursors();
    if (openedDialog)
        openedDialog->Release();
    ReleaseBackgroundGrab();
    if (toolTipFont)
        DeleteObject((HGDIOBJ)toolTipFont);
    if (windowClipper)
        windowClipper->UnknownMethod2();
}

// 0x004857f0
int GUIManager::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    int previous = g_MemTagStack->Push("UI");
    int result = GameObject::UnknownVirtualSlot23(event, entry);
    g_MemTagStack->Pop(previous);
    return result;
}

// 0x00485830
int GUIManager::UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry) {
    int previous = g_MemTagStack->Push("UI");
    int result = GameObject::UnknownVirtualSlot22(event, entry);
    g_MemTagStack->Pop(previous);
    return result;
}

// 0x00485870
int GUIManager::UnknownVirtualSlot10(float frameTime) {
    int previous = g_MemTagStack->Push("UI");
    GameObject::UnknownVirtualSlot10(frameTime);
    if (field_0x1f0) {
        field_0x1f0 = 0;
        UnknownFunction464e90();
    }
    g_MemTagStack->Pop(previous);
    return 1;
}

// 0x004858d0
int GUIManager::UnknownVirtualSlot13() {
    CameraRect rect;

    if (waitFrames)
        return 1;
    if (screenGrab && drawGrabFirst) {
        rect.left = 0;
        rect.top = 0;
        rect.right = screenGrab->field_0x14;
        rect.bottom = screenGrab->field_0x18;
        if (guiBackground) {
            guiBackground->UnknownFunction404480(screenGrab, &rect, 0, 0x1000000, grabRegion, 1, &field_0xe8, 0);
        } else if (!((RenderTarget*)field_0x18)->UnknownVirtualSlot3(&rect, screenGrab, 0, 0x1000000)) {
            GameObject::UnknownVirtualSlot13();
            return 0;
        }
    }
    GameObject::UnknownVirtualSlot13();
    return 1;
}

// 0x00485970
int GUIManager::UnknownVirtualSlot15() {
    CameraRect rect;

    if (!waitFrames) {
        if (screenGrab && !drawGrabFirst) {
            rect.left = 0;
            rect.top = 0;
            rect.right = screenGrab->field_0x14;
            rect.bottom = screenGrab->field_0x18;
            if (guiBackground) {
                guiBackground->UnknownFunction404480(screenGrab, &rect, 0, 0x1000000, grabRegion, 1, &field_0xe8, 0);
            } else if (!((RenderTarget*)field_0x18)->UnknownVirtualSlot3(&rect, screenGrab, 0, 0x1000000)) {
                GameObject::UnknownVirtualSlot15();
                return 0;
            }
        }
        GameObject::UnknownVirtualSlot15();
    } else {
        waitFrames = waitFrames - 1 < 0 ? 0 : waitFrames - 1;
        g_UnknownGlobal56e26c->field_0x10->UnknownVirtualSlot12(0, 0);
        if (!waitFrames && redrawBackground) {
            redrawBackground->UnknownFunction404da0();
            redrawBackground = 0;
        }
    }
    if (guiBackground)
        guiBackground->UnknownFunction404c80();
    return 1;
}

// 0x00485bd0
void GUIManager::UnknownFunction485bd0(UnknownGuiDialog* dialog, int a, int wait) {
    int visible = dialog->field_0x25_bit0;

    dialog->UnknownFunction46ea60(0);
    if (dialog->parentDialog && (dialog->field_0x144 & 1))
        dialog->parentDialog->controlContainer->UnknownFunction469130(dialog, -1);
    else
        dialogContainer->UnknownFunction469190(dialog, -1);
    dialog->UnknownVirtualSlot28(a);
    dialog->field_0xc8 = field_0xec;
    if (guiBackground)
        guiBackground->UnknownFunction404da0();
    if (wait)
        UnknownFunction4865e0(cursorImage, field_0x1f0 == 0);
    if (field_0x1f0)
        waitFrames = 3;
    dialog->UnknownFunction46ea60(visible);
}

// 0x00485d50
void GUIManager::CloseDialogResource() {
    if (openedDialog) {
        openedDialog->Release();
        openedDialog = 0;
    }
}

// 0x00485d70
void GUIManager::UnknownFunction485d70(const char* directory) {
    int last = strlen(directory) - 1;

    strcpy(dialogDirectory, directory);
    if (directory[last] != '\\')
        strcat(dialogDirectory, "\\");
}

// 0x00485df0
UnknownGuiDialog* GUIManager::UnknownFunction485df0() {
    UnknownGuiDialog* found = 0;
    UnknownGuiDialog* dialog;
    GameObjectIterator iterator(this, 1, "UIDialog");

    while ((dialog = (UnknownGuiDialog*)iterator.Next()) != 0) {
        if (!found)
            found = dialog;
        if (mouseDevice && PtInRect(&dialog->screenArea, mouseDevice->pointerPosition))
            found = dialog;
        if (dialog->field_0x148)
            found = dialog;
    }
    return found;
}

// 0x00485ec0
int GUIManager::FindSectionObject(int value) {
    if (openedDialog)
        return openedDialog->FindSectionObject(value);
    return 0;
}

// 0x00485ee0
int GUIManager::UnknownFunction485ee0(int value) {
    int previous = field_0x2c;
    field_0x2c = value;
    return previous;
}

// 0x00485ef0
void GUIManager::UnknownFunction485ef0() {
    int i;

    if (guiBackground)
        return;
    guiBackground = (BackgroundImage*)UnknownFunction469190(
        (new(__FILE__, 672) BackgroundImage(1))->UnknownVirtualSlot8(field_0x18), -1);
    guiBackground->UnknownFunction469260(dialogContainer, -1);
    ownsBackground = 1;
    UnknownFunction4865e0(0, 1);
    for (i = 0; i < userCount; i++)
        UnknownFunction486540(i)->CreateToolTip(field_0x18, this);
}

// 0x00485fc0
void GUIManager::ReleaseBackground() {
    UnknownGuiDialog* dialog;
    int i;

    if (!ownsBackground)
        return;
    ReleaseCursors();
    if (guiBackground)
        guiBackground->Release();
    guiBackground = 0;
    UnknownFunction4865e0(0, 0);
    GameObjectIterator iterator(this, 1, "UIDialog");
    while ((dialog = (UnknownGuiDialog*)iterator.Next()) != 0)
        dialog->UnknownFunction46ffd0(0);
    for (i = 0; i < userCount; i++)
        UnknownFunction486540(i)->CreateToolTip(field_0x18, this);
}

// 0x004860a0
void GUIManager::GrabBackground(int dim) {
    ReleaseBackgroundGrab();
    screenGrab = CopyScreenToTexture(dim, 0);
    if (guiBackground) {
        grabRegion = guiBackground->UnknownFunction4040f0(1);
        guiBackground->UnknownFunction404da0();
        guiBackground->field_0x30 = 0;
    }
}

// 0x004860f0
void GUIManager::ReleaseBackgroundGrab() {
    if (screenGrab)
        screenGrab->Release();
    if (guiBackground && grabRegion >= 0) {
        guiBackground->UnknownFunction404200(grabRegion);
        guiBackground->field_0x30 = 1;
    }
    screenGrab = 0;
    grabRegion = -1;
    field_0xe8 = 0;
}

// 0x00486150
void GUIManager::UnknownFunction486150(Palette8* palette) {
    field_0x1f0 = 1;
    guiPalette = palette;
}

// 0x004864f0
int GUIManager::UnknownFunction4864f0() {
    return field_0x1f8;
}

// 0x00486500
void GUIManager::RedrawFrame() {
    if (guiBackground) {
        guiBackground->UnknownFunction404c80();
        guiBackground->UnknownFunction404da0();
    }
    g_UnknownGlobal56e26c->UnknownVirtualSlot8();
    g_UnknownGlobal56e26c->UnknownVirtualSlot9();
    g_UnknownGlobal56e26c->field_0x10->UnknownFunction4e8cc0();
}

// 0x00486540
GUIUser* GUIManager::UnknownFunction486540(int index) {
    if (index < 4)
        return users[index];
    return 0;
}

// 0x00486560
void GUIManager::UnknownFunction486560(const char* image) {
    strcpy(waitCursorImage, image);
}

// 0x00486590
void GUIManager::UnknownFunction486590(const char* image, int visible) {
    int i;

    for (i = 0; i < userCount; i++) {
        if (users[i])
            users[i]->CreateCursor(image ? image : cursorImage, visible);
    }
}

// 0x004865e0
void GUIManager::UnknownFunction4865e0(const char* image, int redraw) {
    int i;

    for (i = 0; i < userCount; i++) {
        if (users[i])
            users[i]->UnknownFunction488010(image ? image : cursorImage, redraw);
    }
}

// 0x00486630
void GUIManager::UnknownFunction486630(int show) {
    int count = userCount;
    GUIUser* user;
    int i;

    for (i = 0; i < count; i++) {
        user = UnknownFunction486540(i);
        if (user && user->userCursor) {
            if (show)
                user->userCursor->UnknownVirtualSlot5();
            else
                user->userCursor->UnknownVirtualSlot4();
        }
    }
}

// 0x00486680
void GUIManager::ReleaseCursors() {
    int i;

    for (i = 0; i < userCount; i++) {
        if (users[i])
            users[i]->ReleaseCursor();
    }
}

// 0x004866c0
void GUIManager::UnknownFunction4866c0(const char* font) {
    strcpy(toolTipFontFace, font);
    if (toolTipFont)
        DeleteObject((HGDIOBJ)toolTipFont);
    toolTipFont = CreateFontA(12, 0, 0, 0, 400, 0, 0, 0, 1, 0, 0, 2, 2, font);
}

// A pixel of 0x00486740's 24-bit fill.
struct UnknownGuiRgb {
    unsigned char field_0x00;
    unsigned char field_0x01;
    unsigned char field_0x02;
};

// 0x00486740
PCTextureMap* GUIManager::CreateFilledTexture(int width, int height, unsigned long color) {
    PCTextureMap* texture = new(__FILE__, 1140) PCTextureMap(guiTextures, 1);
    UnknownGuiRgb* bits;
    UnknownGuiRgb* pixel;
    UnknownGuiRgb fill;

    if (texture) {
        bits = new(__FILE__, 1144) UnknownGuiRgb[width * height];
        fill.field_0x00 = (unsigned char)(color >> 16);
        fill.field_0x01 = (unsigned char)(color >> 8);
        fill.field_0x02 = (unsigned char)color;
        for (pixel = bits; pixel < bits + width * height; pixel++)
            *pixel = fill;
        if (texture->UnknownVirtualSlot4(bits, width, height, width, width, 0x378,
                                         g_UnknownGlobal56e26c->field_0x10->field_0x28,
                                         (UnknownTexturePalette*)(guiPalette ? guiPalette->field_0x708 : 0), 4,
                                         guiPalette ? guiPalette->field_0x70c : 0, 1, 0, 2, 1, 0, 0x80, 0xff00ff))
            texture->UnknownVirtualSlot18(0xff00ff);
        delete bits;
    }
    return texture;
}

// 0x004868b0
int GUIManager::UnknownFunction4868b0(int enable) {
    UnknownDisplay* display;

    if (enable) {
        if (windowClipper) {
            windowClipper->UnknownMethod7(0, 0);
            windowClipper->UnknownMethod2();
        }
        ((UnknownGuiDirectDraw*)g_UnknownGlobal56e26c->field_0x0c->field_0x190)
            ->UnknownMethod4(0, &windowClipper, 0);
        ((UnknownGuiSurface*)g_UnknownGlobal56e26c->field_0x0c->field_0x19c)->UnknownMethod28(windowClipper);
        if (windowClipper)
            windowClipper->UnknownMethod8(0, g_UnknownGlobal56e26c->field_0x31c);
    } else {
        if (windowClipper) {
            windowClipper->UnknownMethod7(0, 0);
            windowClipper->UnknownMethod2();
            windowClipper = 0;
        }
        ((UnknownGuiSurface*)g_UnknownGlobal56e26c->field_0x0c->field_0x19c)->UnknownMethod28(0);
    }
    display = g_UnknownGlobal56e26c->field_0x0c;
    display->UnknownFunction4cb5b0(enable);
    return 1;
}

// 0x00486990
ToolTip::ToolTip(int flags) : GameObject(flags) {
    shown = 0;
    textTexture = 0;
    restoreFrames = 0;
    screenArea.left = screenArea.top = screenArea.right = screenArea.bottom = 0;
    textArea.left = textArea.top = textArea.right = textArea.bottom = 0;
    backgroundRegion = -1;
    showDelay = -1.0f;
    enabled = 1;
}

// 0x00486a10
ToolTip* ToolTip::UnknownFunction486a10(void* target, GUIManager* gui) {
    GameObject::UnknownVirtualSlot8(target);
    tipGui = gui;
    return this;
}

// 0x00486a30
ToolTip::~ToolTip() {
    if (textTexture)
        textTexture->Release();
    if (tipGui && tipGui->guiBackground && backgroundRegion > -1)
        tipGui->guiBackground->UnknownFunction404200(backgroundRegion);
}

// 0x00486ab0
int ToolTip::UnknownVirtualSlot10(float frameTime) {
    if (enabled && showDelay > 0.0f) {
        showDelay -= frameTime;
        if (showDelay > 0.0f)
            return 1;
        if (textTexture) {
            shown = 1;
            return 1;
        }
    } else if (enabled) {
        return 1;
    }
    shown = 0;
    return 1;
}

// 0x00486db0
int ToolTip::UnknownVirtualSlot15() {
    CameraRect rect;

    if (shown) {
        if (tipGui && tipGui->guiBackground) {
            tipGui->guiBackground->UnknownFunction404480(textTexture, &screenArea, &textArea, 0x1008000, backgroundRegion,
                                                         0, &restoreFrames, 0);
            return 1;
        }
        rect.left = screenArea.left;
        rect.top = screenArea.top;
        rect.right = screenArea.left + textArea.right;
        rect.bottom = screenArea.top + textArea.bottom;
        if (!((RenderTarget*)field_0x18)->UnknownVirtualSlot3(&rect, textTexture, &textArea, 0x1008000))
            return 0;
    } else if (tipGui && tipGui->guiBackground && backgroundRegion > -1) {
        if (restoreFrames > 0) {
            tipGui->guiBackground->UnknownFunction404240(backgroundRegion, &screenArea);
            restoreFrames--;
            return 1;
        }
        tipGui->guiBackground->UnknownFunction404cb0(backgroundRegion);
    }
    return 1;
}

// 0x00486e80
GUIInputDevice::GUIInputDevice() : GameObject(1) {
    rangeMinX = 0;
    rangeMaxX = 0;
    rangeMinY = 0;
    rangeMaxY = 0;
    ownerUser = 0;
}

// 0x00486f20
GUIInputDevice::~GUIInputDevice() {
    if (inputDevice->deviceKind) {
        bindingX.UnknownFunction43cde0();
        bindingY.UnknownFunction43cde0();
    }
}

// 0x00486fb0
GUIInputDevice* GUIInputDevice::Bind(void* target, InputDevice* device, float minX, float maxX,
                                     float minY, float maxY) {
    float rangeX;
    float rangeY;

    GameObject::UnknownVirtualSlot8(target);
    inputDevice = device;
    rangeMinX = minX;
    rangeMaxX = maxX;
    rangeMinY = minY;
    rangeMaxY = maxY;
    rangeX = maxX;
    rangeY = maxY;
    if (minX == 0.0f && maxX == 0.0f && minY == 0.0f && maxY == 0.0f) {
        rangeX = (float)(((RenderTarget*)field_0x18)->field_0x0c - 1);
        rangeY = (float)(((RenderTarget*)field_0x18)->field_0x10 - 1);
    }
    bindingX = UnknownControlBinding(rangeMinX, rangeX, 0.0f,
                                       g_UnknownGlobal56e26c->field_0x14->UnknownFunction43ce90(), 0, 0.0f);
    bindingY = UnknownControlBinding(rangeMinY, rangeY, 0.0f,
                                       g_UnknownGlobal56e26c->field_0x14->UnknownFunction43ce90(), 1, 0.0f);
    if (inputDevice->deviceKind == 2) {
        ((JoystickDevice*)inputDevice)->UnknownFunction489a20(&bindingX, 0.01f, 0.01f);
        ((JoystickDevice*)inputDevice)->UnknownFunction489a20(&bindingY, 0.01f, 0.01f);
    } else if (inputDevice->deviceKind == 1) {
        ((MouseDevice*)inputDevice)->UnknownFunction48a420(&bindingX);
        ((MouseDevice*)inputDevice)->UnknownFunction48a420(&bindingY);
    }
    return this;
}

// 0x00487150
void GUIInputDevice::Rebind() {
    float rangeX;
    float rangeY;
    int x = (int)bindingX.field_0x24;
    int y = (int)bindingY.field_0x24;

    if (!inputDevice)
        return;
    if (inputDevice->deviceKind == 2 || inputDevice->deviceKind == 1) {
        inputDevice->UnknownVirtualSlot0(bindingX.field_0x04);
        inputDevice->UnknownVirtualSlot0(bindingY.field_0x04);
    }
    rangeX = rangeMaxX;
    rangeY = rangeMaxY;
    if (rangeMinX == 0.0f && rangeMaxX == 0.0f && rangeMinY == 0.0f && rangeMaxY == 0.0f) {
        rangeX = (float)(((RenderTarget*)field_0x18)->field_0x0c - 1);
        rangeY = (float)(((RenderTarget*)field_0x18)->field_0x10 - 1);
    }
    bindingX = UnknownControlBinding(rangeMinX, rangeX, (float)x,
                                       g_UnknownGlobal56e26c->field_0x14->UnknownFunction43ce90(), 0, 0.0f);
    bindingY = UnknownControlBinding(rangeMinY, rangeY, (float)y,
                                       g_UnknownGlobal56e26c->field_0x14->UnknownFunction43ce90(), 1, 0.0f);
    if (inputDevice->deviceKind == 2) {
        ((JoystickDevice*)inputDevice)->UnknownFunction489a20(&bindingX, 0.01f, 0.01f);
        ((JoystickDevice*)inputDevice)->UnknownFunction489a20(&bindingY, 0.01f, 0.01f);
    } else if (inputDevice->deviceKind == 1) {
        ((MouseDevice*)inputDevice)->UnknownFunction48a420(&bindingX);
        ((MouseDevice*)inputDevice)->UnknownFunction48a420(&bindingY);
    }
}

// 0x00487320
int GUIInputDevice::UnknownVirtualSlot10(float frameTime) {
    int kind;

    GameObject::UnknownVirtualSlot10(frameTime);
    kind = inputDevice->deviceKind;
    if (kind) {
        if (g_UnknownGlobal56e26c->field_0x2d4_bit1) {
            POINT* position = &pointerPosition;
            position->x = (int)bindingX.field_0x24;
            position->y = (int)bindingY.field_0x24;
            return 1;
        }
        if (kind == 1 && ownerUser && ownerUser->userCursor) {
            pointerPosition.x = ownerUser->userCursor->field_0x54;
            pointerPosition.y = ownerUser->userCursor->field_0x58;
        }
    }
    return 1;
}

// 0x004873b0
int GUIInputDevice::UnknownVirtualSlot25(void* value) {
    GameObject::UnknownVirtualSlot25(value);
    Rebind();
    return 1;
}

// 0x004873d0
GUIUser::GUIUser() : GameObject(1) {
    int i;

    userGui = 0;
    userToolTip = 0;
    userCursor = 0;
    cursorAnimation = 0;
    cursorImage[0] = 0;
    waitImage[0] = 0;
    field_0xc4 = 0;
    strcpy(waitImage, "wait.tga");
    strcpy(cursorImage, "cursor.tga");
    focusControl = 0;
    field_0x1d8 = 0;
    field_0x1cc = 0;
    field_0x1d0 = 0;
    acceptedDevices.Init(1, 1);
    pointerDevice = 0;
    field_0x38 = 0;
    field_0x34 = 0;
    for (i = 0; i < 32; i++)
        field_0x3c[i] = 0;
}

// 0x00487540
GUIUser::~GUIUser() {
}

// 0x004875a0
int GUIUser::UnknownVirtualSlot10(float frameTime) {
    UnknownFunction4875c0(0);
    return GameObject::UnknownVirtualSlot10(frameTime);
}

// 0x004875c0
void GUIUser::UnknownFunction4875c0(int force) {
    if (field_0x1d0) {
        UnknownFunction487870(field_0x1d0, 0);
        field_0x1cc = 0;
        field_0x1d0 = 0;
        return;
    }
    if (field_0x1cc || force) {
        UnknownFunction487800(field_0x1cc, 0);
        field_0x1cc = 0;
    }
}

// 0x00487620
int GUIUser::UnknownVirtualSlot15() {
    GameObject::UnknownVirtualSlot15();
    if (pointerDevice) {
        field_0x34 = pointerDevice->pointerPosition.x;
        field_0x38 = pointerDevice->pointerPosition.y;
    }
    return 1;
}

// 0x00487650
GUIUser* GUIUser::UnknownFunction487650(void* target, GUIManager* gui) {
    GameObject::UnknownVirtualSlot8(target);
    userGui = gui;
    CreateToolTip(target, gui);
    return this;
}

// 0x00487680
void GUIUser::CreateToolTip(void* target, GUIManager* gui) {
    UnknownFunction487710();
    userToolTip = (ToolTip*)UnknownFunction469190(
        (new(__FILE__, 1639) ToolTip(1))->UnknownFunction486a10(target, gui), -1);
}

// 0x00487710
void GUIUser::UnknownFunction487710() {
    if (userToolTip)
        userToolTip->Release();
    userToolTip = 0;
}

// 0x00487730
int GUIUser::UnknownFunction487730(UnknownGuiControl* control, UnknownGuiControl** previous, int update) {
    if (field_0x1cc && strstr(field_0x1cc->field_0x28, "UIDDL"))
        return 0;
    if (previous)
        *previous = field_0x1cc;
    field_0x1cc = control;
    if (update)
        UnknownFunction4875c0(1);
    return 1;
}

// 0x00487790
int GUIUser::UnknownFunction487790(UnknownGuiControl* control, UnknownGuiControl** previous, int update) {
    if (field_0x1d0 && strstr(field_0x1d0->field_0x28, "UIDDL"))
        return 0;
    if (previous)
        *previous = field_0x1d0;
    field_0x1cc = control;
    field_0x1d0 = control;
    if (update)
        UnknownFunction4875c0(1);
    return 1;
}

// 0x00487800
int GUIUser::UnknownFunction487800(UnknownGuiControl* control, UnknownGuiControl** previous) {
    int result = 1;

    if (control && control != field_0x1d8)
        result = control->UnknownVirtualSlot31();
    if (result) {
        if (field_0x1d8 && !field_0x1d8->field_0x25_bit3 && field_0x1d8 != control)
            field_0x1d8->UnknownVirtualSlot32(control);
        if (previous)
            *previous = field_0x1d8;
        field_0x1d8 = control;
    }
    return result;
}

// 0x00487990
void GUIUser::EnableImeInput(int enable) {
    POINT position;
    RECT area;
    COMPOSITIONFORM composition;
    LOGFONTA font;
    HWND window;
    const char* face;
    UnknownGuiControl* control;

    if (enable) {
        ImmAssociateContext((HWND)g_UnknownGlobal56e26c->field_0x31c, (HIMC)g_UnknownGlobal56e26c->field_0x53c);
        area = focusControl->field_0x214;
        composition.dwStyle = CFS_FORCE_POSITION;
        composition.ptCurrentPos.x = area.left;
        composition.ptCurrentPos.y = area.top;
        ImmSetCompositionWindow((HIMC)g_UnknownGlobal56e26c->field_0x53c, &composition);
        font.lfWidth = 0;
        font.lfQuality = 2;
        font.lfPitchAndFamily = 2;
        control = focusControl;
        font.lfEscapement = 0;
        font.lfOrientation = 0;
        font.lfUnderline = 0;
        font.lfStrikeOut = 0;
        font.lfCharSet = 1;
        font.lfOutPrecision = 0;
        font.lfClipPrecision = 0;
        font.lfItalic = 0;
        if (control->fontFace[0]) {
            face = control->fontFace;
            font.lfHeight = control->fontHeight;
            font.lfWeight = control->bold ? 700 : 500;
        } else if (control->ownerDialog->dialogFontFace[0]) {
            face = control->ownerDialog->dialogFontFace;
            font.lfHeight = control->ownerDialog->dialogFontHeight;
            font.lfWeight = control->ownerDialog->field_0x108 ? 700 : 500;
        } else {
            face = control->ownerGui->dialogFontName;
            font.lfHeight = control->ownerGui->dialogFontSize;
            font.lfWeight = control->ownerGui->toolTipBold ? 700 : 500;
            font.lfItalic = (BYTE)control->ownerGui->toolTipItalic;
        }
        strcpy(font.lfFaceName, face);
        ImmSetCompositionFontA((HIMC)g_UnknownGlobal56e26c->field_0x53c, &font);
        position.x = 550;
        position.y = 450;
        ImmSetStatusWindowPos((HIMC)g_UnknownGlobal56e26c->field_0x53c, &position);
        window = ImmGetDefaultIMEWnd((HWND)g_UnknownGlobal56e26c->field_0x31c);
        if (window)
            SendMessageA(window, WM_IME_CONTROL, IMC_OPENSTATUSWINDOW, 0);
        ImmSetOpenStatus((HIMC)g_UnknownGlobal56e26c->field_0x53c, 0);
    } else {
        window = ImmGetDefaultIMEWnd((HWND)g_UnknownGlobal56e26c->field_0x31c);
        if (window)
            SendMessageA(window, WM_IME_CONTROL, IMC_CLOSESTATUSWINDOW, 0);
        ImmNotifyIME((HIMC)g_UnknownGlobal56e26c->field_0x53c, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
        ImmAssociateContext((HWND)g_UnknownGlobal56e26c->field_0x31c, (HIMC)g_UnknownGlobal56e26c->field_0x540);
    }
}

// 0x00487bf0
int GUIUser::UnknownFunction487bf0(UnknownGuiControl** previous) {
    if (focusControl && !focusControl->field_0x25_bit3)
        focusControl->UnknownVirtualSlot33(0);
    if (previous)
        *previous = focusControl;
    focusControl = 0;
    return 1;
}

// 0x00487c30
int GUIUser::AcceptDevice(GUIInputDevice* device) {
    if (device->ownerUser) {
        if (device->ownerUser != this)
            return 0;
    } else {
        acceptedDevices.Add(device);
        device->ownerUser = this;
    }
    return 1;
}

// 0x00487d00
void GUIUser::AcceptKeyboardAndJoysticks() {
    int i;

    if (userGui->keyboardDevice)
        AcceptDevice(userGui->keyboardDevice);
    for (i = 0; i < g_UnknownGlobal56e26c->field_0x14->joystickCount; i++)
        AcceptDevice(userGui->joystickDevices[i]);
}

// 0x00487d60
void GUIUser::ForgetDevices() {
    GUIInputDevice* device;
    int i = 0;

    while ((device = acceptedDevices.Get(i++)) != 0)
        device->ownerUser = 0;
    acceptedDevices.Clear();
}

// 0x00487dd0
void GUIUser::CreateCursor(const char* image, int visible) {
    void* palette;

    if (userGui && userGui->guiBackground)
        userGui->guiBackground->UnknownFunction404c80();
    if (userCursor || !pointerDevice)
        return;
    if (g_UnknownGlobal56e26c->field_0x2d4_bit1) {
        if (g_UnknownGlobal56e26c->field_0x10->field_0x28 > 8 ||
            (g_UnknownGlobal56e26c->field_0x10->field_0x28 == 8 && userGui->guiPalette)) {
            palette = userGui->guiPalette ? userGui->guiPalette->field_0x708 : 0;
            if (!image)
                image = cursorImage;
            userCursor = (GUICursor*)UnknownFunction469190(
                (new(__FILE__, 1890) GUICursor(visible))
                    ->UnknownFunction43ea70(field_0x18, &pointerDevice->bindingX, &pointerDevice->bindingY, image,
                                            userGui->guiTextures, userGui->guiBackground, palette,
                                            userGui->guiPalette),
                -1);
        }
    } else {
        if (g_UnknownGlobal56e26c->field_0x10->field_0x28 > 8 ||
            (g_UnknownGlobal56e26c->field_0x10->field_0x28 == 8 && userGui->guiPalette)) {
            palette = userGui->guiPalette ? userGui->guiPalette->field_0x708 : 0;
            if (!image)
                image = cursorImage;
            userCursor = (GUICursor*)UnknownFunction469130(
                (new(__FILE__, 1898) GUICursor(visible))
                    ->UnknownFunction43eaf0(field_0x18, image, userGui->guiTextures, userGui->guiBackground,
                                            palette, userGui->guiPalette),
                -1);
        }
    }
}

// 0x00487fb0
void GUIUser::UnknownFunction487fb0(UnknownCursorAnimation* animation) {
    UnknownCursorAnimation* current;

    if (!userCursor)
        return;
    if (!animation && cursorAnimation)
        animation = cursorAnimation;
    current = userCursor->field_0x5c;
    if (current && field_0xc4 && animation != current && animation->field_0x10 > current->field_0x08) {
        animation->field_0x08 = current->field_0x08;
        animation->field_0x0c = current->field_0x0c;
        animation->field_0x20 = current->field_0x20;
    }
    userCursor->UnknownFunction485150(animation);
}

// 0x00488010
void GUIUser::UnknownFunction488010(const char* image, int redraw) {
    int visible;
    int defaultImage;

    if (g_UnknownGlobal56e26c->field_0x2d5_bit1 || !pointerDevice)
        return;
    visible = userCursor ? userCursor->field_0x25_bit0 : 1;
    ReleaseCursor();
    if (g_UnknownGlobal56e26c->field_0x10->field_0x28 == 8 && !userGui->guiPalette)
        return;
    defaultImage = !image;
    CreateCursor(image ? image : cursorImage, visible);
    if (defaultImage && cursorAnimation && userCursor)
        userCursor->UnknownFunction485150(cursorAnimation);
    if (redraw)
        userGui->RedrawFrame();
}

// 0x004880c0
void GUIUser::ReleaseCursor() {
    if (!g_UnknownGlobal56e26c->field_0x2d5_bit1 && userCursor)
        userCursor->Release();
    userCursor = 0;
}

// 0x00488110
UIDlgContainer::~UIDlgContainer() {
}

// 0x00488160
int GUIUser::UnknownFunction488160(InputDevice* device) {
    GUIInputDevice* candidate;
    int i;

    if (!pointerDevice || pointerDevice->inputDevice == device)
        return 1;
    i = 0;
    while ((candidate = acceptedDevices.Get(i++)) != 0) {
        if (candidate->inputDevice == device)
            return 1;
    }
    return 0;
}

// 0x004881d0
int GUIUser::UnknownFunction4881d0(UnknownControlEvent* event) {
    if (event->kind == 0)
        return UnknownFunction488160((InputDevice*)g_UnknownGlobal56e26c->field_0x14->keyboard);
    if (event->kind == 1)
        return UnknownFunction488160((InputDevice*)g_UnknownGlobal56e26c->field_0x14->mouse);
    if (event->kind == 2 || event->kind == 3)
        return UnknownFunction488160((InputDevice*)g_UnknownGlobal56e26c->field_0x14->joysticks[event->device]);
    return 0;
}

// 0x00488240
GUIInputDevice* GUIUser::UnknownFunction488240(InputDevice* device) {
    GUIInputDevice* candidate;
    int i;

    if (!pointerDevice || pointerDevice->inputDevice == device)
        return pointerDevice;
    i = 0;
    while ((candidate = acceptedDevices.Get(i++)) != 0) {
        if (candidate->inputDevice == device)
            return candidate;
    }
    return 0;
}

// 0x004882a0
GUIInputDevice* GUIUser::UnknownFunction4882a0(UnknownControlEvent* event) {
    if (event->kind == 0)
        return UnknownFunction488240((InputDevice*)g_UnknownGlobal56e26c->field_0x14->keyboard);
    if (event->kind == 1)
        return UnknownFunction488240((InputDevice*)g_UnknownGlobal56e26c->field_0x14->mouse);
    if (event->kind == 2 || event->kind == 3)
        return UnknownFunction488240((InputDevice*)g_UnknownGlobal56e26c->field_0x14->joysticks[event->device]);
    return 0;
}

// 0x00488310
GUIInputDevice* GUIUser::UnknownFunction488310(int index) {
    return acceptedDevices.Get(index);
}

// 0x0067b468 (initializer 0x00488340; Game.h's g_UnknownGlobal56c470 points
// at it).
HiResMeter g_UnknownHiResMeter67b468(0);

// 0x00488380
HiResMeter::HiResMeter(int flags) : GameObject(flags) {
    field_0x3c = -1;
    field_0x30 = 0;
    UnknownFunction488580();
    if (UnknownVirtualSlot8(0))
        field_0x2c = 1;
    else
        field_0x2c = 0;
}

// 0x00488430
HiResMeter::~HiResMeter() {
}

// 0x00488510
void HiResMeter::UnknownFunction488510() {
    int i;

    for (i = 0; i < field_0x34; i++) {
        field_0x40[i].field_0x10 = 0;
        field_0x40[i].field_0x14 = 0;
    }
}

// 0x00488540
void HiResMeter::UnknownFunction488540() {
    int i;

    for (i = 0; i < field_0x34; i++) {
        field_0x40[i].field_0x08 = 0;
        field_0x40[i].field_0x0c = 0;
        field_0x40[i].field_0x00 = 0;
        field_0x40[i].field_0x04 = 0;
        field_0x40[i].field_0x10 = 0;
        field_0x40[i].field_0x14 = 0;
        field_0x40[i].field_0x18 = 0;
        field_0x40[i].field_0x1c = 0;
        field_0x40[i].field_0x20 = 0;
    }
    field_0x38 = 0;
}

// 0x00488580
void HiResMeter::UnknownFunction488580() {
    int i;

    for (i = 0; i < 50; i++) {
        field_0x40[i].field_0x08 = 0;
        field_0x40[i].field_0x0c = 0;
        field_0x40[i].field_0x00 = 0;
        field_0x40[i].field_0x04 = 0;
        field_0x40[i].field_0x10 = 0;
        field_0x40[i].field_0x14 = 0;
        field_0x40[i].field_0x18 = 0;
        field_0x40[i].field_0x1c = 0;
        field_0x40[i].field_0x20 = 0;
        field_0x40[i].field_0x24[0] = 0;
    }
    field_0x34 = 0;
    field_0x38 = 0;
}

// 0x004885c0
int HiResMeter::UnknownVirtualSlot10(float frameTime) {
    UnknownHiResMeterEntry* entry;
    int i;

    if (g_UnknownGlobal56e26c->field_0x38) {
        if (field_0x3c < 0)
            field_0x3c = g_UnknownGlobal56e26c->field_0x38->NewPage();
        if (g_UnknownGlobal56e26c->field_0x38->field_0x26c4 != field_0x3c) {
            UnknownFunction488540();
            return 1;
        }
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447fa0(
            field_0x3c, "MS Timers [CallsThisTick-->TotMSThisTick (AvgMSPerTick)] : (Per-call Peak/Avg)");
        g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(field_0x3c, "Previous Game Tick: % 3d  MS",
                                                                g_UnknownGlobal65b534);
        for (i = 0; i < field_0x34; i++) {
            entry = &field_0x40[i];
            if (entry->field_0x08 != 0.0f) {
                entry->field_0x1c += 1.0f;
                entry->field_0x18 = entry->field_0x18 + entry->field_0x14;
                g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(
                    field_0x3c, "%24.24s [%4.0f --> %6.3f (%6.3f)] : (%6.3f / %5.3f) MS", entry->field_0x24,
                    entry->field_0x10, entry->field_0x14, entry->field_0x18 / entry->field_0x1c,
                    entry->field_0x20, entry->field_0x0c / entry->field_0x08);
            }
        }
    }
    return 1;
}
