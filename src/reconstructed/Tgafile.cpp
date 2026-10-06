// Tgafile.cpp: the format helpers before its literals (0x00511b54) and the
// TGA readers and writers after them (0x00511d00-0x00512dd0). Names are
// provisional; see Tgafile.h for the format codes.

#include "Tgafile.h"

#include <string.h>

#include "DebugAlloc.h"
#include "RenderInterfaces.h"
#include "TextureMap.h"

// 0x00511740: bytes per stored pixel of a file format.
int UnknownFunction511740(int fileFormat) {
    switch (fileFormat) {
    case 12:
    case 15:
    case 27:
    case 30:
    case 32:
    case 34:
        return 4;
    case 1:
    case 9:
    case 16:
    case 24:
        return 3;
    case 2:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 11:
    case 13:
    case 14:
    case 17:
    case 18:
    case 20:
    case 22:
    case 23:
    case 25:
    case 26:
    case 28:
    case 29:
    case 31:
    case 33:
        return 2;
    case 4:
    case 6:
    case 19:
    case 21:
        return 1;
    }
    return 0;
}

// 0x00511800: whether a file format is compressed. Formats 0..34 are all
// listed: retail's table starts at 0 and merges the 0 cases with the
// default.
int UnknownFunction511800(int fileFormat) {
    switch (fileFormat) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 28:
    case 29:
    case 30:
        return 0;
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 31:
    case 32:
    case 33:
    case 34:
        return 1;
    }
    return 0;
}

// 0x00511850: whether a file format stores mip levels.
int UnknownFunction511850(int fileFormat) {
    switch (fileFormat) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 31:
    case 32:
        return 0;
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 33:
    case 34:
        return 1;
    }
    return 0;
}

// 0x005118a0: the pixel format a file format decodes to.
int UnknownFunction5118a0(int fileFormat) {
    switch (fileFormat) {
    case 12:
    case 15:
    case 27:
    case 30:
    case 32:
    case 34:
        return 0x22b8;
    case 1:
    case 9:
    case 16:
    case 24:
        return 0x378;
    case 2:
    case 5:
    case 7:
    case 17:
    case 20:
    case 22:
        return 0x22b;
    case 3:
    case 8:
    case 18:
    case 23:
        return 0x235;
    case 10:
    case 13:
    case 25:
    case 28:
    case 31:
    case 33:
        return 0x613;
    case 11:
    case 14:
    case 26:
    case 29:
        return 0x115c;
    case 4:
    case 6:
    case 19:
    case 21:
        return 8;
    }
    return 0;
}

// 0x00511970: bytes per pixel of a pixel format. The 16-bit formats return
// separately; VC6 merges them into the block retail places first.
int UnknownFunction511970(int format) {
    switch (format) {
    case 8:
        return 1;
    case 0x22b:
        return 2;
    case 0x235:
        return 2;
    case 0x378:
        return 3;
    case 0x613:
        return 2;
    case 0x115c:
        return 2;
    case 0x22b8:
        return 4;
    }
    return 0;
}

// 0x005119c0: fills a DirectDraw pixel format (flags 0x40 RGB, 0x41 with
// alpha, 0x60 palettised). 8888 falls into the 24-bit case, so it reports
// 24 bits, and 1555 into 555, as in retail.
void UnknownFunction5119c0(int format, void* pixelFormat) {
    UnknownPixelFormat* description = (UnknownPixelFormat*)pixelFormat;
    memset(description, 0, sizeof(*description));
    description->size = sizeof(*description);
    description->flags = 0x40;
    switch (format) {
    case 0x235:
        description->bitCount = 16;
        description->masks[0] = 0xf800;
        description->masks[1] = 0x7e0;
        description->masks[2] = 0x1f;
        break;
    case 8:
        description->flags = 0x60;
        description->bitCount = 8;
        break;
    case 0x22b8:
        description->flags = 0x41;
        description->masks[3] = 0xff000000;
    case 0x378:
        description->bitCount = 24;
        description->masks[0] = 0xff0000;
        description->masks[1] = 0xff00;
        description->masks[2] = 0xff;
        break;
    case 0x115c:
        description->flags = 0x41;
        description->masks[3] = 0xf000;
        description->bitCount = 16;
        description->masks[0] = 0xf00;
        description->masks[1] = 0xf0;
        description->masks[2] = 0xf;
        break;
    case 0x613:
        description->flags = 0x41;
        description->masks[3] = 0x8000;
    case 0x22b:
        description->bitCount = 16;
        description->masks[0] = 0x7c00;
        description->masks[1] = 0x3e0;
        description->masks[2] = 0x1f;
        break;
    }
}

