// Near-miss Game candidates, kept out of src/reconstructed until they
// match. See docs/GAME.md.
//
// Game::UnknownFunction467b70 (0x00467b70, 775 bytes; the candidate is
// 798): 747 of 798 bytes match. The frame, the slots and the body agree;
// only the tail differs. Retail's two failure exits (slot 32 and slot 3
// returning zero) jump to the single epilogue with eax still zero, and the
// slot 4 result (`neg; sbb; neg`) falls through into it. VC6 here emits
// `xor eax, eax` in the shared exit and gives the slot 4 return its own
// copy of the epilogue. Writing the slot 3 test as `if (slot3()) return
// slot4() != 0; return 0;` merges the two failure exits as retail does
// (406 -> 747; the early `if (!slot3()) return 0;` duplicated the epilogue
// for every return). Tried for the tail (session 10): `&&`, a ternary,
// `!!`, an if/else on a result variable, nesting the body in `if (slot32())`,
// gotos to a shared label and inline helpers (all 733-747); a result
// variable holding the call value gives retail's tail but tests it with
// `cmp eax, ebp` and moves the frame (402-488).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/Game.h"

#include "../../src/reconstructed/DebugOverlay.h"
#include "../../src/reconstructed/GameObject.h"
#include "../../src/reconstructed/MemTag.h"
#include "../../src/reconstructed/SoundInterface.h"
#include "../../src/reconstructed/TextureMapManager.h"

// Static in Game.cpp (decoded in place by Game's constructor).
extern char s_UnknownEncoded56b334[];
extern char s_UnknownEncoded56b350[];

// ADVAPI32 imports (called through the import table).
extern "C" __declspec(dllimport) long __stdcall RegOpenKeyExA(void* key, const char* subKey,
                                                             unsigned long options,
                                                             unsigned long access, void** result);
extern "C" __declspec(dllimport) long __stdcall RegQueryValueExA(void* key, const char* name,
                                                                unsigned long* reserved,
                                                                unsigned long* type,
                                                                unsigned char* data,
                                                                unsigned long* size);
extern "C" __declspec(dllimport) long __stdcall RegCloseKey(void* key);

// 0x00467b70: builds the two root objects (the second a child of the
// first), the sound interface and the random seed, reads the "TestKey"
// value under HKEY_LOCAL_MACHINE\\SOFTWARE\\Rainbow Studios into bit 2 of
// +0x2d4, then (when slot 32 allows) the texture manager, the debug overlay
// (only with that bit) and the "AllowFreezeCamera" setting.
int Game::UnknownFunction467b70(char*) {
    rootObject = new(__FILE__, 222) GameObject(1);
    childRoot = new(__FILE__, 227) GameObject(1);
    rootObject->AppendChild(childRoot, -1);
    soundInterface = new(__FILE__, 235) PCSoundInterface;
    srand(ReadClock());
    int count = ReadClock() & 0xff;
    do
        rand();
    while (count--);
    field_0x2d4_bit2 = 0;
    void* key;
    unsigned long type;
    unsigned long value;
    unsigned long size = sizeof(value);
    if (RegOpenKeyExA((void*)0x80000002, s_UnknownEncoded56b334, 0, 0x20019, &key) == 0 &&
        RegQueryValueExA(key, s_UnknownEncoded56b350, 0, &type, (unsigned char*)&value, &size) == 0 &&
        size == 4)
        field_0x2d4_bit2 = value;
    RegCloseKey(key);
    if (!UnknownVirtualSlot32())
        return 0;
    textureManager = new(__FILE__, 282) TextureMapManager;
    childRoot->AppendChild(textureManager->UnknownVirtualSlot8(renderTarget), -1);
    field_0x1c = textureManager;
    field_0x20 = 0;
    field_0x24 = 0;
    field_0x28 = renderTarget->surfacePixelFormat;
    field_0x2c = 0x115c;
    field_0x30 = 0;
    if (field_0x2d4_bit2) {
        int category = g_MemTagStack->Push("DebugOverlay");
        debugOverlay = (new(__FILE__, 305) DebugOverlay(0))->Setup(renderTarget, textureManager, 8, 3, 3);
        if (debugOverlay) {
            rootObject->AppendChild(debugOverlay, -1);
            if (g_UnknownGlobal56c470->field_0x2c) {
                rootObject->AppendChild(g_UnknownGlobal56c470, -1);
                g_UnknownGlobal56c470->UnknownVirtualSlot5();
            }
        }
        g_MemTagStack->Pop(category);
    }
    field_0x2d4_bit0 = GetRegistryFlag("AllowFreezeCamera", 0);
    if (UnknownVirtualSlot3())
        return UnknownVirtualSlot4() != 0;
    return 0;
}
