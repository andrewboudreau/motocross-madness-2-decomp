#include <stdio.h>
#include <string.h>

#include "bmpfile.h"

#include "DebugAlloc.h"

// 0x00424140
UnknownBitmapFile* UnknownFunction424140(const char* name, UnknownBitmapFile* bitmap) {
    FILE* file;
    int stride;
    unsigned char* bits;
    int x;
    int y;
    int c;

    file = fopen(name, "rb");
    if (file) {
        if (!bitmap) {
            bitmap = (UnknownBitmapFile*)DebugMalloc(sizeof(UnknownBitmapFile), __FILE__, 35);
            if (!bitmap)
                goto close;
            bitmap->bits = 0;
        }
        if (fread(&bitmap->fileHeader.type, 2, 1, file) != 1)
            goto failed;
        if (fread(&bitmap->fileHeader.size, 4, 1, file) != 1)
            goto failed;
        if (fread(&bitmap->fileHeader.reserved1, 2, 1, file) != 1)
            goto failed;
        if (fread(&bitmap->fileHeader.reserved2, 2, 1, file) != 1)
            goto failed;
        if (fread(&bitmap->fileHeader.offBits, 4, 1, file) != 1)
            goto failed;
        if (fread(&bitmap->infoHeader, sizeof(bitmap->infoHeader), 1, file) != 1)
            goto failed;
        if (bitmap->infoHeader.bitCount != 8)
            goto failed;
        // Rows are padded to four bytes.
        stride = bitmap->infoHeader.width;
        if (stride & 3)
            stride = (stride | 3) + 1;
        fread(bitmap->palette, 4, 256, file);
        if (!bitmap->bits) {
            bitmap->bits = DebugMalloc(bitmap->infoHeader.width * bitmap->infoHeader.height, __FILE__, 75);
            if (!bitmap->bits)
                goto failed;
        }
        fseek(file, bitmap->fileHeader.offBits, SEEK_SET);
        bits = (unsigned char*)bitmap->bits;
        for (y = 0; y < bitmap->infoHeader.height; y++) {
            for (x = 0; x < bitmap->infoHeader.width; x++) {
                c = fgetc(file);
                if (c == EOF)
                    goto failed;
                *bits++ = c;
            }
            for (; x < stride; x++)
                fgetc(file);
        }
        strcpy(bitmap->name, name);
        fclose(file);
        return bitmap;

    failed:
        if (bitmap) {
            if (bitmap->bits)
                DebugFree(bitmap->bits, __FILE__, 101);
            DebugFree(bitmap, __FILE__, 102);
        }
    close:
        fclose(file);
    }
    return 0;
}

// 0x00424380
int UnknownFunction424380(UnknownBitmapFile* bitmap) {
    FILE* file;
    int stride;
    int x;
    int y;
    int i;

    file = fopen(bitmap->name, "wb");
    if (file) {
        if (fwrite(&bitmap->fileHeader.type, 2, 1, file) == 1
            && fwrite(&bitmap->fileHeader.size, 4, 1, file) == 1
            && fwrite(&bitmap->fileHeader.reserved1, 2, 1, file) == 1
            && fwrite(&bitmap->fileHeader.reserved2, 2, 1, file) == 1
            && fwrite(&bitmap->fileHeader.offBits, 4, 1, file) == 1
            && fwrite(&bitmap->infoHeader, sizeof(bitmap->infoHeader), 1, file) == 1) {
            stride = bitmap->infoHeader.width;
            if (stride & 3)
                stride = (stride | 3) + 1;
            if (bitmap->infoHeader.bitCount == 8)
                fwrite(bitmap->palette, 4, 256, file);
            fseek(file, bitmap->fileHeader.offBits, SEEK_SET);
            if (bitmap->infoHeader.bitCount == 8) {
                unsigned char* bits = (unsigned char*)bitmap->bits;
                for (y = 0; y < bitmap->infoHeader.height; y++) {
                    for (x = 0; x < bitmap->infoHeader.width; x++)
                        fputc(*bits++, file);
                    for (; x < stride; x++)
                        fputc(0, file);
                }
            } else if (bitmap->infoHeader.bitCount == 24) {
                // 16.16 fixed-point components, three per pixel.
                int* bits = (int*)bitmap->bits;
                for (i = 0; i < bitmap->infoHeader.height * bitmap->infoHeader.width; i++) {
                    unsigned char red = *bits++ >> 16;
                    unsigned char green = *bits++ >> 16;
                    unsigned char blue = *bits++ >> 16;
                    fputc(red, file);
                    fputc(green, file);
                    fputc(blue, file);
                }
            }
            fclose(file);
            return 1;
        }
        fclose(file);
    }
    return 0;
}

// 0x004245b0
void UnknownFunction4245b0(UnknownBitmapFile* bitmap) {
    if (bitmap) {
        if (bitmap->bits)
            DebugFree(bitmap->bits, __FILE__, 200);
        DebugFree(bitmap, __FILE__, 201);
    }
}

// 0x004245f0
void UnknownFunction4245f0(UnknownBitmapFile* bitmap, void* bits, unsigned char (*palette)[3], int width,
                           int height) {
    int i;

    bitmap->fileHeader.type = *(unsigned short*)"BM";
    bitmap->fileHeader.size = width * height + 0x436;
    bitmap->fileHeader.reserved1 = 0;
    bitmap->fileHeader.reserved2 = 0;
    bitmap->fileHeader.offBits = 0x436;
    bitmap->infoHeader.size = sizeof(bitmap->infoHeader);
    bitmap->infoHeader.width = width;
    bitmap->infoHeader.height = height;
    bitmap->infoHeader.planes = 1;
    bitmap->infoHeader.bitCount = 8;
    bitmap->infoHeader.compression = 0;
    bitmap->infoHeader.sizeImage = width * height;
    bitmap->infoHeader.xPelsPerMeter = 0;
    bitmap->infoHeader.yPelsPerMeter = 0;
    bitmap->infoHeader.clrUsed = 256;
    bitmap->infoHeader.clrImportant = 0;
    for (i = 0; i < 256; i++) {
        bitmap->palette[i][0] = (*palette)[2];
        bitmap->palette[i][1] = (*palette)[1];
        bitmap->palette[i][2] = (*palette)[0];
        bitmap->palette[i][3] = 0;
        palette++;
    }
    bitmap->bits = bits;
}
