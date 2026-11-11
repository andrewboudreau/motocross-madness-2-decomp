// DropTextOverlaySetup.cpp -- DropTextOverlay::Setup (0x0051af00, 361 B, ret 0x18) of
// D:\aardvark\VC\krusty2\TrackOverlay.cpp (canonical source src/reconstructed/TrackOverlay.cpp,
// which does not define it).  Status: best-guess-unreviewed.  The game build links it with
// /QIfist (config/game_build.json candidate_sources) in place of the
// src/bringup/Group4_Overlay.cpp copy; Tick (0x0051b070) stays there.
//
// GameObject slot 8 (SetRenderContext, 0x004692f0) on `value`; keeps dropSeconds (+0x38);
// places the text given on a 640x480 screen (textX +0x3c, textYOffset +0x40, textY +0x44);
// copies the text (+0x48); creates the 28-high font (the localized tooltip face with its bold
// and italic flags, else bold italic "Arial").  `textures` is not read.  Returns this, or 0
// after slot 2 (Release) when the font fails.
//
// Checked against the retail disassembly: the call, every field offset, both scale constants
// (1/640 at 0x005550c4, 1/480 at 0x00555a94) and the product order (target size * scale * x),
// the LOGFONT bytes, the face and weight choice (700 or 500 from gui+0x3d4, italic from
// gui+0x3d8, "" at 0x00577738 without a gui) and the failure path.  Retail rounds both
// coordinates with its inline fistp helper (0x0051af3e, 0x0051af64): floor of the value apart
// from exact ties, under the current rounding mode.  Screen coordinates are in int range, so
// FistpFloatToInt's 64-bit store equals retail's 32-bit one.  Differentially tested with
// tools/emu_cases/drop_text_setup.py: 0 of 4000 runs differ (53- and 24-bit precision).
//
// Shape (linked_accuracy masked, /QIfist): 0.632, 124 against 126 instructions.  Reading
// g_TrackGame->ui->gui at each use follows retail, which reloads it for the italic flag; a
// `gui` local scores 0.613.  VC6 still schedules both coordinate products before the first
// store, where retail's inline-asm helper finishes x (pointer kept in the `value` slot) before
// y; with the ebp frame that is most of the remaining difference.
#include <string.h>

#include "../../src/reconstructed/TrackOverlay.h"
#include "../../src/reconstructed/GUIManagerCore.h"
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/PCRenderTarget.h"
#include "../common/FistpFloatToInt.h"

// GDI32 import, as in TrackOverlay.cpp.
struct UnknownLogFont {
    long lfHeight;
    long lfWidth;
    long lfEscapement;
    long lfOrientation;
    long lfWeight;
    unsigned char lfItalic;
    unsigned char lfUnderline;
    unsigned char lfStrikeOut;
    unsigned char lfCharSet;
    unsigned char lfOutPrecision;
    unsigned char lfClipPrecision;
    unsigned char lfQuality;
    unsigned char lfPitchAndFamily;
    char lfFaceName[32];
};
extern "C" __declspec(dllimport) void* __stdcall CreateFontIndirectA(const UnknownLogFont* font);

DropTextOverlay* DropTextOverlay::Setup(void* value, TextureMapManager* textures, float dropSeconds,
                                        int x, int y, const char* text)
{
    UnknownLogFont logFont;
    GameObject::SetRenderContext(value);
    this->dropSeconds = dropSeconds;
    PCRenderTarget* target = (PCRenderTarget*)renderContext;
    // The parentheses are redundant in C but stop VC6 from reassociating the
    // product as size * x * scale, which rounds differently under 24-bit
    // precision (tools/emu_cases/drop_text_setup.py).
    FistpFloatToInt(&textX, (target->targetWidth * (1.0f / 640.0f)) * x - 0.5f);
    FistpFloatToInt(&textYOffset, (target->targetHeight * (1.0f / 480.0f)) * y - 0.5f);
    textY = textYOffset;
    strcpy(caption, text);
    logFont.lfHeight = 28;
    logFont.lfCharSet = 1;
    logFont.lfItalic = 1;
    logFont.lfWidth = 0;
    logFont.lfEscapement = 0;
    logFont.lfOrientation = 0;
    logFont.lfUnderline = 0;
    logFont.lfStrikeOut = 0;
    logFont.lfOutPrecision = 0;
    logFont.lfClipPrecision = 0;
    logFont.lfQuality = 2;
    logFont.lfPitchAndFamily = 2;
    const char* face = g_TrackGame->ui->gui ? g_TrackGame->ui->gui->toolTipFontFace : "";
    if (*face != '\0') {
        logFont.lfWeight = g_TrackGame->ui->gui->toolTipBold ? 700 : 500;
        logFont.lfItalic = (unsigned char)g_TrackGame->ui->gui->toolTipItalic;
    } else {
        logFont.lfWeight = 700;
        face = "Arial";
    }
    strcpy(logFont.lfFaceName, face);
    textFont = CreateFontIndirectA(&logFont);
    if (!textFont) {
        Release();
        return 0;
    }
    return this;
}