// 0x00511ad0: whether a pixel format is 4444 or 8888.
int UnknownFunction511ad0(int format) {
    if (format != 0x115c)
        return format == 0x22b8;
    return 1;
}

// 0x00511af0: the pixel format a DirectDraw pixel format describes: 555 or
// 565 by green mask, 24-bit, 8888 for 32 bits, palettised otherwise. Other
// 16-bit masks leave `format` unset (retail returns the argument's slot).
int UnknownFunction511af0(UnknownPixelFormat* pixelFormat) {
    int format;
    if (pixelFormat->bitCount == 16) {
        if (pixelFormat->masks[1] == 0x3e0)
            format = 0x22b;
        else if (pixelFormat->masks[1] == 0x7e0)
            format = 0x235;
    } else if (pixelFormat->bitCount == 24) {
        format = 0x378;
    } else {
        format = pixelFormat->bitCount == 32 ? 0x22b8 : 8;
    }
    return format;
}

// 0x00511b40: reads the header field by field. Only 16, 24 and 32-bit
// files of image type 2 (raw) or 10 (run-length) are accepted.
UnknownTgaFile* UnknownFunction511b40(UnknownTextureStream* stream, UnknownTgaFile* file, int offset) {
    if (!file) {
        file = (UnknownTgaFile*)DebugMalloc(sizeof(UnknownTgaFile), __FILE__, 354);
        if (!file)
            return 0;
        file->bits = 0;
        file->field_0x18 = 0;
        file->field_0x120 = 0;
        file->field_0x124 = 0;
    }
    file->name[0] = 0;
    if (offset == 0) {
        if (stream->field_0x1c)
            stream->UnknownFunction461340(stream->field_0x130, 0, 0);
    } else if (offset > 0) {
        stream->UnknownFunction461340(offset, 0, 1);
    }
    if (stream->UnknownFunction461640(&file->idLength, 1, 1) != 1 ||
        stream->UnknownFunction461640(&file->colorMapType, 1, 1) != 1 ||
        stream->UnknownFunction461640(&file->imageType, 1, 1) != 1 ||
        stream->UnknownFunction461640(&file->colorMapStart, 2, 1) != 1 ||
        stream->UnknownFunction461640(&file->colorMapLength, 2, 1) != 1 ||
        stream->UnknownFunction461640(&file->colorMapDepth, 1, 1) != 1 ||
        stream->UnknownFunction461640(&file->x, 2, 1) != 1 || stream->UnknownFunction461640(&file->y, 2, 1) != 1 ||
        stream->UnknownFunction461640(&file->width, 2, 1) != 1 ||
        stream->UnknownFunction461640(&file->height, 2, 1) != 1 ||
        stream->UnknownFunction461640(&file->bitsPerPixel, 1, 1) != 1 ||
        stream->UnknownFunction461640(&file->descriptor, 1, 1) != 1)
        goto failed;
    if ((file->bitsPerPixel == 16 || file->bitsPerPixel == 24 || file->bitsPerPixel == 32) &&
        (file->imageType == 2 || file->imageType == 10))
        return file;
failed:
    if (file) {
        if (file->bits)
            operator delete(file->bits, __FILE__, 421);
        operator delete(file, __FILE__, 422);
    }
    return 0;
}

// 0x00511d00: opens `path` "rb" in a new stream (Tgafile.cpp line 494) and
// reads its header; 0 when it cannot be opened.
UnknownTgaFile* UnknownFunction511d00(const char* path, UnknownTgaFile* file, int a) {
    UnknownTextureStream* stream = new(__FILE__, 494) UnknownTextureStream(a);
    if (!stream->UnknownFunction460f50(path, "rb", 0)) {
        delete stream;
        return 0;
    }
    file = UnknownFunction511b40(stream, file, 0);
    delete stream;
    return file;
}

