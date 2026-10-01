#include "FollowCamera.h"

// Strong ABI reconstruction: slot 57 returns a 12-byte aggregate by value.
// MSVC uses a hidden return buffer and returns that buffer in EAX; retail then
// copies three dwords into this+0x2A8 and passes the cache to virtual slot 43.
// Retail forms the cache address before the call and copies straight from the
// returned buffer, i.e. the call result is assigned directly to the cache.
void FollowCamera::UnknownVirtualSlot69() {
    CameraValue12* cached = reinterpret_cast<CameraValue12*>(reinterpret_cast<char*>(this) + 0x2A8);
    *cached = UnknownVirtualSlot57(0);
    UnknownVirtualSlot43(*cached);
}

// Semantically strong but compiler-sensitive: retail VC6 zeros EAX once and
// reuses it. Modern clang emits immediate-zero stores instead.
void FollowCamera::UnknownVirtualSlot63() {
    char* p = reinterpret_cast<char*>(this);
    *reinterpret_cast<int*>(p + 0x22C) = 0;
    *reinterpret_cast<int*>(p + 0x234) = 0;
    *reinterpret_cast<int*>(p + 0x220) = 0;
}

void FollowCamera::UnknownVirtualSlot64() {
    char* p = reinterpret_cast<char*>(this);
    *reinterpret_cast<unsigned int*>(p + 0x22C) = 0x3F76FE4Eu;
    *reinterpret_cast<unsigned int*>(p + 0x234) = 0xBE832E86u;
    *reinterpret_cast<unsigned int*>(p + 0x220) = 0x4297F5C3u;
}

void FollowCamera::UnknownVirtualSlot65() {
    char* p = reinterpret_cast<char*>(this);
    *reinterpret_cast<unsigned int*>(p + 0x22C) = 0x3F2EEC82u;
    *reinterpret_cast<unsigned int*>(p + 0x234) = 0xBF1C1D07u;
    *reinterpret_cast<unsigned int*>(p + 0x220) = 0x41B570A4u;
}

void FollowCamera::UnknownVirtualSlot66() {
    char* p = reinterpret_cast<char*>(this);
    *reinterpret_cast<unsigned int*>(p + 0x22C) = 0x3F860A91u;
    *reinterpret_cast<unsigned int*>(p + 0x234) = 0x00000000u;
    *reinterpret_cast<unsigned int*>(p + 0x220) = 0x413B851Fu;
    *reinterpret_cast<unsigned int*>(p + 0x258) = 0x42AA0000u;
}

void FollowCamera::UnknownVirtualSlot67() {
    char* p = reinterpret_cast<char*>(this);
    *reinterpret_cast<unsigned int*>(p + 0x22C) = 0x3F333333u;
    *reinterpret_cast<unsigned int*>(p + 0x234) = 0x40490FDBu;
    *reinterpret_cast<unsigned int*>(p + 0x220) = 0x41700000u;
    *reinterpret_cast<unsigned int*>(p + 0x258) = 0x42AA0000u;
}

// Copies the +0x2B4 triple, raises its second component by 3, derives +0x258
// from the horizontal distance between +0x170/+0x178 and that triple, clamps it
// to [10, 70] and passes the triple by value to slot 29. The parenthesized
// division matters: VC6 folds `/ 180.0f * 60.0f` into one multiply, while retail
// multiplies by 1/180 and then by 60.
void FollowCamera::UnknownVirtualSlot68() {
    char* p = reinterpret_cast<char*>(this);
    CameraFloat3 target = *reinterpret_cast<CameraFloat3*>(p + 0x2B4);
    target.b += 3.0f;
    float dx = *reinterpret_cast<float*>(p + 0x170) - target.a;
    if (dx < 0.0f) dx = -dx;
    float dz = *reinterpret_cast<float*>(p + 0x178) - target.c;
    if (dz < 0.0f) dz = -dz;
    float& param = *reinterpret_cast<float*>(p + 0x258);
    param = ((200.0f - UnknownFunction460b50(dx * dx + dz * dz)) / 180.0f) * 60.0f + 10.0f;
    if (param < 10.0f) param = 10.0f;
    if (param > 70.0f) param = 70.0f;
    UnknownVirtualSlot29(target);
}

// State dispatcher reconstructed from retail 0x00466E50.
// States 0-4 dispatch through FollowCamera virtuals, then snapshot the exact
// preset triplet before a final update virtual.
void FollowCamera::UnknownVirtualSlot71(int value) {
    char* p = reinterpret_cast<char*>(this);

    *reinterpret_cast<int*>(p + 0x244) = value;
    UnknownVirtualSlot58();

    switch (*reinterpret_cast<unsigned int*>(p + 0x244)) {
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
            *reinterpret_cast<int*>(p + 0x258) =
                *reinterpret_cast<int*>(p + 0x2F0);
            UnknownVirtualSlot63();
            break;
        case 4:
            *reinterpret_cast<int*>(p + 0x2F0) =
                *reinterpret_cast<int*>(p + 0x258);
            UnknownVirtualSlot60();
            break;
    }

    *reinterpret_cast<int*>(p + 0x2C4) =
        *reinterpret_cast<int*>(p + 0x220);
    *reinterpret_cast<int*>(p + 0x2C8) =
        *reinterpret_cast<int*>(p + 0x22C);
    *reinterpret_cast<int*>(p + 0x2CC) =
        *reinterpret_cast<int*>(p + 0x234);

    UnknownVirtualSlot61();
}

// Retail behavior strongly matches a simple cyclic state-list advance.
// VC6 keeps the increment/store/compare/wrap sequence explicit; modern clang
// folds the wrap through CMOV on P6+ CPU targets, making this another useful
// historical compiler/profile calibration target.
void FollowCamera::UnknownVirtualSlot72() {
    char* p = reinterpret_cast<char*>(this);

    int index = *reinterpret_cast<int*>(p + 0x30C) + 1;
    *reinterpret_cast<int*>(p + 0x30C) = index;

    if (index >= *reinterpret_cast<int*>(p + 0x310)) {
        *reinterpret_cast<int*>(p + 0x30C) = 0;
    }

    index = *reinterpret_cast<int*>(p + 0x30C);
    int value = *reinterpret_cast<int*>(p + 0x314 + index * 4);

    *reinterpret_cast<int*>(p + 0x244) = value;
    UnknownVirtualSlot71(value);
}

// Strong semantic reconstruction, but not a clang smoke target.
void FollowCamera::UnknownVirtualSlot70(int value) {
    char* p = reinterpret_cast<char*>(this);

    *reinterpret_cast<int*>(p + 0x268) = value & 0xFF;

    if (static_cast<unsigned char>(value) != 0) {
        int current = *reinterpret_cast<int*>(p + 0x244);

        if (current != 5) {
            *reinterpret_cast<int*>(p + 0x248) = current;
            *reinterpret_cast<int*>(p + 0x24C) =
                *reinterpret_cast<int*>(p + 0x258);
            *reinterpret_cast<int*>(p + 0x244) = 5;
        }

        *reinterpret_cast<int*>(p + 0x26C) = 0;
    } else {
        *reinterpret_cast<int*>(p + 0x244) =
            *reinterpret_cast<int*>(p + 0x248);
        *reinterpret_cast<int*>(p + 0x258) =
            *reinterpret_cast<int*>(p + 0x24C);
    }
}
