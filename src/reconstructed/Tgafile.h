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

// 0x005127f0: writes width x height pixels of `depth` bits to the TGA file
// `path`.
int UnknownFunction5127f0(void* bits, int width, int height, int a, const char* path, int depth);