// 0x00511dd0: reads the header and then the pixels for its depth; frees
// the file when the pixels cannot be read.
UnknownTgaFile* UnknownFunction511dd0(UnknownTextureStream* stream, UnknownTgaFile* file, int offset) {
    file = UnknownFunction511b40(stream, file, offset);
    if (!file)
        return 0;
    if (file->bitsPerPixel == 24) {
        if (!UnknownFunction511e80(file, stream))
            goto failed;
    } else if (file->bitsPerPixel == 32) {
        if (!UnknownFunction512100(file, stream))
            goto failed;
    } else if (!UnknownFunction512370(file, stream)) {
        goto failed;
    }
    strcpy(file->name, g_UnknownGlobal577738);
    return file;
failed:
    if (file->bits)
        operator delete(file->bits, __FILE__, 553);
    operator delete(file, __FILE__, 554);
    return 0;
}

// Swaps two bytes (red and blue). Retail's load order needs the reference
// form.
static inline void SwapBytes(unsigned char& a, unsigned char& b) {
    unsigned char swap = a;
    a = b;
    b = swap;
}

// 0x00511e80: reads 24-bit pixels, raw (image type 2) or run-length coded,
// swapping red and blue, then flips bottom-up images through the scratch
// row and marks them top-down.
int UnknownFunction511e80(UnknownTgaFile* file, UnknownTextureStream* stream) {
    int count = file->height * file->width;
    unsigned int size = count * 3;
    if (size > file->field_0x18) {
        if (file->bits)
            operator delete(file->bits, __FILE__, 40);
        file->bits = 0;
        file->field_0x18 = 0;
    }
    if (!file->bits) {
        file->bits = DebugMalloc(size, __FILE__, 49);
        if (!file->bits)
            return 0;
        file->field_0x18 = size;
    }
    UnknownPixel24* pixel = (UnknownPixel24*)file->bits;
    UnknownPixel24* end = (UnknownPixel24*)((unsigned char*)pixel + size);
    if (file->imageType == 2) {
        if (stream->UnknownFunction461640(pixel, 3, count) != count)
            return 0;
        for (int i = 0; i < count; i++, pixel++)
            SwapBytes(pixel->red, pixel->blue);
    } else {
        UnknownPixel24 color;
        while (pixel < end) {
            int packet = stream->UnknownFunction461980();
            if (packet & 0x80) {
                packet &= 0x7f;
                if (stream->UnknownFunction461640(&color, 3, 1) != 1)
                    return 0;
                SwapBytes(color.red, color.blue);
                do
                    *pixel++ = color;
                while (packet--);
            } else {
                packet++;
                if (stream->UnknownFunction461640(pixel, 3, packet) != packet)
                    return 0;
                for (int i = 0; i < packet; i++)
                    SwapBytes(pixel[i].red, pixel[i].blue);
                pixel += packet;
            }
        }
    }
    if (!(file->descriptor & 0x20)) {
        unsigned char* top = (unsigned char*)file->bits;
        unsigned char* bottom = top + (file->height - 1) * file->width * 3;
        if (file->field_0x120 && file->field_0x124 < (unsigned int)(file->width * 3)) {
            operator delete(file->field_0x120, __FILE__, 101);
            file->field_0x124 = 0;
            file->field_0x120 = 0;
        }
        if (!file->field_0x120) {
            file->field_0x120 = DebugMalloc(file->width * 3, __FILE__, 106);
            if (!file->field_0x120)
                return 0;
            file->field_0x124 = file->width * 3;
        }
        void* row = file->field_0x120;
        for (int i = 0; i < file->height / 2; i++) {
            memcpy(row, top, file->width * 3);
            memcpy(top, bottom, file->width * 3);
            memcpy(bottom, row, file->width * 3);
            bottom -= file->width * 3;
            top += file->width * 3;
        }
        file->descriptor |= 0x20;
    }
    return 1;
}

