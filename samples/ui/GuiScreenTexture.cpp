// Complete menu screen capture candidate; original TU ownership remains unproven.
#include <windows.h>
#include <imm.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/reconstructed/GUIManager.h"

#include "../../src/reconstructed/BackgroundImage.h"
#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/DebugOverlay.h"
#include "../../src/reconstructed/JoystickDevice.h"
#include "../../src/reconstructed/MatrixUtil.h"
#include "../../src/reconstructed/MouseDevice.h"
#include "../../src/reconstructed/PCAudio.h"
#include "../../src/reconstructed/PCTextureMap.h"
#include "../../src/reconstructed/Palette8.h"
#include "../../src/reconstructed/Tgafile.h"
#include "../../src/reconstructed/TrackGame.h"


// 0x00486170
PCTextureMap* GUIManager::CopyScreenToTexture(int dim, CameraRect* rect) {
    UnknownSurfaceDesc desc;
    UnknownSurfaceInterface* surface;
    UnknownDisplay* display;
    PCTextureMap* texture = 0;
    unsigned char* bits;
    unsigned char* row;
    unsigned char* pixel;
    unsigned short* pixel16;
    unsigned short mask;
    int format;
    int bytesPerPixel;
    int left;
    int top;
    int right;
    int bottom;
    int width;
    int height;
    int pixels;
    int y;
    unsigned int line;
    unsigned int x;

    format = g_TrackGame->renderTarget->surfacePixelFormat;
    bytesPerPixel = PixelFormatBytesPerPixel(format);
    if (guiBackground)
        guiBackground->ReleaseDc();
    g_TrackGame->childRoot->DisableByClassName("GUICursor");
    g_TrackGame->childRoot->DisableByClassName("ToolTip");
    g_TrackGame->RenderFrame();
    g_TrackGame->PresentFrame();
    g_TrackGame->renderTarget->AdvanceFrame();
    g_TrackGame->childRoot->EnableByClassName("GUICursor");
    g_TrackGame->childRoot->EnableByClassName("ToolTip");
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    display = g_TrackGame->display;
    if (display->renderSurface)
        surface = display->renderSurface;
    else
        surface = display->primarySurface;
    if (!surface->Lock(0, &desc, 0x811, 0)) {
        if (rect) {
            right = rect->right;
            left = rect->left;
            top = rect->top;
            bottom = rect->bottom;
        } else {
            left = 0;
            top = 0;
            right = desc.width;
            bottom = desc.height;
        }
        width = right - left;
        height = bottom - top;
        pixels = width * height;
        bits = (unsigned char*)DebugMalloc(pixels * bytesPerPixel, __FILE__, 849);
        row = bits;
        for (y = 0; y < height; y++) {
            memcpy(row, (unsigned char*)desc.surface + (y + top) * desc.pitch + left * bytesPerPixel,
                   width * bytesPerPixel);
            row += width * bytesPerPixel;
        }
        surface->Unlock(0);
        if (dim) {
            if (bytesPerPixel == 1) {
                for (line = 0; line < (unsigned int)height; line++) {
                    for (x = line * width + (line & 1); x < line * width + width; x += 2)
                        bits[x] = 0;
                }
            } else if (bytesPerPixel == 2) {
                mask = format == 0x22b ? 0x7bde : 0xf7de;
                pixel16 = (unsigned short*)bits;
                for (; pixels > 0; pixels--, pixel16++)
                    *pixel16 = (*pixel16 >> 1) & (mask >> 1);
            } else if (bytesPerPixel == 3) {
                for (pixel = bits; pixels > 0; pixels--, pixel += 3) {
                    pixel[0] >>= 1;
                    pixel[1] >>= 1;
                    pixel[2] >>= 1;
                }
            } else if (bytesPerPixel == 4) {
                for (pixel = bits; pixels > 0; pixels--, pixel += 4) {
                    pixel[0] >>= 1;
                    pixel[1] >>= 1;
                    pixel[2] >>= 1;
                }
            }
        }
        texture = new(__FILE__, 916) PCTextureMap(guiTextures, 1);
        texture->CreateSurface(bits, width, height, width, width, format, format, 0, 4, 0, 0, 0, 2, 1, 0, 0x80,
                               0xff00ff);
        DebugFree(bits, __FILE__, 921);
    }
    return texture;
}
