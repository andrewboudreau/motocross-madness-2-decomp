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
        field_0x38 = field_0x5c->UnknownFunction4730b0();
    return GameCursor::UnknownVirtualSlot15();
}

// 0x00485190
GUIManager::GUIManager(int flags) : GameObject(flags) {
    int i;

    field_0x30 = 0;
    field_0x4c = 0;
    field_0xd4 = 0;
    field_0xd8 = 0;
    field_0x48 = 0;
    field_0xdc = 0;
    field_0xe4 = -1;
    field_0xe8 = 0;
    field_0xec = 1;
    field_0xf0[0] = 0;
    field_0x1f4 = 0;
    field_0x38 = 0;
    field_0x34 = 0;
    field_0x44 = 0;
    field_0x3c = 0;
    field_0x1fc = 0;
    field_0x348 = 0;
    field_0x3dc = 0;
    field_0x1f0 = 0;
    field_0x1f8 = 0;
    field_0x40 = 0;
    field_0x304 = 0;
    field_0x200[0] = 0;
    field_0x280[0] = 0;
    strcpy(field_0x280, "wait.tga");
    strcpy(field_0x200, "cursor.tga");
    field_0x308 = 0;
    field_0x30c = 0;
    for (i = 0; i < 8; i++)
        field_0x310[i] = 0;
    field_0x340 = 0;
    for (i = 0; i < 4; i++)
        field_0x330[i] = 0;
    field_0x344 = 0;
    field_0x2c = 0;
    field_0x34c = 0;
    strcpy(field_0x350, "");
    field_0x3d0 = 0;
    field_0x3d4 = 0;
    field_0x3d8 = 0;
    field_0xe0 = 0;
}

