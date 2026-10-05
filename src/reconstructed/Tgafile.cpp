// Tgafile.cpp's format helpers (the functions before its literals at
// 0x00511b54). Names are provisional; see Tgafile.h for the format codes.

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

// 0x00511d00: opens `path` "rb" in a new stream (Tgafile.cpp line 494) and
// reads its header; 0 when it cannot be opened.
UnknownTgaFile* UnknownFunction511d00(const char* path, int a, int b) {
    UnknownTextureStream* stream = new(__FILE__, 494) UnknownTextureStream(b);
    if (!stream->UnknownFunction460f50(path, "rb", 0)) {
        delete stream;
        return 0;
    }
    UnknownTgaFile* file = UnknownFunction511b40(stream, a, 0);
    delete stream;
    return file;
}

// 0x00511dd0: reads the header and then the pixels for its depth; frees
// the file when the pixels cannot be read.
UnknownTgaFile* UnknownFunction511dd0(UnknownTextureStream* stream, int a, int b) {
    UnknownTgaFile* file = UnknownFunction511b40(stream, a, b);
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
