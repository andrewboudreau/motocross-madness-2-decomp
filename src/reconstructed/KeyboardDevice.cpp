#include "KeyboardDevice.h"

#include "TrackGame.h"

// 0x00489e30
KeyboardDevice::KeyboardDevice() : PCInputDevice(0) {
    unsigned int stamp = UnknownFunction4bfa80();
    buttonCount = 256;
    for (int i = 0; i < 256; i++) {
        keyStates[i].state = 0;
        keyStates[i].releaseTime = stamp;
        keyStates[i].pressTime = stamp;
        keyStates[i].previousReleaseTime = stamp;
        keyStates[i].previousPressTime = stamp;
    }
    for (int j = 0; j < 6; j++)
        axisBindings[j].Init(1, 1);
    modifierState = 0;
}

// 0x00489f80: attaches `binding` to this device's list for its axis and
// gives it the keys that step it down and up.
int KeyboardDevice::UnknownFunction489f80(UnknownControlBinding* binding, int key, int key2,
                                          float step, float interval) {
    binding->field_0x00 = this;
    binding->field_0x1c = step;
    binding->field_0x20 = interval;
    axisBindings[binding->field_0x08].Add(binding);
    binding->UnknownFunction43cce0(key, key2);
    return 1;
}

// 0x0048a030: drops every binding with id `id`.
void KeyboardDevice::UnknownVirtualSlot0(int id) {
    for (int list = 0; list < 6; list++) {
        for (int i = 0; i < axisBindings[list].m_count; i++) {
            UnknownControlBinding* binding = axisBindings[list].Get(i);
            if (binding->field_0x04 == id)
                axisBindings[list].Remove(binding);
        }
    }
}

// 0x0048a0c0: while key +0x0c (or +0x10) is held, its timer advances by
// the frame time and every elapsed interval steps the binding down (or up);
// with neither key held the binding recentres.
void KeyboardDevice::UnknownFunction48a0c0(int list, int value) {
    for (int i = 0; i < axisBindings[list].m_count; i++) {
        UnknownControlBinding* binding = axisBindings[list].Get(i);
        binding->field_0x18 += g_TrackGame->frameTime;
        binding->field_0x14 += g_TrackGame->frameTime;
        if (UnknownVirtualSlot5(binding->field_0x0c, 0x3f, 0)) {
            if (binding->field_0x14 - g_TrackGame->frameTime > binding->field_0x20) {
                binding->UnknownFunction43cd10();
                binding->field_0x14 = 0;
            }
            while (binding->field_0x14 > binding->field_0x20) {
                binding->UnknownFunction43cd90(-binding->field_0x1c);
                binding->field_0x14 -= binding->field_0x20;
            }
        }
        if (UnknownVirtualSlot5(binding->field_0x10, 0x3f, 0)) {
            if (binding->field_0x18 - g_TrackGame->frameTime > binding->field_0x20) {
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
    if (!g_TrackGame->field_0x2d4_bit2 && (modifier & 0x80))
        return 0;
    if (modifier == 0x3f)
        return 1;
    if (g_TrackGame->field_0x2d4_bit2 && modifier == 0x80 && keyStates[0x29].state == 1)
        return 1;
    if (modifier == (int)0x80000000)
        return modifierState == 0;
    if (modifier & 0x40)
        return (modifierState | 0x40) == modifier;
    return (modifierState & modifier) != 0;
}
