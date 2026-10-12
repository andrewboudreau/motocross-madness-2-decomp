#include "KeyboardDevice.h"

#include "TrackGame.h"

// 0x00489e30
KeyboardDevice::KeyboardDevice() : PCInputDevice(0) {
    unsigned int stamp = ReadClock();
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
int KeyboardDevice::BindKeys(UnknownControlBinding* binding, int key, int key2,
                                          float step, float interval) {
    binding->owner = this;
    binding->repeatStep = step;
    binding->repeatInterval = interval;
    axisBindings[binding->axisIndex].Add(binding);
    binding->SetKeys(key, key2);
    return 1;
}

// 0x0048a030: drops every binding with id `id`.
void KeyboardDevice::UnknownVirtualSlot0(int id) {
    for (int list = 0; list < 6; list++) {
        for (int i = 0; i < axisBindings[list].m_count; i++) {
            UnknownControlBinding* binding = axisBindings[list].Get(i);
            if (binding->bindingId == id)
                axisBindings[list].Remove(binding);
        }
    }
}

// 0x0048a0c0: while key +0x0c (or +0x10) is held, its timer advances by
// the frame time and every elapsed interval steps the binding down (or up);
// with neither key held the binding recentres.
void KeyboardDevice::RepeatKeys(int list, int value) {
    for (int i = 0; i < axisBindings[list].m_count; i++) {
        UnknownControlBinding* binding = axisBindings[list].Get(i);
        binding->secondKeyTimer += g_TrackGame->frameTime;
        binding->firstKeyTimer += g_TrackGame->frameTime;
        if (UnknownVirtualSlot5(binding->firstKey, 0x3f, 0)) {
            if (binding->firstKeyTimer - g_TrackGame->frameTime > binding->repeatInterval) {
                binding->Recenter();
                binding->firstKeyTimer = 0;
            }
            while (binding->firstKeyTimer > binding->repeatInterval) {
                binding->Adjust(-binding->repeatStep);
                binding->firstKeyTimer -= binding->repeatInterval;
            }
        }
        if (UnknownVirtualSlot5(binding->secondKey, 0x3f, 0)) {
            if (binding->secondKeyTimer - g_TrackGame->frameTime > binding->repeatInterval) {
                binding->Recenter();
                binding->secondKeyTimer = 0;
            }
            while (binding->secondKeyTimer > binding->repeatInterval) {
                binding->Adjust(binding->repeatStep);
                binding->secondKeyTimer -= binding->repeatInterval;
            }
        }
        if (!UnknownVirtualSlot5(binding->firstKey, 0x3f, 0) &&
            !UnknownVirtualSlot5(binding->secondKey, 0x3f, 0))
            binding->Recenter();
    }
}

// 0x0048a240: 0x3f always holds; with bit 2 of the global's +0x2d4 clear,
// states with bit 7 never do. 0x80 also holds while key 0x29 is down,
// 0x80000000 means no modifier, and bit 6 asks for an exact match.
int KeyboardDevice::ModifierHolds(int modifier) {
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
