// Near-miss TrackOverlay.cpp candidates, kept out of src/reconstructed until
// they match. See docs/TRACKOVERLAY.md. They compile against
// src/reconstructed/TrackOverlay.h.
//
// NameOverlay::UnknownFunction519000 (0x00519000, 119 bytes): the candidate
// is 118 bytes. Retail loads the camera pointer into ecx and adds 0x170 to it
// after pushing `point`, then reloads +0x12c into edx; VC6 here picks eax for
// the camera and ecx for the world. A camera local, an eye-pointer local, a
// terrain local, an inline world wrapper, a block-scoped hit vector and
// `== 0` instead of `!` do not move it.
//
// StatsOverlay::UnknownFunction5198a0 (0x005198a0, 72 bytes plus a 24-byte
// jump table): the switch and its table match. Retail tests each result with
// `test eax, eax; je` and returns 0 with an explicit `xor eax, eax` shared at
// the end (case 2/3 jumps into the case 1/5 test); VC6 here returns the
// callee's zero in eax (`if (!f()) return 0;`), or uses neg/sbb for
// `return f() != 0`, `if (f()) return 1; return 0;` and a result flag.
//
// RadarOverlay::UnknownVirtualSlot23 (0x0051bb60, 246 bytes, 89.69%): the
// zoom keys. Both arms of the field_0x178 test are identical, so retail keeps
// only a dead `test`; it schedules fld/fmul/fidiv before that test and VC6
// here after it. A scale local, a ternary, assigning before the test and an
// empty or return-only test do not reproduce it.

#include "../../src/reconstructed/TrackOverlay.h"
#include "../../src/reconstructed/Camera.h"
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/ControlInterface.h"

// The radar zoom (0x0057513c), a TrackOverlay.cpp static.
static int s_UnknownGlobal57513c = 2;

// 0x00519000: every fourth frame, tests whether `point` can be seen from the
// camera (no terrain in between).
int NameOverlay::UnknownFunction519000(const Vector3* point)
{
    Vector3 hit;
    if (field_0x134 == field_0x130)
        field_0x138 = !field_0x12c->field_0x4c->UnknownFunction506e90(&Target()->field_0x08->field_0x170, point, &hit, 0, 0, 0);
    field_0x134++;
    if (field_0x134 > 3)
        field_0x134 = 0;
    return field_0x138;
}

// 0x005198a0: redraws the panel for the view mode.
int StatsOverlay::UnknownFunction5198a0()
{
    switch (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04) {
    case 0:
        if (!UnknownFunction519a20())
            return 0;
        break;
    case 1:
    case 5:
        if (!UnknownFunction51a560())
            return 0;
        break;
    case 2:
    case 3:
        if (!UnknownFunction519ef0())
            return 0;
        break;
    case 4:
        if (!UnknownFunction51aa40())
            return 0;
        break;
    }
    return 1;
}

// 0x0051bb60: control 0x21 toggles the frame rate, 0x34 and 0x33 zoom.
// Retail tests +0x178 before each scale update although both arms compute
// the same value; the identical if/else keeps that test.
int RadarOverlay::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry)
{
    if (UnknownFunction43caa0(0x21, 0, event, 0x80000000)) {
        field_0x164 = 1 - field_0x164;
        return 1;
    }
    if (UnknownFunction43caa0(0x34, 0, event, 0x80000000)) {
        if (++s_UnknownGlobal57513c > 5)
            s_UnknownGlobal57513c = 5;
        if (field_0x178)
            field_0x198 = field_0x19c / 180.0f / s_UnknownGlobal57513c;
        else
            field_0x198 = field_0x19c / 180.0f / s_UnknownGlobal57513c;
        return 1;
    }
    if (UnknownFunction43caa0(0x33, 0, event, 0x80000000)) {
        if (--s_UnknownGlobal57513c < 1)
            s_UnknownGlobal57513c = 1;
        if (field_0x178)
            field_0x198 = field_0x19c / 180.0f / s_UnknownGlobal57513c;
        else
            field_0x198 = field_0x19c / 180.0f / s_UnknownGlobal57513c;
        return 1;
    }
    return 0;
}