// 0x00485320
GUIManager::~GUIManager() {
    UnknownFunction486680();
    if (field_0x30)
        field_0x30->Release();
    UnknownFunction4860f0();
    if (field_0x1fc)
        DeleteObject((HGDIOBJ)field_0x1fc);
    if (field_0x3dc)
        field_0x3dc->UnknownMethod2();
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

    if (field_0x1f4)
        return 1;
    if (field_0xdc && field_0xe0) {
        rect.left = 0;
        rect.top = 0;
        rect.right = field_0xdc->field_0x14;
        rect.bottom = field_0xdc->field_0x18;
        if (field_0x3c) {
            field_0x3c->UnknownFunction404480(field_0xdc, &rect, 0, 0x1000000, field_0xe4, 1, &field_0xe8, 0);
        } else if (!((RenderTarget*)field_0x18)->UnknownVirtualSlot3(&rect, field_0xdc, 0, 0x1000000)) {
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

    if (!field_0x1f4) {
        if (field_0xdc && !field_0xe0) {
            rect.left = 0;
            rect.top = 0;
            rect.right = field_0xdc->field_0x14;
            rect.bottom = field_0xdc->field_0x18;
            if (field_0x3c) {
                field_0x3c->UnknownFunction404480(field_0xdc, &rect, 0, 0x1000000, field_0xe4, 1, &field_0xe8, 0);
            } else if (!((RenderTarget*)field_0x18)->UnknownVirtualSlot3(&rect, field_0xdc, 0, 0x1000000)) {
                GameObject::UnknownVirtualSlot15();
                return 0;
            }
        }
        GameObject::UnknownVirtualSlot15();
    } else {
        field_0x1f4 = field_0x1f4 - 1 < 0 ? 0 : field_0x1f4 - 1;
        g_UnknownGlobal56e26c->field_0x10->UnknownVirtualSlot12(0, 0);
        if (!field_0x1f4 && field_0x40) {
            field_0x40->UnknownFunction404da0();
            field_0x40 = 0;
        }
    }
    if (field_0x3c)
        field_0x3c->UnknownFunction404c80();
    return 1;
}

// 0x00485bd0
void GUIManager::UnknownFunction485bd0(UnknownGuiDialog* dialog, int a, int wait) {
    int visible = dialog->field_0x25_bit0;

    dialog->UnknownFunction46ea60(0);
    if (dialog->field_0x2c && (dialog->field_0x144 & 1))
        dialog->field_0x2c->field_0x7f3c->UnknownFunction469130(dialog, -1);
    else
        field_0x344->UnknownFunction469190(dialog, -1);
    dialog->UnknownVirtualSlot28(a);
    dialog->field_0xc8 = field_0xec;
    if (field_0x3c)
        field_0x3c->UnknownFunction404da0();
    if (wait)
        UnknownFunction4865e0(field_0x200, field_0x1f0 == 0);
    if (field_0x1f0)
        field_0x1f4 = 3;
    dialog->UnknownFunction46ea60(visible);
}

// 0x00485d50
void GUIManager::UnknownFunction485d50() {
    if (field_0x30) {
        field_0x30->Release();
        field_0x30 = 0;
    }
}

// 0x00485d70
void GUIManager::UnknownFunction485d70(const char* directory) {
    int last = strlen(directory) - 1;

    strcpy(field_0xf0, directory);
    if (directory[last] != '\\')
        strcat(field_0xf0, "\\");
}

// 0x00485df0
UnknownGuiDialog* GUIManager::UnknownFunction485df0() {
    UnknownGuiDialog* found = 0;
    UnknownGuiDialog* dialog;
    GameObjectIterator iterator(this, 1, "UIDialog");

    while ((dialog = (UnknownGuiDialog*)iterator.Next()) != 0) {
        if (!found)
            found = dialog;
        if (field_0x308 && PtInRect(&dialog->field_0x160, field_0x308->field_0xa4))
            found = dialog;
        if (dialog->field_0x148)
            found = dialog;
    }
    return found;
}

// 0x00485ec0
int GUIManager::UnknownFunction485ec0(int value) {
    if (field_0x30)
        return field_0x30->UnknownFunction46e9a0(value);
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

    if (field_0x3c)
        return;
    field_0x3c = (BackgroundImage*)UnknownFunction469190(
        (new(__FILE__, 672) BackgroundImage(1))->UnknownVirtualSlot8(field_0x18), -1);
    field_0x3c->UnknownFunction469260(field_0x344, -1);
    field_0x48 = 1;
    UnknownFunction4865e0(0, 1);
    for (i = 0; i < field_0x340; i++)
        UnknownFunction486540(i)->UnknownFunction487680(field_0x18, this);
}

// 0x00485fc0
void GUIManager::UnknownFunction485fc0() {
    UnknownGuiDialog* dialog;
    int i;

    if (!field_0x48)
        return;
    UnknownFunction486680();
    if (field_0x3c)
        field_0x3c->Release();
    field_0x3c = 0;
    UnknownFunction4865e0(0, 0);
    GameObjectIterator iterator(this, 1, "UIDialog");
    while ((dialog = (UnknownGuiDialog*)iterator.Next()) != 0)
        dialog->UnknownFunction46ffd0(0);
    for (i = 0; i < field_0x340; i++)
        UnknownFunction486540(i)->UnknownFunction487680(field_0x18, this);
}

// 0x004860a0
void GUIManager::UnknownFunction4860a0(int dim) {
    UnknownFunction4860f0();
    field_0xdc = UnknownFunction486170(dim, 0);
    if (field_0x3c) {
        field_0xe4 = field_0x3c->UnknownFunction4040f0(1);
        field_0x3c->UnknownFunction404da0();
        field_0x3c->field_0x30 = 0;
    }
}

// 0x004860f0
void GUIManager::UnknownFunction4860f0() {
    if (field_0xdc)
        field_0xdc->Release();
    if (field_0x3c && field_0xe4 >= 0) {
        field_0x3c->UnknownFunction404200(field_0xe4);
        field_0x3c->field_0x30 = 1;
    }
    field_0xdc = 0;
    field_0xe4 = -1;
    field_0xe8 = 0;
}

// 0x00486150
void GUIManager::UnknownFunction486150(Palette8* palette) {
    field_0x1f0 = 1;
    field_0x34 = palette;
}

// 0x004864f0
int GUIManager::UnknownFunction4864f0() {
    return field_0x1f8;
}

// 0x00486500
void GUIManager::UnknownFunction486500() {
    if (field_0x3c) {
        field_0x3c->UnknownFunction404c80();
        field_0x3c->UnknownFunction404da0();
    }
    g_UnknownGlobal56e26c->UnknownVirtualSlot8();
    g_UnknownGlobal56e26c->UnknownVirtualSlot9();
    g_UnknownGlobal56e26c->field_0x10->UnknownFunction4e8cc0();
}

// 0x00486540
GUIUser* GUIManager::UnknownFunction486540(int index) {
    if (index < 4)
        return field_0x330[index];
    return 0;
}

// 0x00486560
void GUIManager::UnknownFunction486560(const char* image) {
    strcpy(field_0x280, image);
}

// 0x00486590
void GUIManager::UnknownFunction486590(const char* image, int visible) {
    int i;

    for (i = 0; i < field_0x340; i++) {
        if (field_0x330[i])
            field_0x330[i]->UnknownFunction487dd0(image ? image : field_0x200, visible);
    }
}

// 0x004865e0
void GUIManager::UnknownFunction4865e0(const char* image, int redraw) {
    int i;

    for (i = 0; i < field_0x340; i++) {
        if (field_0x330[i])
            field_0x330[i]->UnknownFunction488010(image ? image : field_0x200, redraw);
    }
}

// 0x00486630
void GUIManager::UnknownFunction486630(int show) {
    int count = field_0x340;
    GUIUser* user;
    int i;

    for (i = 0; i < count; i++) {
        user = UnknownFunction486540(i);
        if (user && user->field_0x30) {
            if (show)
                user->field_0x30->UnknownVirtualSlot5();
            else
                user->field_0x30->UnknownVirtualSlot4();
        }
    }
}

// 0x00486680
void GUIManager::UnknownFunction486680() {
    int i;

    for (i = 0; i < field_0x340; i++) {
        if (field_0x330[i])
            field_0x330[i]->UnknownFunction4880c0();
    }
}

// 0x004866c0
void GUIManager::UnknownFunction4866c0(const char* font) {
    strcpy(field_0x350, font);
    if (field_0x1fc)
        DeleteObject((HGDIOBJ)field_0x1fc);
    field_0x1fc = CreateFontA(12, 0, 0, 0, 400, 0, 0, 0, 1, 0, 0, 2, 2, font);
}

// A pixel of 0x00486740's 24-bit fill.
struct UnknownGuiRgb {
    unsigned char field_0x00;
    unsigned char field_0x01;
    unsigned char field_0x02;
};

// 0x00486740
PCTextureMap* GUIManager::UnknownFunction486740(int width, int height, unsigned long color) {
    PCTextureMap* texture = new(__FILE__, 1140) PCTextureMap(field_0xd8, 1);
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
                                         (UnknownTexturePalette*)(field_0x34 ? field_0x34->field_0x708 : 0), 4,
                                         field_0x34 ? field_0x34->field_0x70c : 0, 1, 0, 2, 1, 0, 0x80, 0xff00ff))
            texture->UnknownVirtualSlot18(0xff00ff);
        delete bits;
    }
    return texture;
}

