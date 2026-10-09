#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SoultreeMaterial.h"
#include "DebugAlloc.h"
#include "D3DConstants.h"

// 0x00574060..0x00574068: the colour key used when the render target can key
// its own format (initialised data of this TU).
int g_UnknownInt574060 = 0x80;
int g_UnknownInt574064 = 0x80;
int g_UnknownInt574068 = 0x80;

// 0x00689ed0: texture stage 0 state 0xc, saved while a material clamps.
int g_UnknownInt689ed0;

// 0x004fefe0
SoultreeMaterial::SoultreeMaterial(int flags)
    : GameObject(flags)
{
    strcpy(textureName, "");
    mipMapped = 1;
    textureManager = 0;
    textureName[0] = 0;
    hasTextureName = 0;
    hasColorKey = 0;
    hasSourceBlend = 0;
    hasDestBlend = 0;
    field_0x70 = 0;
    mappingType = 0;
    hasAlpha = 0;
    field_0xcc = 0;
    textureSpeed = 0.0f;
    useVertexColor = 0;
    clampTexture = 0;
    materialAlpha = 1.0f;
    field_0xb8 = 1;
}

// 0x004ff0b0: returns this, like GameObject slot 8.
SoultreeMaterial* SoultreeMaterial::Attach(RenderTarget* target, const SoultreeTextureOptions* options,
                                             TextureMapManager* manager, int format)
{
    GameObject::UnknownVirtualSlot8(target);
    strcpy(textureName, "");
    textureManager = manager;
    field_0x18 = target;
    field_0x70 = 0;
    field_0x74 = *options;
    textureFormat = format;
    return this;
}

// 0x004ff120 (deleting wrapper 0x004ff090)
SoultreeMaterial::~SoultreeMaterial()
{
    if (field_0x70) {
        field_0x70->Release();
        field_0x70 = 0;
    }
}

// 0x004ff180: the render states of the material. Cases 3 and 6 of the
// mapping-type switch are separate identical statements that VC6 merges
// (written as one case label pair, the last case picks the other vtable
// register).
void SoultreeMaterial::ApplyRenderStates()
{
    if (field_0x70) {
        field_0x70->Select();
        int format = field_0x70->texture->field_0x20;
        if (format != 1555 && format != 4444 && format != 8888) {
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHATESTENABLE, 0, 0);
        } else if (hasColorKey) {
            if (hasAlpha)
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot18(0);
            else
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot18(0x80);
        } else {
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot18(0);
        }
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        if (format == 4444 || format == 8888 || (format == 1555 && hasAlpha)) {
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 1, 0);
            if (!hasAlpha && mappingType != 6) {
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            } else {
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 1, 0);
            }
        } else if (hasAlpha) {
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 1, 0);
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        } else {
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 0, 0);
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        }
    } else {
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot11(0);
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
        if (hasAlpha) {
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 1, 0);
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        } else {
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 0, 0);
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        }
    }
    if (clampTexture) {
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot6(0, 0xc, &g_UnknownInt689ed0);
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);
    }
    switch (mappingType) {
    case 3:
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 1, 0);
        break;
    case 4:
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 1, 0);
        break;
    case 6:
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 1, 0);
        break;
    case 8:
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_WRAP0, 1, 0);
        break;
    }
}

// 0x004ff410
void SoultreeMaterial::RestoreRenderStates()
{
    if (mappingType == 8)
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_WRAP0, 0, 0);
    if (clampTexture)
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ADDRESS, g_UnknownInt689ed0);
}

// 0x004ff450
void SoultreeMaterial::ReadSaved(UnknownParameterStream* stream)
{
    stream->UnknownFunction461640(textureName, 0x40, 1);
    stream->UnknownFunction461640(&textureFormat, 4, 1);
    stream->UnknownFunction461640(&colorKey, 4, 1);
    stream->UnknownFunction461640(&sourceBlend, 4, 1);
    stream->UnknownFunction461640(&destBlend, 4, 1);
    stream->UnknownFunction461640(&hasTextureName, 4, 1);
    stream->UnknownFunction461640(&useVertexColor, 4, 1);
    stream->UnknownFunction461640(&mipMapped, 4, 1);
    stream->UnknownFunction461640(&hasColorKey, 4, 1);
    stream->UnknownFunction461640(&hasSourceBlend, 4, 1);
    stream->UnknownFunction461640(&hasDestBlend, 4, 1);
    stream->UnknownFunction461640(&field_0xb8, 4, 1);
    stream->UnknownFunction461640(&clampTexture, 4, 1);
    stream->UnknownFunction461640(&hasAlpha, 4, 1);
    stream->UnknownFunction461640(&materialAlpha, 4, 1);
    stream->UnknownFunction461640(&textureSpeed, 4, 1);
    stream->UnknownFunction461640(&mappingType, 2, 1);
    stream->UnknownFunction461640(&field_0xcc, 4, 1);
    int position = stream->UnknownFunction461600();
    char mode = stream->UnknownFunction43e9e0();
    if (!_stricmp(textureName, "PROCEDURAL"))
        field_0x74.opaqueTextures = field_0x74.alphaTextures = 0;
    if (hasTextureName)
        LoadTexture();
    stream->UnknownFunction461340(position, 0, 0);
    stream->UnknownFunction43e9b0(mode);
}

