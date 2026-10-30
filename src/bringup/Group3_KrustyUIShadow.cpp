// Bring-up definition of the projected shadow's Init (retail 0x004da7b0,
// D:\aardvark\VC\krusty2\ProjectedShadow.cpp), under the name KrustyUI.cpp's
// garage scene calls it by: UnknownKrustyUIShadow::Init.
//
// Status: best-guess-unreviewed bring-up body. The flow follows the near miss
// ProjectedShadow::Init in samples/physics/shadow/ProjectedShadowNearMisses.cpp
// (841 of 842 bytes, block layout matching) and was re-checked against the
// retail disassembly; it is not a strict match. KrustyUI.cpp allocates a
// ProjectedShadow (0x134 bytes) and reinterprets it as UnknownKrustyUIShadow, so
// the body reads the object through the canonical ProjectedShadow layout. The
// callees use their canonical (defined) names: PCTextureMap's constructor
// 0x004c5f00, ShadowCamera's constructor 0x004da520, Camera::SetFieldOfView
// 0x0042e930 and RenderTarget::SetCamera 0x004e8cf0.
#include <stdio.h>
#include <string.h>

#include "shadow/ProjectedShadow.h"
#include "PCTextureMap.h"
#include "ShadowCamera.h"
#include "RenderTarget.h"
#include "TextureMap.h"

// KrustyUI.cpp's view of the shadow object (declared there in the .cpp, so it is
// repeated here with the same class key, base and size; the mangled name of Init
// depends only on the class name and signature).
class UnknownKrustyUIShadow : public GameObject {
public:
    UnknownKrustyUIShadow* Init(void* owner, TextureMapManager* textures);
    unsigned char field_0x2c[0x134 - 0x2c];
};

// The retail DebugMalloc tags name ProjectedShadow.cpp (string 0x00571fd8).
static const char kProjectedShadowFile[] = "D:\\aardvark\\VC\\krusty2\\ProjectedShadow.cpp";

// 0x004da7b0: binds the shadow to its render target, picks a surface format from
// the device capabilities (and the DriverInfo\<driver>\DestColorShadow setting),
// creates the 128x128 shadow texture and its camera, retargeting the render
// target at the texture while the camera binds. Returns this, or releases the
// object and returns 0 when no usable format or allocation exists.
UnknownKrustyUIShadow* UnknownKrustyUIShadow::Init(void* owner, TextureMapManager* textures)
{
    ProjectedShadow* self = reinterpret_cast<ProjectedShadow*>(this);
    ShadowRenderTarget* target = static_cast<ShadowRenderTarget*>(owner);

    GameObject::SetRenderContext(owner);          // non-virtual call 0x004692f0
    self->field_0x5c = (int)target;

    char name[0x100];
    sprintf(name, "DriverInfo\\%s\\DestColorShadow", self->Host()->display->driverName);
    if (reinterpret_cast<ShadowSettings*>(g_TrackGame)->QueryFlag(name, 0)
        && (self->Host()->capsA & 0x100) && (self->Host()->capsB & 1)) {
        self->formatA = 9;
        self->formatB = 1;
    } else {
        unsigned int caps = self->Host()->capsA;
        if ((caps & 0x10) && (self->Host()->capsB & 0x20)) {
            self->formatA = 5;
            self->formatB = 6;
        } else if ((caps & 1) && (self->Host()->capsB & 0x10)) {
            self->formatA = 1;
            self->formatB = 5;
        } else if (caps & 0x800) {
            self->formatA = 0xc;
            self->formatB = 2;
        } else {
            Release();
            return 0;
        }
    }

    if (self->Host()->IsFormatSupported(0x115c)) {
        self->surfaceFormat = 0x115c;
    } else if (self->Host()->IsFormatSupported(0x613) && (self->Host()->capsC & 0x2000)) {
        self->surfaceFormat = 0x613;
        self->field_0x98 = 0;
    } else {
        Release();
        return 0;
    }

    // operator new(0x80, file, 0xaa) + PCTextureMap(textures, 1); the texture's
    // virtual slots are reached through the shadow code's ShadowTexture view.
    PCTextureMap* map = new(kProjectedShadowFile, 0xaa) PCTextureMap(textures, 1);
    ShadowTexture* texture = reinterpret_cast<ShadowTexture*>(map);
    self->texture = texture;
    if (texture
        && texture->Create(0, 0x80, 0x80, 0x80, 0x80, self->surfaceFormat, self->surfaceFormat, 0, 8,
                           0, 0, 0, self->formatA, self->formatB, 0, 0x80, 0xff00ff)) {
        texture->UnknownVirtualSlot8(1, 0, 1);
        self->field_0x60 = texture->pixels;
        if (self->field_0x60) {
            texture->surface->PageLock(0);

            // Point the render target at the texture while the camera binds to it.
            ShadowRenderTarget* host = self->Host();
            int savedHeight = host->height;
            void* savedSurface = host->surface;
            int savedWidth = host->width;
            host->width = texture->size;
            self->Host()->height = texture->height;
            self->Host()->surface = texture->pixels;
            ShadowOwner* savedCamera = self->Host()->owner;

            ShadowCamera* camera = new(kProjectedShadowFile, 0xcd) ShadowCamera(1);
            self->camera = reinterpret_cast<ShadowCameraProxy*>(camera);
            if (camera && camera->SetRenderContext(self->Host())) {
                self->Host()->surface = savedSurface;
                self->Host()->width = savedWidth;
                self->Host()->height = savedHeight;
                // 0x004e8cf0: restore the render target's previous camera.
                reinterpret_cast<RenderTarget*>(self->Host())->SetCamera(reinterpret_cast<Camera*>(savedCamera));
                self->Host()->device->SetTransformLike(savedCamera->block_0x1a0);
                camera->SetFieldOfView(0.3f);
                for (int bits = 0x40; bits; bits >>= 1)
                    self->sizeShift++;
                self->clipRect.left = 0;
                self->clipRect.top = 0;
                self->clipRect.right = texture->size - 1;
                self->clipRect.bottom = texture->size - 1;
                return this;
            }
        }
    }
    Release();
    return 0;
}