// 0x004868b0
int GUIManager::UnknownFunction4868b0(int enable) {
    UnknownDisplay* display;

    if (enable) {
        if (field_0x3dc) {
            field_0x3dc->UnknownMethod7(0, 0);
            field_0x3dc->UnknownMethod2();
        }
        ((UnknownGuiDirectDraw*)g_UnknownGlobal56e26c->field_0x0c->field_0x190)
            ->UnknownMethod4(0, &field_0x3dc, 0);
        ((UnknownGuiSurface*)g_UnknownGlobal56e26c->field_0x0c->field_0x19c)->UnknownMethod28(field_0x3dc);
        if (field_0x3dc)
            field_0x3dc->UnknownMethod8(0, g_UnknownGlobal56e26c->field_0x31c);
    } else {
        if (field_0x3dc) {
            field_0x3dc->UnknownMethod7(0, 0);
            field_0x3dc->UnknownMethod2();
            field_0x3dc = 0;
        }
        ((UnknownGuiSurface*)g_UnknownGlobal56e26c->field_0x0c->field_0x19c)->UnknownMethod28(0);
    }
    display = g_UnknownGlobal56e26c->field_0x0c;
    display->UnknownFunction4cb5b0(enable);
    return 1;
}

// 0x00486990
ToolTip::ToolTip(int flags) : GameObject(flags) {
    field_0x64 = 0;
    field_0x30 = 0;
    field_0x38 = 0;
    field_0x3c.left = field_0x3c.top = field_0x3c.right = field_0x3c.bottom = 0;
    field_0x4c.left = field_0x4c.top = field_0x4c.right = field_0x4c.bottom = 0;
    field_0x34 = -1;
    field_0x60 = -1.0f;
    field_0x5c = 1;
}