// 0x004ff620
void SoultreeMaterial::LoadTexture()
{
    if (!field_0x70)
        field_0x70 = new(__FILE__, 0x118) SurfaceMap(0, 0);
    if (field_0x70->texture) {
        field_0x70->texture->Release();
        field_0x70->texture = 0;
    }
    int format = textureFormat;
    if (hasColorKey) {
        PCRenderTarget* target = (PCRenderTarget*)field_0x18;
        if ((target->triTextureCaps & 8) && field_0x74.opaqueTextures) {
            format = target->field_0x28;
            colorKey = (g_UnknownInt574060 << 8 | g_UnknownInt574064) << 8 | g_UnknownInt574068;
        } else {
            format = 1555;
        }
    }
    if (format == 1555) {
        field_0x74.format = 1555;
        field_0x74.opaqueTextures = 0;
        field_0x74.alphaTextures = 0;
    }
    int procedural = 0;
    if (!_stricmp(textureName, "PROCEDURAL"))
        procedural = 1;
    int alpha = UnknownFunction511ad0(format);
    TextureMap* texture;
    if (procedural) {
        if (field_0x74.opaqueTextures && field_0x74.alphaTextures) {
            texture = new(__FILE__, 0x165) ManagedTexture(field_0x74.textureManager);
            texture->UnknownVirtualSlot4(0, 0x40, 0x40, 0x40, 1, 0, format, 0, mipMapped ? 2 : 0, 0, 0, 0,
                                         hasSourceBlend ? sourceBlend : 5, hasDestBlend ? destBlend : 6, 0,
                                         0x80, 0xff00ff);
            if (alpha) {
                if (field_0x74.alphaTextures)
                    field_0x74.alphaTextures->UnknownFunction50c6c0((ManagedTexture*)texture);
            } else if (field_0x74.opaqueTextures) {
                field_0x74.opaqueTextures->UnknownFunction50c6c0((ManagedTexture*)texture);
            }
        } else {
            texture = new(__FILE__, 0x17a) PCTextureMap(field_0x74.textureManager, 1);
            texture->UnknownVirtualSlot4(0, 0x40, 0x40, 0x40, 1, 0, format, 0, mipMapped ? 2 : 0, 0, 0, 0,
                                         hasSourceBlend ? sourceBlend : 5, hasDestBlend ? destBlend : 6, 0,
                                         0x80, 0xff00ff);
        }
    } else {
        texture = UnknownFunction50a590(textureManager, textureName, format, 0, mipMapped ? 2 : 0,
                                        hasSourceBlend ? sourceBlend : 5, hasDestBlend ? destBlend : 6,
                                        &field_0x74, 0x80, 0xff00ff, 1, 1);
    }
    if (!texture) {
        char message[256];
        sprintf(message, "TextureMap %s Not loaded, please check resources and current directory\n", textureName);
    }
    field_0x70->SetTexture(texture);
    if (texture) {
        texture->Release();
        if (!field_0x70->texture->UnknownVirtualSlot7() && field_0x70->texture->GetRefCount() == 1) {
            if (hasColorKey)
                field_0x70->texture->UnknownVirtualSlot18(colorKey);
            field_0x70->texture->UnknownVirtualSlot8(1, 0, 0);
        }
    }
}

