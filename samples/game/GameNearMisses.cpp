// Near-miss Game candidates, kept out of src/reconstructed until they
// match. See docs/GAME.md.
//
// Game::UnknownFunction467b70 (0x00467b70, 775 bytes): 761 of 775 bytes
// match. Only the frame slots differ: retail places the registry key at
// +0x10, the size at +0x14, the type at +0x18, the value at +0x1c and the
// `new` temporary at +0x20; VC6 here puts the temporary at +0x1c and orders
// the registry locals differently. Declaration order, block scope (763),
// declarations at the top of the function, renamed locals and an inline
// helper (worse) do not reproduce retail's layout.
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
    field_0x2f4 = new(__FILE__, 222) GameObject(1);
    field_0x34 = new(__FILE__, 227) GameObject(1);
    field_0x2f4->UnknownFunction469190(field_0x34, -1);
    field_0x04 = new(__FILE__, 235) PCSoundInterface;
    srand(UnknownFunction4bfa80());
    int count = UnknownFunction4bfa80() & 0xff;
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
    field_0x3c = new(__FILE__, 282) TextureMapManager;
    field_0x34->UnknownFunction469190(field_0x3c->UnknownVirtualSlot8(field_0x10), -1);
    field_0x1c = field_0x3c;
    field_0x20 = 0;
    field_0x24 = 0;
    field_0x28 = field_0x10->field_0x28;
    field_0x2c = 0x115c;
    field_0x30 = 0;
    if (field_0x2d4_bit2) {
        int category = g_MemTagStack->Push("DebugOverlay");
        field_0x38 = (new(__FILE__, 305) DebugOverlay(0))->UnknownFunction447de0(field_0x10, field_0x3c, 8, 3, 3);
        if (field_0x38) {
            field_0x2f4->UnknownFunction469190(field_0x38, -1);
            if (g_UnknownGlobal56c470->field_0x2c) {
                field_0x2f4->UnknownFunction469190(g_UnknownGlobal56c470, -1);
                g_UnknownGlobal56c470->UnknownVirtualSlot5();
            }
        }
        g_MemTagStack->Pop(category);
    }
    field_0x2d4_bit0 = UnknownVirtualSlot22("AllowFreezeCamera", 0);
    if (!UnknownVirtualSlot3())
        return 0;
    return UnknownVirtualSlot4() != 0;
}
