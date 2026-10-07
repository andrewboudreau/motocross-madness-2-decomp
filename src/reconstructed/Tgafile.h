#pragma once

struct UnknownPixelFormat;

// Tgafile.cpp format helpers (all cdecl; Tgafile.cpp's literals follow
// them). File formats are the loader's 1..34 codes; pixel formats are the
// engine's 8 (palettised), 555 (555), 565 (565), 888 (24-bit),
// 1555 (1555), 4444 (4444) and 8888 (8888). Names are provisional.

// 0x00511740: bytes per stored pixel of a file format (0 when unknown).
int BytesPerPixel(int fileFormat);

// 0x00511800: whether a file format is compressed.
int IsCompressedFormat(int fileFormat);

// 0x00511850: whether a file format stores mip levels.
int HasMipLevels(int fileFormat);

// 0x005118a0: the pixel format a file format decodes to.
int DecodedFormat(int fileFormat);

// 0x00511970: bytes per pixel of a pixel format.
int UnknownFunction511970(int format);

// 0x005119c0: fills a DirectDraw pixel format for a pixel format.
void UnknownFunction5119c0(int format, void* pixelFormat);

// 0x00511ad0: whether a pixel format is 4444 or 8888.
int UnknownFunction511ad0(int format);

// 0x00511af0: the pixel format a DirectDraw pixel format describes.
int FormatFromPixelFormat(UnknownPixelFormat* pixelFormat);

// A TGA file being read or written: the header fields (unpacked; 0x00512990 writes
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
    unsigned int bitsSize;                    // size of `bits`
    char name[0x104];
    void* flipRow;                            // scratch row for flipping
    unsigned int flipRowSize;                 // its size
};

// 0x00512990: writes `file` with descriptor byte `descriptor`; rows lie
// `stride` bytes apart (0: packed). 16-bit pixels are 555 when `greenMask`
// is 0x3e0, else 565.
int UnknownFunction512990(UnknownTgaFile* file, unsigned int stride, int greenMask, int descriptor);

class UnknownTextureStream;

// Global buffer at 0x00577738 copied into a loaded file's name.
extern char g_UnknownGlobal577738[];

// 0x00511b40: reads a header from `stream` (after seeking by `offset`) into
// `file`, or a new UnknownTgaFile when it is 0; frees the file on failure.
UnknownTgaFile* ReadTgaHeader(UnknownTextureStream* stream, UnknownTgaFile* file, int offset);

// Pixel readers for a loaded header (24-bit, 32-bit, 16-bit); 0 on failure.
int ReadTgaPixels24(UnknownTgaFile* file, UnknownTextureStream* stream);
int ReadTgaPixels32(UnknownTgaFile* file, UnknownTextureStream* stream);
int ReadTgaPixels16(UnknownTgaFile* file, UnknownTextureStream* stream);

// 0x00511d00: opens `path` and reads its header (0x00511b40).
UnknownTgaFile* UnknownFunction511d00(const char* path, UnknownTgaFile* file, int a);

// 0x00511dd0: reads a whole file from `stream`; 0 on failure.
UnknownTgaFile* UnknownFunction511dd0(UnknownTextureStream* stream, UnknownTgaFile* file, int offset);

// 0x005125c0: loads the whole file at `path`, named after it; 0 on failure.
UnknownTgaFile* UnknownFunction5125c0(const char* path, UnknownTgaFile* file, int a);

// 0x00512dd0: frees a loaded file.
void UnknownFunction512dd0(UnknownTgaFile* file);

// Writers: fill a 24-bit (0x005127a0), 32-bit (0x00512870) or 16-bit
// (0x00512940) header for `bits`, name it `path` and write it.
int WriteTga24(void* bits, int width, int height, unsigned int stride, const char* path, int descriptor);
void FillTgaHeader24(UnknownTgaFile* file, void* bits, int width, int height);
int WriteTga32(void* bits, int width, int height, unsigned int stride, const char* path, int descriptor);
void FillTgaHeader32(UnknownTgaFile* file, void* bits, int width, int height);
int WriteTga16(void* bits, int width, int height, unsigned int stride, int greenMask, const char* path,
                          int descriptor);
void FillTgaHeader16(UnknownTgaFile* file, void* bits, int width, int height);
