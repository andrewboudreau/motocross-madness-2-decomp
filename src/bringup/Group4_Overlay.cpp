// Bring-up definitions (best-guess-unreviewed) for TrackOverlay.cpp methods
// the linked game references but src/reconstructed/TrackOverlay.cpp does not
// provide yet:
//   0x0051af00 DropTextOverlay::Setup  (decoded from retail, written here)
//   0x0051b070 DropTextOverlay::Tick   (vtable 0x005586f0 slot 10; decoded)
//   0x0051cf80 ChatOverlay::Setup      (taken unchanged from the near miss in
//                                       samples/track/TrackOverlayNearMisses.cpp,
//                                       1768 of 1785 non-relocation bytes)
// Not byte matches.
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../reconstructed/TrackOverlay.h"
#include "../reconstructed/GUIManagerCore.h"
#include "../reconstructed/DebugAlloc.h"
#include "../reconstructed/PCTextureMap.h"
#include "../reconstructed/Camera.h"
#include "../reconstructed/FollowCamera.h"
#include "../reconstructed/Track.h"
#include "../reconstructed/TrackGame.h"
#include "../reconstructed/EventManager.h"
#include "../reconstructed/ControlInterface.h"
#include "../reconstructed/D3DConstants.h"
#include "../reconstructed/PCRenderTarget.h"
// GDI32 imports, as in TrackOverlay.cpp.
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
extern "C" __declspec(dllimport) void* __stdcall SelectObject(void* dc, void* object);
extern "C" __declspec(dllimport) unsigned long __stdcall SetBkColor(void* dc, unsigned long color);
extern "C" __declspec(dllimport) int __stdcall SetBkMode(void* dc, int mode);
extern "C" __declspec(dllimport) unsigned long __stdcall SetTextColor(void* dc, unsigned long color);
extern "C" __declspec(dllimport) int __stdcall GetTextExtentPoint32A(void* dc, const char* text, int length,
                                                                     UnknownTextExtent* size);
extern "C" __declspec(dllimport) int __stdcall DrawTextA(void* dc, const char* text, int length,
                                                         UnknownOverlayRect* rect, unsigned int format);

// TrackOverlay.cpp statics: the radar zoom (0x0057513c) and the ChatOverlay

// Retail rounds these screen coordinates with a project `fld; fistp`
// helper applied to (value - 0.5): the x87 default round-to-nearest-even of
// value - 0.5, i.e. floor(value) except exact integers may round down by one
// on ties. best guess: reproduce that rounding portably.
static int Group4Ovl_FistpRound(double value)
{
    double low = floor(value);
    double diff = value - low;
    if (diff > 0.5)
        return (int)low + 1;
    if (diff < 0.5)
        return (int)low;
    return (fmod(low, 2.0) == 0.0) ? (int)low : (int)low + 1;
}

