#include <stddef.h>
#include "Game.h"
#include "DebugOverlay.h"
#include "PeakHold.h"

typedef char PeakHolderSize[sizeof(PeakHold) == 12 ? 1 : -1];
typedef char PeakTimeOffset[offsetof(PeakHold, peakTime) == 0 ? 1 : -1];
typedef char PeakValueOffset[offsetof(PeakHold, peak) == 4 ? 1 : -1];
typedef char PeakHoldOffset[offsetof(PeakHold, holdTime) == 8 ? 1 : -1];
typedef char OverlaySize[sizeof(DebugOverlay) == 0x36e0 ? 1 : -1];
typedef char OverlayPageOffset[offsetof(DebugOverlay, nextPage) == 0x26c0 ? 1 : -1];
typedef char GameOverlayOffset[offsetof(Game, debugOverlay) == 0x38 ? 1 : -1];

// Native compilation also checks the actual owner/member pointer types.
DebugOverlay* GameProfiler(Game* game) { return game->debugOverlay; }
