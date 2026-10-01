#include "KrustyBikeCamera.h"

// 0x00497cb0: restores the saved state (slot 62) and applies it; virtual
// calls in a constructor bind statically.
KrustyBikeCamera::KrustyBikeCamera(int flags) : BikeCamera(flags) {
    UnknownVirtualSlot62();
    UnknownVirtualSlot71(field_0x244);
    field_0x3b4 = 0;
    field_0x3b8 = 0;
    field_0x1d8 = 0;
    field_0x25c = field_0x170;
    field_0x248 = field_0x244;
    field_0x250 = field_0x244;
    field_0x24c = field_0x16c;
}

// 0x00497d80: an explicit empty destructor.
KrustyBikeCamera::~KrustyBikeCamera() {}

// 0x00497df0: the FollowCamera search, unless the global +0x3430 blocks it.
int KrustyBikeCamera::UnknownVirtualSlot23(int a, int b) {
    if (g_UnknownGlobal56e26c->field_0x3430)
        return 0;
    return BikeCamera::UnknownVirtualSlot23(a, b);
}

// 0x00498080
void KrustyBikeCamera::UnknownVirtualSlot55() {
    g_UnknownGlobal56e26c->field_0x14->UnknownVirtualSlot2(0x0B, 0x3F);
}

// 0x004981d0: saves the presets while in vehicle mode with the bike idle.
void KrustyBikeCamera::UnknownVirtualSlot59() {
    if (field_0x390 && !field_0x3b0->field_0x444) {
        g_UnknownGlobal56e26c->field_0x2938 = field_0x22c;
        g_UnknownGlobal56e26c->field_0x293c = field_0x234;
        g_UnknownGlobal56e26c->field_0x2934 = field_0x220;
        g_UnknownGlobal56e26c->field_0x2940 = field_0x258;
    }
}

// 0x00498230: restores the saved presets.
void KrustyBikeCamera::UnknownVirtualSlot60() {
    field_0x22c = g_UnknownGlobal56e26c->field_0x2938;
    field_0x234 = g_UnknownGlobal56e26c->field_0x293c;
    field_0x220 = g_UnknownGlobal56e26c->field_0x2934;
    field_0x258 = g_UnknownGlobal56e26c->field_0x2940;
}

// 0x00498280: restores the saved state.
void KrustyBikeCamera::UnknownVirtualSlot62() {
    field_0x244 = g_UnknownGlobal56e26c->field_0x2930;
}

// 0x004982a0: saves the state.
void KrustyBikeCamera::UnknownVirtualSlot61() {
    g_UnknownGlobal56e26c->field_0x2930 = field_0x244;
}

// 0x004982c0: shows the state's name (string 0x13b9 + state) for 1.5 s,
// except in state 6.
void KrustyBikeCamera::UnknownVirtualSlot58() {
    UnknownMessageTarget* target = g_UnknownGlobal56e26c->field_0x570->UnknownFunction45d340();
    if (target && field_0x244 != 6) {
        char text[260];
        g_UnknownGlobal56e26c->UnknownFunction521970(field_0x244 + 0x13B9, text, 0x80);
        UnknownMessage message(text, 1.5f);
        target->UnknownFunction51b540(&message);
    }
}
