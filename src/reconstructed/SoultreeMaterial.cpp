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
    surfaceMap = 0;
    mappingType = 0;
    hasAlpha = 0;
    texelRatio = 0;
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
    GameObject::SetRenderContext(target);
    strcpy(textureName, "");
    textureManager = manager;
    renderContext = target;
    surfaceMap = 0;
    textureOptions = *options;
    textureFormat = format;
    return this;
}

// 0x004ff120 (deleting wrapper 0x004ff090)
SoultreeMaterial::~SoultreeMaterial()
{
    if (surfaceMap) {
        surfaceMap->Release();
        surfaceMap = 0;
    }
}

// 0x004ff180: the render states of the material. Cases 3 and 6 of the
// mapping-type switch are separate identical statements that VC6 merges
// (written as one case label pair, the last case picks the other vtable
// register).
void SoultreeMaterial::ApplyRenderStates()
{
    if (surfaceMap) {
        surfaceMap->Select();
        int format = surfaceMap->texture->mapFormat;
        if (format != 1555 && format != 4444 && format != 8888) {
            ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, 0, 0);
        } else if (hasColorKey) {
            if (hasAlpha)
                ((RenderTarget*)renderContext)->SetAlphaTest(0);
            else
                ((RenderTarget*)renderContext)->SetAlphaTest(0x80);
        } else {
            ((RenderTarget*)renderContext)->SetAlphaTest(0);
        }
        ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        if (format == 4444 || format == 8888 || (format == 1555 && hasAlpha)) {
            ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, 1, 0);
            if (!hasAlpha && mappingType != 6) {
                ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
                ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            } else {
                ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
                ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
                ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, 1, 0);
            }
        } else if (hasAlpha) {
            ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, 1, 0);
            ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        } else {
            ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, 0, 0);
            ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        }
    } else {
        ((RenderTarget*)renderContext)->ClearTexture(0);
        ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
        if (hasAlpha) {
            ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, 1, 0);
            ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        } else {
            ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, 0, 0);
            ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        }
    }
    if (clampTexture) {
        ((RenderTarget*)renderContext)->GetTextureStageState(0, 0xc, &g_UnknownInt689ed0);
        ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);
    }
    switch (mappingType) {
    case 3:
        ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, 1, 0);
        break;
    case 4:
        ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, 1, 0);
        break;
    case 6:
        ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, 1, 0);
        break;
    case 8:
        ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_WRAP0, 1, 0);
        break;
    }
}

// 0x004ff410
void SoultreeMaterial::RestoreRenderStates()
{
    if (mappingType == 8)
        ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_WRAP0, 0, 0);
    if (clampTexture)
        ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_ADDRESS, g_UnknownInt689ed0);
}

// 0x004ff450
void SoultreeMaterial::ReadSaved(UnknownParameterStream* stream)
{
    stream->Read(textureName, 0x40, 1);
    stream->Read(&textureFormat, 4, 1);
    stream->Read(&colorKey, 4, 1);
    stream->Read(&sourceBlend, 4, 1);
    stream->Read(&destBlend, 4, 1);
    stream->Read(&hasTextureName, 4, 1);
    stream->Read(&useVertexColor, 4, 1);
    stream->Read(&mipMapped, 4, 1);
    stream->Read(&hasColorKey, 4, 1);
    stream->Read(&hasSourceBlend, 4, 1);
    stream->Read(&hasDestBlend, 4, 1);
    stream->Read(&field_0xb8, 4, 1);
    stream->Read(&clampTexture, 4, 1);
    stream->Read(&hasAlpha, 4, 1);
    stream->Read(&materialAlpha, 4, 1);
    stream->Read(&textureSpeed, 4, 1);
    stream->Read(&mappingType, 2, 1);
    stream->Read(&texelRatio, 4, 1);
    int position = stream->Tell();
    char mode = stream->GetTextMode();
    if (!_stricmp(textureName, "PROCEDURAL"))
        textureOptions.opaqueTextures = textureOptions.alphaTextures = 0;
    if (hasTextureName)
        LoadTexture();
    stream->Seek(position, 0, 0);
    stream->SetTextMode(mode);
}

