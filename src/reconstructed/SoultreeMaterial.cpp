#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SoultreeMaterial.h"
#include "DebugAlloc.h"

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
    strcpy(field_0x2c, "");
    field_0xa4 = 1;
    field_0x6c = 0;
    field_0x2c[0] = 0;
    field_0x9c = 0;
    field_0xa8 = 0;
    field_0xac = 0;
    field_0xb0 = 0;
    field_0x70 = 0;
    field_0xc8 = 0;
    field_0xbc = 0;
    field_0xcc = 0;
    field_0xc4 = 0.0f;
    field_0xa0 = 0;
    field_0xb4 = 0;
    field_0xc0 = 1.0f;
    field_0xb8 = 1;
}

// 0x004ff0b0: returns this, like GameObject slot 8.
SoultreeMaterial* SoultreeMaterial::UnknownFunction4ff0b0(RenderTarget* target, const SoultreeTextureOptions* options,
                                             TextureMapManager* manager, int format)
{
    GameObject::UnknownVirtualSlot8(target);
    strcpy(field_0x2c, "");
    field_0x6c = manager;
    field_0x18 = target;
    field_0x70 = 0;
    field_0x74 = *options;
    field_0x8c = format;
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

// 0x004ff410
void SoultreeMaterial::UnknownFunction4ff410()
{
    if (field_0xc8 == 8)
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(0x80, 0, 0);
    if (field_0xb4)
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 0xc, g_UnknownInt689ed0);
}

// 0x004ff450
void SoultreeMaterial::UnknownFunction4ff450(UnknownParameterStream* stream)
{
    stream->UnknownFunction461640(field_0x2c, 0x40, 1);
    stream->UnknownFunction461640(&field_0x8c, 4, 1);
    stream->UnknownFunction461640(&field_0x90, 4, 1);
    stream->UnknownFunction461640(&field_0x94, 4, 1);
    stream->UnknownFunction461640(&field_0x98, 4, 1);
    stream->UnknownFunction461640(&field_0x9c, 4, 1);
    stream->UnknownFunction461640(&field_0xa0, 4, 1);
    stream->UnknownFunction461640(&field_0xa4, 4, 1);
    stream->UnknownFunction461640(&field_0xa8, 4, 1);
    stream->UnknownFunction461640(&field_0xac, 4, 1);
    stream->UnknownFunction461640(&field_0xb0, 4, 1);
    stream->UnknownFunction461640(&field_0xb8, 4, 1);
    stream->UnknownFunction461640(&field_0xb4, 4, 1);
    stream->UnknownFunction461640(&field_0xbc, 4, 1);
    stream->UnknownFunction461640(&field_0xc0, 4, 1);
    stream->UnknownFunction461640(&field_0xc4, 4, 1);
    stream->UnknownFunction461640(&field_0xc8, 2, 1);
    stream->UnknownFunction461640(&field_0xcc, 4, 1);
    int position = stream->UnknownFunction461600();
    char mode = stream->UnknownFunction43e9e0();
    if (!_stricmp(field_0x2c, "PROCEDURAL"))
        field_0x74.field_0x04 = field_0x74.field_0x08 = 0;
    if (field_0x9c)
        UnknownFunction4ff620();
    stream->UnknownFunction461340(position, 0, 0);
    stream->UnknownFunction43e9b0(mode);
}