// 0x00486a10
ToolTip* ToolTip::UnknownFunction486a10(void* target, GUIManager* gui) {
    GameObject::UnknownVirtualSlot8(target);
    field_0x2c = gui;
    return this;
}

// 0x00486a30
ToolTip::~ToolTip() {
    if (field_0x30)
        field_0x30->Release();
    if (field_0x2c && field_0x2c->field_0x3c && field_0x34 > -1)
        field_0x2c->field_0x3c->UnknownFunction404200(field_0x34);
}

// 0x00486ab0
int ToolTip::UnknownVirtualSlot10(float frameTime) {
    if (field_0x5c && field_0x60 > 0.0f) {
        field_0x60 -= frameTime;
        if (field_0x60 > 0.0f)
            return 1;
        if (field_0x30) {
            field_0x64 = 1;
            return 1;
        }
    } else if (field_0x5c) {
        return 1;
    }
    field_0x64 = 0;
    return 1;
}

// 0x00486db0
int ToolTip::UnknownVirtualSlot15() {
    CameraRect rect;

    if (field_0x64) {
        if (field_0x2c && field_0x2c->field_0x3c) {
            field_0x2c->field_0x3c->UnknownFunction404480(field_0x30, &field_0x3c, &field_0x4c, 0x1008000, field_0x34,
                                                         0, &field_0x38, 0);
            return 1;
        }
        rect.left = field_0x3c.left;
        rect.top = field_0x3c.top;
        rect.right = field_0x3c.left + field_0x4c.right;
        rect.bottom = field_0x3c.top + field_0x4c.bottom;
        if (!((RenderTarget*)field_0x18)->UnknownVirtualSlot3(&rect, field_0x30, &field_0x4c, 0x1008000))
            return 0;
    } else if (field_0x2c && field_0x2c->field_0x3c && field_0x34 > -1) {
        if (field_0x38 > 0) {
            field_0x2c->field_0x3c->UnknownFunction404240(field_0x34, &field_0x3c);
            field_0x38--;
            return 1;
        }
        field_0x2c->field_0x3c->UnknownFunction404cb0(field_0x34);
    }
    return 1;
}

// 0x00486e80
GUIInputDevice::GUIInputDevice() : GameObject(1) {
    field_0xb0 = 0;
    field_0xb4 = 0;
    field_0xb8 = 0;
    field_0xbc = 0;
    field_0xc0 = 0;
}

// 0x00486f20
GUIInputDevice::~GUIInputDevice() {
    if (field_0xac->deviceKind) {
        field_0x2c.UnknownFunction43cde0();
        field_0x68.UnknownFunction43cde0();
    }
}

// 0x00486fb0
GUIInputDevice* GUIInputDevice::UnknownFunction486fb0(void* target, InputDevice* device, float minX, float maxX,
                                                      float minY, float maxY) {
    float rangeX;
    float rangeY;

    GameObject::UnknownVirtualSlot8(target);
    field_0xac = device;
    field_0xb0 = minX;
    field_0xb4 = maxX;
    field_0xb8 = minY;
    field_0xbc = maxY;
    rangeX = maxX;
    rangeY = maxY;
    if (minX == 0.0f && maxX == 0.0f && minY == 0.0f && maxY == 0.0f) {
        rangeX = (float)(((RenderTarget*)field_0x18)->field_0x0c - 1);
        rangeY = (float)(((RenderTarget*)field_0x18)->field_0x10 - 1);
    }
    field_0x2c = UnknownControlBinding(field_0xb0, rangeX, 0.0f,
                                       g_UnknownGlobal56e26c->field_0x14->UnknownFunction43ce90(), 0, 0.0f);
    field_0x68 = UnknownControlBinding(field_0xb8, rangeY, 0.0f,
                                       g_UnknownGlobal56e26c->field_0x14->UnknownFunction43ce90(), 1, 0.0f);
    if (field_0xac->deviceKind == 2) {
        ((JoystickDevice*)field_0xac)->UnknownFunction489a20(&field_0x2c, 0.01f, 0.01f);
        ((JoystickDevice*)field_0xac)->UnknownFunction489a20(&field_0x68, 0.01f, 0.01f);
    } else if (field_0xac->deviceKind == 1) {
        ((MouseDevice*)field_0xac)->UnknownFunction48a420(&field_0x2c);
        ((MouseDevice*)field_0xac)->UnknownFunction48a420(&field_0x68);
    }
    return this;
}

