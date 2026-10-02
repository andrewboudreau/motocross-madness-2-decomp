#include <stdio.h>
#include <string.h>

#include "Game.h"

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

// 0x00468a10
Game::~Game() {
    if (s_UnknownFile65b4ac)
        fclose(s_UnknownFile65b4ac);
}

// 0x00468ba0
int Game::UnknownVirtualSlot17(int a, int b, int c, int d, int e) {
    return field_0x2f4->UnknownVirtualSlot24(a, b, c, d, e) != 0;
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