// 0x004ff620
void SoultreeMaterial::UnknownFunction4ff620()
{
    if (!field_0x70)
        field_0x70 = new(__FILE__, 0x118) SurfaceMap(0, 0);
    if (field_0x70->texture) {
        field_0x70->texture->Release();
        field_0x70->texture = 0;
    }
    int format = field_0x8c;
    if (field_0xa8) {
        PCRenderTarget* target = (PCRenderTarget*)field_0x18;
        if ((target->field_0x1c0 & 8) && field_0x74.field_0x04) {
            format = target->field_0x28;
            field_0x90 = (g_UnknownInt574060 << 8 | g_UnknownInt574064) << 8 | g_UnknownInt574068;
        } else {
            format = 0x613;
        }
    }
    if (format == 0x613) {
        field_0x74.field_0x0c = 0x613;
        field_0x74.field_0x04 = 0;
        field_0x74.field_0x08 = 0;
    }
    int procedural = 0;
    if (!_stricmp(field_0x2c, "PROCEDURAL"))
        procedural = 1;
    int alpha = UnknownFunction511ad0(format);
    TextureMap* texture;
    if (procedural) {
        if (field_0x74.field_0x04 && field_0x74.field_0x08) {
            texture = new(__FILE__, 0x165) ManagedTexture(field_0x74.field_0x00);
            texture->UnknownVirtualSlot4(0, 0x40, 0x40, 0x40, 1, 0, format, 0, field_0xa4 ? 2 : 0, 0, 0, 0,
                                         field_0xac ? field_0x94 : 5, field_0xb0 ? field_0x98 : 6, 0,
                                         0x80, 0xff00ff);
            if (alpha) {
                if (field_0x74.field_0x08)
                    field_0x74.field_0x08->UnknownFunction50c6c0((ManagedTexture*)texture);
            } else if (field_0x74.field_0x04) {
                field_0x74.field_0x04->UnknownFunction50c6c0((ManagedTexture*)texture);
            }
        } else {
            texture = new(__FILE__, 0x17a) PCTextureMap(field_0x74.field_0x00, 1);
            texture->UnknownVirtualSlot4(0, 0x40, 0x40, 0x40, 1, 0, format, 0, field_0xa4 ? 2 : 0, 0, 0, 0,
                                         field_0xac ? field_0x94 : 5, field_0xb0 ? field_0x98 : 6, 0,
                                         0x80, 0xff00ff);
        }
    } else {
        texture = UnknownFunction50a590(field_0x6c, field_0x2c, format, 0, field_0xa4 ? 2 : 0,
                                        field_0xac ? field_0x94 : 5, field_0xb0 ? field_0x98 : 6,
                                        &field_0x74, 0x80, 0xff00ff, 1, 1);
    }
    if (!texture) {
        char message[256];
        sprintf(message, "TextureMap %s Not loaded, please check resources and current directory\n", field_0x2c);
    }
    field_0x70->SetTexture(texture);
    if (texture) {
        texture->Release();
        if (!field_0x70->texture->UnknownVirtualSlot7() && field_0x70->texture->GetRefCount() == 1) {
            if (field_0xa8)
                field_0x70->texture->UnknownVirtualSlot18(field_0x90);
            field_0x70->texture->UnknownVirtualSlot8(1, 0, 0);
        }
    }
}