// 0x00512100: reads 32-bit pixels like 0x00511e80; literal run-length
// pixels are read one at a time.
int UnknownFunction512100(UnknownTgaFile* file, UnknownTextureStream* stream) {
    int count = file->height * file->width;
    unsigned int size = count * 4;
    if (size > file->field_0x18) {
        if (file->bits)
            operator delete(file->bits, __FILE__, 142);
        file->bits = 0;
        file->field_0x18 = 0;
    }
    if (!file->bits) {
        file->bits = DebugMalloc(size, __FILE__, 151);
        if (!file->bits)
            return 0;
        file->field_0x18 = size;
    }
    UnknownPixel32* pixel = (UnknownPixel32*)file->bits;
    UnknownPixel32* end = (UnknownPixel32*)((unsigned char*)pixel + size);
    if (file->imageType == 2) {
        if (stream->UnknownFunction461640(pixel, 4, count) != count)
            return 0;
        for (int i = 0; i < count; i++, pixel++)
            SwapBytes(pixel->red, pixel->blue);
    } else {
        UnknownPixel32 color;
        while (pixel < end) {
            int packet = stream->UnknownFunction461980();
            if (packet & 0x80) {
                packet &= 0x7f;
                if (stream->UnknownFunction461640(&color, 4, 1) != 1)
                    return 0;
                SwapBytes(color.red, color.blue);
                do
                    *pixel++ = color;
                while (packet--);
            } else {
                packet++;
                do {
                    if (stream->UnknownFunction461640(pixel, 4, 1) != 1)
                        return 0;
                    unsigned char red = pixel->red;
                    pixel->red = pixel->blue;
                    pixel->blue = red;
                    pixel++;
                } while (--packet);
            }
        }
    }
    if (!(file->descriptor & 0x20)) {
        unsigned char* top = (unsigned char*)file->bits;
        unsigned char* bottom = top + (file->height - 1) * file->width * 4;
        if (file->field_0x120 && file->field_0x124 < (unsigned int)(file->width * 4)) {
            operator delete(file->field_0x120, __FILE__, 203);
            file->field_0x124 = 0;
            file->field_0x120 = 0;
        }
        if (!file->field_0x120) {
            file->field_0x120 = DebugMalloc(file->width * 4, __FILE__, 208);
            if (!file->field_0x120)
                return 0;
            file->field_0x124 = file->width * 4;
        }
        void* row = file->field_0x120;
        for (int i = 0; i < file->height / 2; i++) {
            memcpy(row, top, file->width * 4);
            memcpy(top, bottom, file->width * 4);
            memcpy(bottom, row, file->width * 4);
            bottom -= file->width * 4;
            top += file->width * 4;
        }
        file->descriptor |= 0x20;
    }
    return 1;
}

// 0x00512370: reads 16-bit pixels like 0x00511e80, without the colour
// swap; raw images are read row by row.
int UnknownFunction512370(UnknownTgaFile* file, UnknownTextureStream* stream) {
    int count = file->height * file->width;
    unsigned int size = count * 2;
    if (size > file->field_0x18) {
        if (file->bits)
            operator delete(file->bits, __FILE__, 252);
        file->bits = 0;
        file->field_0x18 = 0;
    }
    if (!file->bits) {
        file->bits = DebugMalloc(size, __FILE__, 261);
        if (!file->bits)
            return 0;
        file->field_0x18 = size;
    }
    unsigned short* pixel = (unsigned short*)file->bits;
    unsigned short* end = pixel + count;
    if (file->imageType == 2) {
        for (int row = 0; row < file->height; row++) {
            if (stream->UnknownFunction461640(pixel, 2, file->width) != file->width)
                return 0;
            pixel += file->width;
        }
    } else {
        unsigned short color;
        while (pixel < end) {
            int packet = stream->UnknownFunction461980();
            if (packet & 0x80) {
                packet &= 0x7f;
                if (stream->UnknownFunction461640(&color, 2, 1) != 1)
                    return 0;
                do
                    *pixel++ = color;
                while (packet--);
            } else {
                packet++;
                if (stream->UnknownFunction461640(pixel, 2, packet) != packet)
                    return 0;
                pixel += packet;
            }
        }
    }
    if (!(file->descriptor & 0x20)) {
        unsigned char* top = (unsigned char*)file->bits;
        unsigned char* bottom = top + (file->height - 1) * file->width * 2;
        if (file->field_0x120 && file->field_0x124 < (unsigned int)(file->width * 2)) {
            operator delete(file->field_0x120, __FILE__, 311);
            file->field_0x124 = 0;
            file->field_0x120 = 0;
        }
        if (!file->field_0x120) {
            file->field_0x120 = DebugMalloc(file->width * 2, __FILE__, 316);
            if (!file->field_0x120)
                return 0;
            file->field_0x124 = file->width * 2;
        }
        void* row = file->field_0x120;
        for (int i = 0; i < file->height / 2; i++) {
            memcpy(row, top, file->width * 2);
            memcpy(top, bottom, file->width * 2);
            memcpy(bottom, row, file->width * 2);
            bottom -= file->width * 2;
            top += file->width * 2;
        }
        file->descriptor |= 0x20;
    }
    return 1;
}

