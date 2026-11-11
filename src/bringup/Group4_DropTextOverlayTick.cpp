// Bring-up definition (best-guess-unreviewed) for a TrackOverlay.cpp method the
// linked game references but src/reconstructed/TrackOverlay.cpp does not
// provide yet:
//   0x0051b070 DropTextOverlay::Tick   (vtable 0x005586f0 slot 10; decoded)
// Not a byte match. It was split out of the former Group4_Overlay.cpp so that the game
// build can compile it with /QIfist (config/game_build.json bringup_flags);
// retail's body has the EBP frame of its inline-asm rounding helper, which a
// pragma keeps for Tick alone (docs/GAME_BUILD.md#frame-pointer-units).
//
// Retail rounds the drop with a project `fld; fistp` helper applied to
// (value - 0.5) (0x0051b0c3): floor(value) except that exact integers round to
// the even neighbour. Under /QIfist, FistpFloatToIntChecked's cast is a FISTP
// under the current rounding mode; the drop is the unit's only float-to-int
// conversion. The range check stays because dropSeconds near zero can push the
// drop outside int (or to an infinity or NaN at zero), where retail's 32-bit
// store gives 0x80000000.
#include "../reconstructed/TrackOverlay.h"
#include "../reconstructed/TrackGame.h"
#include "../reconstructed/PCRenderTarget.h"
#include "../../samples/common/FistpFloatToInt.h"

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
