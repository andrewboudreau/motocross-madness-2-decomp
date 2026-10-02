// SurfaceMap.cpp -- SurfaceMap (0x005051f0..0x00505350), a texture plus material wrapper.
// Placement and ownership: see SurfaceMap.h.
#include "SurfaceMap.h"

SurfaceMap::SurfaceMap(SurfaceTexture* tex, const SurfaceMaterial* mat)
{
    texture = tex;
    if (tex)
        tex->BaseObjectVirtualSlot1();
    if (mat) {
        material = *mat;
    } else {
        material.specular[0] = 0.0f;
        material.diffuse[0] = 1.0f;
        material.diffuse[1] = 1.0f;
        material.diffuse[2] = 1.0f;
        material.diffuse[3] = 1.0f;
        material.ambient[0] = 1.0f;
        material.ambient[1] = 1.0f;
        material.ambient[2] = 1.0f;
        material.ambient[3] = 1.0f;
        material.specular[1] = 0.0f;
        material.specular[2] = 0.0f;
        material.specular[3] = 0.0f;
        material.emissive[0] = 0.0f;
        material.emissive[1] = 0.0f;
        material.emissive[2] = 0.0f;
        material.emissive[3] = 0.0f;
        material.power = 0.0f;
    }
    Apply();
}

SurfaceMap::~SurfaceMap()
{
    if (texture)
        texture->BaseObjectVirtualSlot2();
}

void SurfaceMap::SetTexture(SurfaceTexture* tex)
{
    if (texture)
        texture->BaseObjectVirtualSlot2();
    texture = tex;
    if (tex)
        tex->BaseObjectVirtualSlot1();
    Apply();
}

int SurfaceMap::Apply()
{
    return g_pGame->display->device->SetMaterial(&material) == 0;
}

void SurfaceMap::Select()
{
    Apply();
    if (texture) {
        texture->Bind();
        return;
    }
    g_pGame->display->Slot7(0, 1, 1);
    g_pGame->display->Slot7(0, 4, 1);
}