// 0x005125c0: opens `path` "rb" in a new stream (line 567), reads the header
// and the pixels for its depth and names the file after the path; frees the
// file when the pixels cannot be read. Each failure deletes the stream on
// its own path.
UnknownTgaFile* UnknownFunction5125c0(const char* path, UnknownTgaFile* file, int a) {
    UnknownTextureStream* stream = new(__FILE__, 567) UnknownTextureStream(a);
    if (!stream->UnknownFunction460f50(path, "rb", 0)) {
        delete stream;
        return 0;
    }
    file = UnknownFunction511b40(stream, file, 0);
    if (!file) {
        delete stream;
        return 0;
    }
    if (file->bitsPerPixel == 24) {
        if (!UnknownFunction511e80(file, stream))
            goto failed;
    } else if (file->bitsPerPixel == 32) {
        if (!UnknownFunction512100(file, stream))
            goto failed;
    } else if (!UnknownFunction512370(file, stream)) {
        goto failed;
    }
    strcpy(file->name, path);
    delete stream;
    return file;
failed:
    if (file->bits)
        operator delete(file->bits, __FILE__, 608);
    operator delete(file, __FILE__, 609);
    delete stream;
    return 0;
}

// 0x00512720: writes 24-bit `bits` to the TGA file `path`.
int UnknownFunction512720(void* bits, int width, int height, unsigned int stride, const char* path, int descriptor) {
    UnknownTgaFile file;
    UnknownFunction5127a0(&file, bits, width, height);
    strcpy(file.name, path);
    return UnknownFunction512990(&file, stride, 0, descriptor);
}

// 0x005127a0: fills a 24-bit uncompressed true-colour header for `bits`.
void UnknownFunction5127a0(UnknownTgaFile* file, void* bits, int width, int height) {
    file->idLength = 0;
    file->colorMapType = 0;
    file->imageType = 2;
    file->colorMapStart = 0;
    file->colorMapLength = 0;
    file->colorMapDepth = 0;
    file->x = 0;
    file->y = 0;
    file->width = width;
    file->height = height;
    file->bitsPerPixel = 24;
    file->descriptor = 0;
    file->bits = bits;
}

// 0x005127f0: writes 32-bit `bits` to the TGA file `path`.
int UnknownFunction5127f0(void* bits, int width, int height, unsigned int stride, const char* path, int descriptor) {
    UnknownTgaFile file;
    UnknownFunction512870(&file, bits, width, height);
    strcpy(file.name, path);
    return UnknownFunction512990(&file, stride, 0, descriptor);
}

// 0x00512870: fills a 32-bit uncompressed true-colour header for `bits`.
void UnknownFunction512870(UnknownTgaFile* file, void* bits, int width, int height) {
    file->idLength = 0;
    file->colorMapType = 0;
    file->imageType = 2;
    file->colorMapStart = 0;
    file->colorMapLength = 0;
    file->colorMapDepth = 0;
    file->x = 0;
    file->y = 0;
    file->width = width;
    file->height = height;
    file->bitsPerPixel = 32;
    file->descriptor = 0;
    file->bits = bits;
}

// 0x005128c0: writes 16-bit `bits` to the TGA file `path`.
int UnknownFunction5128c0(void* bits, int width, int height, unsigned int stride, int greenMask, const char* path,
                          int descriptor) {
    UnknownTgaFile file;
    UnknownFunction512940(&file, bits, width, height);
    strcpy(file.name, path);
    return UnknownFunction512990(&file, stride, greenMask, descriptor);
}

// 0x00512940: fills a 16-bit uncompressed true-colour header for `bits`.
void UnknownFunction512940(UnknownTgaFile* file, void* bits, int width, int height) {
    file->idLength = 0;
    file->colorMapType = 0;
    file->imageType = 2;
    file->colorMapStart = 0;
    file->colorMapLength = 0;
    file->colorMapDepth = 0;
    file->x = 0;
    file->y = 0;
    file->width = width;
    file->height = height;
    file->bitsPerPixel = 16;
    file->descriptor = 0;
    file->bits = bits;
}

// 0x00512dd0: frees a loaded file, its pixels and +0x120.
void UnknownFunction512dd0(UnknownTgaFile* file) {
    if (file) {
        if (file->bits)
            operator delete(file->bits, __FILE__, 865);
        if (file->field_0x120)
            operator delete(file->field_0x120, __FILE__, 866);
        operator delete(file, __FILE__, 867);
    }
}
