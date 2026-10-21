#include "Game.h"
#include "HiResMeter.h"
#include <stddef.h>

typedef char MeterSize[sizeof(HiResMeter) == 0xe50 ? 1 : -1];
typedef char MeterCounter[offsetof(HiResMeter, counterUsable) == 0x2c ? 1 : -1];
typedef char ViewCounter[offsetof(GameHiResMeterView, counterUsable) == 0x2c ? 1 : -1];

GameObject* MeterObjectBase(HiResMeter* meter) { return meter; }
GameObject* MeterViewBase(GameHiResMeterView* view) { return view; }