// 0x0051af00 (ret 0x18): GameObject slot 8 (SetRenderContext) on `value`;
// keeps dropSeconds; places the text given on a 640x480 screen; copies the
// text; creates the 28-high font (the localized tooltip face, else bold
// italic "Arial"). `textures` is not read. Returns this, or 0 after slot 2
// when the font fails.
DropTextOverlay* DropTextOverlay::Setup(void* value, TextureMapManager* textures, float dropSeconds,
                                        int x, int y, const char* text)
{
    UnknownLogFont logFont;
    GameObject::SetRenderContext(value);
    this->dropSeconds = dropSeconds;
    PCRenderTarget* target = (PCRenderTarget*)renderContext;
    textX = Group4Ovl_FistpRound((float)(target->targetWidth * (1.0f / 640.0f) * x) - 0.5f);
    textYOffset = Group4Ovl_FistpRound((float)(target->targetHeight * (1.0f / 480.0f) * y) - 0.5f);
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
    UnknownKrustyUIGui* gui = g_TrackGame->ui->gui;
    const char* face = gui ? gui->toolTipFontFace : "";
    if (*face != '\0') {
        logFont.lfWeight = gui->toolTipBold ? 700 : 500;
        logFont.lfItalic = (unsigned char)gui->toolTipItalic;
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

// 0x0051b070 (slot 10, ret 4): runs the base update with the game's frame
// time (TrackGame +0x2f0, not the argument), then while shown drops the
// text by elapsedTime / dropSeconds screen heights and hides it once it
// passes the bottom.
int DropTextOverlay::Tick(float frameTime)
{
    GameObject::Tick(g_TrackGame->frameTime);
    if (visible) {
        elapsedTime += g_TrackGame->frameTime;
        int drop = Group4Ovl_FistpRound((float)(elapsedTime / this->dropSeconds
                                                * ((PCRenderTarget*)renderContext)->targetHeight) - 0.5f);
        int y = textYOffset + drop;
        if (y > ((PCRenderTarget*)renderContext)->targetHeight)
            visible = 0;
        textY = y;
    }
    return 1;
}

extern "C" __declspec(dllimport) void* __stdcall CreatePen(int style, int width, unsigned long color);

// 0x0051cf80: sets the chat overlay up: the chat and cue rectangles, the
// texture, the side panels on wide screens, the pen, two fonts and the
// thirteen name-tag rectangles.
ChatOverlay* ChatOverlay::Setup(RenderTarget* target, TextureMapManager* manager, void* camera,
                                                UnknownOverlayRect screen, UnknownOverlayRect cue)
{
    const char* font;
    UnknownOverlayRect source;
    UnknownLogFont logFont;
    char name[260];
    char path[260];

    largeLayout = 1;
    chatRect.left = screen.right / 2 - 128;
    chatRect.top = 0;
    chatRect.right = chatRect.left + 256;
    chatRect.bottom = 68;
    strcpy(name, "chat256.tga");
    chatCamera = (UnknownChatCamera*)camera;
    cueRect = cue;
    timeSum = 0.08f;
    backingTexture = TerrainLoadTexture(manager, name, 4444, 0, 8, 5, 6, 0, 0x10, 0xff00ff, 1, 1);
    if (!backingTexture) {
        Release();
        return 0;
    }
    overlayTexture = backingTexture->Clone();
    if (overlayTexture->mapFormat == 4444)
        overlayTexture->SetAddressModes(5, 6);
    overlayTexture->CreateTextureSurface(1, 0, 0);
    source.left = 0;
    source.right = largeLayout ? 256 : 128;
    source.top = 0;
    source.bottom = largeLayout ? 68 : 37;
    font = "arialsm";
    sprintf(path, "%s\\%s", "Res", "Fonts.res");
    Attach(target, overlayTexture, &chatRect, 0, &source, 0.00001f, 1, path, &font, 1, 4444, 0);
    if (screen.right - screen.left > 512) {
        UnknownOverlayRect rect = chatRect;
        source.left = 3;
        source.top = 69;
        source.right = largeLayout ? 38 : 19;
        source.bottom = largeLayout ? 137 : 103;
        rect.left = chatRect.left - (largeLayout ? 38 : 19);
        rect.right = chatRect.left;
        leftPanel = new (__FILE__, 2284) Overlay(0, 1);
        leftPanel->Attach(target, overlayTexture, &rect, 0, &source, 0.00001f, 0, 0, 0, 0, 1555, 0);
        AppendChild(leftPanel, -1);
        source.left += largeLayout ? 38 : 19;
        source.top = 69;
        source.right += largeLayout ? 38 : 19;
        source.bottom = largeLayout ? 137 : 103;
        rect.left = chatRect.right;
        rect.right = chatRect.right + (largeLayout ? 38 : 19);
        rightPanel = new (__FILE__, 2294) Overlay(0, 1);
        rightPanel->Attach(target, overlayTexture, &rect, 0, &source, 0.00001f, 0, 0, 0, 0, 1555, 0);
        AppendChild(rightPanel, -1);
    }
    pen = CreatePen(0, 3, 0xc0c0c0);
    logFont.lfWidth = 0;
    logFont.lfEscapement = 0;
    logFont.lfOrientation = 0;
    logFont.lfUnderline = 0;
    logFont.lfStrikeOut = 0;
    logFont.lfCharSet = 1;
    logFont.lfOutPrecision = 0;
    logFont.lfClipPrecision = 0;
    logFont.lfQuality = 2;
    logFont.lfPitchAndFamily = 2;
    logFont.lfItalic = 0;
    logFont.lfHeight = 9;
    UnknownKrustyUIGui* gui = g_TrackGame->ui->gui;
    const char* face = gui ? gui->toolTipFontFace : "";
    if (*face != '\0') {
        logFont.lfWeight = gui->toolTipBold ? 700 : 500;
        strcpy(logFont.lfFaceName, face);
    } else {
        logFont.lfWeight = 400;
        strcpy(logFont.lfFaceName, "Lucida Console");
    }
    chatFont = CreateFontIndirectA(&logFont);
    if (*face == '\0')
        strcpy(logFont.lfFaceName, "Small Fonts");
    logFont.lfHeight = 10;
    nameTagFont = CreateFontIndirectA(&logFont);
    nameTagRects[0] = UnknownTrackOverlayRect(0, 0x80, 0x89, 0x93);
    nameTagRects[1] = UnknownTrackOverlayRect(0x81, 0xff, 0x89, 0x93);
    nameTagRects[2] = UnknownTrackOverlayRect(0, 0x80, 0x94, 0x9e);
    nameTagRects[3] = UnknownTrackOverlayRect(0x81, 0xff, 0x94, 0x9e);
    nameTagRects[4] = UnknownTrackOverlayRect(0, 0x80, 0x9f, 0xa9);
    nameTagRects[5] = UnknownTrackOverlayRect(0x81, 0xff, 0x9f, 0xa9);
    nameTagRects[6] = UnknownTrackOverlayRect(0, 0x80, 0xaa, 0xb4);
    nameTagRects[7] = UnknownTrackOverlayRect(0x81, 0xff, 0xaa, 0xb4);
    nameTagRects[8] = UnknownTrackOverlayRect(0, 0x80, 0xb5, 0xbf);
    nameTagRects[9] = UnknownTrackOverlayRect(0x81, 0xff, 0xb5, 0xbf);
    nameTagRects[10] = UnknownTrackOverlayRect(0, 0x80, 0xc0, 0xca);
    nameTagRects[11] = UnknownTrackOverlayRect(0x81, 0xff, 0xc0, 0xca);
    nameTagRects[12] = UnknownTrackOverlayRect(0, 0x80, 0xcb, 0xd5);
    return this;
}
