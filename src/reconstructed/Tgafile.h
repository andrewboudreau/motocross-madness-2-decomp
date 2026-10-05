#pragma once

struct UnknownPixelFormat;

// Tgafile.cpp format helpers (all cdecl; Tgafile.cpp's literals follow
// them). File formats are the loader's 1..34 codes; pixel formats are the
// engine's 8 (palettised), 0x22b (555), 0x235 (565), 0x378 (24-bit),
// 0x613 (1555), 0x115c (4444) and 0x22b8 (8888). Names are provisional.

// 0x00511740: bytes per stored pixel of a file format (0 when unknown).
int UnknownFunction511740(int fileFormat);

// 0x00511800: whether a file format is compressed.
int UnknownFunction511800(int fileFormat);

// 0x00511850: whether a file format stores mip levels.
int UnknownFunction511850(int fileFormat);

// 0x005118a0: the pixel format a file format decodes to.
int UnknownFunction5118a0(int fileFormat);

// 0x00511970: bytes per pixel of a pixel format.
int UnknownFunction511970(int format);

// 0x005119c0: fills a DirectDraw pixel format for a pixel format.
void UnknownFunction5119c0(int format, void* pixelFormat);

// 0x00511ad0: whether a pixel format has an 8-bit or 4-bit alpha channel.
int UnknownFunction511ad0(int format);

// 0x00511af0: the pixel format a DirectDraw pixel format describes.
int UnknownFunction511af0(UnknownPixelFormat* pixelFormat);

// A TGA file being written: the header fields (unpacked; 0x00512990 writes
// them one by one), the pixels and the file name.
struct UnknownTgaFile {
    unsigned char idLength;
    unsigned char colorMapType;
    unsigned char imageType;                  // 2: uncompressed true colour
    unsigned char field_0x03;
    unsigned short colorMapStart;
    unsigned short colorMapLength;
    unsigned char colorMapDepth;
    unsigned char field_0x09;
    unsigned short x;
    unsigned short y;
    short width;
    short height;
    unsigned char bitsPerPixel;
    unsigned char descriptor;                 // 0x20: top-left origin
    void* bits;
    int field_0x18;
    char name[0x104];
    void* field_0x120;
    int field_0x124;
};

// 0x00512990: writes `file` with descriptor byte `descriptor`; rows lie
// `stride` bytes apart (0: packed). 16-bit pixels are 555 when `greenMask`
// is 0x3e0, else 565.
int UnknownFunction512990(UnknownTgaFile* file, unsigned int stride, int greenMask, int descriptor);

class UnknownTextureStream;

// Global buffer at 0x00577738 copied into a loaded file's name.
extern char g_UnknownGlobal577738[];

// 0x00511b40: reads a file's header from `stream` into a new UnknownTgaFile.
UnknownTgaFile* UnknownFunction511b40(UnknownTextureStream* stream, int a, int b);

// Pixel readers for a loaded header (24-bit, 32-bit, 16-bit); 0 on failure.
int UnknownFunction511e80(UnknownTgaFile* file, UnknownTextureStream* stream);
int UnknownFunction512100(UnknownTgaFile* file, UnknownTextureStream* stream);
int UnknownFunction512370(UnknownTgaFile* file, UnknownTextureStream* stream);

// 0x00511d00: opens `path` and reads its header (0x00511b40).
UnknownTgaFile* UnknownFunction511d00(const char* path, int a, int b);

// 0x00511dd0: reads a whole file from `stream`; 0 on failure.
UnknownTgaFile* UnknownFunction511dd0(UnknownTextureStream* stream, int a, int b);

// 0x00512dd0: frees a loaded file.
void UnknownFunction512dd0(UnknownTgaFile* file);

// Writers: fill a 24-bit (0x005127a0), 32-bit (0x00512870) or 16-bit
// (0x00512940) header for `bits`, name it `path` and write it.
int UnknownFunction512720(void* bits, int width, int height, unsigned int stride, const char* path, int descriptor);
void UnknownFunction5127a0(UnknownTgaFile* file, void* bits, int width, int height);
int UnknownFunction5127f0(void* bits, int width, int height, unsigned int stride, const char* path, int descriptor);
void UnknownFunction512870(UnknownTgaFile* file, void* bits, int width, int height);
int UnknownFunction5128c0(void* bits, int width, int height, unsigned int stride, int greenMask, const char* path,
                          int descriptor);
void UnknownFunction512940(UnknownTgaFile* file, void* bits, int width, int height);