// 0x004ff9e0
void SoultreeMaterial::UnknownFunction4ff9e0(UnknownParameterBlock* block, int load)
{
    char value[128];
    field_0x2c[0] = 0;
    field_0xa4 = 1;
    field_0x9c = 0;
    field_0xa8 = 0;
    field_0xac = 0;
    field_0xb0 = 0;
    field_0x70 = 0;
    field_0xc8 = 0;
    field_0xbc = 0;
    field_0xcc = 0;
    field_0xc4 = 0.0f;
    field_0xa0 = 0;
    field_0xc0 = 1.0f;
    if (block->UnknownFunction4b7b30("TextureMap", field_0x2c, -1))
        field_0x9c = 1;
    int managed = field_0x74.field_0x04 || (field_0x74.field_0x08 && field_0x9c);
    block->UnknownFunction4b7cf0("TextureFormat", &field_0x8c);
    block->UnknownFunction4b7cf0("UseVertexColor", &field_0xa0);
    block->UnknownFunction4b7cf0("MipMapped", &field_0xa4);
    block->UnknownFunction4b7cf0("ManagedTexture", &managed);
    block->UnknownFunction4b7f10("ClampTexture", 0, &field_0xb4);
    if (block->UnknownFunction4b7b30("MappingType", value, -1)) {
        if (!_stricmp(value, "STANDARD"))
            field_0xc8 = 0;
        if (!_stricmp(value, "FIRE"))
            field_0xc8 = 1;
        if (!_stricmp(value, "CHROME"))
            field_0xc8 = 2;
        if (!_stricmp(value, "LIGHT"))
            field_0xc8 = 3;
        if (!_stricmp(value, "CEL"))
            field_0xc8 = 4;
        if (!_stricmp(value, "WATERREFLECTION"))
            field_0xc8 = 5;
        if (!_stricmp(value, "SPECULAR"))
            field_0xc8 = 6;
        if (!_stricmp(value, "PROJECTION"))
            field_0xc8 = 7;
        if (!_stricmp(value, "ENVIRONMENT"))
            field_0xc8 = 8;
        if (!_stricmp(value, "NONE"))
            field_0xc8 = 9;
    }
    if (!managed) {
        field_0x74.field_0x04 = 0;
        field_0x74.field_0x08 = 0;
    }
    if (block->UnknownFunction4b7b30("ColorKey", value, -1)) {
        field_0xa8 = 1;
        UnknownTokenizer tokens(value);
        int red = atoi(tokens.UnknownFunction515df0(","));
        int green = atoi(tokens.UnknownFunction515df0(","));
        int blue = atoi(tokens.UnknownFunction515df0("\n"));
        field_0x90 = (red << 8 | green) << 8 | blue;
    }
    if (block->UnknownFunction4b7e70("Alpha", &field_0xc0))
        field_0xbc = 1;
    block->UnknownFunction4b7e70("TextureSpeed", &field_0xc4);
    if (block->UnknownFunction4b7b30("SourceBlend", value, -1)) {
        field_0xac = 1;
        if (!_stricmp(value, "ZERO"))
            field_0x94 = 1;
        if (!_stricmp(value, "ONE"))
            field_0x94 = 2;
        if (!_stricmp(value, "SRCCOLOR"))
            field_0x94 = 3;
        if (!_stricmp(value, "INVSRCCOLOR"))
            field_0x94 = 4;
        if (!_stricmp(value, "SRCALPHA"))
            field_0x94 = 5;
        if (!_stricmp(value, "INVSRCALPHA"))
            field_0x94 = 6;
        if (!_stricmp(value, "DESTALPHA"))
            field_0x94 = 7;
        if (!_stricmp(value, "INVDESTALPHA"))
            field_0x94 = 8;
        if (!_stricmp(value, "DESTCOLOR"))
            field_0x94 = 9;
        if (!_stricmp(value, "INVDESTCOLOR"))
            field_0x94 = 10;
        if (!_stricmp(value, "SRCALPHASAT"))
            field_0x94 = 11;
        if (!_stricmp(value, "BOTHSRCALPHA"))
            field_0x94 = 12;
        if (!_stricmp(value, "BOTHINVSRCALPHA"))
            field_0x94 = 13;
    }
    if (block->UnknownFunction4b7b30("DestBlend", value, -1)) {
        field_0xb0 = 1;
        if (!_stricmp(value, "ZERO"))
            field_0x98 = 1;
        if (!_stricmp(value, "ONE"))
            field_0x98 = 2;
        if (!_stricmp(value, "SRCCOLOR"))
            field_0x98 = 3;
        if (!_stricmp(value, "INVSRCCOLOR"))
            field_0x98 = 4;
        if (!_stricmp(value, "SRCALPHA"))
            field_0x98 = 5;
        if (!_stricmp(value, "INVSRCALPHA"))
            field_0x98 = 6;
        if (!_stricmp(value, "DESTALPHA"))
            field_0x98 = 7;
        if (!_stricmp(value, "INVDESTALPHA"))
            field_0x98 = 8;
        if (!_stricmp(value, "DESTCOLOR"))
            field_0x98 = 9;
        if (!_stricmp(value, "INVDESTCOLOR"))
            field_0x98 = 10;
        if (!_stricmp(value, "SRCALPHASAT"))
            field_0x98 = 11;
        if (!_stricmp(value, "BOTHSRCALPHA"))
            field_0x98 = 12;
        if (!_stricmp(value, "BOTHINVSRCALPHA"))
            field_0x98 = 13;
    }
    if (field_0x9c && load)
        UnknownFunction4ff620();
}

// 0x005000b0
void SoultreeMaterial::UnknownFunction5000b0()
{
    field_0xc8 = 9;
    field_0x2c[0] = 0;
    field_0x9c = 0;
    field_0xa8 = 0;
    field_0xa4 = 0;
    field_0xac = 0;
    field_0xb0 = 0;
    field_0x70 = 0;
    field_0xbc = 0;
}

// 0x005000f0
void SoultreeMaterial::UnknownFunction5000f0(const SoultreeMaterial* other)
{
    strcpy(field_0x2c, other->field_0x2c);
    field_0x74 = other->field_0x74;
    field_0x8c = other->field_0x8c;
    field_0x90 = other->field_0x90;
    field_0x94 = other->field_0x94;
    field_0x98 = other->field_0x98;
    field_0x9c = other->field_0x9c;
    field_0xa0 = other->field_0xa0;
    field_0xa4 = other->field_0xa4;
    field_0xa8 = other->field_0xa8;
    field_0xac = other->field_0xac;
    field_0xb0 = other->field_0xb0;
    field_0xbc = other->field_0xbc;
    field_0xc0 = other->field_0xc0;
    field_0xc4 = other->field_0xc4;
    field_0xc8 = other->field_0xc8;
    field_0xcc = other->field_0xcc;
    field_0xb8 = other->field_0xb8;
    field_0xb4 = other->field_0xb4;
    field_0x70 = 0;
    if (field_0x9c)
        UnknownFunction4ff620();
}