// 0x004ff620
void SoultreeMaterial::LoadTexture()
{
    if (!surfaceMap)
        surfaceMap = new(__FILE__, 0x118) SurfaceMap(0, 0);
    if (surfaceMap->texture) {
        surfaceMap->texture->Release();
        surfaceMap->texture = 0;
    }
    int format = textureFormat;
    if (hasColorKey) {
        PCRenderTarget* target = (PCRenderTarget*)renderContext;
        if ((target->triTextureCaps & 8) && textureOptions.opaqueTextures) {
            format = target->surfacePixelFormat;
            colorKey = (g_UnknownInt574060 << 8 | g_UnknownInt574064) << 8 | g_UnknownInt574068;
        } else {
            format = 1555;
        }
    }
    if (format == 1555) {
        textureOptions.format = 1555;
        textureOptions.opaqueTextures = 0;
        textureOptions.alphaTextures = 0;
    }
    int procedural = 0;
    if (!_stricmp(textureName, "PROCEDURAL"))
        procedural = 1;
    int alpha = IsFormat4444Or8888(format);
    TextureMap* texture;
    if (procedural) {
        if (textureOptions.opaqueTextures && textureOptions.alphaTextures) {
            texture = new(__FILE__, 0x165) ManagedTexture(textureOptions.textureManager);
            texture->CreateSurface(0, 0x40, 0x40, 0x40, 1, 0, format, 0, mipMapped ? 2 : 0, 0, 0, 0,
                                   hasSourceBlend ? sourceBlend : 5, hasDestBlend ? destBlend : 6, 0,
                                   0x80, 0xff00ff);
            if (alpha) {
                if (textureOptions.alphaTextures)
                    textureOptions.alphaTextures->AddTexture((ManagedTexture*)texture);
            } else if (textureOptions.opaqueTextures) {
                textureOptions.opaqueTextures->AddTexture((ManagedTexture*)texture);
            }
        } else {
            texture = new(__FILE__, 0x17a) PCTextureMap(textureOptions.textureManager, 1);
            texture->CreateSurface(0, 0x40, 0x40, 0x40, 1, 0, format, 0, mipMapped ? 2 : 0, 0, 0, 0,
                                   hasSourceBlend ? sourceBlend : 5, hasDestBlend ? destBlend : 6, 0,
                                   0x80, 0xff00ff);
        }
    } else {
        texture = TerrainLoadTexture(textureManager, textureName, format, 0, mipMapped ? 2 : 0,
                                        hasSourceBlend ? sourceBlend : 5, hasDestBlend ? destBlend : 6,
                                        &textureOptions, 0x80, 0xff00ff, 1, 1);
    }
    if (!texture) {
        char message[256];
        sprintf(message, "TextureMap %s Not loaded, please check resources and current directory\n", textureName);
    }
    surfaceMap->SetTexture(texture);
    if (texture) {
        texture->Release();
        if (!surfaceMap->texture->IsResident() && surfaceMap->texture->GetRefCount() == 1) {
            if (hasColorKey)
                surfaceMap->texture->ApplyColorKey(colorKey);
            surfaceMap->texture->CreateTextureSurface(1, 0, 0);
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
    surfaceMap = 0;
    mappingType = 0;
    hasAlpha = 0;
    texelRatio = 0;
    textureSpeed = 0.0f;
    useVertexColor = 0;
    materialAlpha = 1.0f;
    if (block->ReadString("TextureMap", textureName, -1))
        hasTextureName = 1;
    int managed = textureOptions.opaqueTextures || (textureOptions.alphaTextures && hasTextureName);
    block->ReadInt("TextureFormat", &textureFormat);
    block->ReadInt("UseVertexColor", &useVertexColor);
    block->ReadInt("MipMapped", &mipMapped);
    block->ReadInt("ManagedTexture", &managed);
    block->GetInt("ClampTexture", 0, &clampTexture);
    if (block->ReadString("MappingType", value, -1)) {
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
        textureOptions.opaqueTextures = 0;
        textureOptions.alphaTextures = 0;
    }
    if (block->ReadString("ColorKey", value, -1)) {
        hasColorKey = 1;
        UnknownTokenizer tokens(value);
        int red = atoi(tokens.NextToken(","));
        int green = atoi(tokens.NextToken(","));
        int blue = atoi(tokens.NextToken("\n"));
        colorKey = (red << 8 | green) << 8 | blue;
    }
    if (block->ReadFloat("Alpha", &materialAlpha))
        hasAlpha = 1;
    block->ReadFloat("TextureSpeed", &textureSpeed);
    if (block->ReadString("SourceBlend", value, -1)) {
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
    if (block->ReadString("DestBlend", value, -1)) {
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
    surfaceMap = 0;
    hasAlpha = 0;
}

// 0x005000f0
void SoultreeMaterial::CopyFrom(const SoultreeMaterial* other)
{
    strcpy(textureName, other->textureName);
    textureOptions = other->textureOptions;
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
    texelRatio = other->texelRatio;
    field_0xb8 = other->field_0xb8;
    clampTexture = other->clampTexture;
    surfaceMap = 0;
    if (hasTextureName)
        LoadTexture();
}
