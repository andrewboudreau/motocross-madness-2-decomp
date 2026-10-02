#include <stdio.h>
#include <string.h>

#include "DebugAlloc.h"
#include "Game.h"

#include "ControlInterface.h"

// 0x0065b4ac: a file cleared by the constructor and closed by the
// destructor (0x00534c3d is the CRT's fclose).
static FILE* s_UnknownFile65b4ac;

// 0x00467ae0 (identical code for slots 5 and 6)
int Game::UnknownVirtualSlot1() {
    return 1;
}

int Game::UnknownVirtualSlot5() {
    return 1;
}

int Game::UnknownVirtualSlot6() {
    return 1;
}

// 0x00467e80: slot 31's result is kept at +0x10 and handed to the interface.
int Game::UnknownVirtualSlot33() {
    field_0x10 = UnknownVirtualSlot31();
    if (!field_0x10)
        return 0;
    if (field_0x2f4)
        field_0x2f4->UnknownVirtualSlot25(field_0x10);
    return 1;
}

// 0x004685c0
int Game::UnknownVirtualSlot9() {
    field_0x0c->UnknownVirtualSlot3();
    return 1;
}

// 0x004688a0
int Game::UnknownVirtualSlot11(int value) {
    if (field_0x08)
        return 0;
    field_0x2d5_bit0 = value;
    field_0x2f4->UnknownVirtualSlot16(value);
    return 1;
}

// 0x004688e0
int Game::UnknownVirtualSlot12(int value) {
    return field_0x2f4->UnknownVirtualSlot19(value) != 0;
}

// 0x00468900: releases go to the interface unless bit 0 of +0x2d5 is set.
int Game::UnknownVirtualSlot13(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (field_0x2d5_bit0)
        return 1;
    return field_0x2f4->UnknownVirtualSlot22(event, entry) != 0;
}

// 0x00468930: presses go to the interface unless bit 0 of +0x2d5 is set.
// Control 0x20 also drives the +0x38 object; with bit 0 of +0x2d4, Ctrl+F
// (key 0x21 with modifier 0xc) toggles +0x1c4 and E (0x12) clears it,
// setting +0x1c8.
int Game::UnknownVirtualSlot14(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (field_0x2d5_bit0)
        return 1;
    if ((field_0x2d4 & 4) && field_0x38 && UnknownFunction43caa0(0x20, 0, event, 0x80)) {
        if (field_0x38->field_0x25_bit0) {
            if (field_0x38->UnknownFunction4484f0())
                field_0x38->UnknownVirtualSlot4();
        } else {
            field_0x38->UnknownVirtualSlot5();
        }
    }
    if (field_0x2d4 & 1) {
        if (field_0x14->UnknownVirtualSlot3(0x21, 0, 0xc, 0)) {
            field_0x1c4 = 1 - field_0x1c4;
        } else if (field_0x14->UnknownVirtualSlot3(0x12, 0, 0x3f, 0) && field_0x1c4) {
            field_0x1c8 = 1;
            field_0x1c4 = 0;
        }
    }
    return field_0x2f4->UnknownVirtualSlot23(event, entry) != 0;
}

// 0x00468a10
Game::~Game() {
    if (s_UnknownFile65b4ac)
        fclose(s_UnknownFile65b4ac);
}

// 0x00468a30: tears down the interface, the network object, the owners and
// the ControlInterface.
int Game::UnknownVirtualSlot15() {
    field_0x2d5_bit1 = 1;
    if (g_UnknownGlobal56c470 && field_0x38 && g_UnknownGlobal56c470->field_0x2c)
        g_UnknownGlobal56c470->UnknownFunction4691f0();
    g_UnknownStatic65b478.UnknownFunction4677c0();
    if (field_0x2f4) {
        field_0x2f4->UnknownVirtualSlot2();
        field_0x2f4 = 0;
    }
    if (field_0x08) {
        delete field_0x08;
        field_0x08 = 0;
    }
    if (field_0x04) {
        delete field_0x04;
        field_0x04 = 0;
    }
    if (field_0x14) {
        delete field_0x14;
        field_0x14 = 0;
    }
    if (field_0x10) {
        delete field_0x10;
        field_0x10 = 0;
    }
    field_0x0c = 0;
    UnknownFunction52d0d0();
    return 1;
}

// 0x00468ae0
int Game::UnknownVirtualSlot16(int value) {
    field_0x08 = new(__FILE__, 979) UnknownNetObject;
    if (field_0x08 && field_0x08->UnknownFunction4ab6b0(value) < 0) {
        delete field_0x08;
        field_0x08 = 0;
        return 0;
    }
    return field_0x08 != 0;
}

// 0x00468ba0
int Game::UnknownVirtualSlot17(int a, int b, int c, int d, int e) {
    return field_0x2f4->UnknownVirtualSlot24(a, b, c, d, e) != 0;
}

// 0x00468bd0: `path` = the +0x1cc directory, "\\" and `name`.
int Game::UnknownVirtualSlot18(const char* name, char* path) {
    strcpy(path, field_0x1cc);
    strcat(path, "\\");
    strcat(path, name);
    return 1;
}

// 0x00468c60
int Game::UnknownVirtualSlot30(int, char* text) {
    strcpy(text, "No Strings Available");
    return 0;
}

// 0x00468c90 (identical code for slot 4)
int Game::UnknownVirtualSlot3() {
    return 1;
}

int Game::UnknownVirtualSlot4() {
    return 1;
}
