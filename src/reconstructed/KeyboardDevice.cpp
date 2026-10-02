#include "KeyboardDevice.h"

#include "UnknownObject56e26c.h"

// 0x00489e30
KeyboardDevice::KeyboardDevice() : PCInputDevice(0) {
    unsigned int stamp = UnknownFunction4bfa80();
    field_0x08 = 256;
    for (int i = 0; i < 256; i++) {
        field_0x260[i].state = 0;
        field_0x260[i].field_0x04 = stamp;
        field_0x260[i].field_0x08 = stamp;
        field_0x260[i].field_0x0c = stamp;
        field_0x260[i].field_0x10 = stamp;
    }
    for (int j = 0; j < 6; j++)
        field_0x1660[j].Init(1, 1);
    field_0x16d8 = 0;
}

// 0x0048a030: drops every binding with id `id`.
void KeyboardDevice::UnknownVirtualSlot0(int id) {
    for (int list = 0; list < 6; list++) {
        for (int i = 0; i < field_0x1660[list].m_count; i++) {
            UnknownControlBinding* binding = field_0x1660[list].Get(i);
            if (binding->field_0x04 == id)
                field_0x1660[list].Remove(binding);
        }
    }
}

// 0x0048a0c0: while key +0x0c (or +0x10) is held, its timer advances by
// the frame time and every elapsed interval steps the binding down (or up);
// with neither key held the binding recentres.
void KeyboardDevice::UnknownFunction48a0c0(int list, int value) {
    for (int i = 0; i < field_0x1660[list].m_count; i++) {
        UnknownControlBinding* binding = field_0x1660[list].Get(i);
        binding->field_0x18 += g_UnknownGlobal56e26c->field_0x2f0;
        binding->field_0x14 += g_UnknownGlobal56e26c->field_0x2f0;
        if (UnknownVirtualSlot5(binding->field_0x0c, 0x3f, 0)) {
            if (binding->field_0x14 - g_UnknownGlobal56e26c->field_0x2f0 > binding->field_0x20) {
                binding->UnknownFunction43cd10();
                binding->field_0x14 = 0;
            }
            while (binding->field_0x14 > binding->field_0x20) {
                binding->UnknownFunction43cd90(-binding->field_0x1c);
                binding->field_0x14 -= binding->field_0x20;
            }
        }
        if (UnknownVirtualSlot5(binding->field_0x10, 0x3f, 0)) {
            if (binding->field_0x18 - g_UnknownGlobal56e26c->field_0x2f0 > binding->field_0x20) {
                binding->UnknownFunction43cd10();
                binding->field_0x18 = 0;
            }
            while (binding->field_0x18 > binding->field_0x20) {
                binding->UnknownFunction43cd90(binding->field_0x1c);
                binding->field_0x18 -= binding->field_0x20;
            }
        }
        if (!UnknownVirtualSlot5(binding->field_0x0c, 0x3f, 0) &&
            !UnknownVirtualSlot5(binding->field_0x10, 0x3f, 0))
            binding->UnknownFunction43cd10();
    }
}

// 0x0048a240: 0x3f always holds; with bit 2 of the global's +0x2d4 clear,
// states with bit 7 never do. 0x80 also holds while key 0x29 is down,
// 0x80000000 means no modifier, and bit 6 asks for an exact match.
int KeyboardDevice::UnknownFunction48a240(int modifier) {
    unsigned char flag = g_UnknownGlobal56e26c->field_0x2d4 & 4;
    if (!flag && (modifier & 0x80))
        return 0;
    if (modifier == 0x3f)
        return 1;
    if (flag && modifier == 0x80 && field_0x260[0x29].state == 1)
        return 1;
    if (modifier == (int)0x80000000)
        return field_0x16d8 == 0;
    if (modifier & 0x40)
        return (field_0x16d8 | 0x40) == modifier;
    return (field_0x16d8 & modifier) != 0;
}
