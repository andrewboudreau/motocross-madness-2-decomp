#include "FollowCamera.h"

// 0x00463350 (scalar deleting wrapper 0x00463120): frees the owned values,
// then the Camera destructor (PCCamera's is implicit).
FollowCamera::~FollowCamera() {
    if (field_0x2e4)
        delete[] field_0x2e4;
    if (field_0x27c)
        delete field_0x27c;
    if (field_0x280)
        delete field_0x280;
    if (field_0x284)
        delete field_0x284;
    if (field_0x288)
        delete field_0x288;
    if (field_0x28c)
        delete field_0x28c;
    if (field_0x290)
        delete field_0x290;
    if (field_0x294)
        delete field_0x294;
    if (field_0x298)
        delete field_0x298;
}

// 0x00463620: only while the two points are within a tenth of Camera's
// +0x1c4/+0x1c8 in x and y, eases +0x220 toward a target that shrinks with
// the source height (capped at 335), keeping it at 20 or more.
void FollowCamera::UnknownVirtualSlot73(const Vector3& from, const Vector3& to, float dt) {
    if (to.x - from.x < field_0x1c4 * 0.1f && to.y - from.y < field_0x1c8 * 0.1f) {
        float height = FollowCameraMin(from.z, 335.0f);
        float target = field_0x220 - (height - 15.0f) * dt * 0.028f;
        field_0x220 = field_0x298->Update(target, dt);
        if (field_0x220 < 20.0f)
            field_0x220 = 20.0f;
    }
}

// 0x00463450
bool FollowCamera::UnknownFunction463450(int index, const Vector3* position, const float* a,
                                         const float* b, const float* c, const float* d,
                                         const float* e) {
    if (index < field_0x2e0) {
        if (position)
            field_0x2e4[index].position = *position;
        if (a)
            field_0x2e4[index].field_0x0c = *a;
        if (b)
            field_0x2e4[index].field_0x10 = *b;
        if (c)
            field_0x2e4[index].field_0x14 = *c;
        if (d)
            field_0x2e4[index].field_0x18 = *d;
        if (e)
            field_0x2e4[index].field_0x1c = *e;
        return true;
    }
    return false;
}

// 0x004639f0: with a non-empty table, entry 0 takes the target point before
// slot 38 runs.
void FollowCamera::UnknownVirtualSlot40(int a) {
    if (field_0x2e0 > 0) {
        UnknownFunction463450(0, &field_0x2b4, 0, 0, 0, 0, 0);
        UnknownVirtualSlot38(a, 0, 0);
    }
}

// 0x00464a80: the point the camera follows, by state.
Vector3 FollowCamera::UnknownVirtualSlot48(int a, bool flag, int b) {
    Vector3 result;
    if (field_0x244 == 5) {
        if (field_0x268)
            result = field_0x2a8;
        else
            result = UnknownVirtualSlot34(b);
    } else if (field_0x244 == 7) {
        result = field_0x2b4;
        result.y += 3.0f;
    } else if (flag) {
        result = UnknownVirtualSlot35(a, b);
    } else {
        result = UnknownVirtualSlot37();
    }
    return result;
}

// 0x00464ee0 and 0x00464f70: feed a vector into the +0x27c..+0x284 and
// +0x288..+0x290 values with a rate chosen by the subject's +0xbe8.
void FollowCamera::UnknownVirtualSlot43(const Vector3& value) {
    float rate = (field_0x240->field_0xbe8 == 1) ? 0.25f : 0.3f;
    field_0x27c->Set(value.x, rate);
    field_0x280->Set(value.y, rate);
    field_0x284->Set(value.z, rate);
}

void FollowCamera::UnknownVirtualSlot44(const Vector3& value) {
    float rate = (field_0x240->field_0xbe8 == 1) ? 0.25f : 0.3f;
    field_0x288->Set(value.x, rate);
    field_0x28c->Set(value.y, rate);
    field_0x290->Set(value.z, rate);
}

// Strong ABI reconstruction: slot 57 returns a 12-byte aggregate by value.
// MSVC uses a hidden return buffer and returns that buffer in EAX; retail then
// copies three dwords into this+0x2A8 and passes the cache to virtual slot 43.
// Retail forms the cache address before the call and copies straight from the
// returned buffer, i.e. the call result is assigned directly to the cache.
void FollowCamera::UnknownVirtualSlot69() {
    field_0x2a8 = UnknownVirtualSlot57(0);
    UnknownVirtualSlot43(field_0x2a8);
}

// Semantically strong but compiler-sensitive: retail VC6 zeros EAX once and
// reuses it. Modern clang emits immediate-zero stores instead.
void FollowCamera::UnknownVirtualSlot63() {
    field_0x22c = 0.0f;
    field_0x234 = 0.0f;
    field_0x220 = 0.0f;
}

// Slots 64-67 load fixed presets (retail stores the float bit patterns as
// immediates).
void FollowCamera::UnknownVirtualSlot64() {
    field_0x22c = 0.9648179f;
    field_0x234 = -0.25621432f;
    field_0x220 = 75.98f;
}

void FollowCamera::UnknownVirtualSlot65() {
    field_0x22c = 0.6832963f;
    field_0x234 = -0.6098179f;
    field_0x220 = 22.68f;
}

