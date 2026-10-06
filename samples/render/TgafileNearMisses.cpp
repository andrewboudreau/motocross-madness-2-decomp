// Near-miss Tgafile.cpp candidates, kept out of src/reconstructed until they
// match. See docs/TGAFILE.md.
//
// UnknownFunction512990 (0x00512990, 1074 bytes): the TGA writer behind
// 0x00512720/0x005127f0/0x005128c0. The header writes, the three depth
// branches and their exits line up, and with a `pitch` local and
// function-scope loop counters the frame matches (one local plus the four
// dead argument slots). VC6 here assigns the loop variables to different
// argument slots: retail keeps the 24-bit row offset in the file slot, the
// column in the stride slot, the row in the mask slot and the pixel in the
// descriptor slot. Declaration order, explicit offsets and a function-scope
// pixel do not reproduce that.

#include <stdio.h>
#include <string.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/RenderInterfaces.h"
#include "../../src/reconstructed/TextureMap.h"
#include "../../src/reconstructed/Tgafile.h"

// 0x00512990: writes `file` to file->name: the header field by field, then
// the rows. 24-bit pixels go one by one with red and blue swapped; 32-bit
// pixels are swapped in place and written in blocks; 16-bit rows go as
// they are for 555 and pixel by pixel as 555 for 565. The 16-bit failures
// return without closing the file, as in retail.
int UnknownFunction512990(UnknownTgaFile* file, unsigned int stride, int greenMask, int descriptor) {
    int row;
    int column;
    unsigned int pitch;
    FILE* stream = fopen(file->name, "wb");
    if (!stream)
        return 0;
    file->descriptor = descriptor;
    file->imageType = 2;
    if (fwrite(&file->idLength, 1, 1, stream) != 1 || fwrite(&file->colorMapType, 1, 1, stream) != 1 ||
        fwrite(&file->imageType, 1, 1, stream) != 1 || fwrite(&file->colorMapStart, 2, 1, stream) != 1 ||
        fwrite(&file->colorMapLength, 2, 1, stream) != 1 || fwrite(&file->colorMapDepth, 1, 1, stream) != 1 ||
        fwrite(&file->x, 2, 1, stream) != 1 || fwrite(&file->y, 2, 1, stream) != 1 ||
        fwrite(&file->width, 2, 1, stream) != 1 || fwrite(&file->height, 2, 1, stream) != 1 ||
        fwrite(&file->bitsPerPixel, 1, 1, stream) != 1 || fwrite(&file->descriptor, 1, 1, stream) != 1)
        goto failed;
    if (file->bitsPerPixel == 24) {
        pitch = (stride ? stride : file->width * 3) / 3;
        for (row = 0; row < file->height; row++) {
            UnknownPixel24* source = (UnknownPixel24*)file->bits + row * pitch;
            for (column = 0; column < file->width; column++, source++) {
                UnknownPixel24 pixel = *source;
                unsigned char red = pixel.red;
                pixel.red = pixel.blue;
                pixel.blue = red;
                if (fwrite(&pixel, 3, 1, stream) != 1)
                    goto failed;
            }
        }
        fclose(stream);
        return 1;
    }
    if (file->bitsPerPixel == 32) {
        pitch = (stride ? stride : file->width * 4) / 4;
        for (row = 0; row < file->height; row++) {
            UnknownPixel32* pixel = (UnknownPixel32*)file->bits + row * pitch;
            for (column = 0; column < file->width; column++, pixel++) {
                UnknownPixel32 swapped = *pixel;
                unsigned char red = swapped.red;
                swapped.red = swapped.blue;
                swapped.blue = red;
                *pixel = swapped;
            }
        }
        int count = file->height * file->width;
        int block = file->width * pitch;
        UnknownPixel32* pixels = (UnknownPixel32*)file->bits;
        for (int written = 0; written < count; written += block) {
            if ((int)fwrite(pixels, 4, block, stream) != block)
                goto failed;
            pixels += block;
        }
        fclose(stream);
        return 1;
    }
    {
        pitch = (stride ? stride : file->width * 2) / 2;
        if (greenMask == 0x3e0) {
            for (row = 0; row < file->height; row++) {
                if ((int)fwrite((unsigned short*)file->bits + row * pitch, 2, file->width, stream) != file->width)
                    return 0;
            }
        } else {
            for (row = 0; row < file->height; row++) {
                unsigned short* source = (unsigned short*)file->bits + row * pitch;
                for (column = 0; column < file->width; column++, source++) {
                    int pixel = (*source >> 1) & 0x7fe0 | *source & 0x1f;
                    if (fwrite(&pixel, 2, 1, stream) != 1)
                        return 0;
                }
            }
        }
        fclose(stream);
        return 1;
    }
failed:
    fclose(stream);
    return 0;
}
