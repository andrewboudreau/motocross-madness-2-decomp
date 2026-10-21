// Validation-only prepared storage; actual production default bodies are linked.
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include "../../src/krusty2/motion/D3DIMSoultreeCharacter.h"

typedef char CharacterSize[sizeof(Character) == 0x1cc ? 1 : -1];
static unsigned char storage[sizeof(Character) + 32], expected[sizeof(storage)];
static int checks;

static void Call(Character* object, int slot, int argument)
{
    switch (slot) {
    case 0: object->Character::CharacterVirtualSlot0(); break;
    case 2: object->Character::CharacterVirtualSlot2(); break;
    case 3: object->Character::CharacterVirtualSlot3(argument); break;
    case 9: object->Character::CharacterVirtualSlot9(argument); break;
    case 10: object->Character::CharacterVirtualSlot10(); break;
    }
}

static int Check(Character* object)
{
    const int slots[] = {0, 2, 3, 9, 10};
    const unsigned arguments[] = {0, 1, 0xffffffff, 0x80000000, 0x7fffffff,
                                  0x12345678, 0x55555555, 0xaaaaaaaa};
    for (int index = 0; index < 5; ++index) {
        int count = slots[index] == 3 || slots[index] == 9 ? 8 : 1;
        for (int argument = 0; argument < count; ++argument) {
            unsigned control = _controlfp(0, 0), status = _statusfp();
            __try {
                Call(object, slots[index], (int)arguments[argument]);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                return 1;
            }
            if (_controlfp(0, 0) != control || _statusfp() != status ||
                memcmp(storage, expected, sizeof(storage)))
                return 2;
            ++checks;
        }
    }
    return 0;
}

int main()
{
    const unsigned rounding[] = {_RC_NEAR, _RC_DOWN, _RC_UP, _RC_CHOP};
    const unsigned precision[] = {_PC_64, _PC_53, _PC_24};
    for (unsigned seed = 0; seed < 256; ++seed) {
        for (unsigned i = 0; i < sizeof(storage); ++i)
            storage[i] = (unsigned char)(seed + 37 * i);
        memcpy(expected, storage, sizeof(storage));
        _fpreset();
        _controlfp(rounding[seed % 4] | precision[seed % 3], _MCW_RC | _MCW_PC);
        if (Check(reinterpret_cast<Character*>(storage + 16))) return 3;
    }
    // These qualified calls must not read a vptr, virtual-base table or argument.
    void* inaccessible = VirtualAlloc(0, 4096, MEM_RESERVE | MEM_COMMIT, PAGE_NOACCESS);
    if (!inaccessible) return 4;
    if (Check(0) || Check(static_cast<Character*>(inaccessible))) return 5;
    VirtualFree(inaccessible, 0, MEM_RELEASE);
    _fpreset();
    printf("Character defaults: %d complete-storage and no-read calls passed\n", checks);
    return 0;
}
