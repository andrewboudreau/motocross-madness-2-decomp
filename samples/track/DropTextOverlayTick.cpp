// DropTextOverlayTick.cpp -- DropTextOverlay::Tick (0x0051b070, 123 B, ret 4; vtable
// 0x005586f0 slot 10) of D:\aardvark\VC\krusty2\TrackOverlay.cpp (canonical source
// src/reconstructed/TrackOverlay.cpp, which does not define it).  Status:
// reviewed-nonexact (decoded from retail, differentially tested); not a byte match.
// The game build links it with /QIfist (config/game_build.json candidate_sources) in
// place of the former src/bringup/Group4_DropTextOverlayTick.cpp copy; Setup
// (0x0051af00) is DropTextOverlaySetup.cpp next to it.
//
// Retail rounds the drop with a project `fld; fistp` helper applied to
// (value - 0.5) (0x0051b0c3): floor(value) except that exact integers round to
// the even neighbour. Under /QIfist, FistpFloatToIntChecked's cast is a FISTP
// under the current rounding mode; the drop is the file's only float-to-int
// conversion. The range check stays because dropSeconds near zero can push the
// drop outside int (or to an infinity or NaN at zero), where retail's 32-bit
// store gives 0x80000000.  Differentially tested with tools/emu_cases/drop_text_tick.py
// (0 of 400 runs differ, 2026-11-28).  Retail's body has the EBP frame of its
// inline-asm rounding helper, which a pragma keeps for Tick alone
// (docs/GAME_BUILD.md#frame-pointer-units).
#include "../../src/reconstructed/TrackOverlay.h"
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/PCRenderTarget.h"
#include "../common/FistpFloatToInt.h"

// 0x0051b070 (slot 10, ret 4): runs the base update with the game's frame
// time (TrackGame +0x2f0, not the argument), then while shown drops the
// text by elapsedTime / dropSeconds screen heights and hides it once it
// passes the bottom.
// Retail's inline-asm fistp helper gives this function an EBP frame; the
// pragma keeps the frame pointer for it alone (docs/GAME_BUILD.md#frame-pointer-units).
#pragma optimize("y", off)
int DropTextOverlay::Tick(float frameTime)
{
    GameObject::Tick(g_TrackGame->frameTime);
    if (visible) {
        elapsedTime += g_TrackGame->frameTime;
        int drop;
        FistpFloatToIntChecked(&drop, elapsedTime / this->dropSeconds
                                      * ((PCRenderTarget*)renderContext)->targetHeight - 0.5f);
        int y = textYOffset + drop;
        if (y > ((PCRenderTarget*)renderContext)->targetHeight)
            visible = 0;
        textY = y;
    }
    return 1;
}
#pragma optimize("", on)
