#include "KrustyBikeCamera.h"

// 0x00497cb0: restores the saved state (slot 62) and applies it; virtual
// calls in a constructor bind statically.
KrustyBikeCamera::KrustyBikeCamera(int flags) : BikeCamera(flags) {
    UnknownVirtualSlot62();
    UnknownVirtualSlot71(cameraState);
    krustyBike = 0;
    raceView = 0;
    field_0x1d8 = 0;
    field_0x25c = field_0x170;
    savedCameraState = cameraState;
    field_0x250 = cameraState;
    savedParameter = field_0x16c;
}

// 0x00497d80: an explicit empty destructor.
KrustyBikeCamera::~KrustyBikeCamera() {}

// 0x00497df0: the FollowCamera search, unless the global +0x3430 blocks it.
int KrustyBikeCamera::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (g_UnknownGlobal56e26c->uiInteractionBlocked)
        return 0;
    return BikeCamera::UnknownVirtualSlot23(event, entry);
}

// 0x00497fa0: keeps the point at least 3.5 above the subject's ground height
// (easing +0x22c when it lifts it), and in modes 3 and 4 no higher than 400.
void KrustyBikeCamera::UnknownVirtualSlot52(Vector3* point) {
    bool capped = g_UnknownGlobal56e26c->mode.UnknownFunction524100() == 3 ||
                  g_UnknownGlobal56e26c->mode.UnknownFunction524100() == 4;
    Vector3 ground = *point;
    field_0x240->UnknownFunction507c10(&ground, 0, 0, 0);
    float minimum = ground.y + 3.5f;
    if (minimum > point->y) {
        if (field_0x22c < 0.0f)
            field_0x22c = (minimum - point->y) / field_0x220 + field_0x22c;
        point->y = minimum;
    } else if (capped && point->y > 400.0f) {
        point->y = 400.0f;
    }
}

// 0x00498080
unsigned char KrustyBikeCamera::UnknownVirtualSlot55() {
    return g_UnknownGlobal56e26c->field_0x14->UnknownVirtualSlot2(0x0B, 0x3F);
}

// 0x004980a0: input 0x0a (unless +0x38c) clears +0x391/+0x392 when absent;
// outside state 7 it also needs the bike's +0x108, its axis below -2 and no
// +0x735 in vehicle mode.
bool KrustyBikeCamera::UnknownVirtualSlot56() {
    bool pressed;
    if (!field_0x38c && g_UnknownGlobal56e26c->field_0x14->UnknownVirtualSlot2(0x0A, 0x3F)) {
        pressed = true;
    } else {
        pressed = false;
        field_0x391 = pressed;
        field_0x392 = pressed;
    }
    if (cameraState == 7)
        return pressed;
    if (pressed && (!vehicleMode || (krustyBike->field_0x108 &&
                                     krustyBike->field_0x5f4->field_0x150 < -2.0f &&
                                     !krustyBike->field_0x735)))
        return true;
    return false;
}

// 0x00498130: +0x308 from the fov/zoom ratio and the bike's +0x43c.
void KrustyBikeCamera::UnknownVirtualSlot42(bool flag) {
    float ratio = field_0x16c / field_0x1dc;
    if (cameraState != 3) {
        if (vehicleMode && !flag &&
            (!krustyBike->field_0x7a4 || g_UnknownGlobal56e26c->field_0x18 > 1))
            field_0x308 = ratio * krustyBike->field_0x43c * 0.42f;
        else
            field_0x308 = 0.0f;
    } else {
        field_0x308 = ratio * krustyBike->field_0x43c * 0.55f;
    }
}

// 0x004981d0: saves the presets while in vehicle mode with the bike idle.
void KrustyBikeCamera::UnknownVirtualSlot59() {
    if (vehicleMode && !bike->field_0x444) {
        g_UnknownGlobal56e26c->mode.field_0x23c0 = field_0x22c;
        g_UnknownGlobal56e26c->mode.field_0x23c4 = field_0x234;
        g_UnknownGlobal56e26c->mode.field_0x23bc = field_0x220;
        g_UnknownGlobal56e26c->mode.field_0x23c8 = field_0x258;
    }
}

// 0x00498230: restores the saved presets.
void KrustyBikeCamera::UnknownVirtualSlot60() {
    field_0x22c = g_UnknownGlobal56e26c->mode.field_0x23c0;
    field_0x234 = g_UnknownGlobal56e26c->mode.field_0x23c4;
    field_0x220 = g_UnknownGlobal56e26c->mode.field_0x23bc;
    field_0x258 = g_UnknownGlobal56e26c->mode.field_0x23c8;
}

// 0x00498280: restores the saved state.
void KrustyBikeCamera::UnknownVirtualSlot62() {
    cameraState = g_UnknownGlobal56e26c->mode.field_0x23b8;
}

// 0x004982a0: saves the state.
void KrustyBikeCamera::UnknownVirtualSlot61() {
    g_UnknownGlobal56e26c->mode.field_0x23b8 = cameraState;
}

// 0x004982c0: shows the state's name (string 0x13b9 + state) for 1.5 s,
// except in state 6.
void KrustyBikeCamera::UnknownVirtualSlot58() {
    TextQueueOverlay* target = g_UnknownGlobal56e26c->eventManager->UnknownFunction45d340();
    if (target && cameraState != 6) {
        char text[260];
        g_UnknownGlobal56e26c->UnknownFunction521970(cameraState + 0x13B9, text, 0x80);
        UnknownMessage message(text, 1.5f);
        target->UnknownFunction51b540(&message);
    }
}

// 0x004985b0: FollowCamera slot 48 plus a raw-target case while the +0x3b8
// view flags are set.
Vector3 KrustyBikeCamera::UnknownVirtualSlot48(int a, bool flag, int b) {
    Vector3 result;
    if (cameraState == 5) {
        if (overrideActive)
            result = cachedTarget;
        else
            result = UnknownVirtualSlot34(b);
    } else if (cameraState == 7) {
        result = targetPoint;
        result.y += 3.0f;
    } else if (raceView->field_0x3f8 || raceView->field_0x3f9) {
        result = targetPoint;
    } else if (flag) {
        result = UnknownVirtualSlot35(a, b);
    } else {
        result = UnknownVirtualSlot37();
    }
    return result;
}