void FollowCamera::UnknownVirtualSlot66() {
    field_0x22c = 1.0471975f;
    field_0x234 = 0.0f;
    field_0x220 = 11.72f;
    field_0x258 = 85.0f;
}

void FollowCamera::UnknownVirtualSlot67() {
    field_0x22c = 0.7f;
    field_0x234 = 3.1415927f;
    field_0x220 = 15.0f;
    field_0x258 = 85.0f;
}

// Copies the +0x2B4 triple, raises its second component by 3, derives +0x258
// from the horizontal (x/z) distance between Camera's +0x170 and that triple,
// clamps it to [10, 70] and passes the triple by value to slot 29. The
// parenthesized division matters: VC6 folds `/ 180.0f * 60.0f` into one
// multiply, while retail multiplies by 1/180 and then by 60.
void FollowCamera::UnknownVirtualSlot68() {
    Vector3 target = field_0x2b4;
    target.y += 3.0f;
    float dx = field_0x170.x - target.x;
    if (dx < 0.0f) dx = -dx;
    float dz = field_0x170.z - target.z;
    if (dz < 0.0f) dz = -dz;
    field_0x258 = ((200.0f - UnknownFunction460b50(dx * dx + dz * dz)) / 180.0f) * 60.0f + 10.0f;
    if (field_0x258 < 10.0f) field_0x258 = 10.0f;
    if (field_0x258 > 70.0f) field_0x258 = 70.0f;
    UnknownVirtualSlot29(target);
}

// State dispatcher reconstructed from retail 0x00466E50.
// States 0-4 dispatch through FollowCamera virtuals, then snapshot the exact
// preset triplet before a final update virtual.
void FollowCamera::UnknownVirtualSlot71(int value) {
    field_0x244 = value;
    UnknownVirtualSlot58();

    switch (static_cast<unsigned int>(field_0x244)) {
        case 0:
            UnknownVirtualSlot66();
            break;
        case 1:
            UnknownVirtualSlot65();
            break;
        case 2:
            UnknownVirtualSlot64();
            break;
        case 3:
            field_0x258 = field_0x2f0;
            UnknownVirtualSlot63();
            break;
        case 4:
            field_0x2f0 = field_0x258;
            UnknownVirtualSlot60();
            break;
    }

    field_0x2c4 = field_0x220;
    field_0x2c8 = field_0x22c;
    field_0x2cc = field_0x234;

    UnknownVirtualSlot61();
}

// Retail behavior strongly matches a simple cyclic state-list advance.
// VC6 keeps the increment/store/compare/wrap sequence explicit; modern clang
// folds the wrap through CMOV on P6+ CPU targets, making this another useful
// historical compiler/profile calibration target.
void FollowCamera::UnknownVirtualSlot72() {
    field_0x30c = field_0x30c + 1;
    if (field_0x30c >= field_0x310)
        field_0x30c = 0;

    int value = field_0x314[field_0x30c];
    field_0x244 = value;
    UnknownVirtualSlot71(value);
}

// Strong semantic reconstruction, but not a clang smoke target.
void FollowCamera::UnknownVirtualSlot70(int value) {
    field_0x268 = value & 0xFF;

    if (static_cast<unsigned char>(value) != 0) {
        int current = field_0x244;

        if (current != 5) {
            field_0x248 = current;
            field_0x24c = field_0x258;
            field_0x244 = 5;
        }

        field_0x26c = 0;
    } else {
        field_0x244 = field_0x248;
        field_0x258 = field_0x24c;
    }
}

// 0x00464a10: the target point.
Vector3 FollowCamera::UnknownVirtualSlot37() {
    Vector3 result = field_0x2b4;
    return result;
}

// 0x00464a40: the target point raised by 3 (as slot 68 uses it).
Vector3 FollowCamera::UnknownVirtualSlot34(int) {
    Vector3 result = field_0x2b4;
    result.y += 3.0f;
    return result;
}

// 0x00464e80 and 0x00464e90 (slots 53, 54 and 58-62 share the empty body).
void FollowCamera::UnknownVirtualSlot52(int) {}
void FollowCamera::UnknownVirtualSlot53() {}
void FollowCamera::UnknownVirtualSlot54() {}
void FollowCamera::UnknownVirtualSlot58() {}
void FollowCamera::UnknownVirtualSlot59() {}
void FollowCamera::UnknownVirtualSlot60() {}
void FollowCamera::UnknownVirtualSlot61() {}
void FollowCamera::UnknownVirtualSlot62() {}

// 0x00464ea0 and 0x00464ec0: slot 5 of the interface behind the 0x0056e26c
// object, with codes 0x38 and 0x2a.
void FollowCamera::UnknownVirtualSlot55() {
    g_UnknownGlobal56e26c->field_0x14->field_0x34->UnknownVirtualSlot5(0x38, 0x3F, 0);
}

void FollowCamera::UnknownVirtualSlot56() {
    g_UnknownGlobal56e26c->field_0x14->field_0x34->UnknownVirtualSlot5(0x2A, 0x3F, 0);
}

// 0x00404fc0: states 5 and 2.
int FollowCamera::UnknownVirtualSlot75() {
    return field_0x244 == 5 || field_0x244 == 2;
}
