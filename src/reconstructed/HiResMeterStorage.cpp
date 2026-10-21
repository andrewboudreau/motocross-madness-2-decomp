#include "HiResMeter.h"

class GameHiResMeterView;
extern HiResMeter g_HiResMeterInstance;

// Initialized writable pointer at 0x0056c470, pointing at the existing
// HiResMeter instance (0x0067b468). Retail RTTI places its GameObject base
// at offset zero. Game's smaller view shares that base and counterUsable
// at +0x2c; the cast does not construct a second object.
GameHiResMeterView* g_HiResMeter =
    reinterpret_cast<GameHiResMeterView*>(&g_HiResMeterInstance);