// 0x004ff9e0
void SoultreeMaterial::ReadKeys(UnknownParameterBlock* block, int load)
{
    char value[128];
    textureName[0] = 0;
    mipMapped = 1;
    hasTextureName = 0;
    hasColorKey = 0;
    hasSourceBlend = 0;
    hasDestBlend = 0;
    field_0x70 = 0;
    mappingType = 0;
    hasAlpha = 0;
    field_0xcc = 0;
    textureSpeed = 0.0f;
    useVertexColor = 0;
    materialAlpha = 1.0f;
    if (block->UnknownFunction4b7b30("TextureMap", textureName, -1))
        hasTextureName = 1;
    int managed = field_0x74.opaqueTextures || (field_0x74.alphaTextures && hasTextureName);
    block->UnknownFunction4b7cf0("TextureFormat", &textureFormat);
    block->UnknownFunction4b7cf0("UseVertexColor", &useVertexColor);
    block->UnknownFunction4b7cf0("MipMapped", &mipMapped);
    block->UnknownFunction4b7cf0("ManagedTexture", &managed);
    block->UnknownFunction4b7f10("ClampTexture", 0, &clampTexture);
    if (block->UnknownFunction4b7b30("MappingType", value, -1)) {
        if (!_stricmp(value, "STANDARD"))
            mappingType = 0;
        if (!_stricmp(value, "FIRE"))
            mappingType = 1;
        if (!_stricmp(value, "CHROME"))
            mappingType = 2;
        if (!_stricmp(value, "LIGHT"))
            mappingType = 3;
        if (!_stricmp(value, "CEL"))
            mappingType = 4;
        if (!_stricmp(value, "WATERREFLECTION"))
            mappingType = 5;
        if (!_stricmp(value, "SPECULAR"))
            mappingType = 6;
        if (!_stricmp(value, "PROJECTION"))
            mappingType = 7;
        if (!_stricmp(value, "ENVIRONMENT"))
            mappingType = 8;
        if (!_stricmp(value, "NONE"))
            mappingType = 9;
    }
    if (!managed) {
        field_0x74.opaqueTextures = 0;
        field_0x74.alphaTextures = 0;
    }
    if (block->UnknownFunction4b7b30("ColorKey", value, -1)) {
        hasColorKey = 1;
        UnknownTokenizer tokens(value);
        int red = atoi(tokens.UnknownFunction515df0(","));
        int green = atoi(tokens.UnknownFunction515df0(","));
        int blue = atoi(tokens.UnknownFunction515df0("\n"));
        colorKey = (red << 8 | green) << 8 | blue;
    }
    if (block->UnknownFunction4b7e70("Alpha", &materialAlpha))
        hasAlpha = 1;
    block->UnknownFunction4b7e70("TextureSpeed", &textureSpeed);
    if (block->UnknownFunction4b7b30("SourceBlend", value, -1)) {
        hasSourceBlend = 1;
        if (!_stricmp(value, "ZERO"))
            sourceBlend = 1;
        if (!_stricmp(value, "ONE"))
            sourceBlend = 2;
        if (!_stricmp(value, "SRCCOLOR"))
            sourceBlend = 3;
        if (!_stricmp(value, "INVSRCCOLOR"))
            sourceBlend = 4;
        if (!_stricmp(value, "SRCALPHA"))
            sourceBlend = 5;
        if (!_stricmp(value, "INVSRCALPHA"))
            sourceBlend = 6;
        if (!_stricmp(value, "DESTALPHA"))
            sourceBlend = 7;
        if (!_stricmp(value, "INVDESTALPHA"))
            sourceBlend = 8;
        if (!_stricmp(value, "DESTCOLOR"))
            sourceBlend = 9;
        if (!_stricmp(value, "INVDESTCOLOR"))
            sourceBlend = 10;
        if (!_stricmp(value, "SRCALPHASAT"))
            sourceBlend = 11;
        if (!_stricmp(value, "BOTHSRCALPHA"))
            sourceBlend = 12;
        if (!_stricmp(value, "BOTHINVSRCALPHA"))
            sourceBlend = 13;
    }
    if (block->UnknownFunction4b7b30("DestBlend", value, -1)) {
        hasDestBlend = 1;
        if (!_stricmp(value, "ZERO"))
            destBlend = 1;
        if (!_stricmp(value, "ONE"))
            destBlend = 2;
        if (!_stricmp(value, "SRCCOLOR"))
            destBlend = 3;
        if (!_stricmp(value, "INVSRCCOLOR"))
            destBlend = 4;
        if (!_stricmp(value, "SRCALPHA"))
            destBlend = 5;
        if (!_stricmp(value, "INVSRCALPHA"))
            destBlend = 6;
        if (!_stricmp(value, "DESTALPHA"))
            destBlend = 7;
        if (!_stricmp(value, "INVDESTALPHA"))
            destBlend = 8;
        if (!_stricmp(value, "DESTCOLOR"))
            destBlend = 9;
        if (!_stricmp(value, "INVDESTCOLOR"))
            destBlend = 10;
        if (!_stricmp(value, "SRCALPHASAT"))
            destBlend = 11;
        if (!_stricmp(value, "BOTHSRCALPHA"))
            destBlend = 12;
        if (!_stricmp(value, "BOTHINVSRCALPHA"))
            destBlend = 13;
    }
    if (hasTextureName && load)
        LoadTexture();
}

// 0x005000b0
void SoultreeMaterial::MakeUntextured()
{
    mappingType = 9;
    textureName[0] = 0;
    hasTextureName = 0;
    hasColorKey = 0;
    mipMapped = 0;
    hasSourceBlend = 0;
    hasDestBlend = 0;
    field_0x70 = 0;
    hasAlpha = 0;
}

// 0x005000f0
void SoultreeMaterial::CopyFrom(const SoultreeMaterial* other)
{
    strcpy(textureName, other->textureName);
    field_0x74 = other->field_0x74;
    textureFormat = other->textureFormat;
    colorKey = other->colorKey;
    sourceBlend = other->sourceBlend;
    destBlend = other->destBlend;
    hasTextureName = other->hasTextureName;
    useVertexColor = other->useVertexColor;
    mipMapped = other->mipMapped;
    hasColorKey = other->hasColorKey;
    hasSourceBlend = other->hasSourceBlend;
    hasDestBlend = other->hasDestBlend;
    hasAlpha = other->hasAlpha;
    materialAlpha = other->materialAlpha;
    textureSpeed = other->textureSpeed;
    mappingType = other->mappingType;
    field_0xcc = other->field_0xcc;
    field_0xb8 = other->field_0xb8;
    clampTexture = other->clampTexture;
    field_0x70 = 0;
    if (hasTextureName)
        LoadTexture();
}
