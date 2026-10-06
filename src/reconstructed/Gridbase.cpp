// Gridbase.cpp -- reconstruction of part of D:\aardvark\VC\krusty2\Gridbase.cpp.
// See Gridbase.h and docs/GRIDDRAW.md. The only __FILE__ reference
// (0x0047dc36, line 136) is in the GridBaseBlock constructor 0x0047dc20.

#include "Gridbase.h"

#include <math.h>
#include <string.h>

#include "DebugAlloc.h"
#include "Lzw.h"
#include "TextureMap.h"

unsigned char g_gridBaseCurveInverse[0x10000];        // 0x006652a4
unsigned short g_gridBaseCurve[256];                  // 0x006752a4
int g_gridBaseCurveBuilt;                             // 0x006754a4
unsigned char* g_gridBaseLoadBuffer;                  // 0x006754a8

// 0x0047db60: g_gridBaseCurve[i] = (i + 1) ^ (1 + i*i / 255^2) - 1, and the
// inverse byte table maps every value up to that entry back to i.
void UnknownFunction47db60()
{
    if (g_gridBaseCurveBuilt == 0) {
        int next = 0;
        for (int i = 0; i < 256; i++) {
            g_gridBaseCurve[i] = (unsigned short)((int)pow((double)i + 1.0,
                (double)i * (double)i * (1.0 / 65025.0) + 1.0) - 1);
            if (next <= g_gridBaseCurve[i]) {
                int count = g_gridBaseCurve[i] - next + 1;
                memset(&g_gridBaseCurveInverse[next], i, count);
                next += count;
            }
        }
        g_gridBaseCurveBuilt = 1;
    }
}

// 0x0047dc20: each array is stored as (packed size, unpacked size); a packed
// size smaller than the unpacked one means LZW data.
GridBaseBlock::GridBaseBlock(UnknownTextureStream* stream)
{
    int packed;
    int size;

    if (g_gridBaseLoadBuffer == 0)
        g_gridBaseLoadBuffer = (unsigned char*)DebugMalloc(0x908, __FILE__, 136);

    stream->UnknownFunction461640(&packed, 4, 1);
    stream->UnknownFunction461640(&size, 4, 1);
    if (packed < size) {
        stream->UnknownFunction461640(g_gridBaseLoadBuffer, 1, packed);
        UnknownFunction4a03d0(field_0x908, g_gridBaseLoadBuffer, 0x200);
    } else {
        stream->UnknownFunction461640(field_0x908, 1, size);
    }

    stream->UnknownFunction461640(&packed, 4, 1);
    stream->UnknownFunction461640(&size, 4, 1);
    if (packed < size) {
        stream->UnknownFunction461640(g_gridBaseLoadBuffer, 1, packed);
        UnknownFunction4a03d0(field_0xb08, g_gridBaseLoadBuffer, 0x244);
    } else {
        stream->UnknownFunction461640(field_0xb08, 1, size);
    }

    stream->UnknownFunction461640(&packed, 4, 1);
    stream->UnknownFunction461640(&size, 4, 1);
    if (packed < size) {
        stream->UnknownFunction461640(g_gridBaseLoadBuffer, 1, packed);
        UnknownFunction4a03d0((unsigned char*)cells, g_gridBaseLoadBuffer, 0x908);
    } else {
        stream->UnknownFunction461640(cells, 1, size);
    }

    stream->UnknownFunction461640(&field_0xd4c, 4, 1);
}