// 0x00487150
void GUIInputDevice::UnknownFunction487150() {
    float rangeX;
    float rangeY;
    int x = (int)field_0x2c.field_0x24;
    int y = (int)field_0x68.field_0x24;

    if (!field_0xac)
        return;
    if (field_0xac->deviceKind == 2 || field_0xac->deviceKind == 1) {
        field_0xac->UnknownVirtualSlot0(field_0x2c.field_0x04);
        field_0xac->UnknownVirtualSlot0(field_0x68.field_0x04);
    }
    rangeX = field_0xb4;
    rangeY = field_0xbc;
    if (field_0xb0 == 0.0f && field_0xb4 == 0.0f && field_0xb8 == 0.0f && field_0xbc == 0.0f) {
        rangeX = (float)(((RenderTarget*)field_0x18)->field_0x0c - 1);
        rangeY = (float)(((RenderTarget*)field_0x18)->field_0x10 - 1);
    }
    field_0x2c = UnknownControlBinding(field_0xb0, rangeX, (float)x,
                                       g_UnknownGlobal56e26c->field_0x14->UnknownFunction43ce90(), 0, 0.0f);
    field_0x68 = UnknownControlBinding(field_0xb8, rangeY, (float)y,
                                       g_UnknownGlobal56e26c->field_0x14->UnknownFunction43ce90(), 1, 0.0f);
    if (field_0xac->deviceKind == 2) {
        ((JoystickDevice*)field_0xac)->UnknownFunction489a20(&field_0x2c, 0.01f, 0.01f);
        ((JoystickDevice*)field_0xac)->UnknownFunction489a20(&field_0x68, 0.01f, 0.01f);
    } else if (field_0xac->deviceKind == 1) {
        ((MouseDevice*)field_0xac)->UnknownFunction48a420(&field_0x2c);
        ((MouseDevice*)field_0xac)->UnknownFunction48a420(&field_0x68);
    }
}

// 0x00487320
int GUIInputDevice::UnknownVirtualSlot10(float frameTime) {
    int kind;

    GameObject::UnknownVirtualSlot10(frameTime);
    kind = field_0xac->deviceKind;
    if (kind) {
        if (g_UnknownGlobal56e26c->field_0x2d4_bit1) {
            POINT* position = &field_0xa4;
            position->x = (int)field_0x2c.field_0x24;
            position->y = (int)field_0x68.field_0x24;
            return 1;
        }
        if (kind == 1 && field_0xc0 && field_0xc0->field_0x30) {
            field_0xa4.x = field_0xc0->field_0x30->field_0x54;
            field_0xa4.y = field_0xc0->field_0x30->field_0x58;
        }
    }
    return 1;
}

// 0x004873b0
int GUIInputDevice::UnknownVirtualSlot25(void* value) {
    GameObject::UnknownVirtualSlot25(value);
    UnknownFunction487150();
    return 1;
}

// 0x004873d0
GUIUser::GUIUser() : GameObject(1) {
    int i;

    field_0xbc = 0;
    field_0xc0 = 0;
    field_0x30 = 0;
    field_0x1c8 = 0;
    field_0xc8[0] = 0;
    field_0x148[0] = 0;
    field_0xc4 = 0;
    strcpy(field_0x148, "wait.tga");
    strcpy(field_0xc8, "cursor.tga");
    field_0x1d4 = 0;
    field_0x1d8 = 0;
    field_0x1cc = 0;
    field_0x1d0 = 0;
    field_0x1e0.Init(1, 1);
    field_0x2c = 0;
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
    if (field_0x2c) {
        field_0x34 = field_0x2c->field_0xa4.x;
        field_0x38 = field_0x2c->field_0xa4.y;
    }
    return 1;
}

