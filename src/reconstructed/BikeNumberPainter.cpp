#include <string.h>

#include "BikeNumberPainter.h"
#include "D3DConstants.h"
#include "D3DIMSoulTree.h"
#include "PCVideoCard.h"
#include "SoultreeMaterial.h" // also declares the loader 0x0050a590

// 0x00417500. Retail stores the widths in this order (+0x08 last).
UnknownBikeNumberPainter::UnknownBikeNumberPainter(TextureMapManager* textures) {
    sheet = (PCTextureMap*)UnknownFunction50a590(textures, "BikeNumbers128.tga", 0x115c, 0, 0, 5, 6,
                                                 0, 0x80, 0xff00ff, 1, 1);
    digitWidths[0] = 19;
    digitWidths[2] = 19;
    digitWidths[3] = 19;
    digitWidths[4] = 19;
    digitWidths[5] = 19;
    digitWidths[6] = 19;
    digitWidths[7] = 19;
    digitWidths[8] = 19;
    digitWidths[9] = 19;
    digitWidths[1] = 13;
}

// 0x00417570
UnknownBikeNumberPainter::~UnknownBikeNumberPainter() {
    sheet->Release();
}

// 0x00417580
void UnknownBikeNumberPainter::UnknownFunction417580(int digit, int x, PCTextureMap* target) {
    int row;
    if (digit < 4)
        row = 0;
    else
        row = digit < 8 ? 1 : 2;
    // The four tests cover the digits 0..9 and leave `column` unset otherwise;
    // retail reads that unset value from the slot it shares with `digit`.
    int column;
    if (digit == 0 || digit == 4 || digit == 8)
        column = 0;
    if (digit == 1 || digit == 5 || digit == 9)
        column = 1;
    if (digit == 2 || digit == 6)
        column = 2;
    if (digit == 3 || digit == 7)
        column = 3;
    int width = digitWidths[digit];
    CameraRect source;
    CameraRect destination;
    source.left = column * 32;
    source.top = row * 32;
    source.right = column * 32 + width;
    source.bottom = (row + 1) * 32;
    destination.top = 4;
    destination.left = x + 4;
    destination.bottom = 36;
    destination.right = x + 4 + width;
    if (destination.right > 64)
        destination.right = 60;
    target->systemSurface->Blt(&destination, sheet->systemSurface, &source, DDBLT_WAIT, 0);
}

// 0x00417670: paints `number` on every "PROCEDURAL" material texture of
// `model` (cleared to colour 0, digits centred on x 28), then, when the
// number has at most three digits in all (`digits` counts across every
// plate), scales and offsets the texture coordinates of the plate surfaces'
// vertices with z >= -0.5 to fit them. `number` itself loses its hundreds
// and tens as they are painted.
void UnknownBikeNumberPainter::UnknownFunction417670(D3DIMSoultreeObject* model, int number) {
    int digits = 0;
    int plate = 0;
    int hundreds = 0;
    for (int i = 0; i < model->materialCount; i++) {
        if (_stricmp(model->materialTable[i]->textureName, "PROCEDURAL"))
            continue;
        plate = i;
        PCTextureMap* texture = (PCTextureMap*)model->materialTable[i]->field_0x70->texture;
        UnknownBltFx fx;
        memset(&fx, 0, sizeof(fx));
        fx.size = sizeof(fx);
        fx.fillColor = 0;
        CameraRect rect;
        rect.left = 0;
        rect.top = 0;
        rect.right = texture->field_0x14;
        rect.bottom = texture->field_0x18;
        texture->systemSurface->Blt(&rect, 0, 0, DDBLT_COLORFILL, &fx);
        int width = 0;
        for (int n = number; n > 0;) {
            int digit = n % 10;
            n /= 10;
            width += digitWidths[digit];
            digits++;
        }
        int x = 28 - (int)(width * 0.5);
        if (number > 99) {
            int digit = number / 100;
            UnknownFunction417580(digit, x, texture);
            hundreds = 1;
            x += digitWidths[digit];
            number -= digit * 100;
        }
        if (number > 9 || hundreds) {
            int digit = number / 10;
            UnknownFunction417580(digit, x, texture);
            x += digitWidths[digit];
            number -= digit * 10;
        }
        UnknownFunction417580(number, x, texture);
        texture->UnknownVirtualSlot15(2);
        if (texture->field_0x68 & 1)
            ((ManagedTexture*)texture)->UnknownFunction510670();
        else
            texture->UnknownVirtualSlot9(0, -1);
    }
    if (digits >= 4)
        return;
    float scale;
    float offsetU;
    float offsetV;
    if (digits == 3) {
        scale = 1.0f;
        offsetU = 0.0f;
        offsetV = 0.0f;
    } else if (digits == 2) {
        scale = 0.75f;
        offsetU = 0.11f;
        offsetV = 0.1f;
    } else {
        scale = 0.65f;
        offsetU = 0.175f;
        offsetV = 0.11f;
    }
    for (int lod = 0; lod < model->lodCount; lod++) {
        for (int s = 0; s < model->lodTable[lod].surfaceCount; s++) {
            int uses = 0;
            for (int m = 0; m < model->lodTable[lod].surfaces[s].materialCount; m++) {
                if (model->lodTable[lod].surfaces[s].materialIndices[m] == plate)
                    uses = 1;
            }
            if (!uses)
                continue;
            for (int v = 0; v < model->lodTable[lod].surfaces[s].vertexCount; v++) {
                if (model->lodTable[lod].surfaces[s].drawnVertices[v].position.z < -0.5f)
                    continue;
                model->lodTable[lod].surfaces[s].drawnVertices[v].tu = model->lodTable[lod].surfaces[s].uvs[v].u;
                model->lodTable[lod].surfaces[s].drawnVertices[v].tv = model->lodTable[lod].surfaces[s].uvs[v].v;
                model->lodTable[lod].surfaces[s].vertices[v].tu = model->lodTable[lod].surfaces[s].uvs[v].u;
                model->lodTable[lod].surfaces[s].vertices[v].tv = model->lodTable[lod].surfaces[s].uvs[v].v;
                model->lodTable[lod].surfaces[s].drawnVertices[v].tu *= scale;
                model->lodTable[lod].surfaces[s].drawnVertices[v].tu += offsetU;
                model->lodTable[lod].surfaces[s].vertices[v].tu *= scale;
                model->lodTable[lod].surfaces[s].vertices[v].tu += offsetU;
                model->lodTable[lod].surfaces[s].drawnVertices[v].tv *= scale;
                model->lodTable[lod].surfaces[s].drawnVertices[v].tv += offsetV;
                model->lodTable[lod].surfaces[s].vertices[v].tv *= scale;
                model->lodTable[lod].surfaces[s].vertices[v].tv += offsetV;
            }
        }
    }
}
