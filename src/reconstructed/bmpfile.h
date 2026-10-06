#pragma once

// bmpfile.cpp (literal __FILE__ "D:\aardvark\VC\krusty2\bmpfile.cpp",
// 0x00568290): cdecl helpers that read and write uncompressed .bmp files.
// The record (0x4bc bytes: DebugMalloc'd at line 35) holds the file header,
// the info header at +0x10 and a 256-entry palette at +0x38; the field
// offsets and the values 0x4245f0 stores ("BM", 0x28, 0x436) agree with the
// Windows BITMAPFILEHEADER / BITMAPINFOHEADER / RGBQUAD layout, an inference
// the names below follow. Function names are provisional.

#pragma pack(push, 2)
// BITMAPFILEHEADER-shaped (14 bytes).
struct UnknownBitmapFileHeader {
    unsigned short type;                      // "BM"
    unsigned long size;
    unsigned short reserved1;
    unsigned short reserved2;
    unsigned long offBits;                    // where the pixels start
};
#pragma pack(pop)

// BITMAPINFOHEADER-shaped (0x28 bytes).
struct UnknownBitmapInfoHeader {
    unsigned long size;
    long width;
    long height;
    unsigned short planes;
    unsigned short bitCount;
    unsigned long compression;
    unsigned long sizeImage;
    long xPelsPerMeter;
    long yPelsPerMeter;
    unsigned long clrUsed;
    unsigned long clrImportant;
};

struct UnknownBitmapFile {
    UnknownBitmapFileHeader fileHeader;       // +0x000
    UnknownBitmapInfoHeader infoHeader;       // +0x010
    unsigned char palette[256][4];            // +0x038, blue/green/red/0
    void* bits;                               // +0x438, width x height bytes
    char name[0x80];                          // +0x43c, the file it was read from
};

// 0x00424140: reads the 8-bit bitmap `name` into `bitmap` (a new record when
// 0); 0 on failure, when the record is freed.
UnknownBitmapFile* UnknownFunction424140(const char* name, UnknownBitmapFile* bitmap);
// 0x00424380: writes `bitmap` (8-bit, or 24-bit from 16.16 components) to
// its `name`; 1 on success.
int UnknownFunction424380(UnknownBitmapFile* bitmap);
// 0x004245b0: frees a record and its pixels.
void UnknownFunction4245b0(UnknownBitmapFile* bitmap);
// 0x004245f0: describes an 8-bit width x height bitmap of `bits` with a
// 256-entry RGB palette.
void UnknownFunction4245f0(UnknownBitmapFile* bitmap, void* bits, unsigned char (*palette)[3], int width,
                           int height);