// 0x00487650
GUIUser* GUIUser::UnknownFunction487650(void* target, GUIManager* gui) {
    GameObject::UnknownVirtualSlot8(target);
    field_0xbc = gui;
    UnknownFunction487680(target, gui);
    return this;
}

// 0x00487680
void GUIUser::UnknownFunction487680(void* target, GUIManager* gui) {
    UnknownFunction487710();
    field_0xc0 = (ToolTip*)UnknownFunction469190(
        (new(__FILE__, 1639) ToolTip(1))->UnknownFunction486a10(target, gui), -1);
}

// 0x00487710
void GUIUser::UnknownFunction487710() {
    if (field_0xc0)
        field_0xc0->Release();
    field_0xc0 = 0;
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
void GUIUser::UnknownFunction487990(int enable) {
    POINT position;
    RECT area;
    COMPOSITIONFORM composition;
    LOGFONTA font;
    HWND window;
    const char* face;
    UnknownGuiControl* control;

    if (enable) {
        ImmAssociateContext((HWND)g_UnknownGlobal56e26c->field_0x31c, (HIMC)g_UnknownGlobal56e26c->field_0x53c);
        area = field_0x1d4->field_0x214;
        composition.dwStyle = CFS_FORCE_POSITION;
        composition.ptCurrentPos.x = area.left;
        composition.ptCurrentPos.y = area.top;
        ImmSetCompositionWindow((HIMC)g_UnknownGlobal56e26c->field_0x53c, &composition);
        font.lfWidth = 0;
        font.lfQuality = 2;
        font.lfPitchAndFamily = 2;
        control = field_0x1d4;
        font.lfEscapement = 0;
        font.lfOrientation = 0;
        font.lfUnderline = 0;
        font.lfStrikeOut = 0;
        font.lfCharSet = 1;
        font.lfOutPrecision = 0;
        font.lfClipPrecision = 0;
        font.lfItalic = 0;
        if (control->field_0x130[0]) {
            face = control->field_0x130;
            font.lfHeight = control->field_0x160;
            font.lfWeight = control->field_0x158 ? 700 : 500;
        } else if (control->field_0xb8->field_0xe0[0]) {
            face = control->field_0xb8->field_0xe0;
            font.lfHeight = control->field_0xb8->field_0xdc;
            font.lfWeight = control->field_0xb8->field_0x108 ? 700 : 500;
        } else {
            face = control->field_0xbc->field_0x50;
            font.lfHeight = control->field_0xbc->field_0xd0;
            font.lfWeight = control->field_0xbc->field_0x3d4 ? 700 : 500;
            font.lfItalic = (BYTE)control->field_0xbc->field_0x3d8;
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
    if (field_0x1d4 && !field_0x1d4->field_0x25_bit3)
        field_0x1d4->UnknownVirtualSlot33(0);
    if (previous)
        *previous = field_0x1d4;
    field_0x1d4 = 0;
    return 1;
}

// 0x00487c30
int GUIUser::UnknownFunction487c30(GUIInputDevice* device) {
    if (device->field_0xc0) {
        if (device->field_0xc0 != this)
            return 0;
    } else {
        field_0x1e0.Add(device);
        device->field_0xc0 = this;
    }
    return 1;
}

// 0x00487d00
void GUIUser::UnknownFunction487d00() {
    int i;

    if (field_0xbc->field_0x30c)
        UnknownFunction487c30(field_0xbc->field_0x30c);
    for (i = 0; i < g_UnknownGlobal56e26c->field_0x14->joystickCount; i++)
        UnknownFunction487c30(field_0xbc->field_0x310[i]);
}

// 0x00487d60
void GUIUser::UnknownFunction487d60() {
    GUIInputDevice* device;
    int i = 0;

    while ((device = field_0x1e0.Get(i++)) != 0)
        device->field_0xc0 = 0;
    field_0x1e0.Clear();
}

// 0x00487dd0
void GUIUser::UnknownFunction487dd0(const char* image, int visible) {
    void* palette;

    if (field_0xbc && field_0xbc->field_0x3c)
        field_0xbc->field_0x3c->UnknownFunction404c80();
    if (field_0x30 || !field_0x2c)
        return;
    if (g_UnknownGlobal56e26c->field_0x2d4_bit1) {
        if (g_UnknownGlobal56e26c->field_0x10->field_0x28 > 8 ||
            (g_UnknownGlobal56e26c->field_0x10->field_0x28 == 8 && field_0xbc->field_0x34)) {
            palette = field_0xbc->field_0x34 ? field_0xbc->field_0x34->field_0x708 : 0;
            if (!image)
                image = field_0xc8;
            field_0x30 = (GUICursor*)UnknownFunction469190(
                (new(__FILE__, 1890) GUICursor(visible))
                    ->UnknownFunction43ea70(field_0x18, &field_0x2c->field_0x2c, &field_0x2c->field_0x68, image,
                                            field_0xbc->field_0xd8, field_0xbc->field_0x3c, palette,
                                            field_0xbc->field_0x34),
                -1);
        }
    } else {
        if (g_UnknownGlobal56e26c->field_0x10->field_0x28 > 8 ||
            (g_UnknownGlobal56e26c->field_0x10->field_0x28 == 8 && field_0xbc->field_0x34)) {
            palette = field_0xbc->field_0x34 ? field_0xbc->field_0x34->field_0x708 : 0;
            if (!image)
                image = field_0xc8;
            field_0x30 = (GUICursor*)UnknownFunction469130(
                (new(__FILE__, 1898) GUICursor(visible))
                    ->UnknownFunction43eaf0(field_0x18, image, field_0xbc->field_0xd8, field_0xbc->field_0x3c,
                                            palette, field_0xbc->field_0x34),
                -1);
        }
    }
}

// 0x00487fb0
void GUIUser::UnknownFunction487fb0(UnknownCursorAnimation* animation) {
    UnknownCursorAnimation* current;

    if (!field_0x30)
        return;
    if (!animation && field_0x1c8)
        animation = field_0x1c8;
    current = field_0x30->field_0x5c;
    if (current && field_0xc4 && animation != current && animation->field_0x10 > current->field_0x08) {
        animation->field_0x08 = current->field_0x08;
        animation->field_0x0c = current->field_0x0c;
        animation->field_0x20 = current->field_0x20;
    }
    field_0x30->UnknownFunction485150(animation);
}

// 0x00488010
void GUIUser::UnknownFunction488010(const char* image, int redraw) {
    int visible;
    int defaultImage;

    if (g_UnknownGlobal56e26c->field_0x2d5_bit1 || !field_0x2c)
        return;
    visible = field_0x30 ? field_0x30->field_0x25_bit0 : 1;
    UnknownFunction4880c0();
    if (g_UnknownGlobal56e26c->field_0x10->field_0x28 == 8 && !field_0xbc->field_0x34)
        return;
    defaultImage = !image;
    UnknownFunction487dd0(image ? image : field_0xc8, visible);
    if (defaultImage && field_0x1c8 && field_0x30)
        field_0x30->UnknownFunction485150(field_0x1c8);
    if (redraw)
        field_0xbc->UnknownFunction486500();
}

// 0x004880c0
void GUIUser::UnknownFunction4880c0() {
    if (!g_UnknownGlobal56e26c->field_0x2d5_bit1 && field_0x30)
        field_0x30->Release();
    field_0x30 = 0;
}

// 0x00488110
UIDlgContainer::~UIDlgContainer() {
}

// 0x00488160
int GUIUser::UnknownFunction488160(InputDevice* device) {
    GUIInputDevice* candidate;
    int i;

    if (!field_0x2c || field_0x2c->field_0xac == device)
        return 1;
    i = 0;
    while ((candidate = field_0x1e0.Get(i++)) != 0) {
        if (candidate->field_0xac == device)
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

    if (!field_0x2c || field_0x2c->field_0xac == device)
        return field_0x2c;
    i = 0;
    while ((candidate = field_0x1e0.Get(i++)) != 0) {
        if (candidate->field_0xac == device)
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
    return field_0x1e0.Get(index);
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
