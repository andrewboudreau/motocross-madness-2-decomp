#include "VfwDeco.h"

#include <windows.h>
#include <vfw.h>

#include "DebugAlloc.h"

// vfwdeco.cpp: retail code 0x0052d050..0x0052d0c0, between Vehicle.cpp's
// VehicleCamera code and VideoCard.cpp (0x0052d0d0).

// 0x0052d050: closes the compressor, then sends ICM_DECOMPRESSEX_END to the
// (already closed) handle, in that retail order, then frees three buffers.
UnknownVideoDecoder::~UnknownVideoDecoder() {
    if (compressor) {
        ICClose(compressor);
        ICDecompressExEnd(compressor);
    }
    if (field_0x00)
        DebugFree(field_0x00, __FILE__, 178);
    if (field_0x10)
        DebugFree(field_0x10, __FILE__, 179);
    if (field_0x14)
        DebugFree(field_0x14, __FILE__, 180);
}
