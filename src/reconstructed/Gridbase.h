#pragma once

// Gridbase.h -- the part of D:\aardvark\VC\krusty2\Gridbase.cpp that is
// reconstructed (0x0047db60..0x0047dd9b). See docs/GRIDDRAW.md.

class UnknownTextureStream;

// One eight-byte height-field sample. Tier 2: Griddraw's vertex cache
// 0x0047de90 reads the halfword at +2 and the signed bytes at +4..+6 of
// sample i; the names are tier 3.
struct GridBaseCell {
    unsigned char surface;  // +0x0  copied to GetCellCorners' bytes; Terrain::QueryGround returns the nearest corner's surface byte
    unsigned char errorIndex;  // +0x1  index into g_gridBaseCurve: CanDropVertex's vertex error; compared with the error limits' curve index
    unsigned short height;                   // +2 (relative to the block's +0xd4c)
    signed char normal[3];                   // +4..+6 (scaled by 1/127)
    unsigned char field_0x7_b0 : 1;          // +7 bit 0 (Griddraw's 0x0047fe70)
    unsigned char field_0x7_b1 : 7;
};

// One terrain height-field block. Tier 2: Terrain's loader (0x00505e73)
// allocates 0xd50 bytes with operator new(size, "Terrain.cpp", 0x1ab),
// constructs them with 0x0047dc20(stream) and stores the pointer in
// Terrain+0xc14[]. The three arrays are loaded raw or LZW-compressed.
class GridBaseBlock {
public:
    GridBaseBlock(UnknownTextureStream* stream);   // 0x0047dc20 (ret 4)

    GridBaseCell cells[17 * 17];             // +0x000 (0x908 bytes)
    short firstVertexByError[0x100];  // +0x908  first verticesByError position per curve value (0x00482760 walks the bands)
    short verticesByError[0x244 / 2];  // +0xB08  vertex indices sorted by curve error (0x00482760)
    int baseHeight;  // +0xD4C  height = ((cell.height - base) * reliefScale + base) * heightScale (0x00483910)
};

// 0x0047db60: builds the two lookup tables once (tier 3 name).
void BuildCurveTables();

extern unsigned short g_gridBaseCurve[256];           // 0x006752a4
extern unsigned char g_gridBaseCurveInverse[0x10000]; // 0x006652a4
