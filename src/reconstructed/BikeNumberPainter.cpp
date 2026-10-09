#include "BikeNumberPainter.h"
#include "D3DConstants.h"

// 0x0050a590 (Texmap.cpp, cdecl): loads the texture file `name`; 0 on failure
// (SoultreeMaterial.h has the fuller declaration).
TextureMap* UnknownFunction50a590(TextureMapManager* manager, const char* name, int format,
                                  int a4, int flags, int sourceBlend, int destBlend,
                                  void* options, int alphaThreshold, unsigned int key,
                                  int a11, int a12);

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
