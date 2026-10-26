// GUIManager.cpp -- reconstruction of D:\aardvark\VC\krusty2\GUIManager.cpp.
// See GUIManager.h for the TU extent and the classes.

#include <windows.h>
#include <imm.h>
#include <stdlib.h>
#include <string.h>

#include "GUIManager.h"
#include "GameUi.h"

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
extern int g_TickTime;

// cdecl 0x00460b50: table-driven square root (FollowCamera.h).
float FastSqrt(float value);

#define SoundSystem() ((PCSoundInterface*)g_TrackGame->soundInterface)

// 0x00484f10
void UnknownGroundFogShader::FogVertices(UnknownFogVertex* vertices, int count) {
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

    if (groundFog == (UnknownGroundFog*)1)
        return;
    if (!groundFog) {
        groundFog = (UnknownGroundFog*)g_TrackGame->childRoot->FindByClassName(1, "GroundFog");
        if (!groundFog) {
            groundFog = (UnknownGroundFog*)1;
            return;
        }
    }
    fog = groundFog;
    camera = fog->renderContext->currentCamera;
    alphaLimit = fog->alphaLimit;
    eyeX = camera->eyeX;
    eyeY = camera->eyeY;
    eyeZ = camera->eyeZ;
    depthLimit = (int)fog->depthLimit;
    density = fog->density;
    for (i = 0; i < count; i++) {
        if (vertices[i].y >= groundFog->planeHeight && groundFog->planeHeight <= eyeY) {
            vertices[i].specular = 0xff000000;
            continue;
        }
        if (groundFog->planeHeight > eyeY && vertices[i].y > eyeY)
            depth = groundFog->planeHeight - eyeY;
        else
            depth = groundFog->planeHeight - vertices[i].y;
        dx = eyeX - vertices[i].x;
        dy = eyeY - vertices[i].y;
        dz = eyeZ - vertices[i].z;
        if (dy < 0.0f)
            dy = -dy;
        distance = FastSqrt(dx * dx + dy * dy + dz * dz);
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
    cursorAnimation = 0;
}

// 0x00485140
GUICursor::~GUICursor() {
}

// 0x00485150
void GUICursor::SetAnimation(UnknownCursorAnimation* animation) {
    cursorAnimation = animation;
    if (!animation)
        currentFrame = 0;
}

// 0x00485170
int GUICursor::RenderPost3D() {
    if (cursorAnimation)
        currentFrame = reinterpret_cast<int>(
            reinterpret_cast<UIAnim*>(cursorAnimation)->AdvancePastSounds());
    return GameCursor::RenderPost3D();
}

// 0x00485190
GUIManager::GUIManager(int flags) : GameObject(flags) {
    int i;

    openedDialog = 0;
    ownsSoundGroup = 0;
    setupCallback = 0;
    guiTextures = 0;
    ownsBackground = 0;
    screenGrab = 0;
    grabRegion = -1;
    grabCopyFrames = 0;
    uiAnimations = 1;
    dialogDirectory[0] = 0;
    waitFrames = 0;
    setupPalette = 0;
    guiPalette = 0;
    guiSoundGroup = 0;
    guiBackground = 0;
    toolTipFont = 0;
    languageModule = 0;
    windowClipper = 0;
    paletteChanged = 0;
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
    modalDialog = 0;
    soundVolume = 0;
    strcpy(toolTipFontFace, "");
    fontSizeOffset = 0;
    toolTipBold = 0;
    toolTipItalic = 0;
    drawGrabFirst = 0;
}

// 0x004853b0.  `fontSize` is a short: VC6 copies a sub-int parameter at entry, so its
// slot is the second dead argument slot handed out (after `target`'s) and the
// ControlInterface pointer lands there; with an int parameter the slot frees only at
// the font-size store and the pointer takes `target`'s slot (docs/VC6_FRAME_LAYOUT.md).
GUIManager* GUIManager::SetUp(void* target, Palette8* palette, TextureMapManager* textures,
                              BackgroundImage* background, int startSound, SoundGroup* sound,
                              const char* font, short fontSize, const char* cursor,
                              int callback) {
    ControlInterface* controls;
    int i;

    GameObject::SetRenderContext(target);
    guiTextures = textures;
    guiSoundGroup = sound;
    if (startSound && SoundSystem()->InitializeSound(22050, 1, 16, 4000000, 0) == 0)
        SoundSystem()->SetPrimaryFormat(22050, 1, 16);
    if (!sound) {
        guiSoundGroup = new(__FILE__, 130) SoundGroup(1);
        AppendChild(guiSoundGroup, -1);
        ownsSoundGroup = 1;
    }
    dialogContainer = (GameObject*)AppendChild(new(__FILE__, 136) UIDlgContainer, -1);
    setupPalette = palette;
    guiPalette = palette;
    guiBackground = background;
    strcpy(dialogFontName, font);
    dialogFontSize = fontSize;
    if (cursor)
        strcpy(cursorImage, cursor);
    OnActivate();
    setupCallback = callback;
    toolTipFont = CreateFontA(12, 0, 0, 0, 400, 0, 0, 0, 1, 0, 0, 2, 2, "Arial");
    controls = g_TrackGame->controlInterface;
    if (controls->mouse) {
        mouseDevice = (GUIInputDevice*)AppendChild(
            (new(__FILE__, 162) GUIInputDevice)
                ->Bind(renderContext, (InputDevice*)controls->mouse, 0, 0, 0, 0),
            -1);
        mouseDevice->InsertBefore(dialogContainer, -1);
        mouseDevice->Rebind();
    }
    if (controls->keyboard) {
        keyboardDevice = (GUIInputDevice*)AppendChild(
            (new(__FILE__, 169) GUIInputDevice)
                ->Bind(renderContext, (InputDevice*)controls->keyboard, 0, 0, 0, 0),
            -1);
        keyboardDevice->InsertBefore(dialogContainer, -1);
        keyboardDevice->Rebind();
    }
    if (controls->joystickCount) {
        for (i = 0; i < controls->joystickCount; i++) {
            joystickDevices[i] = (GUIInputDevice*)AppendChild(
                (new(__FILE__, 177) GUIInputDevice)
                    ->Bind(renderContext, (InputDevice*)controls->joysticks[i], -2.0f, 2.0f, -2.0f,
                           2.0f),
                -1);
            joystickDevices[i]->InsertBefore(dialogContainer, -1);
            joystickDevices[i]->Rebind();
        }
    }
    users[0] = (GUIUser*)AppendChild(
        (new(__FILE__, 188) GUIUser)->Create(renderContext, this), -1);
    if (users[0]) {
        userCount = 1;
        users[0]->AcceptKeyboardAndJoysticks();
        if (mouseDevice) {
            users[0]->SetPointerDevice(mouseDevice);
            CreateCursors(0, 1);
            RecreateCursors(0, 0);
        }
    }
    users[0]->EnableImeInput(0);
    languageModule = LoadLibraryA("uilang.dll");
    return this;
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
        windowClipper->Release();
}

// 0x004857f0
int GUIManager::OnControlPress(UnknownControlEvent* event, UnknownInputEntry* entry) {
    int previous = g_MemTagStack->Push("UI");
    int result = GameObject::OnControlPress(event, entry);
    g_MemTagStack->Pop(previous);
    return result;
}

// 0x00485830
int GUIManager::OnControlRelease(UnknownControlEvent* event, UnknownInputEntry* entry) {
    int previous = g_MemTagStack->Push("UI");
    int result = GameObject::OnControlRelease(event, entry);
    g_MemTagStack->Pop(previous);
    return result;
}

// 0x00485870
int GUIManager::Tick(float frameTime) {
    int previous = g_MemTagStack->Push("UI");
    GameObject::Tick(frameTime);
    if (paletteChanged) {
        paletteChanged = 0;
        UnknownFunction464e90();
    }
    g_MemTagStack->Pop(previous);
    return 1;
}

// 0x004858d0
int GUIManager::RenderPre3D() {
    CameraRect rect;

    if (waitFrames)
        return 1;
    if (screenGrab && drawGrabFirst) {
        rect.left = 0;
        rect.top = 0;
        rect.right = screenGrab->textureWidth;
        rect.bottom = screenGrab->textureHeight;
        if (guiBackground) {
            guiBackground->DrawImage(screenGrab, &rect, 0, 0x1000000, grabRegion, 1, &grabCopyFrames, 0);
        } else if (!((RenderTarget*)renderContext)->Blit(&rect, screenGrab, 0, 0x1000000)) {
            GameObject::RenderPre3D();
            return 0;
        }
    }
    GameObject::RenderPre3D();
    return 1;
}

// 0x00485970
int GUIManager::RenderPost3D() {
    CameraRect rect;

    if (!waitFrames) {
        if (screenGrab && !drawGrabFirst) {
            rect.left = 0;
            rect.top = 0;
            rect.right = screenGrab->textureWidth;
            rect.bottom = screenGrab->textureHeight;
            if (guiBackground) {
                guiBackground->DrawImage(screenGrab, &rect, 0, 0x1000000, grabRegion, 1, &grabCopyFrames, 0);
            } else if (!((RenderTarget*)renderContext)->Blit(&rect, screenGrab, 0, 0x1000000)) {
                GameObject::RenderPost3D();
                return 0;
            }
        }
        GameObject::RenderPost3D();
    } else {
        waitFrames = waitFrames - 1 < 0 ? 0 : waitFrames - 1;
        g_TrackGame->renderTarget->Clear(0, 0);
        if (!waitFrames && redrawBackground) {
            redrawBackground->MarkAllDirty();
            redrawBackground = 0;
        }
    }
    if (guiBackground)
        guiBackground->ReleaseDc();
    return 1;
}

// 0x00485a70
UIDialog* GUIManager::ShowDialog(UIDialog* dialog, int menu, int flags, int area,
                                 UIDialog* parent, int initParam, int name, int wait) {
    CameraRect screen;
    CameraRect* rect;

    if (!(flags & 4) && !(flags & 2)) {
        if (!FindInputDialog() || !FindInputDialog()->isModal)
            ReleaseBackgroundGrab();
    } else {
        rect = 0;
        if (flags & 0x10) {
            screen.left = screen.top = 0;
            screen.right = ((RenderTarget*)renderContext)->targetWidth;
            screen.bottom = ((RenderTarget*)renderContext)->targetHeight;
            rect = &screen;
        }
        if (parent)
            parent->SetModalPause(1, flags & 8, rect);
        else if ((flags & 4) && (flags & 0x10))
            GrabBackground(flags & 8);
    }
    if (wait)
        RecreateCursors(waitCursorImage, paletteChanged == 0);
    if (dialog->UnknownVirtualSlot27(renderContext, (CameraRect*)area, menu, flags, guiSoundGroup, guiTextures, dialogDirectory,
                                     parent, guiBackground, dialogFontName, dialogFontSize, this, users[0],
                                     (const char*)name)) {
        dialog->showFlags = flags;
        AttachDialog(dialog, initParam, wait);
        return dialog;
    }
    if (wait)
        RecreateCursors(cursorImage, paletteChanged == 0);
    return 0;
}

// 0x00485bd0
void GUIManager::AttachDialog(UIDialog* dialog, int initParam, int wait) {
    int visible = dialog->isEnabled;

    dialog->SetShown(0);
    if (dialog->parentDialog && (dialog->showFlags & 1))
        dialog->parentDialog->controlContainer->AppendSibling(dialog, -1);
    else
        dialogContainer->AppendChild(dialog, -1);
    dialog->UnknownVirtualSlot28(initParam);
    dialog->animatesControls = uiAnimations;
    if (guiBackground)
        guiBackground->MarkAllDirty();
    if (wait)
        RecreateCursors(cursorImage, paletteChanged == 0);
    if (paletteChanged)
        waitFrames = 3;
    dialog->SetShown(visible);
}

// 0x00485c80: opens `resource` as a plain UIDialog (0x7f58 bytes) with the
// GUI's sound group, textures and directory, in Arial 14.
int GUIManager::OpenDialogResource(const char* resource) {
    CloseDialogResource();
    openedDialog = new(__FILE__, 523) UIDialog(0, resource);
    if (openedDialog) {
        openedDialog->UnknownVirtualSlot27(renderContext, 0, 0, 0, guiSoundGroup, guiTextures, dialogDirectory, 0, 0, "Arial",
                                         14, 0, 0, 0);
        return 1;
    }
    return 0;
}

// 0x00485d50
void GUIManager::CloseDialogResource() {
    if (openedDialog) {
        openedDialog->Release();
        openedDialog = 0;
    }
}

// 0x00485d70
void GUIManager::SetDialogDirectory(const char* directory) {
    int last = strlen(directory) - 1;

    strcpy(dialogDirectory, directory);
    if (directory[last] != '\\')
        strcat(dialogDirectory, "\\");
}

// 0x00485df0
UIDialog* GUIManager::FindInputDialog() {
    UIDialog* found = 0;
    UIDialog* dialog;
    GameObjectIterator iterator(this, 1, "UIDialog");

    while ((dialog = (UIDialog*)iterator.Next()) != 0) {
        if (!found)
            found = dialog;
        if (mouseDevice && PtInRect((RECT*)&dialog->screenArea, mouseDevice->pointerPosition))
            found = dialog;
        if (dialog->isModal)
            found = dialog;
    }
    return found;
}

// 0x00485ec0
Sound* GUIManager::FindSectionObject(const char* name) {
    if (openedDialog)
        return openedDialog->FindSectionObject(name);
    return 0;
}

// 0x00485ee0
int GUIManager::SwapModalDialog(int value) {
    int previous = modalDialog;
    modalDialog = value;
    return previous;
}

// 0x00485ef0
void GUIManager::CreateBackground() {
    int i;

    if (guiBackground)
        return;
    guiBackground = (BackgroundImage*)AppendChild(
        (new(__FILE__, 672) BackgroundImage(1))->SetRenderContext(renderContext), -1);
    guiBackground->InsertBefore(dialogContainer, -1);
    ownsBackground = 1;
    RecreateCursors(0, 1);
    for (i = 0; i < userCount; i++)
        GetUser(i)->CreateToolTip(renderContext, this);
}

// 0x00485fc0
void GUIManager::ReleaseBackground() {
    UIDialog* dialog;
    int i;

    if (!ownsBackground)
        return;
    ReleaseCursors();
    if (guiBackground)
        guiBackground->Release();
    guiBackground = 0;
    RecreateCursors(0, 0);
    GameObjectIterator iterator(this, 1, "UIDialog");
    while ((dialog = (UIDialog*)iterator.Next()) != 0)
        dialog->SetBackground(0);
    for (i = 0; i < userCount; i++)
        GetUser(i)->CreateToolTip(renderContext, this);
}

// 0x004860a0
void GUIManager::GrabBackground(int dim) {
    ReleaseBackgroundGrab();
    screenGrab = CopyScreenToTexture(dim, 0);
    if (guiBackground) {
        grabRegion = guiBackground->AllocRegion(1);
        guiBackground->MarkAllDirty();
        guiBackground->imageVisible = 0;
    }
}

// 0x004860f0
void GUIManager::ReleaseBackgroundGrab() {
    if (screenGrab)
        screenGrab->Release();
    if (guiBackground && grabRegion >= 0) {
        guiBackground->ReleaseRegion(grabRegion);
        guiBackground->imageVisible = 1;
    }
    screenGrab = 0;
    grabRegion = -1;
    grabCopyFrames = 0;
}

// 0x00486150
void GUIManager::SetGuiPalette(Palette8* palette) {
    paletteChanged = 1;
    guiPalette = palette;
}

// 0x004864f0
int GUIManager::UnknownFunction4864f0() {
    return field_0x1f8;
}

// 0x00486500
void GUIManager::RedrawFrame() {
    if (guiBackground) {
        guiBackground->ReleaseDc();
        guiBackground->MarkAllDirty();
    }
    g_TrackGame->RenderFrame();
    g_TrackGame->PresentFrame();
    g_TrackGame->renderTarget->AdvanceFrame();
}

// 0x00486540
GUIUser* GUIManager::GetUser(int index) {
    if (index < 4)
        return users[index];
    return 0;
}

// 0x00486560
void GUIManager::SetWaitCursorImage(const char* image) {
    strcpy(waitCursorImage, image);
}

// 0x00486590
void GUIManager::CreateCursors(const char* image, int visible) {
    int i;

    for (i = 0; i < userCount; i++) {
        if (users[i])
            users[i]->CreateCursor(image ? image : cursorImage, visible);
    }
}

// 0x004865e0
void GUIManager::RecreateCursors(const char* image, int redraw) {
    int i;

    for (i = 0; i < userCount; i++) {
        if (users[i])
            users[i]->RecreateCursor(image ? image : cursorImage, redraw);
    }
}

// 0x00486630
void GUIManager::ShowCursors(int show) {
    int count = userCount;
    GUIUser* user;
    int i;

    for (i = 0; i < count; i++) {
        user = GetUser(i);
        if (user && user->userCursor) {
            if (show)
                user->userCursor->Enable();
            else
                user->userCursor->Disable();
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
void GUIManager::SetToolTipFont(const char* font) {
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
        if (texture->CreateSurface(bits, width, height, width, width, 0x378,
                                   g_TrackGame->renderTarget->surfacePixelFormat,
                                   (UnknownTexturePalette*)(guiPalette ? guiPalette->colorMapper : 0), 4,
                                   guiPalette ? guiPalette->ddPalette : 0, 1, 0, 2, 1, 0, 0x80, 0xff00ff))
            texture->ApplyColorKey(0xff00ff);
        delete bits;
    }
    return texture;
}

// 0x004868b0
int GUIManager::EnableWindowClipper(int enable) {
    UnknownDisplay* display;

    if (enable) {
        if (windowClipper) {
            windowClipper->UnknownMethod7(0, 0);
            windowClipper->Release();
        }
        ((UnknownGuiDirectDraw*)g_TrackGame->display->directDraw)
            ->UnknownMethod4(0, &windowClipper, 0);
        ((UnknownGuiSurface*)g_TrackGame->display->primarySurface)->UnknownMethod28(windowClipper);
        if (windowClipper)
            windowClipper->UnknownMethod8(0, g_TrackGame->windowHandle);
    } else {
        if (windowClipper) {
            windowClipper->UnknownMethod7(0, 0);
            windowClipper->Release();
            windowClipper = 0;
        }
        ((UnknownGuiSurface*)g_TrackGame->display->primarySurface)->UnknownMethod28(0);
    }
    display = g_TrackGame->display;
    display->SetGDISurfaceVisible(enable);
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
ToolTip* ToolTip::Create(void* target, GUIManager* gui) {
    GameObject::SetRenderContext(target);
    tipGui = gui;
    return this;
}

// 0x00486a30
ToolTip::~ToolTip() {
    if (textTexture)
        textTexture->Release();
    if (tipGui && tipGui->guiBackground && backgroundRegion > -1)
        tipGui->guiBackground->ReleaseRegion(backgroundRegion);
}

// 0x00486ab0
int ToolTip::Tick(float frameTime) {
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

// 0x00486b80: shows `text` at `position` for `time` seconds. The padded
// width (clamped two pixels inside the target) and height are written back
// into `size`; retail keeps those two dead stores, and the named limit gives
// its `sub eax, 2`.
void ToolTip::ShowText(const char* text, int* position, float time) {
    PCRenderTarget* target;
    PCTextureMap* texture;
    HDC dc;
    HGDIOBJ previous;
    SIZE size;
    int width;
    int height;
    int x;

    if (textTexture) {
        textTexture->Release();
        textTexture = 0;
    }
    if (text) {
        if (tipGui->guiBackground && backgroundRegion == -1) {
            backgroundRegion = tipGui->guiBackground->AllocRegion(0);
            restoreFrames = 3;
        }
        if (!((PCRenderTarget*)renderContext)->renderSurface->GetDC((void**)&dc)) {
            previous = SelectObject(dc, (HGDIOBJ)tipGui->toolTipFont);
            GetTextExtentPoint32A(dc, text, strlen(text), &size);
            SelectObject(dc, previous);
            ((PCRenderTarget*)renderContext)->renderSurface->ReleaseDC(dc);
            target = (PCRenderTarget*)renderContext;
            int limit = target->targetWidth - 2;
            width = __min(size.cx + 2, limit);
            size.cx = width;
            size.cy += 2;
            height = size.cy;
            textArea.top = 0;
            textArea.left = 0;
            textArea.right = width;
            textArea.bottom = height;
            screenArea.left = position[0];
            screenArea.top = position[1];
            x = screenArea.left - width / 2;
            x = x < 0 ? 0 : x;
            screenArea.left = __min(target->targetWidth - width, x);
            screenArea.top = __min(target->targetHeight - height, screenArea.top);
            screenArea.right = screenArea.left + width;
            screenArea.bottom = screenArea.top + height;
            texture = tipGui->CreateFilledTexture(width, height, 0x808080);
            textTexture = texture;
            if (texture && texture->systemSurface) {
                if (!texture->systemSurface->GetDC((void**)&dc)) {
                    previous = SelectObject(dc, (HGDIOBJ)tipGui->toolTipFont);
                    SetTextColor(dc, 0xcccccc);
                    SetBkColor(dc, 0x404040);
                    DrawTextA(dc, text, strlen(text), &textArea, 0x8025);
                    SelectObject(dc, previous);
                    texture->systemSurface->ReleaseDC(dc);
                }
                shown = 1;
                showDelay = time;
                return;
            }
            shown = 0;
            showDelay = -1.0f;
            return;
        }
    }
    shown = 0;
    showDelay = -1.0f;
}

// 0x00486db0
int ToolTip::RenderPost3D() {
    CameraRect rect;

    if (shown) {
        if (tipGui && tipGui->guiBackground) {
            tipGui->guiBackground->DrawImage(textTexture, &screenArea, &textArea, 0x1008000, backgroundRegion,
                                                         0, &restoreFrames, 0);
            return 1;
        }
        rect.left = screenArea.left;
        rect.top = screenArea.top;
        rect.right = screenArea.left + textArea.right;
        rect.bottom = screenArea.top + textArea.bottom;
        if (!((RenderTarget*)renderContext)->Blit(&rect, textTexture, &textArea, 0x1008000))
            return 0;
    } else if (tipGui && tipGui->guiBackground && backgroundRegion > -1) {
        if (restoreFrames > 0) {
            tipGui->guiBackground->SetRegionRect(backgroundRegion, &screenArea);
            restoreFrames--;
            return 1;
        }
        tipGui->guiBackground->MarkRegionPending(backgroundRegion);
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
        bindingX.Unbind();
        bindingY.Unbind();
    }
}

// 0x00486fb0
GUIInputDevice* GUIInputDevice::Bind(void* target, InputDevice* device, float minX, float maxX,
                                     float minY, float maxY) {
    float rangeX;
    float rangeY;

    GameObject::SetRenderContext(target);
    inputDevice = device;
    rangeMinX = minX;
    rangeMaxX = maxX;
    rangeMinY = minY;
    rangeMaxY = maxY;
    rangeX = maxX;
    rangeY = maxY;
    if (minX == 0.0f && maxX == 0.0f && minY == 0.0f && maxY == 0.0f) {
        rangeX = (float)(((RenderTarget*)renderContext)->targetWidth - 1);
        rangeY = (float)(((RenderTarget*)renderContext)->targetHeight - 1);
    }
    bindingX = UnknownControlBinding(rangeMinX, rangeX, 0.0f,
                                       g_TrackGame->controlInterface->NextBindingId(), 0, 0.0f);
    bindingY = UnknownControlBinding(rangeMinY, rangeY, 0.0f,
                                       g_TrackGame->controlInterface->NextBindingId(), 1, 0.0f);
    if (inputDevice->deviceKind == 2) {
        ((JoystickDevice*)inputDevice)->AttachAxisBinding(&bindingX, 0.01f, 0.01f);
        ((JoystickDevice*)inputDevice)->AttachAxisBinding(&bindingY, 0.01f, 0.01f);
    } else if (inputDevice->deviceKind == 1) {
        ((MouseDevice*)inputDevice)->AttachBinding(&bindingX);
        ((MouseDevice*)inputDevice)->AttachBinding(&bindingY);
    }
    return this;
}

// 0x00487150
void GUIInputDevice::Rebind() {
    float rangeX;
    float rangeY;
    int x = (int)bindingX.currentValue;
    int y = (int)bindingY.currentValue;

    if (!inputDevice)
        return;
    if (inputDevice->deviceKind == 2 || inputDevice->deviceKind == 1) {
        inputDevice->RemoveBindings(bindingX.bindingId);
        inputDevice->RemoveBindings(bindingY.bindingId);
    }
    rangeX = rangeMaxX;
    rangeY = rangeMaxY;
    if (rangeMinX == 0.0f && rangeMaxX == 0.0f && rangeMinY == 0.0f && rangeMaxY == 0.0f) {
        rangeX = (float)(((RenderTarget*)renderContext)->targetWidth - 1);
        rangeY = (float)(((RenderTarget*)renderContext)->targetHeight - 1);
    }
    bindingX = UnknownControlBinding(rangeMinX, rangeX, (float)x,
                                       g_TrackGame->controlInterface->NextBindingId(), 0, 0.0f);
    bindingY = UnknownControlBinding(rangeMinY, rangeY, (float)y,
                                       g_TrackGame->controlInterface->NextBindingId(), 1, 0.0f);
    if (inputDevice->deviceKind == 2) {
        ((JoystickDevice*)inputDevice)->AttachAxisBinding(&bindingX, 0.01f, 0.01f);
        ((JoystickDevice*)inputDevice)->AttachAxisBinding(&bindingY, 0.01f, 0.01f);
    } else if (inputDevice->deviceKind == 1) {
        ((MouseDevice*)inputDevice)->AttachBinding(&bindingX);
        ((MouseDevice*)inputDevice)->AttachBinding(&bindingY);
    }
}

// 0x00487320
int GUIInputDevice::Tick(float frameTime) {
    int kind;

    GameObject::Tick(frameTime);
    kind = inputDevice->deviceKind;
    if (kind) {
        if (g_TrackGame->fullScreen) {
            POINT* position = &pointerPosition;
            position->x = (int)bindingX.currentValue;
            position->y = (int)bindingY.currentValue;
            return 1;
        }
        if (kind == 1 && ownerUser && ownerUser->userCursor) {
            pointerPosition.x = ownerUser->userCursor->cursorX;
            pointerPosition.y = ownerUser->userCursor->cursorY;
        }
    }
    return 1;
}

// 0x004873b0
int GUIInputDevice::AttachRenderContext(void* context) {
    GameObject::AttachRenderContext(context);
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
    hoverControl = 0;
    pendingHoverControl = 0;
    pendingFocusControl = 0;
    acceptedDevices.Init(1, 1);
    pointerDevice = 0;
    pointerY = 0;
    pointerX = 0;
    for (i = 0; i < 32; i++)
        controlDown[i] = 0;
}

// 0x00487540
GUIUser::~GUIUser() {
}

// 0x004875a0
int GUIUser::Tick(float frameTime) {
    UpdateFocus(0);
    return GameObject::Tick(frameTime);
}

// 0x004875c0
void GUIUser::UpdateFocus(int force) {
    if (pendingFocusControl) {
        MoveFocus(pendingFocusControl, 0);
        pendingHoverControl = 0;
        pendingFocusControl = 0;
        return;
    }
    if (pendingHoverControl || force) {
        SetHoverControl(pendingHoverControl, 0);
        pendingHoverControl = 0;
    }
}

// 0x00487620
int GUIUser::RenderPost3D() {
    GameObject::RenderPost3D();
    if (pointerDevice) {
        pointerX = pointerDevice->pointerPosition.x;
        pointerY = pointerDevice->pointerPosition.y;
    }
    return 1;
}

// 0x00487650
GUIUser* GUIUser::Create(void* target, GUIManager* gui) {
    GameObject::SetRenderContext(target);
    userGui = gui;
    CreateToolTip(target, gui);
    return this;
}

// 0x00487680
void GUIUser::CreateToolTip(void* target, GUIManager* gui) {
    ReleaseToolTip();
    userToolTip = (ToolTip*)AppendChild(
        (new(__FILE__, 1639) ToolTip(1))->Create(target, gui), -1);
}

// 0x00487710
void GUIUser::ReleaseToolTip() {
    if (userToolTip)
        userToolTip->Release();
    userToolTip = 0;
}

// 0x00487730
int GUIUser::RequestHover(UnknownGuiControl* control, UnknownGuiControl** previous, int update) {
    if (pendingHoverControl && strstr(pendingHoverControl->classNames, "UIDDL"))
        return 0;
    if (previous)
        *previous = pendingHoverControl;
    pendingHoverControl = control;
    if (update)
        UpdateFocus(1);
    return 1;
}

// 0x00487790
int GUIUser::RequestFocus(UnknownGuiControl* control, UnknownGuiControl** previous, int update) {
    if (pendingFocusControl && strstr(pendingFocusControl->classNames, "UIDDL"))
        return 0;
    if (previous)
        *previous = pendingFocusControl;
    pendingHoverControl = control;
    pendingFocusControl = control;
    if (update)
        UpdateFocus(1);
    return 1;
}

// 0x00487800
int GUIUser::SetHoverControl(UnknownGuiControl* control, UnknownGuiControl** previous) {
    int result = 1;

    if (control && control != hoverControl)
        result = control->UnknownVirtualSlot31();
    if (result) {
        if (hoverControl && !hoverControl->pendingRelease && hoverControl != control)
            hoverControl->UnknownVirtualSlot32(control);
        if (previous)
            *previous = hoverControl;
        hoverControl = control;
    }
    return result;
}

// 0x00487870: moves the focus to `control`; a digits-only edit box turns the
// IME off.
int GUIUser::MoveFocus(UnknownGuiControl* control, UnknownGuiControl** previous) {
    int result = 1;

    if (control && control != hoverControl)
        result = control->UnknownVirtualSlot31();
    if (result) {
        if (hoverControl && !hoverControl->pendingRelease && hoverControl != control)
            hoverControl->UnknownVirtualSlot32(control);
        if (focusControl && !focusControl->pendingRelease && focusControl != control)
            focusControl->UnknownVirtualSlot33(control);
        if (previous)
            *previous = focusControl;
        hoverControl = control;
        focusControl = control;
        if (g_TrackGame->display->freezeFrameIndex) {
            if (control->controlType == 11) {
                if (control->acceptedCharacters && !strcmp(control->acceptedCharacters, "0123456789"))
                    EnableImeInput(0);
                else
                    EnableImeInput(1);
            } else {
                EnableImeInput(0);
            }
        }
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
        ImmAssociateContext((HWND)g_TrackGame->windowHandle, (HIMC)g_TrackGame->inputContext);
        area = focusControl->caretRect;
        composition.dwStyle = CFS_FORCE_POSITION;
        composition.ptCurrentPos.x = area.left;
        composition.ptCurrentPos.y = area.top;
        ImmSetCompositionWindow((HIMC)g_TrackGame->inputContext, &composition);
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
            font.lfWeight = control->ownerDialog->dialogFontBold ? 700 : 500;
        } else {
            face = control->ownerGui->dialogFontName;
            font.lfHeight = control->ownerGui->dialogFontSize;
            font.lfWeight = control->ownerGui->toolTipBold ? 700 : 500;
            font.lfItalic = (BYTE)control->ownerGui->toolTipItalic;
        }
        strcpy(font.lfFaceName, face);
        ImmSetCompositionFontA((HIMC)g_TrackGame->inputContext, &font);
        position.x = 550;
        position.y = 450;
        ImmSetStatusWindowPos((HIMC)g_TrackGame->inputContext, &position);
        window = ImmGetDefaultIMEWnd((HWND)g_TrackGame->windowHandle);
        if (window)
            SendMessageA(window, WM_IME_CONTROL, IMC_OPENSTATUSWINDOW, 0);
        ImmSetOpenStatus((HIMC)g_TrackGame->inputContext, 0);
    } else {
        window = ImmGetDefaultIMEWnd((HWND)g_TrackGame->windowHandle);
        if (window)
            SendMessageA(window, WM_IME_CONTROL, IMC_CLOSESTATUSWINDOW, 0);
        ImmNotifyIME((HIMC)g_TrackGame->inputContext, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
        ImmAssociateContext((HWND)g_TrackGame->windowHandle, (HIMC)g_TrackGame->previousInputContext);
    }
}

// 0x00487bf0
int GUIUser::ClearFocus(UnknownGuiControl** previous) {
    if (focusControl && !focusControl->pendingRelease)
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
    for (i = 0; i < g_TrackGame->controlInterface->joystickCount; i++)
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
        userGui->guiBackground->ReleaseDc();
    if (userCursor || !pointerDevice)
        return;
    if (g_TrackGame->fullScreen) {
        if (g_TrackGame->renderTarget->surfacePixelFormat > 8 ||
            (g_TrackGame->renderTarget->surfacePixelFormat == 8 && userGui->guiPalette)) {
            palette = userGui->guiPalette ? userGui->guiPalette->colorMapper : 0;
            if (!image)
                image = cursorImage;
            userCursor = (GUICursor*)AppendChild(
                (new(__FILE__, 1890) GUICursor(visible))
                    ->CreateBound(renderContext, &pointerDevice->bindingX, &pointerDevice->bindingY, image,
                                            userGui->guiTextures, userGui->guiBackground, palette,
                                            userGui->guiPalette),
                -1);
        }
    } else {
        if (g_TrackGame->renderTarget->surfacePixelFormat > 8 ||
            (g_TrackGame->renderTarget->surfacePixelFormat == 8 && userGui->guiPalette)) {
            palette = userGui->guiPalette ? userGui->guiPalette->colorMapper : 0;
            if (!image)
                image = cursorImage;
            userCursor = (GUICursor*)AppendSibling(
                (new(__FILE__, 1898) GUICursor(visible))
                    ->Create(renderContext, image, userGui->guiTextures, userGui->guiBackground,
                                            palette, userGui->guiPalette),
                -1);
        }
    }
}

// 0x00487fb0
void GUIUser::SetCursorAnimation(UnknownCursorAnimation* animation) {
    UnknownCursorAnimation* current;

    if (!userCursor)
        return;
    if (!animation && cursorAnimation)
        animation = cursorAnimation;
    current = userCursor->cursorAnimation;
    if (current && field_0xc4 && animation != current && animation->frameCount > current->currentFrame) {
        animation->currentFrame = current->currentFrame;
        animation->playForwards = current->playForwards;
        animation->lastStepTime = current->lastStepTime;
    }
    userCursor->SetAnimation(animation);
}

// 0x00488010
void GUIUser::RecreateCursor(const char* image, int redraw) {
    int visible;
    int defaultImage;

    if (g_TrackGame->shuttingDown || !pointerDevice)
        return;
    visible = userCursor ? userCursor->isEnabled : 1;
    ReleaseCursor();
    if (g_TrackGame->renderTarget->surfacePixelFormat == 8 && !userGui->guiPalette)
        return;
    defaultImage = !image;
    CreateCursor(image ? image : cursorImage, visible);
    if (defaultImage && cursorAnimation && userCursor)
        userCursor->SetAnimation(cursorAnimation);
    if (redraw)
        userGui->RedrawFrame();
}

// 0x004880c0
void GUIUser::ReleaseCursor() {
    if (!g_TrackGame->shuttingDown && userCursor)
        userCursor->Release();
    userCursor = 0;
}

// 0x00488110
UIDlgContainer::~UIDlgContainer() {
}

// 0x00488160
int GUIUser::UsesDevice(InputDevice* device) {
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
int GUIUser::UsesEventDevice(UnknownControlEvent* event) {
    if (event->kind == 0)
        return UsesDevice((InputDevice*)g_TrackGame->controlInterface->keyboard);
    if (event->kind == 1)
        return UsesDevice((InputDevice*)g_TrackGame->controlInterface->mouse);
    if (event->kind == 2 || event->kind == 3)
        return UsesDevice((InputDevice*)g_TrackGame->controlInterface->joysticks[event->device]);
    return 0;
}

// 0x00488240
GUIInputDevice* GUIUser::FindDevice(InputDevice* device) {
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
GUIInputDevice* GUIUser::FindEventDevice(UnknownControlEvent* event) {
    if (event->kind == 0)
        return FindDevice((InputDevice*)g_TrackGame->controlInterface->keyboard);
    if (event->kind == 1)
        return FindDevice((InputDevice*)g_TrackGame->controlInterface->mouse);
    if (event->kind == 2 || event->kind == 3)
        return FindDevice((InputDevice*)g_TrackGame->controlInterface->joysticks[event->device]);
    return 0;
}

// 0x00488310
GUIInputDevice* GUIUser::GetAcceptedDevice(int index) {
    return acceptedDevices.Get(index);
}

// 0x0067b468 (initializer 0x00488340; Game.h's g_HiResMeter points
// at it).
HiResMeter g_HiResMeterInstance(0);

// 0x00488380
HiResMeter::HiResMeter(int flags) : GameObject(flags) {
    overlayPage = -1;
    millisecondsPerCounterTick = 0;
    ClearTimers();
    if (SetRenderContext(0))
        counterUsable = 1;
    else
        counterUsable = 0;
}

// 0x00488430
HiResMeter::~HiResMeter() {
}

// 0x00488510
void HiResMeter::ClearTickTimes() {
    int i;

    for (i = 0; i < timerCount; i++) {
        timers[i].callsThisTick = 0;
        timers[i].msThisTick = 0;
    }
}

// 0x00488540
void HiResMeter::ResetTimerStats() {
    int i;

    for (i = 0; i < timerCount; i++) {
        timers[i].calls = 0;
        timers[i].totalMs = 0;
        timers[i].startStampLow = 0;
        timers[i].startStampHigh = 0;
        timers[i].callsThisTick = 0;
        timers[i].msThisTick = 0;
        timers[i].msAllTicks = 0;
        timers[i].ticks = 0;
        timers[i].peakMs = 0;
    }
    field_0x38 = 0;
}

// 0x00488580
void HiResMeter::ClearTimers() {
    int i;

    for (i = 0; i < 50; i++) {
        timers[i].calls = 0;
        timers[i].totalMs = 0;
        timers[i].startStampLow = 0;
        timers[i].startStampHigh = 0;
        timers[i].callsThisTick = 0;
        timers[i].msThisTick = 0;
        timers[i].msAllTicks = 0;
        timers[i].ticks = 0;
        timers[i].peakMs = 0;
        timers[i].name[0] = 0;
    }
    timerCount = 0;
    field_0x38 = 0;
}

// 0x004885c0
int HiResMeter::Tick(float frameTime) {
    UnknownHiResMeterEntry* entry;
    int i;

    if (g_TrackGame->debugOverlay) {
        if (overlayPage < 0)
            overlayPage = g_TrackGame->debugOverlay->NewPage();
        if (g_TrackGame->debugOverlay->shownPage != overlayPage) {
            ResetTimerStats();
            return 1;
        }
        g_TrackGame->debugOverlay->Title(
            overlayPage, "MS Timers [CallsThisTick-->TotMSThisTick (AvgMSPerTick)] : (Per-call Peak/Avg)");
        g_TrackGame->debugOverlay->Print(overlayPage, "Previous Game Tick: % 3d  MS",
                                                                g_TickTime);
        for (i = 0; i < timerCount; i++) {
            entry = &timers[i];
            if (entry->calls != 0.0f) {
                entry->ticks += 1.0f;
                entry->msAllTicks = entry->msAllTicks + entry->msThisTick;
                g_TrackGame->debugOverlay->Print(
                    overlayPage, "%24.24s [%4.0f --> %6.3f (%6.3f)] : (%6.3f / %5.3f) MS", entry->name,
                    entry->callsThisTick, entry->msThisTick, entry->msAllTicks / entry->ticks,
                    entry->peakMs, entry->totalMs / entry->calls);
            }
        }
    }
    return 1;
}
