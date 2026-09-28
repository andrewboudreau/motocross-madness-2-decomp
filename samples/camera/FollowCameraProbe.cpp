// RTTI confirms FollowCamera : PCCamera. FollowCamera introduces slots 63-70;
// VehicleCamera/BikeCamera inherit these addresses unchanged where their vtables
// do not override them.
//
// FollowCam.cpp is the strongest recovered translation-unit candidate
// (filename overlap + nearby __FILE__ xrefs).
//
// Semantic field names remain unknown; preserve raw offsets until call-site
// evidence improves them.
class FollowCamera {
public:
    virtual void UnknownVirtualSlot63();
    virtual void UnknownVirtualSlot64();
    virtual void UnknownVirtualSlot65();
    virtual void UnknownVirtualSlot66();
    virtual void UnknownVirtualSlot67();
    virtual void UnknownVirtualSlot70(int value);
};

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

// Strong semantic reconstruction, but not a clang smoke target.
// The retail compiler uses materially different register allocation/code shape.
//
// Field names/types are intentionally not guessed:
//   +0x244 current state/mode candidate
//   +0x248 saved state/mode candidate
//   +0x24C saved copy of +0x258
//   +0x268 low-byte enable/toggle input materialized into a dword field
//   +0x26C reset state/flag
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
